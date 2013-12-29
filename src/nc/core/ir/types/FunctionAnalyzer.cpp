/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "FunctionAnalyzer.h"

#include <algorithm>

#include <nc/common/Foreach.h>
#include <nc/common/Unreachable.h>

#include <nc/core/ir/Terms.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/Value.h>
#include <nc/core/ir/liveness/Liveness.h>
#include <nc/core/ir/misc/CensusVisitor.h>

#include "Type.h"
#include "Types.h"

namespace nc {
namespace core {
namespace ir {
namespace types {

FunctionAnalyzer::FunctionAnalyzer(Types &types, const Function *function, const dflow::Dataflow &dataflow,
    const liveness::Liveness &liveness, calling::Hooks &hooks
):
    types_(types), dataflow_(dataflow)
{
    assert(function != NULL);

    /*
     * We want to keep the natural ordering of terms in function's code.
     * Iterative process converges much faster then.
     * This is why we don't just take liveness().liveTerms_.
     */
    ir::misc::CensusVisitor census(&hooks);
    census(function);

    terms_.swap(census.terms());

    terms_.erase(
        std::remove_if(terms_.begin(), terms_.end(), [&](const Term *term) { return !liveness.isLive(term); }),
        terms_.end());

    /*
     * Unite types of arguments of left and right hand sides of assignments.
     */
    foreach (const Term *term, terms_) {
        if (term->source()) {
            types_.getType(term)->unionSet(types_.getType(term->source()));
        }
    }
}

bool FunctionAnalyzer::analyze() {
    /*
     * Going in both directions makes the process
     * converge much faster on some examples.
     */
    foreach (const Term *term, terms_) {
        analyze(term);
    }
    reverse_foreach (const Term *term, terms_) {
        analyze(term);
    }

    bool changed = false;
    foreach (auto &termAndType, types_.map()) {
        if (termAndType.second->changed()) {
            changed = true;
        }
    }
    return changed;
}

void FunctionAnalyzer::analyze(const Term *term) {
    switch (term->kind()) {
        case Term::INT_CONST: /* FALLTHROUGH */
        case Term::INTRINSIC: /* FALLTHROUGH */
        case Term::UNDEFINED: /* FALLTHROUGH */
        case Term::MEMORY_LOCATION_ACCESS:
            break;
        case Term::DEREFERENCE: {
            auto dereference = term->asDereference();
            types_.getType(dereference->address())->makePointer(types_.getType(dereference));
            break;
        }
        case Term::UNARY_OPERATOR:
            analyze(term->asUnaryOperator());
            break;
        case Term::BINARY_OPERATOR:
            analyze(term->asBinaryOperator());
            break;
        case Term::CHOICE:
            break;
        default:
            unreachable();
            break;
    }
}

void FunctionAnalyzer::analyze(const UnaryOperator *unary) {
    Type *type = types_.getType(unary);
    Type *operandType = types_.getType(unary->operand());

    switch (unary->operatorKind()) {
        case UnaryOperator::NOT: /* FALLTHROUGH */
            operandType->makeInteger();
            type->makeInteger();
            break;
        case UnaryOperator::NEGATION:
            operandType->makeInteger();
            type->makeInteger();
            operandType->makeSigned();
            type->makeSigned();
            break;
        case UnaryOperator::SIGN_EXTEND:
            operandType->makeSigned();
            break;
        case UnaryOperator::ZERO_EXTEND:
            if (operandType->isSigned()) {
                type->makeUnsigned();
            }
            break;
        case UnaryOperator::TRUNCATE:
            break;
        default:
            unreachable();
            break;
    }
}

void FunctionAnalyzer::analyze(const BinaryOperator *binary) {
    /* Be careful: these pointers become invalid after the first call to unionSet(). */
    Type *type = types_.getType(binary);
    Type *leftType = types_.getType(binary->left());
    Type *rightType = types_.getType(binary->right());

    const dflow::Value *value = dataflow_.getValue(binary->left());
    const dflow::Value *leftValue = dataflow_.getValue(binary->left());
    const dflow::Value *rightValue = dataflow_.getValue(binary->right());

    switch (binary->operatorKind()) {
        case BinaryOperator::AND: /* FALLTHROUGH */
        case BinaryOperator::OR: /* FALLTHROUGH */
        case BinaryOperator::XOR:
            leftType->makeInteger();
            rightType->makeInteger();
            type->makeInteger();

            leftType->makeUnsigned();
            rightType->makeUnsigned();
            type->makeUnsigned();
            break;

        case BinaryOperator::SHL:
            leftType->makeInteger();
            rightType->makeInteger();
            type->makeInteger();

            rightType->makeUnsigned();
            if (leftType->isSigned()) {
                type->makeSigned();
            }
            if (leftType->isUnsigned()) {
                type->makeUnsigned();
            }
            if (type->isSigned()) {
                leftType->makeSigned();
            }
            if (type->isUnsigned()) {
                leftType->makeUnsigned();
            }

            if (rightValue->abstractValue().isConcrete()) {
                type->updateFactor(leftType->factor() *
                    shiftLeft<ConstantValue>(1, rightValue->abstractValue().asConcrete().value()));
            }
            break;

        case BinaryOperator::SHR:
            leftType->makeInteger();
            rightType->makeInteger();
            type->makeInteger();

            leftType->makeUnsigned();
            type->makeUnsigned();
            break;

        case BinaryOperator::SAR:
            leftType->makeInteger();
            rightType->makeInteger();
            type->makeInteger();

            leftType->makeSigned();
            type->makeSigned();
            break;

        case BinaryOperator::ADD:
            if (leftType->isInteger() && rightType->isInteger()) {
                type->makeInteger();
            }
            if ((leftType->isInteger() && rightType->isPointer()) ||
                (leftType->isPointer() && rightType->isInteger())) {
                type->makePointer();
            }
            if (type->isInteger()) {
                leftType->makeInteger();
                rightType->makeInteger();
            }
            if (type->isPointer()) {
                if (leftType->isInteger()) {
                    rightType->makePointer();
                }
                if (rightType->isInteger()) {
                    leftType->makePointer();
                }
                if (leftType->isPointer()) {
                    rightType->makeInteger();
                }
                if (rightType->isPointer()) {
                    leftType->makeInteger();
                }
                if (!leftType->isPointer() && !rightType->isPointer()) {
                    if (leftValue->isProduct()) {
                        rightType->makePointer();
                    } else if (rightValue->isProduct()) {
                        leftType->makePointer();
                    } else if (leftValue->abstractValue().isConcrete()) {
                        if (leftValue->abstractValue().asConcrete().value() < 4096) {
                            leftType->makeInteger();
                        } else {
                            leftType->makePointer();
                        }
                    } else if (rightValue->abstractValue().isConcrete()) {
                        if (rightValue->abstractValue().asConcrete().value() < 4096) {
                            rightType->makeInteger();
                        } else {
                            rightType->makePointer();
                        }
                    }
                }
            }

            if (leftType->isUnsigned() || rightType->isUnsigned()) {
                type->makeUnsigned();
            }
            if (leftType->isSigned() && rightType->isSigned()) {
                type->makeSigned();
            }
            if (type->isSigned()) {
                leftType->makeSigned();
                rightType->makeSigned();
            }
            if (type->isUnsigned()) {
                if (leftType->isSigned()) {
                    rightType->makeUnsigned();
                }
                if (rightType->isSigned()) {
                    rightType->makeUnsigned();
                }
            }

            if (rightValue->abstractValue().isConcrete()) {
                if (type == leftType) {
                    type->updateFactor(rightValue->abstractValue().asConcrete().absoluteValue());
                } else {
#ifdef NC_STRUCT_RECOVERY
                    if (!value->isStackOffset()) {
                        leftType->addOffset(rightValue->abstractValue().asConcrete().signedValue(), type);
                    }
#endif
                }
            }
            if (leftValue->abstractValue().isConcrete()) {
                if (type == rightType) {
                    type->updateFactor(leftValue->abstractValue().asConcrete().absoluteValue());
                } else {
#ifdef NC_STRUCT_RECOVERY
                    if (!value->isStackOffset()) {
                        rightType->addOffset(leftValue->abstractValue().asConcrete().signedValue(), type);
                    }
#endif
                }
            }

            if (leftType->isPointer() && rightValue->isProduct()) {
                type->makePointer(leftType->pointee());
            }
            if (rightType->isPointer() && leftValue->isProduct()) {
                type->makePointer(rightType->pointee());
            }
            break;

        case BinaryOperator::SUB:
            if (leftType->isInteger() && rightType->isInteger()) {
                type->makeInteger();
            }
            if (leftType->isPointer() && rightType->isInteger()) {
                type->makePointer();
            }
            if (type->isPointer()) {
                leftType->makePointer();
                rightType->makeInteger();
            }

            if (leftType->isUnsigned() || rightType->isUnsigned()) {
                type->makeUnsigned();
            }
            if (leftType->isSigned() && rightType->isSigned()) {
                type->makeSigned();
            }
            if (type->isSigned()) {
                leftType->makeSigned();
                rightType->makeSigned();
            }
            if (type->isUnsigned()) {
                if (leftType->isSigned()) {
                    rightType->makeUnsigned();
                }
                if (rightType->isSigned()) {
                    rightType->makeUnsigned();
                }
            }

            if (rightValue->abstractValue().isConcrete()) {
                if (type == leftType) {
                    type->updateFactor(rightValue->abstractValue().asConcrete().absoluteValue());
                } else {
#ifdef NC_STRUCT_RECOVERY
                    if (!value->isStackOffset()) {
                        leftType->addOffset(-rightValue->abstractValue().asConcrete().signedValue(), type);
                    }
#endif
                }
            }

            if (leftType->isPointer() && rightValue->isProduct()) {
                type->makePointer(leftType->pointee());
            }
            break;

        case BinaryOperator::MUL:
            type->makeInteger();
            leftType->makeInteger();
            rightType->makeInteger();

            if (leftType->isUnsigned() || rightType->isUnsigned()) {
                type->makeUnsigned();
            }
            if (leftType->isSigned() && rightType->isSigned()) {
                type->makeSigned();
            }
            if (type->isSigned()) {
                leftType->makeSigned();
                rightType->makeSigned();
            }
            if (type->isUnsigned()) {
                if (leftType->isSigned()) {
                    rightType->makeUnsigned();
                }
                if (rightType->isSigned()) {
                    rightType->makeUnsigned();
                }
            }

            if (rightValue->abstractValue().isConcrete()) {
                type->updateFactor(leftType->factor() * rightValue->abstractValue().asConcrete().value());
            }
            if (leftValue->abstractValue().isConcrete()) {
                type->updateFactor(rightType->factor() * leftValue->abstractValue().asConcrete().value());
            }
            break;

        case BinaryOperator::SIGNED_DIV:
            leftType->makeInteger();
            rightType->makeInteger();
            type->makeInteger();

            leftType->makeSigned();
            rightType->makeSigned();
            type->makeSigned();
            break;

        case BinaryOperator::SIGNED_REM:
            leftType->makeInteger();
            rightType->makeInteger();
            type->makeInteger();

            leftType->makeSigned();
            rightType->makeSigned();
            type->makeSigned();
            break;

        case BinaryOperator::UNSIGNED_DIV:
            type->makeInteger();
            leftType->makeInteger();
            rightType->makeInteger();

            if (leftType->isSigned()) {
                rightType->makeUnsigned();
            }
            if (rightType->isUnsigned()) {
                leftType->makeSigned();
            }
            type->makeUnsigned();
            break;

        case BinaryOperator::UNSIGNED_REM:
            type->makeInteger();
            leftType->makeInteger();
            rightType->makeInteger();

            if (leftType->isSigned()) {
                rightType->makeUnsigned();
            }
            if (rightType->isUnsigned()) {
                leftType->makeSigned();
            }
            type->makeUnsigned();
            break;

        case BinaryOperator::EQUAL:
            leftType->unionSet(rightType);
            break;

        case BinaryOperator::SIGNED_LESS: /* FALLTHROUGH */
        case BinaryOperator::SIGNED_LESS_OR_EQUAL: /* FALLTHROUGH */
            leftType->makeSigned();
            rightType->makeSigned();
            leftType->unionSet(rightType);
            break;

        case BinaryOperator::UNSIGNED_LESS: /* FALLTHROUGH */
        case BinaryOperator::UNSIGNED_LESS_OR_EQUAL: /* FALLTHROUGH */
            if (rightType->isSigned()) {
                leftType->makeUnsigned();
            } else if (leftType->isSigned()) {
                rightType->makeUnsigned();
            } else {
                leftType->makeUnsigned();
                rightType->makeUnsigned();
            }
            leftType->unionSet(rightType);
            break;

        default:
            unreachable();
            break;
    }
}

}}}} // namespace nc::core::ir::types

/* vim:set et sts=4 sw=4: */
