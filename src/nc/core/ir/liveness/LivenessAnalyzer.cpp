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

#include "LivenessAnalyzer.h"

#include <cassert>

#include <nc/common/Foreach.h>
#include <nc/common/Warnings.h>

#include <nc/core/arch/Architecture.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/calling/CallHook.h>
#include <nc/core/ir/calling/Hooks.h>
#include <nc/core/ir/calling/ReturnHook.h>
#include <nc/core/ir/calling/Signatures.h>
#include <nc/core/ir/cflow/BasicNode.h>
#include <nc/core/ir/cflow/Graph.h>
#include <nc/core/ir/cflow/Switch.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/Value.h>

#include "Liveness.h"

namespace nc {
namespace core {
namespace ir {
namespace liveness {

void LivenessAnalyzer::analyze() {
    deadJumps_.clear();

    foreach (auto node, regionGraph().nodes()) {
        if (auto region = node->as<cflow::Region>()) {
            if (auto witch = region->as<cflow::Switch>()) {
                if (witch->boundsCheckNode()) {
                    deadJumps_.push_back(witch->boundsCheckNode()->basicBlock()->getJump());
                }
            }
        }
    }

    std::sort(deadJumps_.begin(), deadJumps_.end());

    foreach (const BasicBlock *basicBlock, function()->basicBlocks()) {
        foreach (const Statement *statement, basicBlock->statements()) {
            computeLiveness(statement);
        }
    }
}

void LivenessAnalyzer::computeLiveness(const Statement *statement) {
    switch (statement->kind()) {
        case Statement::INLINE_ASSEMBLY:
            break;
        case Statement::ASSIGNMENT: {
            auto assignment = statement->asAssignment();
            auto memoryLocation = dataflow().getMemoryLocation(assignment->left());

            if (!memoryLocation || architecture()->isGlobalMemory(memoryLocation)) {
                makeLive(assignment->left());
            }
            break;
        }
        case Statement::JUMP: {
            const Jump *jump = statement->asJump();

            if (!std::binary_search(deadJumps_.begin(), deadJumps_.end(), jump)) {
                if (jump->condition()) {
                    makeLive(jump->condition());
                }
                if (jump->thenTarget().address()) {
                    makeLive(jump->thenTarget().address());
                }
                if (jump->elseTarget().address()) {
                    makeLive(jump->elseTarget().address());
                }
            }
            break;
        }
        case Statement::CALL: {
            const Call *call = statement->asCall();

            makeLive(call->target());

            if (auto signature = signatures().getSignature(call)) {
                if (auto callHook = hooks().getCallHook(call)) {
                    foreach (const auto &argument, signature->arguments()) {
                        makeLive(callHook->getArgumentTerm(argument.get()));
                    }
                }
            }

            break;
        }
        case Statement::RETURN: {
            auto ret = statement->asReturn();

            if (auto signature = signatures().getSignature(function())) {
                if (signature->returnValue()) {
                    if (auto returnHook = hooks().getReturnHook(ret)) {
                        makeLive(returnHook->getReturnValueTerm(signature->returnValue().get()));
                    }
                }
            }
            break;
        }
        case Statement::TOUCH:
            break;
        default:
            ncWarning("Unknown statement kind: '%1'.", static_cast<int>(statement->kind()));
            break;
    }
}

void LivenessAnalyzer::propagateLiveness(const Term *term) {
    assert(term != NULL);

#ifdef NC_PREFER_CONSTANTS_TO_EXPRESSIONS
    if (term->isRead() && dataflow().getValue(term)->abstractValue().isConcrete()) {
        return;
    }
#endif

    switch (term->kind()) {
        case Term::INT_CONST:
            break;
        case Term::INTRINSIC:
            break;
        case Term::MEMORY_LOCATION_ACCESS: {
            if (term->isRead()) {
                foreach (auto &chunk, dataflow().getDefinitions(term).chunks()) {
                    foreach (const Term *definition, chunk.definitions()) {
                        makeLive(definition);
                    }
                }
            } else if (term->isWrite()) {
                if (term->source()) {
                    makeLive(term->source());
                }
            }
            break;
        }
        case Term::DEREFERENCE: {
            if (term->isRead()) {
                foreach (auto &chunk, dataflow().getDefinitions(term).chunks()) {
                    foreach (const Term *definition, chunk.definitions()) {
                        makeLive(definition);
                    }
                }
            } else if (term->isWrite()) {
                if (term->source()) {
                    makeLive(term->source());
                }
            }

            if (!dataflow().getMemoryLocation(term)) {
                makeLive(term->asDereference()->address());
            }
            break;
        }
        case Term::UNARY_OPERATOR: {
            const UnaryOperator *unary = term->asUnaryOperator();
            makeLive(unary->operand());
            break;
        }
        case Term::BINARY_OPERATOR: {
            const BinaryOperator *binary = term->asBinaryOperator();
            makeLive(binary->left());
            makeLive(binary->right());
            break;
        }
        case Term::CHOICE: {
            const Choice *choice = term->asChoice();
            if (!dataflow().getDefinitions(choice->preferredTerm()).empty()) {
                makeLive(choice->preferredTerm());
            } else {
                makeLive(choice->defaultTerm());
            }
            break;
        }
        default:
            ncWarning("Unknown term kind: '%1'.", static_cast<int>(term->kind()));
            break;
    }
}

void LivenessAnalyzer::makeLive(const Term *term) {
    assert(term != NULL);
    if (!liveness().isLive(term)) {
        liveness().makeLive(term);
        propagateLiveness(term);
    }
}

} // namespace liveness
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
