//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "DefinitionGenerator.h"

#include <cstdint> /* For std::uintptr_t. */

#include <nc/core/Context.h>
#include <nc/core/Module.h>
#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/Instruction.h>
#include <nc/core/arch/Registers.h>
#ifdef NC_PREFER_CSTRINGS_TO_CONSTANTS
#include <nc/core/image/Image.h>
#endif

#include <nc/common/Foreach.h>
#include <nc/common/Unreachable.h>
#include <nc/common/make_unique.h>

#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/calls/CallAnalyzer.h>
#include <nc/core/ir/calls/CallsData.h>
#include <nc/core/ir/calls/FunctionAnalyzer.h>
#include <nc/core/ir/calls/FunctionSignature.h>
#include <nc/core/ir/calls/ReturnAnalyzer.h>
#include <nc/core/ir/cflow/BasicNode.h>
#include <nc/core/ir/cflow/Dfs.h>
#include <nc/core/ir/cflow/Graph.h>
#include <nc/core/ir/cflow/Switch.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/types/Type.h>
#include <nc/core/ir/types/Types.h>
#include <nc/core/ir/usage/Usage.h>
#include <nc/core/ir/vars/Variables.h>
#include <nc/core/likec/BinaryOperator.h>
#include <nc/core/likec/Block.h>
#include <nc/core/likec/Break.h>
#include <nc/core/likec/CallOperator.h>
#include <nc/core/likec/CaseLabel.h>
#include <nc/core/likec/CommentStatement.h>
#include <nc/core/likec/Continue.h>
#include <nc/core/likec/DefaultLabel.h>
#include <nc/core/likec/DoWhile.h>
#include <nc/core/likec/ExpressionStatement.h>
#include <nc/core/likec/FunctionDefinition.h>
#include <nc/core/likec/FunctionIdentifier.h>
#include <nc/core/likec/Goto.h>
#include <nc/core/likec/If.h>
#include <nc/core/likec/InlineAssembly.h>
#include <nc/core/likec/IntegerConstant.h>
#include <nc/core/likec/LabelIdentifier.h>
#include <nc/core/likec/LabelStatement.h>
#include <nc/core/likec/Return.h>
#include <nc/core/likec/String.h>
#include <nc/core/likec/Switch.h>
#include <nc/core/likec/Tree.h>
#include <nc/core/likec/Typecast.h>
#include <nc/core/likec/UnaryOperator.h>
#include <nc/core/likec/VariableIdentifier.h>
#include <nc/core/likec/While.h>

#include "SwitchContext.h"

