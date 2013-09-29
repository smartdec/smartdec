/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

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

#include "DataflowAnalyzer.h"

#include <boost/unordered_map.hpp>

#include <nc/common/CancellationToken.h>
#include <nc/common/Foreach.h>
#include <nc/common/Unreachable.h>
#include <nc/common/Warnings.h>

#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/Instruction.h>
#include <nc/core/arch/Register.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/CFG.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/calls/CallAnalyzer.h>
#include <nc/core/ir/calls/CallsData.h>
#include <nc/core/ir/calls/FunctionAnalyzer.h>
#include <nc/core/ir/calls/ReturnAnalyzer.h>

#include "Dataflow.h"
#include "ExecutionContext.h"
#include "Value.h"

namespace nc {
namespace core {
namespace ir {
namespace dflow {

void DataflowAnalyzer::analyze(const CancellationToken &canceled) {
    assert(function() != NULL);

    /*
     * Returns true if the given term does not cover given memory location.
     */
    auto doesNotCover = [this](const MemoryLocation &mloc, const Term *term) -> bool {
        return !dataflow().getMemoryLocation(term).covers(mloc);
    };

    /* Control-flow graph to run abstract interpretation loop on. */
    CFG cfg(function()->basicBlocks());

    /* Mapping of a basic block to the definitions reaching its end. */
    boost::unordered_map<const BasicBlock *, ReachingDefinitions> outDefinitions;

    /*
     * Running abstract interpretation until at fixpoint twice in a row.
     */
    int niterations = 0;
    bool changed;
    bool fixpointReached = false;

    do {
        changed = false;

        /*
         * Run abstract interpretation on all basic blocks.
         */
        foreach (const BasicBlock *basicBlock, function()->basicBlocks()) {
            ExecutionContext context(*this, fixpointReached);

            /* Merge the reaching definitions from predecessors. */
            foreach (const BasicBlock *predecessor, cfg.getPredecessors(basicBlock)) {
                context.definitions().merge(outDefinitions[predecessor]);
            }

            /* Remove definitions that do not cover the memory location that they define. */
            context.definitions().filterOut(doesNotCover);

            /* If this is a function entry, run the calling convention-specific code. */
            if (basicBlock == function()->entry()) {
                if (callsData()) {
                    if (auto functionAnalyzer = callsData()->getFunctionAnalyzer(function())) {
                        functionAnalyzer->executeEnter(context);
                    }
                }
            }

            /* Simulate all the statements in the basic block. */
            foreach (const Statement *statement, basicBlock->statements()) {
                execute(statement, context);
            }

            /* Something changed? */
            ReachingDefinitions &definitions(outDefinitions[basicBlock]);
            if (definitions != context.definitions()) {
                definitions = context.definitions();
                changed = true;
            }
        }

        /*
         * Compute uses.
         */
        foreach (auto &termAndUses, dataflow().term2uses()) {
            termAndUses.second.clear();
        }

        foreach (auto &termAndDefinitions, dataflow().term2definitions()) {
            termAndDefinitions.second.filterOut(doesNotCover);

            foreach (const auto &pair, termAndDefinitions.second.pairs()) {
                foreach (const Term *definition, pair.second) {
                    dataflow().getUses(definition).push_back(termAndDefinitions.first);
                }
            }
        }

        /*
         * Have we reached a fixpoint?
         */
        if (changed) {
            fixpointReached = false;
        } else if (!fixpointReached) {
            fixpointReached = true;
            changed = true;
        }

        /*
         * Do we loop infinitely?
         */
        if (++niterations >= 30) {
            ncWarning("Didn't reach a fixpoint after %1 iterations while analyzing dataflow of %2. Giving up.",
                niterations, function()->name());
            break;
        }
    } while (changed && !canceled);
}

void DataflowAnalyzer::execute(const Statement *statement, ExecutionContext &context) {
    switch (statement->kind()) {
        case Statement::COMMENT:
            break;
        case Statement::INLINE_ASSEMBLY:
            /*
             * To be completely correct, one should clear reaching definitions.
             * However, not doing this usually leads to better code.
             */
            break;
        case Statement::ASSIGNMENT: {
            auto assignment = statement->asAssignment();
            execute(assignment->right(), context);
            execute(assignment->left(), context);
            break;
        }
        case Statement::KILL: {
            auto kill = statement->asKill();
            execute(kill->term(), context);
            break;
        }
        case Statement::JUMP: {
            auto jump = statement->asJump();

            if (jump->condition()) {
                execute(jump->condition(), context);
            }
            if (jump->thenTarget().address()) {
                execute(jump->thenTarget().address(), context);
            }
            if (jump->elseTarget().address()) {
                execute(jump->elseTarget().address(), context);
            }
            break;
        }
        case Statement::CALL: {
            auto call = statement->asCall();
            execute(call->target(), context);

            if (callsData()) {
                const Value *targetValue = dataflow().getValue(call->target());
                if (targetValue->abstractValue().isConcrete()) {
                    callsData()->setCalledAddress(call, targetValue->abstractValue().asConcrete().value());
                }
                if (auto callAnalyzer = callsData()->getCallAnalyzer(call)) {
                    callAnalyzer->executeCall(context);
                }
            }
            break;
        }
        case Statement::RETURN: {
            if (function() && callsData()) {
                if (auto returnAnalyzer = callsData()->getReturnAnalyzer(function(), statement->asReturn())) {
                    returnAnalyzer->executeReturn(context);
                }
            }
            break;
        }
        default:
            ncWarning("Unsupported statement kind: '%1'.", static_cast<int>(statement->kind()));
            break;
    }
}

void DataflowAnalyzer::execute(const Term *term, ExecutionContext &context) {
    assert(term != NULL);

    switch (term->kind()) {
        case Term::INT_CONST: {
            auto constant = term->asConstant();
            Value *value = dataflow().getValue(constant);
            value->setAbstractValue(constant->value());
            value->makeNotStackOffset();
            value->makeNotProduct();
            break;
        }
        case Term::INTRINSIC: /* FALLTHROUGH */
        case Term::UNDEFINED: {
            Value *value = dataflow().getValue(term);
            value->setAbstractValue(AbstractValue(term->size(), -1, -1));
            value->makeNotStackOffset();
            value->makeNotProduct();
            break;
        }
        case Term::MEMORY_LOCATION_ACCESS: {
            auto access = term->asMemoryLocationAccess();
            setMemoryLocation(access, access->memoryLocation(), context);

            /* The value of instruction pointer is always easy to guess. */
            if (architecture()->instructionPointer() &&
                access->memoryLocation() == architecture()->instructionPointer()->memoryLocation() &&
                access->statement() &&
                access->statement()->instruction())
            {
                dataflow().getValue(access)->setAbstractValue(
                    SizedValue(term->size(), access->statement()->instruction()->addr()));
            }
            break;
        }
        case Term::DEREFERENCE: {
            auto dereference = term->asDereference();
            execute(dereference->address(), context);

            /* Compute memory location. */
            auto addressValue = dataflow().getValue(dereference->address());
            if (addressValue->abstractValue().isConcrete()) {
                if (dereference->domain() == MemoryDomain::MEMORY) {
                    setMemoryLocation(
                        dereference,
                        MemoryLocation(
                            dereference->domain(),
                            addressValue->abstractValue().asConcrete().value() * CHAR_BIT,
                            dereference->size()),
                        context);
                } else {
                    setMemoryLocation(
                        dereference,
                            MemoryLocation(
                                dereference->domain(),
                                addressValue->abstractValue().asConcrete().value(),
                                dereference->size()),
                        context);
                }
            } else if (addressValue->isStackOffset()) {
                setMemoryLocation(
                    dereference,
                    MemoryLocation(MemoryDomain::STACK, addressValue->stackOffset() * CHAR_BIT, dereference->size()),
                    context);
            } else {
                setMemoryLocation(dereference, MemoryLocation(), context);
            }
            break;
        }
        case Term::UNARY_OPERATOR:
            executeUnaryOperator(term->asUnaryOperator(), context);
            break;
        case Term::BINARY_OPERATOR:
            executeBinaryOperator(term->asBinaryOperator(), context);
            break;
        case Term::CHOICE: {
            auto choice = term->asChoice();
            execute(choice->preferredTerm(), context);
            execute(choice->defaultTerm(), context);

            if (!dataflow().getDefinitions(choice->preferredTerm()).empty()) {
                *dataflow().getValue(choice) = *dataflow().getValue(choice->preferredTerm());
            } else {
                *dataflow().getValue(choice) = *dataflow().getValue(choice->defaultTerm());
            }
            break;
        }
        default:
            ncWarning("Unknown term kind: '%1'.", static_cast<int>(term->kind()));
            break;
    }
}

void DataflowAnalyzer::setMemoryLocation(const Term *term, const MemoryLocation &newMemoryLocation, ExecutionContext &context) {
    auto oldMemoryLocation = dataflow().getMemoryLocation(term);

    /*
     * If the term has changed its location, remember the new location.
     */
    if (oldMemoryLocation != newMemoryLocation) {
        dataflow().setMemoryLocation(term, newMemoryLocation);

        /*
         * If the term is a write and had a memory location before,
         * reaching definitions can indicate that it defines the old
         * memory location. Fix this.
         */
        if (oldMemoryLocation && term->isWrite()) {
            context.definitions().filterOut(
                [term](const MemoryLocation &, const Term *definition) -> bool {
                    return definition == term;
                }
            );
        }
    }

    /*
     * If the term has a memory location and not a global variable,
     * remember or update reaching definitions accordingly.
     */
    if (newMemoryLocation && !architecture()->isGlobalMemory(newMemoryLocation)) {
        if (term->isRead()) {
            auto &definitions = dataflow().getDefinitions(term);
            context.definitions().project(newMemoryLocation, definitions);
            mergeReachingValues(term, newMemoryLocation, definitions);
        }
        if (term->isWrite()) {
            context.definitions().addDefinition(newMemoryLocation, term);
        }
        if (term->isKill()) {
            context.definitions().killDefinitions(newMemoryLocation);
        }
    } else {
        if (term->isRead() && oldMemoryLocation) {
            dataflow().getDefinitions(term).clear();
        }
    }
}

void DataflowAnalyzer::mergeReachingValues(const Term *term, const MemoryLocation &termLocation, const ReachingDefinitions &definitions) {
    assert(term);
    assert(term->isRead());
    assert(termLocation);

    if (definitions.empty()) {
        return;
    }

    /*
     * Merge abstract values.
     */
    auto termValue = dataflow().getValue(term);
    auto termAbstractValue = termValue->abstractValue();

    foreach (const auto &pair, definitions.pairs()) {
        auto &definedLocation = pair.first;
        assert(termLocation.covers(definedLocation));

        /*
         * Mask of bits inside termAbstractValue which are covered by definedLocation.
         */
        auto mask = bitMask<ConstantValue>(definedLocation.size());
        if (architecture()->byteOrder() == ByteOrder::LittleEndian) {
            mask = bitShift(mask, definedLocation.addr() - termLocation.addr());
        } else {
            mask = bitShift(mask, termLocation.endAddr() - definedLocation.endAddr());
        }

        foreach (const Term *definition, pair.second) {
            auto definitionLocation = dataflow().getMemoryLocation(definition);
            assert(definitionLocation.covers(definedLocation));

            auto definitionValue = dataflow().getValue(definition);
            auto definitionAbstractValue = definitionValue->abstractValue();

            /*
             * Shift definition's abstract value to match term's location.
             */
            if (architecture()->byteOrder() == ByteOrder::LittleEndian) {
                definitionAbstractValue.shift(definitionLocation.addr() - termLocation.addr());
            } else {
                definitionAbstractValue.shift(termLocation.endAddr() - definitionLocation.endAddr());
            }

            /* Project the value to the defined location. */
            definitionAbstractValue.project(mask);

            /* Update term's value. */
            termAbstractValue.merge(definitionAbstractValue);
        }
    }

    termValue->setAbstractValue(termAbstractValue.resize(term->size()));

    /*
     * Merge stack offset and product flags.
     *
     * Heuristic: merge information only from terms that define lower bits of the term's value.
     */
    const std::vector<const Term *> *lowerBitsDefinitions = NULL;

    if (architecture()->byteOrder() == ByteOrder::LittleEndian) {
        if (definitions.pairs().front().first.addr() == termLocation.addr()) {
            lowerBitsDefinitions = &definitions.pairs().front().second;
        }
    } else {
        if (definitions.pairs().back().first.endAddr() == termLocation.endAddr()) {
            lowerBitsDefinitions = &definitions.pairs().back().second;
        }
    }

    if (lowerBitsDefinitions) {
        foreach (auto definition, *lowerBitsDefinitions) {
            auto definitionValue = dataflow().getValue(definition);

            if (definitionValue->isNotStackOffset()) {
                termValue->makeNotStackOffset();
            } else if (definitionValue->isStackOffset()) {
                termValue->makeStackOffset(definitionValue->stackOffset());
            }

            if (definitionValue->isNotProduct()) {
                termValue->makeNotProduct();
            } else if (definitionValue->isProduct()) {
                termValue->makeProduct();
            }
        }
    }
}

void DataflowAnalyzer::executeUnaryOperator(const UnaryOperator *unary, ExecutionContext &context) {
    execute(unary->operand(), context);

    Value *value = dataflow().getValue(unary);
    Value *operandValue = dataflow().getValue(unary->operand());

    value->setAbstractValue(apply(unary, operandValue->abstractValue()).merge(value->abstractValue()));

    switch (unary->operatorKind()) {
        case UnaryOperator::SIGN_EXTEND:
        case UnaryOperator::ZERO_EXTEND:
        case UnaryOperator::TRUNCATE:
            if (operandValue->isNotStackOffset()) {
                value->makeNotStackOffset();
            } else if (operandValue->isStackOffset()) {
                value->makeStackOffset(operandValue->stackOffset());
            }
            if (operandValue->isNotProduct()) {
                value->makeNotProduct();
            } else if (operandValue->isProduct()) {
                value->makeProduct();
            }
            break;
        default:
            value->makeNotStackOffset();
            value->makeNotProduct();
            break;
    }
}

void DataflowAnalyzer::executeBinaryOperator(const BinaryOperator *binary, ExecutionContext &context) {
    execute(binary->left(), context);
    execute(binary->right(), context);

    Value *value = dataflow().getValue(binary);
    Value *leftValue = dataflow().getValue(binary->left());
    Value *rightValue = dataflow().getValue(binary->right());

    value->setAbstractValue(apply(binary, leftValue->abstractValue(), rightValue->abstractValue()).merge(value->abstractValue()));

    /* Compute stack offset. */
    switch (binary->operatorKind()) {
        case BinaryOperator::ADD:
            if (leftValue->abstractValue().isConcrete()) {
                if (rightValue->isStackOffset()) {
                    value->makeStackOffset(leftValue->abstractValue().asConcrete().signedValue() + rightValue->stackOffset());
                } else if (rightValue->isNotStackOffset()) {
                    value->makeNotStackOffset();
                }
            } else if (leftValue->abstractValue().isNondeterministic()) {
                value->makeNotStackOffset();
            }
            if (rightValue->abstractValue().isConcrete()) {
                if (leftValue->isStackOffset()) {
                    value->makeStackOffset(leftValue->stackOffset() + rightValue->abstractValue().asConcrete().signedValue());
                } else if (leftValue->isNotStackOffset()) {
                    value->makeNotStackOffset();
                }
            } else if (rightValue->abstractValue().isNondeterministic()) {
                value->makeNotStackOffset();
            }
            break;

        case BinaryOperator::SUB:
            if (leftValue->isStackOffset() && rightValue->abstractValue().isConcrete()) {
                value->makeStackOffset(leftValue->stackOffset() - rightValue->abstractValue().asConcrete().signedValue());
            } else if (leftValue->isNotStackOffset() || rightValue->abstractValue().isNondeterministic()) {
                value->makeNotStackOffset();
            }
            break;

        case BinaryOperator::AND:
            /* Sometimes used for getting aligned stack pointer values. */
            if (leftValue->isStackOffset() && rightValue->abstractValue().isConcrete()) {
                value->makeStackOffset(leftValue->stackOffset() & rightValue->abstractValue().asConcrete().value());
            } else if (rightValue->isStackOffset() && leftValue->abstractValue().isConcrete()) {
                value->makeStackOffset(rightValue->stackOffset() & leftValue->abstractValue().asConcrete().value());
            } else if ((leftValue->abstractValue().isNondeterministic() && leftValue->isNotStackOffset()) ||
                       (rightValue->abstractValue().isNondeterministic() && rightValue->isNotStackOffset())) {
                value->makeNotStackOffset();
            }
            break;

        default:
            value->makeNotStackOffset();
            break;
    }

    /* Compute product flag. */
    switch (binary->operatorKind()) {
        case BinaryOperator::MUL:
        case BinaryOperator::SHL:
            value->makeProduct();
            break;
        default:
            value->makeNotProduct();
            break;
    }
}

AbstractValue DataflowAnalyzer::apply(const UnaryOperator *unary, const AbstractValue &a) {
    switch (unary->operatorKind()) {
        case UnaryOperator::NOT:
            return ~a;
        case UnaryOperator::NEGATION:
            return -a;
        case UnaryOperator::SIGN_EXTEND:
            return dflow::AbstractValue(a).signExtend(unary->size());
        case UnaryOperator::ZERO_EXTEND:
            return dflow::AbstractValue(a).zeroExtend(unary->size());
        case UnaryOperator::TRUNCATE:
            return dflow::AbstractValue(a).resize(unary->size());
        default:
            ncWarning("Unknown unary operator kind: %1", unary->operatorKind());
            return dflow::AbstractValue();
    }
}

AbstractValue DataflowAnalyzer::apply(const BinaryOperator *binary, const AbstractValue &a, const AbstractValue &b) {
    switch (binary->operatorKind()) {
        case BinaryOperator::AND:
            return a & b;
        case BinaryOperator::OR:
            return a | b;
        case BinaryOperator::XOR:
            return a ^ b;
        case BinaryOperator::SHL:
            return a << b;
        case BinaryOperator::SHR:
            return dflow::UnsignedAbstractValue(a) >> b;
        case BinaryOperator::SAR:
            return dflow::SignedAbstractValue(a) >> b;
        case BinaryOperator::ADD:
            return a + b;
        case BinaryOperator::SUB:
            return a - b;
        case BinaryOperator::MUL:
            return a * b;
        case BinaryOperator::SIGNED_DIV:
            return dflow::SignedAbstractValue(a) / b;
        case BinaryOperator::UNSIGNED_DIV:
            return dflow::UnsignedAbstractValue(a) / b;
        case BinaryOperator::SIGNED_REM:
            return dflow::SignedAbstractValue(a) % b;
        case BinaryOperator::UNSIGNED_REM:
            return dflow::UnsignedAbstractValue(a) % b;
        case BinaryOperator::EQUAL:
            return a == b;
        case BinaryOperator::SIGNED_LESS:
            return dflow::SignedAbstractValue(a) < b;
        case BinaryOperator::SIGNED_LESS_OR_EQUAL:
            return dflow::SignedAbstractValue(a) <= b;
        case BinaryOperator::UNSIGNED_LESS:
            return dflow::UnsignedAbstractValue(a) < b;
        case BinaryOperator::UNSIGNED_LESS_OR_EQUAL:
            return dflow::UnsignedAbstractValue(a) <= b;
        default:
            ncWarning("Unknown binary operator kind: %1", binary->operatorKind());
            return dflow::AbstractValue();
    }
}

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
