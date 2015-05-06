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

#include "TypeAnalyzer.h"

#include <nc/common/CancellationToken.h>
#include <nc/common/Foreach.h>
#include <nc/common/Unreachable.h>
#include <nc/common/Warnings.h>

#include <nc/core/ir/Function.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/calls/CallsData.h>
#include <nc/core/ir/calls/FunctionSignature.h>
#include <nc/core/ir/calls/ReturnAnalyzer.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/Value.h>
#include <nc/core/ir/misc/CensusVisitor.h>
#include <nc/core/ir/usage/Usage.h>

#include "Type.h"
#include "Types.h"

namespace nc {
namespace core {
namespace ir {
namespace types {

void TypeAnalyzer::analyze(const Function *function, const CancellationToken &canceled) {
    ir::misc::CensusVisitor census(callsData());
    census(function);

    /* Join term types with types of definitions. */
    foreach (const Term *term, census.terms()) {
        if (term->isRead()) {
            foreach (const Term *definition, dataflow().getDefinitions(term)) {
                types().getType(term)->unionSet(types().getType(definition));
            }
        }
    } 

    /* Join types of terms used for return values. */
    if (callsData()) {
        if (const calls::FunctionSignature *signature = callsData()->getFunctionSignature(function)) {
            if (signature->returnValue()) {
                const Term *firstReturnTerm = NULL;
                foreach (const Return *ret, callsData()->getReturns(function)) {
                    if (calls::ReturnAnalyzer *returnAnalyzer = callsData()->getReturnAnalyzer(function, ret)) {
                        const Term *returnTerm = returnAnalyzer->getReturnValueTerm(signature->returnValue());
                        if (firstReturnTerm == NULL) {
                            firstReturnTerm = returnTerm;
                        } else {
                            types().getType(firstReturnTerm)->unionSet(types().getType(returnTerm));
                        }
                    }
                }
            }
        }
    }

    /*
     * We want to keep the natural ordering of terms in function's code.
     * Iterative process converges much faster then.
     * This is why we don't just take usage().usedTerms_.
     */
    std::vector<const Term *> terms(census.terms().begin(), census.terms().end());
    terms.erase(std::remove_if(terms.begin(), terms.end(),
        [this](const Term *term) { return !this->usage().isUsed(term); }), terms.end());

    bool changed;
    do {
        foreach (const Term *term, terms) {
            analyze(term);
        }
        reverse_foreach (const Term *term, terms) {
            analyze(term);
        }
        foreach (const Statement *statement, census.statements()) {
            analyze(statement);
        }
        reverse_foreach (const Statement *statement, census.statements()) {
            analyze(statement);
        }

        changed = false;
        foreach (auto &item, types().types()) {
            if (item.second->changed()) {
                changed = true;
            }
        }
    } while (changed && !canceled);
}

void TypeAnalyzer::analyze(const Term *term) {
    switch (term->kind()) {
        case Term::INT_CONST:
            analyze(term->asConstant());
            break;
        case Term::INTRINSIC:
            break;
        case Term::UNDEFINED:
            break;
        case Term::MEMORY_LOCATION_ACCESS:
            break;
        case Term::DEREFERENCE:
            analyze(term->asDereference());
            break;
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

void TypeAnalyzer::analyze(const Constant * /*constant*/) {
    /* Nothing to do. */
}

void TypeAnalyzer::analyze(const Dereference *dereference) {
    types().getType(dereference->address())->makePointer(types().getType(dereference));
}

void TypeAnalyzer::analyze(const UnaryOperator *unary) {
    Type *type = types().getType(unary);
    Type *operandType = types().getType(unary->operand());

    switch (unary->operatorKind()) {
        case UnaryOperator::BITWISE_NOT: /* FALLTHROUGH */
        case UnaryOperator::LOGICAL_NOT:
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
        case UnaryOperator::RESIZE:
            break;
        default:
            unreachable();
            break;
    }
}

void TypeAnalyzer::analyze(const BinaryOperator *binary) {
    /* Be careful: these pointers become kinda invalid after doing unionSet(). */
    Type *type = types().getType(binary);
    Type *leftType = types().getType(binary->left());
    Type *rightType = types().getType(binary->right());

    const dflow::Value *value = dataflow().getValue(binary->left());
    const dflow::Value *leftValue = dataflow().getValue(binary->left());
    const dflow::Value *rightValue = dataflow().getValue(binary->right());

    switch (binary->operatorKind()) {
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
                    if (leftValue->isMultiplication()) {
                        rightType->makePointer();
                    } else if (rightValue->isMultiplication()) {
                        leftType->makePointer();
                    } else if (leftValue->isConstant()) {
                        if (leftValue->constantValue().value() < 4096) {
                            leftType->makeInteger();
                        } else {
                            leftType->makePointer();
                        }
                    } else if (rightValue->isConstant()) {
                        if (rightValue->constantValue().value() < 4096) {
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

            if (rightValue->isConstant()) {
                if (type == leftType) {
                    type->updateFactor(rightValue->constantValue().absoluteValue());
                } else {
#ifdef NC_STRUCT_RECOVERY
                    if (!value->isStackOffset()) {
                        leftType->addOffset(rightValue->constantValue().signedValue(), type);
                    }
#endif
                }
            }
            if (leftValue->isConstant()) {
                if (type == rightType) {
                    type->updateFactor(leftValue->constantValue().absoluteValue());
                } else {
#ifdef NC_STRUCT_RECOVERY
                    if (!value->isStackOffset()) {
                        rightType->addOffset(leftValue->constantValue().signedValue(), type);
                    }
#endif
                }
            }

            if (leftType->isPointer() && rightValue->isMultiplication()) {
                type->makePointer(leftType->pointee());
            }
            if (rightType->isPointer() && leftValue->isMultiplication()) {
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

            if (rightValue->isConstant()) {
                if (type == leftType) {
                    type->updateFactor(rightValue->constantValue().absoluteValue());
                } else {
#ifdef NC_STRUCT_RECOVERY
                    if (!value->isStackOffset()) {
                        leftType->addOffset(-rightValue->constantValue().signedValue(), type);
                    }
#endif
                }
            }

            if (leftType->isPointer() && rightValue->isMultiplication()) {
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

            if (rightValue->isConstant()) {
                type->updateFactor(leftType->factor() * rightValue->constantValue().value());
            }
            if (leftValue->isConstant()) {
                type->updateFactor(rightType->factor() * leftValue->constantValue().value());
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

        case BinaryOperator::SIGNED_REM:
            leftType->makeInteger();
            rightType->makeInteger();
            type->makeInteger();

            leftType->makeSigned();
            rightType->makeSigned();
            type->makeSigned();
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

        case BinaryOperator::BITWISE_AND: /* FALLTHROUGH */
        case BinaryOperator::LOGICAL_AND: /* FALLTHROUGH */
        case BinaryOperator::BITWISE_OR: /* FALLTHROUGH */
        case BinaryOperator::LOGICAL_OR: /* FALLTHROUGH */
        case BinaryOperator::BITWISE_XOR:
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

            if (rightValue->isConstant()) {
                type->updateFactor(leftType->factor() * (ConstantValue(1) << rightValue->constantValue().value()));
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

        case BinaryOperator::EQUAL:
            leftType->unionSet(rightType);
            break;

        case BinaryOperator::SIGNED_LESS: /* FALLTHROUGH */
        case BinaryOperator::SIGNED_LESS_OR_EQUAL: /* FALLTHROUGH */
        case BinaryOperator::SIGNED_GREATER: /* FALLTHROUGH */
        case BinaryOperator::SIGNED_GREATER_OR_EQUAL:
            leftType->makeSigned();
            rightType->makeSigned();
            leftType->unionSet(rightType);
            break;

        case BinaryOperator::UNSIGNED_LESS: /* FALLTHROUGH */
        case BinaryOperator::UNSIGNED_LESS_OR_EQUAL: /* FALLTHROUGH */
        case BinaryOperator::UNSIGNED_GREATER: /* FALLTHROUGH */
        case BinaryOperator::UNSIGNED_GREATER_OR_EQUAL:
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

void TypeAnalyzer::analyze(const Statement *statement) {
    switch (statement->kind()) {
        case Statement::COMMENT:
            break;
        case Statement::INLINE_ASSEMBLY:
            break;
        case Statement::ASSIGNMENT: {
            const Assignment *assignment = statement->asAssignment();
            types().getType(assignment->left())->unionSet(types().getType(assignment->right()));
            break;
        }
        case Statement::KILL:  /* FALLTHROUGH */
        case Statement::JUMP: /* FALLTHROUGH */
        case Statement::CALL: /* FALLTHROUGH */
        case Statement::RETURN: /* FALLTHROUGH */
            break;
        default:
            ncWarning("Was called for unsupported kind of statement.");
            break;
    }
}

}}}} // namespace nc::core::ir::types

/* vim:set et sts=4 sw=4: */