namespace nc {
namespace core {
namespace ir {
namespace cgen {

DefinitionGenerator::DefinitionGenerator(CodeGenerator &parent, const Function *function):
    DeclarationGenerator(parent, function),
    dataflow_(parent.context().getDataflow(function)),
    variables_(parent.context().getVariables(function)),
    usage_(parent.context().getUsage(function)),
    regionGraph_(parent.context().getRegionGraph(function)),
    definition_(NULL),
    serial_(0)
{
}

void DefinitionGenerator::setDefinition(likec::FunctionDefinition *definition) {
    assert(!definition_); 
    definition_ = definition;
    setDeclaration(definition);
}

std::unique_ptr<likec::FunctionDefinition> DefinitionGenerator::createDefinition() {
    auto functionDefinition = std::make_unique<likec::FunctionDefinition>(tree(),
        function()->name(), makeReturnType(), variadic());

    functionDefinition->setComment(function()->comment().text());

    setDefinition(functionDefinition.get());

    if (const calls::FunctionSignature *signature = context().callsData()->getFunctionSignature(function())) {
        if (calls::FunctionAnalyzer *functionAnalyzer = context().callsData()->getFunctionAnalyzer(function())) {
            foreach (const MemoryLocation &memoryLocation, signature->arguments()) {
                makeArgumentDeclaration(functionAnalyzer->getArgumentTerm(memoryLocation));
            }
        }
    }

    parent().setFunctionDeclaration(function(), functionDefinition.get());

    if (calls::FunctionAnalyzer *functionAnalyzer = context().callsData()->getFunctionAnalyzer(function())) {
        foreach (const ir::Statement *statement, functionAnalyzer->entryStatements()) {
            if (auto likecStatement = makeStatement(statement, NULL, NULL, NULL)) {
                definition()->block()->addStatement(std::move(likecStatement));
            }
        }
    }

    SwitchContext switchContext;
    makeStatements(regionGraph().root(), definition()->block(), NULL, NULL, NULL, switchContext);

    return functionDefinition;
}

likec::ArgumentDeclaration *DefinitionGenerator::makeArgumentDeclaration(const Term *term) {
    likec::VariableDeclaration *&variableDeclaration = variableDeclarations_[variables().getVariable(term)];
    assert(!variableDeclaration);

    likec::ArgumentDeclaration *result = DeclarationGenerator::makeArgumentDeclaration(term);
    variableDeclaration = result;

    return result;
}

likec::VariableDeclaration *DefinitionGenerator::makeLocalVariableDeclaration(const Term *term) {
    likec::VariableDeclaration *&result = variableDeclarations_[variables().getVariable(term)];
    if (!result) {
        QString basename(QLatin1String("v"));

#ifdef NC_REGISTER_VARIABLE_NAMES
        if(const MemoryLocationAccess *access = term->asMemoryLocationAccess()) {
            if (const arch::Register *reg = context().module()->architecture()->registers()->regizter(access->memoryLocation())) {
                basename = reg->lowercaseName();
                if (basename.isEmpty() || basename[basename.size() - 1].isDigit()) {
                    basename.push_back('_');
                }
            }
        }
#endif

        auto variableDeclaration = std::make_unique<likec::VariableDeclaration>(tree(),
            QString("%1%2").arg(basename).arg(++serial_),
            parent().makeType(types().getType(term)));

        result = variableDeclaration.get();
        definition()->block()->addDeclaration(std::move(variableDeclaration));
    }
    return result;
}

likec::LabelDeclaration *DefinitionGenerator::makeLabel(const BasicBlock *basicBlock) {
    likec::LabelDeclaration *&result = labels_[basicBlock];
    if (!result) {
        auto label = std::make_unique<likec::LabelDeclaration>(
            tree(),
            basicBlock->address() ?
                QString("addr_0x%1_%2").arg(basicBlock->address().get(), 0, 16).arg(labels_.size()) :
                QString("label_%1").arg(labels_.size())
            );
        result = label.get();
        definition()->addLabel(std::move(label));
    }
    return result;
}

void DefinitionGenerator::addLabels(const BasicBlock *basicBlock, likec::Block *block, SwitchContext &switchContext) {
    assert(basicBlock != NULL);
    assert(block != NULL);

    /* Add usual label. */
    block->addStatement(std::make_unique<likec::LabelStatement>(tree(), makeLabel(basicBlock)));

    /* Add case labels. */
    if (basicBlock->address()) {
        if (basicBlock == switchContext.defaultBasicBlock()) {
            block->addStatement(std::make_unique<likec::DefaultLabel>(tree()));
        } else {
            foreach (ConstantValue value, switchContext.getCaseValues(*basicBlock->address())) {
                block->addStatement(std::make_unique<likec::CaseLabel>(tree(), 
                    std::make_unique<likec::IntegerConstant>(tree(), value, switchContext.valueType())));
            }
        }
        switchContext.eraseCaseValues(*basicBlock->address());
    }
}

void DefinitionGenerator::makeStatements(const cflow::Node *node, likec::Block *block, const BasicBlock *nextBB, const BasicBlock *breakBB, const BasicBlock *continueBB, SwitchContext &switchContext) {
    switch (node->nodeKind()) {
    case cflow::Node::BASIC: {
        auto basicNode = node->as<cflow::BasicNode>();

        addLabels(basicNode->basicBlock(), block, switchContext);

        foreach (const ir::Statement *statement, basicNode->basicBlock()->statements()) {
            if (auto likecStatement = makeStatement(statement, nextBB, breakBB, continueBB)) {
                block->addStatement(std::move(likecStatement));
            }
        }

        break;
    }
    case cflow::Node::REGION: {
        auto region = node->as<cflow::Region>();

        switch (region->regionKind()) {
        case cflow::Region::UNKNOWN: {
            assert(region->nodes().size() > 0);

            /*
             * We tend to process nodes in DFS order because it is likely
             * to minimize the number of generated gotos.
             */
            makeStatements(cflow::Dfs(region).preordering(), block, nextBB, breakBB, continueBB, switchContext);
            break;
        }
        case cflow::Region::BLOCK: {
            assert(region->nodes().size() > 0);

            makeStatements(region->nodes(), block, nextBB, breakBB, continueBB, switchContext);
            break;
        }
        case cflow::Region::COMPOUND_CONDITION: {
            assert(region->nodes().size() == 2);

            makeStatements(region->nodes(), block, nextBB, breakBB, continueBB, switchContext);
            break;
        }
        case cflow::Region::IF_THEN_ELSE: {
            assert(region->nodes().size() == 3);

            std::unique_ptr<likec::Expression> condition(makeExpression(region->nodes()[0], block,
                region->nodes()[1]->getEntryBasicBlock(), region->nodes()[2]->getEntryBasicBlock(), switchContext));

            auto thenBlock = std::make_unique<likec::Block>(tree());
            makeStatements(region->nodes()[1], thenBlock.get(), nextBB, breakBB, continueBB, switchContext);

            auto elseBlock = std::make_unique<likec::Block>(tree());
            makeStatements(region->nodes()[2], elseBlock.get(), nextBB, breakBB, continueBB, switchContext);

            block->addStatement(std::make_unique<likec::If>(tree(), std::move(condition), std::move(thenBlock), std::move(elseBlock)));

            break;
        }
        case cflow::Region::IF_THEN: {
            assert(region->nodes().size() == 2);
            assert(region->exitBasicBlock() != NULL);

            std::unique_ptr<likec::Expression> condition(makeExpression(region->nodes()[0], block,
                region->nodes()[1]->getEntryBasicBlock(), region->exitBasicBlock(), switchContext));

            auto thenBlock = std::make_unique<likec::Block>(tree());
            makeStatements(region->nodes()[1], thenBlock.get(), nextBB, breakBB, continueBB, switchContext);

            block->addStatement(std::make_unique<likec::If>(tree(), std::move(condition), std::move(thenBlock)));

            break;
        }
        case cflow::Region::LOOP: {
            assert(region->nodes().size() > 0);

            auto condition = std::make_unique<likec::IntegerConstant>(tree(), 1, tree().makeIntegerType(tree().intSize(), false));

            cflow::Dfs dfs(region);

            auto body = std::make_unique<likec::Block>(tree());
            const BasicBlock *entryBB = region->entry()->getEntryBasicBlock();

            makeStatements(dfs.preordering(), body.get(), entryBB, nextBB, entryBB, switchContext);

            block->addStatement(std::make_unique<likec::While>(tree(), std::move(condition), std::move(body)));

            break;
        }
        case cflow::Region::WHILE: {
            assert(region->nodes().size() > 0);
            assert(region->exitBasicBlock() != NULL);

            addLabels(region->entry()->getEntryBasicBlock(), block, switchContext);

            cflow::Node *bodyEntry = region->entry()->uniqueSuccessor();

            auto condition = makeExpression(region->entry(), NULL,
                bodyEntry ? bodyEntry->getEntryBasicBlock() : region->entry()->getEntryBasicBlock(),
                region->exitBasicBlock(), switchContext);

            cflow::Dfs dfs(region);
            auto &nodes = dfs.preordering();

            assert(nodes.front() == region->entry());
            nodes.erase(nodes.begin());

            auto body = std::make_unique<likec::Block>(tree());
            const BasicBlock *conditionBB = region->entry()->getEntryBasicBlock();

            makeStatements(nodes, body.get(), conditionBB, region->exitBasicBlock(), conditionBB, switchContext);

            block->addStatement(std::make_unique<likec::While>(tree(), std::move(condition), std::move(body)));

            if (auto jump = makeJump(region->exitBasicBlock(), nextBB, breakBB, continueBB)) {
                block->addStatement(std::move(jump));
            }

            break;
        }
        case cflow::Region::DO_WHILE: {
            assert(region->nodes().size() > 0);
            assert(region->exitBasicBlock() != NULL);
            assert(region->loopCondition() != NULL);

            cflow::Dfs dfs(region);
            auto &nodes = dfs.preordering();

            assert(nc::contains(nodes, region->loopCondition()));
            nodes.erase(std::find(nodes.begin(), nodes.end(), region->loopCondition()));

            auto body = std::make_unique<likec::Block>(tree());
            const BasicBlock *conditionBB = region->loopCondition()->getEntryBasicBlock();

            makeStatements(nodes, body.get(), conditionBB, nextBB, conditionBB, switchContext);

            auto condition = makeExpression(region->loopCondition(), body.get(),
                region->entry()->getEntryBasicBlock(),
                region->exitBasicBlock(),
                switchContext);

            block->addStatement(std::make_unique<likec::DoWhile>(tree(), std::move(body), std::move(condition)));

            if (auto jump = makeJump(region->exitBasicBlock(), nextBB, breakBB, continueBB)) {
                block->addStatement(std::move(jump));
            }

            break;
        }
        case cflow::Region::SWITCH: {
            auto witch = region->as<cflow::Switch>();

            /*
             * Generates code for the basic block, except the code for its terminator.
             */
            auto makeStatementsButLast = [&](const BasicBlock *basicBlock) {
                addLabels(basicBlock, block, switchContext);

                for (std::size_t i = 0, size = basicBlock->statements().size() - 1; i < size; ++i) {
                    /* We do not care about breakBB and other: we will not create gotos. */
                    if (auto likecStatement = makeStatement(basicBlock->statements()[i], NULL, NULL, NULL)) {
                        block->addStatement(std::move(likecStatement));
                    }
                }
            };

            /* Generate code for the basic block doing the bounds check. */
            if (witch->boundsCheckNode()) {
                makeStatementsButLast(witch->boundsCheckNode()->basicBlock());
            }

            /* Generate code for the basic block with the table-based jump. */
            makeStatementsButLast(witch->switchNode()->basicBlock());

            /* The jump via the jump table. */
            const Jump *jump = witch->switchNode()->basicBlock()->getJump();
            assert(jump != NULL);
            assert(jump->isUnconditional());

            /* The jump table. */
            const JumpTable *jumpTable = jump->thenTarget().table();
            assert(jumpTable != NULL);

            /*
             * Make a new switch context.
             */
            SwitchContext newSwitchContext;

            newSwitchContext.setValueType(tree().makeIntegerType(witch->switchTerm()->size(), true));

            for (std::size_t i = 0, size = witch->jumpTableSize(); i < size; ++i) {
                newSwitchContext.addCaseValue((*jumpTable)[i].address(), i);
            }

            if (witch->defaultBasicBlock()) {
                newSwitchContext.setDefaultBasicBlock(witch->defaultBasicBlock());
            }

            /* Exit basic block of the switch. */
            const BasicBlock *exitBB = witch->exitBasicBlock();
            if (!exitBB) {
                exitBB = nextBB;
            }

            /*
             * Generate the switch expression.
             */
            auto expression = std::make_unique<likec::Typecast>(tree(),
                newSwitchContext.valueType(),
                makeExpression(witch->switchTerm()));

            /*
             * Generate the body of the switch.
             */
            cflow::Dfs dfs(region);
            auto &nodes = dfs.preordering();

            nodes.erase(
                std::remove_if(nodes.begin(), nodes.end(),
                    [witch](const cflow::Node *node){ return node == witch->boundsCheckNode() || node == witch->switchNode(); }),
                nodes.end());

            auto body = std::make_unique<likec::Block>(tree());

            makeStatements(nodes, body.get(), exitBB, exitBB, continueBB, newSwitchContext);

            /*
             * Generate case labels that were not generated before.
             */
            foreach (const auto &pair, newSwitchContext.caseValuesMap()) {
                foreach (ConstantValue value, pair.second) {
                    body->addStatement(std::make_unique<likec::CaseLabel>(tree(), 
                        std::make_unique<likec::IntegerConstant>(tree(), value, newSwitchContext.valueType())));
                }
                body->addStatement(std::make_unique<likec::Goto>(tree(),
                    std::make_unique<likec::IntegerConstant>(tree(),
                        pair.first,
                        tree().makeIntegerType(tree().pointerSize(), true))));
            }

            /* Generate the switch. */
            block->addStatement(std::make_unique<likec::Switch>(tree(), std::move(expression), std::move(body)));

            /* Generate a jump to the exit basic block, if it's not the nextBB. */
            if (auto jump = makeJump(exitBB, nextBB, breakBB, continueBB)) {
                block->addStatement(std::move(jump));
            }

            break;
        }
        default:
            unreachable();
        }
        break;
    }
    default:
        unreachable();
    }
}

void DefinitionGenerator::makeStatements(const std::vector<cflow::Node *> &nodes, likec::Block *block, const BasicBlock *nextBB, const BasicBlock *breakBB, const BasicBlock *continueBB, SwitchContext &switchContext) {
    if (nodes.empty()) {
        return;
    }
    for (std::size_t i = 0, last = nodes.size() - 1; i != last; ++i) {
        makeStatements(nodes[i], block, nodes[i + 1]->getEntryBasicBlock(), breakBB, continueBB, switchContext);
    }
    makeStatements(nodes.back(), block, nextBB, breakBB, continueBB, switchContext);
}

std::unique_ptr<likec::Expression> DefinitionGenerator::makeExpression(const cflow::Node *node, likec::Block *block, const BasicBlock *thenBB, const BasicBlock *elseBB, SwitchContext &switchContext) {
    assert(node != NULL);
    assert(thenBB != NULL);
    assert(elseBB != NULL);
    assert(node->isCondition() && "Can only generate expressions from condition nodes.");

    std::unique_ptr<likec::Expression> result;

    if (const cflow::BasicNode *basicNode = node->as<cflow::BasicNode>()) {
        if (block) {
            addLabels(basicNode->basicBlock(), block, switchContext);
        }

        foreach (const ir::Statement *statement, basicNode->basicBlock()->statements()) {
            std::unique_ptr<likec::Expression> expression;

            if (const Jump *jump = statement->asJump()) {
                assert(jump == basicNode->basicBlock()->getJump());

                expression = makeExpression(jump->condition());

                assert((jump->thenTarget().basicBlock() == thenBB && jump->elseTarget().basicBlock() == elseBB) ||
                       (jump->thenTarget().basicBlock() == elseBB && jump->elseTarget().basicBlock() == thenBB));

                if (jump->thenTarget().basicBlock() != thenBB) {
                    expression = std::make_unique<likec::UnaryOperator>(tree(), likec::UnaryOperator::LOGICAL_NOT,
                        std::move(expression));
                }
            } else if (auto stmt = makeStatement(statement, NULL, NULL, NULL)) {
                if (block) {
                    block->addStatement(std::move(stmt));
                } else if (likec::ExpressionStatement *expressionStatement = stmt->as<likec::ExpressionStatement>()) {
                    expression = expressionStatement->releaseExpression();
                }
            }

            if (expression) {
                if (!result) {
                    result = std::move(expression);
                } else {
                    result = std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::COMMA,
                        std::move(result), std::move(expression));
                }
            }
        }
    } else if (const cflow::Region *region = node->as<cflow::Region>()) {
        assert(region->regionKind() == cflow::Region::COMPOUND_CONDITION);
        assert(region->nodes().size() == 2);

        /*
         * Distinguishing AND from OR:
         *
         * if (a || b) { then } { else }
         *
         * a -> then || b
         * b -> then || else
         *
         * if (a && b) { then } { else }
         *
         * a -> b || else
         * b -> then || else
         */

        const cflow::Node *n = region->nodes()[0];
        while (const cflow::Region *r = n->as<cflow::Region>()) {
            assert(r->regionKind() == cflow::Region::COMPOUND_CONDITION);
            assert(r->nodes().size() == 2);
            n = r->nodes()[1];
        }

        const cflow::BasicNode *b = n->as<cflow::BasicNode>();
        assert(b != NULL);

        const Jump *j = b->basicBlock()->getJump();
        assert(j != NULL);

        if (j->thenTarget().basicBlock() == thenBB || j->elseTarget().basicBlock() == thenBB) {
            auto left  = makeExpression(region->nodes()[0], block, thenBB, region->nodes()[1]->getEntryBasicBlock(), switchContext);
            auto right = makeExpression(region->nodes()[1], NULL, thenBB, elseBB, switchContext);

            result = std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::LOGICAL_OR,
                std::move(left), std::move(right));
        } else if (j->thenTarget().basicBlock() == elseBB || j->elseTarget().basicBlock() == elseBB) {
            auto left  = makeExpression(region->nodes()[0], block, region->nodes()[1]->getEntryBasicBlock(), elseBB, switchContext);
            auto right = makeExpression(region->nodes()[1], NULL, thenBB, elseBB, switchContext);

            result = std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::LOGICAL_AND,
                std::move(left), std::move(right));
        } else {
            assert(!"First component of compound condition must contain a jump to thenBB or elseBB.");
        }
    } else {
        assert(!"Node must be a basic block node or a region.");
    }

