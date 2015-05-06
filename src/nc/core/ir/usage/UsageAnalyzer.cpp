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

#include "UsageAnalyzer.h"

#include <cassert>

#include <nc/common/Foreach.h>
#include <nc/common/Warnings.h>

#include <nc/core/arch/Architecture.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/calls/CallAnalyzer.h>
#include <nc/core/ir/calls/CallsData.h>
#include <nc/core/ir/calls/FunctionSignature.h>
#include <nc/core/ir/calls/ReturnAnalyzer.h>
#include <nc/core/ir/cflow/BasicNode.h>
#include <nc/core/ir/cflow/Graph.h>
#include <nc/core/ir/cflow/Switch.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/misc/CensusVisitor.h>

#include "Usage.h"

namespace nc {
namespace core {
namespace ir {
namespace usage {

UsageAnalyzer::UsageAnalyzer(Usage &usage, const Function *function,
    const dflow::Dataflow *dataflow, const arch::Architecture *architecture,
    const cflow::Graph *regionGraph, calls::CallsData *callsData
):
    usage_(usage), function_(function), dataflow_(dataflow),
    architecture_(architecture), regionGraph_(regionGraph), callsData_(callsData)
{
    assert(function != NULL);
    assert(dataflow != NULL);
    assert(architecture != NULL);
}

void UsageAnalyzer::analyze() {
    uselessJumps_.clear();

    if (regionGraph()) {
        foreach (auto node, regionGraph()->nodes()) {
            if (auto region = node->as<cflow::Region>()) {
                if (auto witch = region->as<cflow::Switch>()) {
                    if (witch->boundsCheckNode()) {
                        uselessJumps_.push_back(witch->boundsCheckNode()->basicBlock()->getJump());
                    }
                }
            }
        }

        std::sort(uselessJumps_.begin(), uselessJumps_.end());
    }

    misc::CensusVisitor census(callsData());
    census(function());

    foreach (const Term *term, census.terms()) {
        usage().makeUnused(term);
    }
    foreach (const Statement *statement, census.statements()) {
        computeUsage(statement);
    }
    foreach (const Term *term, census.terms()) {
        computeUsage(term);
    }

    if (callsData()) {
        if (const calls::FunctionSignature *signature = callsData()->getFunctionSignature(function())) {
            if (signature->returnValue()) {
                foreach (const Return *ret, callsData()->getReturns(function())) {
                    if (calls::ReturnAnalyzer *returnAnalyzer = callsData()->getReturnAnalyzer(function(), ret)) {
                        makeUsed(returnAnalyzer->getReturnValueTerm(signature->returnValue()));
                    }
                }
            }
        }
    }
}

void UsageAnalyzer::computeUsage(const Statement *statement) {
    switch (statement->kind()) {
        case Statement::COMMENT:
            break;
        case Statement::INLINE_ASSEMBLY:
            break;
        case Statement::ASSIGNMENT:
            break;
        case Statement::KILL:
            break;
        case Statement::JUMP: {
            const Jump *jump = statement->asJump();

            if (!std::binary_search(uselessJumps_.begin(), uselessJumps_.end(), jump)) {
                if (jump->condition()) {
                    makeUsed(jump->condition());
                }
                if (jump->thenTarget().address()) {
                    makeUsed(jump->thenTarget().address());
                }
                if (jump->elseTarget().address()) {
                    makeUsed(jump->elseTarget().address());
                }
            }
            break;
        }
        case Statement::CALL: {
            const Call *call = statement->asCall();

            makeUsed(call->target());

            if (callsData()) {
                if (const calls::FunctionSignature *signature = callsData()->getFunctionSignature(call)) {
                    if (calls::CallAnalyzer *callAnalyzer = callsData()->getCallAnalyzer(call)) {
                        foreach (const MemoryLocation &memoryLocation, signature->arguments()) {
                            makeUsed(callAnalyzer->getArgumentTerm(memoryLocation));
                        }
                    }
                }
            }

            break;
        }
        case Statement::RETURN:
            break;
        default:
            ncWarning("Was called for unsupported kind of statement.");
            break;
    }
}

void UsageAnalyzer::computeUsage(const Term *term) {
    switch (term->kind()) {
        case Term::INT_CONST:
            break;
        case Term::INTRINSIC:
            break;
        case Term::UNDEFINED:
            break;
        case Term::MEMORY_LOCATION_ACCESS: {
            if (term->isWrite()) {
                const MemoryLocationAccess *access = term->asMemoryLocationAccess();
                if (architecture()->isGlobalMemory(access->memoryLocation())) {
                    makeUsed(access);
                }
            }
            break;
        }
        case Term::DEREFERENCE: {
            if (term->isWrite()) {
                const Dereference *dereference = term->asDereference();
                const MemoryLocation &memoryLocation = dataflow()->getMemoryLocation(dereference);
                if (!memoryLocation || architecture()->isGlobalMemory(memoryLocation)) {
                    makeUsed(dereference);
                }
            }
            break;
        }
        case Term::UNARY_OPERATOR:
            break;
        case Term::BINARY_OPERATOR:
            break;
        case Term::CHOICE:
            break;
        default:
            ncWarning("Was called for unsupported kind of term.");
            break;
    }
}

void UsageAnalyzer::propagateUsage(const Term *term) {
    assert(term != NULL);

#ifdef NC_PREFER_CONSTANTS_TO_EXPRESSIONS
    if (term->isRead() && dataflow()->getValue(term)->isConstant()) {
        return;
    }
#endif

    switch (term->kind()) {
        case Term::INT_CONST:
            break;
        case Term::INTRINSIC:
            break;
        case Term::UNDEFINED:
            break;
        case Term::MEMORY_LOCATION_ACCESS: {
            if (term->isRead()) {
                foreach (const Term *definition, dataflow()->getDefinitions(term)) {
                    makeUsed(definition);
                }
            } else if (term->isWrite()) {
                if (term->assignee()) {
                    makeUsed(term->assignee());
                }
            }
            break;
        }
        case Term::DEREFERENCE: {
            if (term->isRead()) {
                foreach (const Term *definition, dataflow()->getDefinitions(term)) {
                    makeUsed(definition);
                }
            } else if (term->isWrite()) {
                if (term->assignee()) {
                    makeUsed(term->assignee());
                }
            }

            const Dereference *dereference = term->asDereference();
            const dflow::Value *addressValue = dataflow()->getValue(dereference->address());
            if (!addressValue->isStackOffset() && !addressValue->isConstant()) {
                makeUsed(dereference->address());
            }
            break;
        }
        case Term::UNARY_OPERATOR: {
            const UnaryOperator *unary = term->asUnaryOperator();
            makeUsed(unary->operand());
            break;
        }
        case Term::BINARY_OPERATOR: {
            const BinaryOperator *binary = term->asBinaryOperator();
            makeUsed(binary->left());
            makeUsed(binary->right());
            break;
        }
        case Term::CHOICE: {
            const Choice *choice = term->asChoice();
            if (!dataflow()->getDefinitions(choice->preferredTerm()).empty()) {
                makeUsed(choice->preferredTerm());
            } else {
                makeUsed(choice->defaultTerm());
            }
            break;
        }
        default:
            ncWarning("Was called for unsupported kind of term.");
            break;
    }
}

void UsageAnalyzer::makeUsed(const Term *term) {
    if (!usage().isUsed(term)) {
        usage().makeUsed(term);
        propagateUsage(term);
    }
}

} // namespace usage
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
