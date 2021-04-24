/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

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

#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/Instruction.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/CFG.h>
#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>

#include "Dataflow.h"
#include "Value.h"

namespace nc {
namespace core {
namespace ir {
namespace dflow {

namespace {

template<class Map, class Pred>
void remove_if(Map &map, Pred pred) {
    auto i = map.begin();
    auto iend = map.end();

    while (i != iend) {
        if (pred(i->first)) {
            i = map.erase(i);
        } else {
            ++i;
        }
    }
}

} // anonymous namespace

void DataflowAnalyzer::analyze(const CFG &cfg) {
    /*
     * Returns true if the given term does not cover given memory location.
     */
    auto notCovered = [this](const MemoryLocation &mloc, const Term *term) -> bool {
        return !dataflow().getMemoryLocation(term).covers(mloc);
    };

    /* Mapping of a basic block to the definitions reaching its end. */
    boost::unordered_map<const BasicBlock *, ReachingDefinitions> outDefinitions;

    /*
     * Running abstract interpretation until reaching a fixpoint several times in a row.
     */
    int niterations = 0;
    int nfixpoints = 0;

    while (nfixpoints++ < 3) {
        /*
         * Run abstract interpretation on all basic blocks.
         */
        foreach (auto basicBlock, cfg.basicBlocks()) {
            ReachingDefinitions definitions;

            /* Merge reaching definitions from predecessors. */
            foreach (const BasicBlock *predecessor, cfg.getPredecessors(basicBlock)) {
                definitions.merge(outDefinitions[predecessor]);
            }

            /* Remove definitions that do not cover the memory location that they define. */
            definitions.filterOut(notCovered);

            /* Execute all the statements in the basic block. */
            foreach (auto statement, basicBlock->statements()) {
                execute(statement, definitions);
            }

            /* Something has changed? */
            ReachingDefinitions &oldDefinitions(outDefinitions[basicBlock]);
            if (oldDefinitions != definitions) {
                oldDefinitions = std::move(definitions);
                nfixpoints = 0;
            }
        }

        /*
         * Some terms might have changed their addresses. Filter again.
         */
        foreach (auto &termAndDefinitions, dataflow().term2definitions()) {
            termAndDefinitions.second.filterOut(notCovered);
        }

        /*
         * Do we loop infinitely?
         */
        if (++niterations >= 30) {
            log_.warning(tr("%1: Fixpoint was not reached after %2 iterations.").arg(Q_FUNC_INFO).arg(niterations));
            break;
        }

        canceled_.poll();
    }

    /*
     * Remove information about terms that disappeared.
     * Terms can disappear if e.g. a call is deinstrumented during the analysis.
     */
    auto disappeared = [](const Term *term){ return term->statement()->basicBlock() == nullptr; };

    std::vector<const Term *> disappearedTerms;
    foreach (auto &termAndDefinitions, dataflow().term2definitions()) {
        termAndDefinitions.second.filterOut([disappeared](const MemoryLocation &, const Term *term) { return disappeared(term); } );
    }

    remove_if(dataflow().term2value(), disappeared);
    remove_if(dataflow().term2location(), disappeared);
    remove_if(dataflow().term2definitions(), disappeared);
}

void DataflowAnalyzer::execute(const Statement *statement, ReachingDefinitions &definitions) {
    switch (statement->kind()) {
        case Statement::INLINE_ASSEMBLY:
            /*
             * To be completely correct, one should clear reaching definitions.
             * However, not doing this usually leads to better code.
             */
            break;
        case Statement::ASSIGNMENT: {
            auto assignment = statement->asAssignment();
            computeValue(assignment->right(), definitions);
            handleWrite(assignment->left(), computeMemoryLocation(assignment->left(), definitions), definitions);
            break;
        }
        case Statement::JUMP: {
            auto jump = statement->asJump();

            if (jump->condition()) {
                computeValue(jump->condition(), definitions);
            }
            if (jump->thenTarget().address()) {
                computeValue(jump->thenTarget().address(), definitions);
            }
            if (jump->elseTarget().address()) {
                computeValue(jump->elseTarget().address(), definitions);
            }
            break;
        }
        case Statement::CALL: {
            auto call = statement->asCall();
            computeValue(call->target(), definitions);
            break;
        }
        case Statement::HALT: {
            break;
        }
        case Statement::TOUCH: {
            auto touch = statement->asTouch();
            switch (touch->accessType()) {
                case Term::READ:
                    computeValue(touch->term(), definitions);
                    break;
                case Term::WRITE:
                    handleWrite(touch->term(), computeMemoryLocation(touch->term(), definitions), definitions);
                    break;
                default:
                    unreachable();
            }
            break;
        }
        case Statement::CALLBACK: {
            statement->asCallback()->function()();
            break;
        }
        case Statement::REMEMBER_REACHING_DEFINITIONS: {
            dataflow_.getDefinitions(statement) = definitions;
            break;
        }
        default:
            log_.warning(tr("%1: Unknown statement kind: %2.").arg(Q_FUNC_INFO).arg(statement->kind()));
            break;
    }
}

Value *DataflowAnalyzer::computeValue(const Term *term, const ReachingDefinitions &definitions) {
    switch (term->kind()) {
        case Term::INT_CONST: {
            auto constant = term->asConstant();
            Value *value = dataflow().getValue(constant);
            value->setAbstractValue(constant->value());
            value->makeNotStackOffset();
            value->makeNotProduct();
            value->makeNotReturnAddress();
            return value;
        }
        case Term::INTRINSIC: {
            auto intrinsic = term->asIntrinsic();
            Value *value = dataflow().getValue(intrinsic);

            switch (intrinsic->intrinsicKind()) {
                case Intrinsic::UNKNOWN: /* FALLTHROUGH */
                case Intrinsic::UNDEFINED: {
                    value->setAbstractValue(AbstractValue(term->size(), -1, -1));
                    value->makeNotStackOffset();
                    value->makeNotProduct();
                    value->makeNotReturnAddress();
                    break;
                }
                case Intrinsic::ZERO_STACK_OFFSET: {
                    value->setAbstractValue(AbstractValue(term->size(), -1, -1));
                    value->makeStackOffset(0);
                    value->makeNotProduct();
                    value->makeNotReturnAddress();
                    break;
                }
                case Intrinsic::RETURN_ADDRESS: {
                    value->setAbstractValue(AbstractValue(term->size(), -1, -1));
                    value->makeNotStackOffset();
                    value->makeNotProduct();
                    value->makeReturnAddress();
                    break;
                }
                default: {
                    log_.warning(tr("%1: Unknown kind of intrinsic: %2.").arg(Q_FUNC_INFO).arg(intrinsic->intrinsicKind()));
                    break;
                }
            }
            return value;
        }
        case Term::MEMORY_LOCATION_ACCESS: /* FALLTHROUGH */
        case Term::DEREFERENCE: {
            const auto &memoryLocation = computeMemoryLocation(term, definitions);
            const auto &reachingDefinitions = computeReachingDefinitions(term, memoryLocation, definitions);
            return computeValue(term, memoryLocation, reachingDefinitions);
        }
        case Term::UNARY_OPERATOR:
            return computeValue(term->asUnaryOperator(), definitions);
        case Term::BINARY_OPERATOR:
            return computeValue(term->asBinaryOperator(), definitions);
        default: {
            log_.warning(tr("%1: Unknown term kind: %2.").arg(Q_FUNC_INFO).arg(term->kind()));
            return dataflow().getValue(term);
        }
    }
}

const MemoryLocation &DataflowAnalyzer::computeMemoryLocation(const Term *term, const ReachingDefinitions &definitions) {
    return dataflow().setMemoryLocation(term, [&]() -> MemoryLocation {
        switch (term->kind()) {
            case Term::MEMORY_LOCATION_ACCESS: {
                return term->asMemoryLocationAccess()->memoryLocation();
            }
            case Term::DEREFERENCE: {
                auto dereference = term->asDereference();
                auto addressValue = computeValue(dereference->address(), definitions);

                if (addressValue->abstractValue().isConcrete()) {
                    if (dereference->domain() == MemoryDomain::MEMORY) {
                        return MemoryLocation(
                            dereference->domain(),
                            addressValue->abstractValue().asConcrete().value() * CHAR_BIT,
                            dereference->size());
                    } else {
                        return MemoryLocation(
                            dereference->domain(),
                            addressValue->abstractValue().asConcrete().value(),
                            dereference->size());
                    }
                } else if (addressValue->isStackOffset()) {
                    return MemoryLocation(MemoryDomain::STACK, addressValue->stackOffset() * CHAR_BIT, dereference->size());
                } else {
                    return MemoryLocation();
                }
                break;
            }
            default: {
                log_.warning(tr("%1: Term kind %2 cannot have a memory location.").arg(Q_FUNC_INFO).arg(term->kind()));
                return MemoryLocation();
            }
        }
    }());
}

const ReachingDefinitions &DataflowAnalyzer::computeReachingDefinitions(const Term *term,
                                                                        const MemoryLocation &memoryLocation,
                                                                        const ReachingDefinitions &definitions) {
    auto &termDefinitions = dataflow().getDefinitions(term);

    if (isTracked(memoryLocation)) {
        definitions.project(memoryLocation, termDefinitions);
    } else {
        termDefinitions.clear();
    }

    return termDefinitions;
}

bool DataflowAnalyzer::isTracked(const MemoryLocation &memoryLocation) const {
    return memoryLocation && !architecture()->isGlobalMemory(memoryLocation);
}

Value *DataflowAnalyzer::computeValue(const Term *term, const MemoryLocation &memoryLocation,
                                      const ReachingDefinitions &definitions) {
    assert(term);
    assert(term->isRead());
    assert(memoryLocation || definitions.empty());

    auto value = dataflow().getValue(term);

    if (definitions.empty()) {
        return value;
    }

    auto byteOrder = architecture()->getByteOrder(memoryLocation.domain());

    /*
     * Merge abstract values.
     */
    AbstractValue abstractValue(value->abstractValue().size(), 0, 0);

    foreach (const auto &chunk, definitions.chunks()) {
        assert(memoryLocation.covers(chunk.location()));

        /*
         * Mask of bits inside abstractValue which are covered by chunk's location.
         */
        auto mask = bitMask<ConstantValue>(chunk.location().size());
        if (byteOrder == ByteOrder::LittleEndian) {
            mask = bitShift(mask, chunk.location().addr() - memoryLocation.addr());
        } else {
            mask = bitShift(mask, memoryLocation.endAddr() - chunk.location().endAddr());
        }

        foreach (auto definition, chunk.definitions()) {
            auto definitionLocation = dataflow().getMemoryLocation(definition);
            assert(definitionLocation.covers(chunk.location()));

            auto definitionValue = dataflow().getValue(definition);
            auto definitionAbstractValue = definitionValue->abstractValue();

            /*
             * Shift definition's abstract value to match term's location.
             */
            if (byteOrder == ByteOrder::LittleEndian) {
                definitionAbstractValue.shift(definitionLocation.addr() - memoryLocation.addr());
            } else {
                definitionAbstractValue.shift(memoryLocation.endAddr() - definitionLocation.endAddr());
            }

            /* Project the value to the defined location. */
            definitionAbstractValue.project(mask);

            /* Update the new abstract value. */
            abstractValue = AbstractValue(abstractValue.size(),
                abstractValue.zeroBits() | definitionAbstractValue.zeroBits(),
                abstractValue.oneBits() | definitionAbstractValue.oneBits());
        }
    }

    value->setAbstractValue(abstractValue);

    /*
     * Merge stack offset and product flags.
     *
     * Heuristic: merge information only from terms that define lower bits of the term's value.
     */
    const std::vector<const Term *> *lowerBitsDefinitions = nullptr;

    if (byteOrder == ByteOrder::LittleEndian) {
        if (definitions.chunks().front().location().addr() == memoryLocation.addr()) {
            lowerBitsDefinitions = &definitions.chunks().front().definitions();
        }
    } else {
        if (definitions.chunks().back().location().endAddr() == memoryLocation.endAddr()) {
            lowerBitsDefinitions = &definitions.chunks().back().definitions();
        }
    }

    if (lowerBitsDefinitions) {
        foreach (auto definition, *lowerBitsDefinitions) {
            auto definitionValue = dataflow().getValue(definition);

            if (definitionValue->isNotStackOffset()) {
                value->makeNotStackOffset();
            } else if (definitionValue->isStackOffset()) {
                value->makeStackOffset(definitionValue->stackOffset());
            }

            if (definitionValue->isNotProduct()) {
                value->makeNotProduct();
            } else if (definitionValue->isProduct()) {
                value->makeProduct();
            }
        }
    }

    /*
     * Merge return address flag.
     */
    if (definitions.chunks().front().location() == memoryLocation) {
        foreach (auto definition, definitions.chunks().front().definitions()) {
            auto definitionValue = dataflow().getValue(definition);
            if (definitionValue->isNotReturnAddress()) {
                value->makeNotReturnAddress();
            } else if (definitionValue->isReturnAddress()) {
                value->makeReturnAddress();
            }
        }
    }

    return value;
}

Value *DataflowAnalyzer::computeValue(const UnaryOperator *unary, const ReachingDefinitions &definitions) {
    auto value = dataflow().getValue(unary);
    auto operandValue = computeValue(unary->operand(), definitions);

    value->setAbstractValue(apply(unary, operandValue->abstractValue()));

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

    value->makeNotReturnAddress();

    return value;
}

Value *DataflowAnalyzer::computeValue(const BinaryOperator *binary, const ReachingDefinitions &definitions) {
    auto value = dataflow().getValue(binary);
    auto leftValue = computeValue(binary->left(), definitions);
    auto rightValue = computeValue(binary->right(), definitions);

    value->setAbstractValue(apply(binary, leftValue->abstractValue(), rightValue->abstractValue()));

    /* Compute stack offset. */
    switch (binary->operatorKind()) {
        case BinaryOperator::ADD: {
            if (leftValue->isStackOffset()) {
                if (rightValue->abstractValue().isConcrete()) {
                    value->makeStackOffset(leftValue->stackOffset() + rightValue->abstractValue().asConcrete().signedValue());
                } else if (rightValue->abstractValue().isNondeterministic()) {
                    value->makeNotStackOffset();
                }
            }
            if (rightValue->isStackOffset()) {
                if (leftValue->abstractValue().isConcrete()) {
                    value->makeStackOffset(rightValue->stackOffset() + leftValue->abstractValue().asConcrete().signedValue());
                } else if (leftValue->abstractValue().isNondeterministic()) {
                    value->makeNotStackOffset();
                }
            }
            if (leftValue->isNotStackOffset() && rightValue->isNotStackOffset()) {
                value->makeNotStackOffset();
            }
            break;
        }
        case BinaryOperator::SUB: {
            if (leftValue->isStackOffset() && rightValue->abstractValue().isConcrete()) {
                value->makeStackOffset(leftValue->stackOffset() - rightValue->abstractValue().asConcrete().signedValue());
            } else if (leftValue->isNotStackOffset() || rightValue->abstractValue().isNondeterministic()) {
                value->makeNotStackOffset();
            }
            break;
        }
        case BinaryOperator::AND: {
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
        }
        default: {
            value->makeNotStackOffset();
            break;
        }
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

    value->makeNotReturnAddress();

    return value;
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
            log_.warning(tr("%1: Unknown unary operator kind: %2.").arg(Q_FUNC_INFO).arg(unary->operatorKind()));
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
            return a.asUnsigned() >> b;
        case BinaryOperator::SAR:
            return a.asSigned() >> b;
        case BinaryOperator::ADD:
            return a + b;
        case BinaryOperator::SUB:
            return a - b;
        case BinaryOperator::MUL:
            return a * b;
        case BinaryOperator::SIGNED_DIV:
            return a.asSigned() / b;
        case BinaryOperator::SIGNED_REM:
            return a.asSigned() % b;
        case BinaryOperator::UNSIGNED_DIV:
            return a.asUnsigned() / b;
        case BinaryOperator::UNSIGNED_REM:
            return a.asUnsigned() % b;
        case BinaryOperator::EQUAL:
            return a == b;
        case BinaryOperator::SIGNED_LESS:
            return a.asSigned() < b;
        case BinaryOperator::SIGNED_LESS_OR_EQUAL:
            return a.asSigned() <= b;
        case BinaryOperator::UNSIGNED_LESS:
            return a.asUnsigned() < b;
        case BinaryOperator::UNSIGNED_LESS_OR_EQUAL:
            return a.asUnsigned() <= b;
        default:
            log_.warning(tr("%1: Unknown binary operator kind: %2.").arg(Q_FUNC_INFO).arg(binary->operatorKind()));
            return dflow::AbstractValue();
    }
}

void DataflowAnalyzer::handleWrite(const Term *term, const MemoryLocation &memoryLocation, ReachingDefinitions &definitions) {
    definitions.filterOut(
        [term](const MemoryLocation &, const Term *definition) -> bool {
            return definition == term;
        }
    );

    if (isTracked(memoryLocation)) {
        definitions.addDefinition(memoryLocation, term);
    }
}

void DataflowAnalyzer::handleKill(const MemoryLocation &memoryLocation, ReachingDefinitions &definitions) {
    if (isTracked(memoryLocation)) {
        definitions.killDefinitions(memoryLocation);
    }
}

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