    assert(result != NULL && "Something is very wrong.");

    return result;
}

std::unique_ptr<likec::Statement> DefinitionGenerator::makeStatement(const Statement *statement, const BasicBlock *nextBB, const BasicBlock *breakBB, const BasicBlock *continueBB) {
    assert(statement);

    auto result = doMakeStatement(statement, nextBB, breakBB, continueBB);

    if (result != NULL) {
        class StatementSetterVisitor: public Visitor<likec::TreeNode> {
            const ir::Statement *statement_;

        public:
            StatementSetterVisitor(const ir::Statement *statement): statement_(statement) {
                assert(statement != NULL);
            }

            virtual void operator()(likec::TreeNode *node) override {
                if (auto stmt = node->as<likec::Statement>()) {
                    if (stmt->statement() == NULL) {
                        stmt->setStatement(statement_);
                        stmt->visitChildNodes(*this);
                    }
                }
            }
        };

        StatementSetterVisitor visitor(statement);
        visitor(result.get());
    }

    return result;
}

std::unique_ptr<likec::Statement> DefinitionGenerator::doMakeStatement(const Statement *statement, const BasicBlock *nextBB, const BasicBlock *breakBB, const BasicBlock *continueBB) {
    switch (statement->kind()) {
        case Statement::COMMENT: {
            return std::make_unique<likec::CommentStatement>(tree(), statement->asComment()->text());
        }
        case Statement::INLINE_ASSEMBLY: {
            return std::make_unique<likec::InlineAssembly>(tree(), statement->instruction() ? statement->instruction()->toString(): QString());
        }
        case Statement::ASSIGNMENT: {
            const Assignment *assignment = statement->asAssignment();

            if (usage().isUsed(assignment->left()) && !isIntermediate(assignment->left())) {
                std::unique_ptr<likec::Expression> left(makeExpression(assignment->left()));
                std::unique_ptr<likec::Expression> right(makeExpression(assignment->right()));

                return std::make_unique<likec::ExpressionStatement>(tree(),
                    std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::ASSIGN,
                        std::move(left),
                        std::make_unique<likec::Typecast>(tree(),
                            parent().makeType(types().getType(assignment->left())),
                            std::move(right))));
            } else {
                return NULL;
            }
        }
        case Statement::KILL: {
            return NULL;
        }
        case Statement::JUMP: {
            const Jump *jump = statement->asJump();

            if (jump->isConditional()) {
                auto thenJump = makeJump(jump->thenTarget(), nextBB, breakBB, continueBB);
                auto elseJump = makeJump(jump->elseTarget(), nextBB, breakBB, continueBB);
                auto condition = makeExpression(jump->condition());

                if (thenJump == NULL) {
                    if (elseJump == NULL) {
                        return NULL;
                    } else {
                        std::swap(thenJump, elseJump);
                        condition = std::make_unique<likec::UnaryOperator>(tree(), likec::UnaryOperator::LOGICAL_NOT, std::move(condition));
                    }
                }
                return std::make_unique<likec::If>(tree(), std::move(condition), std::move(thenJump), std::move(elseJump));
            } else {
                return makeJump(jump->thenTarget(), nextBB, breakBB, continueBB);
            }
        }
        case Statement::CALL: {
            const Call *call = statement->asCall();

            std::unique_ptr<likec::Expression> target;

            const dflow::Value *targetValue = dataflow().getValue(call->target());
            if (targetValue->isConstant()) {
                if (likec::FunctionDeclaration *functionDeclaration = parent().makeFunctionDeclaration(targetValue->constantValue().value())) {
                    target = std::make_unique<likec::FunctionIdentifier>(tree(), functionDeclaration);
                    target->setTerm(call->target());
                }
            }

            if (!target) {
                target = makeExpression(call->target());
            }

            auto callOperator = std::make_unique<likec::CallOperator>(tree(), std::move(target));

            if (const calls::FunctionSignature *signature = context().callsData()->getFunctionSignature(call)) {
                if (calls::CallAnalyzer *callAnalyzer = context().callsData()->getCallAnalyzer(call)) {
                    foreach (const MemoryLocation &memoryLocation, signature->arguments()) {
                        callOperator->addArgument(makeExpression(callAnalyzer->getArgumentTerm(memoryLocation)));
                    }

                    if (signature->returnValue()) {
                        const Term *returnValueTerm = callAnalyzer->getReturnValueTerm(signature->returnValue());

                        return std::make_unique<likec::ExpressionStatement>(tree(),
                            std::make_unique<likec::BinaryOperator>(tree(),
                                likec::BinaryOperator::ASSIGN,
                                makeExpression(returnValueTerm),
                                std::make_unique<likec::Typecast>(tree(),
                                    parent().makeType(types().getType(returnValueTerm)),
                                    std::move(callOperator))));
                    }
                }
            }

            return std::make_unique<likec::ExpressionStatement>(tree(), std::move(callOperator));
        }
        case Statement::RETURN: {
            if (const calls::FunctionSignature *signature = context().callsData()->getFunctionSignature(function())) {
                if (calls::ReturnAnalyzer *returnAnalyzer = context().callsData()->getReturnAnalyzer(function(), statement->asReturn())) {
                    if (signature->returnValue()) {
                        return std::make_unique<likec::Return>(
                            tree(),
                            makeExpression(returnAnalyzer->getReturnValueTerm(signature->returnValue())));
                    }
                }
            }
            return std::make_unique<likec::Return>(tree());
        }
    }

    unreachable();
    return NULL;
}

std::unique_ptr<likec::Statement> DefinitionGenerator::makeJump(const BasicBlock *target, const BasicBlock *nextBB, const BasicBlock *breakBB, const BasicBlock *continueBB) {
    if (target == nextBB) {
        return NULL;
    } else if (target == breakBB) {
        return std::make_unique<likec::Break>(tree());
    } else if (target == continueBB) {
        return std::make_unique<likec::Continue>(tree());
    } else {
        return std::make_unique<likec::Goto>(tree(),
                std::make_unique<likec::LabelIdentifier>(tree(), makeLabel(target)));
    }
}

std::unique_ptr<likec::Statement> DefinitionGenerator::makeJump(const JumpTarget &target, const BasicBlock *nextBB, const BasicBlock *breakBB, const BasicBlock *continueBB) {
    if (target.basicBlock()) {
        return makeJump(target.basicBlock(), nextBB, breakBB, continueBB);
    } else if (target.address()) {
        return std::make_unique<likec::Goto>(tree(), makeExpression(target.address()));
    } else {
        return std::make_unique<likec::Goto>(tree(), std::make_unique<likec::String>(tree(), QLatin1String("???")));
    }
}

std::unique_ptr<likec::Expression> DefinitionGenerator::makeExpression(const Term *term) {
    assert(term != NULL);

    auto result = doMakeExpression(term);
    assert(result != NULL);

    class TermSetterVisitor: public Visitor<likec::TreeNode> {
        const ir::Term *term_;

    public:
        TermSetterVisitor(const ir::Term *term): term_(term) {
            assert(term != NULL);
        }

        virtual void operator()(likec::TreeNode *node) override {
            if (auto expression = node->as<likec::Expression>()) {
                if (expression->term() == NULL) {
                    expression->setTerm(term_);
                    expression->visitChildNodes(*this);
                }
            }
        }
    };

    TermSetterVisitor visitor(term);
    visitor(result.get());

    return result;
}

std::unique_ptr<likec::Expression> DefinitionGenerator::doMakeExpression(const Term *term) {
#ifdef NC_PREFER_CONSTANTS_TO_EXPRESSIONS
    if (term->isRead()) {
        const dflow::Value *value = dataflow().getValue(term);

        if (value->isConstant()) {
            return makeConstant(term, value->constantValue());
        }
    }
#endif

    switch (term->kind()) {
        case Term::INT_CONST: {
            return makeConstant(term, term->asConstant()->value());
        }
        case Term::INTRINSIC: {
            return std::make_unique<likec::CallOperator>(tree(), std::make_unique<likec::String>(tree(), "intrinsic"));
        }
        case Term::UNDEFINED: {
            return std::make_unique<likec::CallOperator>(tree(), std::make_unique<likec::String>(tree(), "undefined"));
        }
        case Term::MEMORY_LOCATION_ACCESS: {
            const MemoryLocationAccess *access = term->asMemoryLocationAccess();
            if (context().module()->architecture()->isGlobalMemory(access->memoryLocation())) {
                return std::make_unique<likec::VariableIdentifier>(
                    tree(),
                    parent().makeGlobalVariableDeclaration(access->memoryLocation(), types().getType(term)));
            } else if (isIntermediate(term)) {
                return makeExpression(dataflow().getDefinitions(term).front()->assignee());
            } else {
                return std::make_unique<likec::VariableIdentifier>(tree(), makeLocalVariableDeclaration(term));
            }
        }
        case Term::DEREFERENCE: {
            const Dereference *dereference = term->asDereference();
            if (const MemoryLocation &memoryLocation = dataflow().getMemoryLocation(term)) {
                if (context().module()->architecture()->isGlobalMemory(memoryLocation)) {
                    return std::make_unique<likec::VariableIdentifier>(
                        tree(),
                        parent().makeGlobalVariableDeclaration(memoryLocation, types().getType(term)));
                } else if (isIntermediate(dereference)) {
                    return makeExpression(dataflow().getDefinitions(dereference).front()->assignee());
                } else {
                    return std::make_unique<likec::VariableIdentifier>(tree(), makeLocalVariableDeclaration(dereference));
                }
            } else {
                const types::Type *type = types().getType(dereference);
                const types::Type *addressType = types().getType(dereference->address());
                return std::make_unique<likec::UnaryOperator>(tree(), likec::UnaryOperator::DEREFERENCE,
                    std::make_unique<likec::Typecast>(tree(),
                        tree().makePointerType(addressType->size(), parent().makeType(type)),
                        makeExpression(dereference->address())));
            }
        }
        case Term::UNARY_OPERATOR: {
            return doMakeExpression(term->asUnaryOperator());
        }
        case Term::BINARY_OPERATOR: {
            return doMakeExpression(term->asBinaryOperator());
        }
        case Term::CHOICE: {
            const Choice *choice = term->asChoice();
            if (!dataflow().getDefinitions(choice->preferredTerm()).empty()) {
                return makeExpression(choice->preferredTerm());
            } else {
                return makeExpression(choice->defaultTerm());
            }
        }
        default: {
            unreachable();
            return NULL;
        }
    }
}

std::unique_ptr<likec::Expression> DefinitionGenerator::doMakeExpression(const UnaryOperator *unary) {
    std::unique_ptr<likec::Expression> operand(makeExpression(unary->operand()));

    switch (unary->operatorKind()) {
        case UnaryOperator::BITWISE_NOT: {
            const types::Type *operandType = types().getType(unary->operand());
            return std::make_unique<likec::UnaryOperator>(tree(), likec::UnaryOperator::BITWISE_NOT,
                std::make_unique<likec::Typecast>(tree(),
                    tree().makeIntegerType(operandType->size(), operandType->isUnsigned()), std::move(operand)));
        }
        case UnaryOperator::LOGICAL_NOT: {
            const types::Type *operandType = types().getType(unary->operand());
            return std::make_unique<likec::UnaryOperator>(tree(), likec::UnaryOperator::LOGICAL_NOT,
                std::make_unique<likec::Typecast>(tree(),
                    tree().makeIntegerType(operandType->size(), operandType->isUnsigned()), std::move(operand)));
        }
        case UnaryOperator::NEGATION: {
            const types::Type *operandType = types().getType(unary->operand());
            return std::make_unique<likec::UnaryOperator>(tree(), likec::UnaryOperator::NEGATION,
                std::make_unique<likec::Typecast>(tree(),
                    tree().makeIntegerType(operandType->size(), operandType->isUnsigned()), std::move(operand)));
        }
        case UnaryOperator::SIGN_EXTEND: {
            return std::make_unique<likec::Typecast>(tree(),
                tree().makeIntegerType(unary->size(), false),
                std::make_unique<likec::Typecast>(tree(),
                    tree().makeIntegerType(unary->operand()->size(), false), std::move(operand)));
        }
        case UnaryOperator::ZERO_EXTEND: {
            return std::make_unique<likec::Typecast>(tree(),
                tree().makeIntegerType(unary->size(), true),
                std::make_unique<likec::Typecast>(tree(),
                    tree().makeIntegerType(unary->operand()->size(), true), std::move(operand)));
        }
        case UnaryOperator::RESIZE: {
            const types::Type *type = types().getType(unary);
            return std::make_unique<likec::Typecast>(tree(), parent().makeType(type), std::move(operand));
        }
        default:
            unreachable();
            return NULL;
    }
}

std::unique_ptr<likec::Expression> DefinitionGenerator::doMakeExpression(const BinaryOperator *binary) {
    const types::Type *leftType = types().getType(binary->left());
    const types::Type *rightType = types().getType(binary->right());

    std::unique_ptr<likec::Expression> left(makeExpression(binary->left()));
    std::unique_ptr<likec::Expression> right(makeExpression(binary->right()));

    switch (binary->operatorKind()) {
        case BinaryOperator::ADD:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::ADD,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), leftType->isUnsigned()), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::SUB:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::SUB,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), leftType->isUnsigned()), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::MUL:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::MUL,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), leftType->isUnsigned()), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::SIGNED_DIV:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::DIV,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), false), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), false), std::move(right)));

        case BinaryOperator::UNSIGNED_DIV:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::DIV,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), true), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), true), std::move(right)));

        case BinaryOperator::SIGNED_REM:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::REM,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), false), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), false), std::move(right)));

        case BinaryOperator::UNSIGNED_REM:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::REM,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), true), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), true), std::move(right)));

        case BinaryOperator::BITWISE_AND:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::BITWISE_AND,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), leftType->isUnsigned()), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::LOGICAL_AND:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::LOGICAL_AND,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), leftType->isUnsigned()), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::BITWISE_OR:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::BITWISE_OR,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), leftType->isUnsigned()), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::LOGICAL_OR:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::LOGICAL_OR,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), leftType->isUnsigned()), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::BITWISE_XOR:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::BITWISE_XOR,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), leftType->isUnsigned()), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::SHL:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::SHL,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), leftType->isUnsigned()), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::SAR:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::SHR,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), false), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::SHR:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::SHR,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), true), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::EQUAL:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::EQ,
                std::move(left),
                std::move(right));

        case BinaryOperator::SIGNED_LESS:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::LT,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), false), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), false), std::move(right)));

        case BinaryOperator::SIGNED_LESS_OR_EQUAL:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::LEQ,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), false), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), false), std::move(right)));

        case BinaryOperator::SIGNED_GREATER:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::GT,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), false), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), false), std::move(right)));

        case BinaryOperator::SIGNED_GREATER_OR_EQUAL:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::GEQ,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), false), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), false), std::move(right)));

        case BinaryOperator::UNSIGNED_LESS:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::LT,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), true), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), true), std::move(right)));

        case BinaryOperator::UNSIGNED_LESS_OR_EQUAL:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::LEQ,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), true), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), true), std::move(right)));

        case BinaryOperator::UNSIGNED_GREATER:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::GT,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), true), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), true), std::move(right)));

        case BinaryOperator::UNSIGNED_GREATER_OR_EQUAL:
            return std::make_unique<likec::BinaryOperator>(tree(), likec::BinaryOperator::GEQ,
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(leftType->size(), true), std::move(left)),
                std::make_unique<likec::Typecast>(tree(), tree().makeIntegerType(rightType->size(), true), std::move(right)));

        default:
            unreachable();
            return NULL;
    }
}

#ifdef NC_PREFER_CSTRINGS_TO_CONSTANTS
namespace {

bool isAscii(const QString &string) {
    foreach (QChar c, string) {
        if (c >= 0x80) {
            return false;
        }
    }
    return true;
}

} // anonymous namespace
#endif

std::unique_ptr<likec::Expression> DefinitionGenerator::makeConstant(const Term *term, const SizedValue &value) {
    const types::Type *type = types().getType(term);

#ifdef NC_PREFER_CSTRINGS_TO_CONSTANTS
    if (type->pointee() && type->pointee()->size() == 1) {
        foreach (const core::image::Section *section, parent().context().module()->image()->sections()) {
            if (section->isAllocated() && section->containsAddress(value.value())) {
                QString string = section->readAsciizString(value.value(), 1024);

                if (!string.isNull() && isAscii(string)) {
                    return std::make_unique<likec::String>(tree(), string);
                }
                break;
            }
        }
    }
#endif

#ifdef NC_PREFER_GLOBAL_VARIABLES_TO_CONSTANTS
    if (type->pointee() && type->pointee()->size()) {
        return std::make_unique<likec::UnaryOperator>(
            tree(),
            likec::UnaryOperator::REFERENCE,
            std::make_unique<likec::VariableIdentifier>(
                tree(),
                parent().makeGlobalVariableDeclaration(
                    MemoryLocation(MemoryDomain::MEMORY, value.value() * CHAR_BIT, type->pointee()->size()),
                    type)));
    }
#endif

    return std::make_unique<likec::Typecast>(tree(),
        parent().makeType(type),
        std::make_unique<likec::IntegerConstant>(
            tree(),
            value,
            tree().makeIntegerType(type->size(), type->isUnsigned())
        ));
}

bool DefinitionGenerator::isIntermediate(const Term *term) const {
    if (term->isWrite()) {
        const std::vector<const Term *> &reads = dataflow().getUses(term);

        const Term *usedRead;

        int usedReadsCount = 0;
        foreach (const Term *read, reads) {
            if (usage().isUsed(read)) {
                ++usedReadsCount;
                usedRead = read;
            }
        }

        if (usedReadsCount == 1 && term->assignee()) {
            const Term *write = term->assignee();

            return usedRead->statement() && write->statement() &&
                   usedRead->statement()->basicBlock() == write->statement()->basicBlock() &&
                   usedRead->statement()->basicBlock() != NULL;
        }
        return false;
    }
    if (term->isRead()) {
        const std::vector<const Term *> &writes = dataflow().getDefinitions(term);
        return writes.size() == 1 && isIntermediate(writes.front());
    }
    return false;
}

} // namespace cgen
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
