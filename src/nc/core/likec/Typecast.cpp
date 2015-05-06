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

#include "Typecast.h"

#include <nc/common/make_unique.h>

#include "BinaryOperator.h"
#include "IntegerConstant.h"
#include "PrintContext.h"
#include "MemberAccessOperator.h"
#include "StructType.h"
#include "UnaryOperator.h"
#include "Types.h"
#include "Tree.h"

namespace nc {
namespace core {
namespace likec {

void Typecast::visitChildNodes(Visitor<TreeNode> &visitor) {
    visitor(operand());
}

Expression *Typecast::rewrite() {
    rewriteChild(operand_);

    /* Convert cast of pointer to a structure to a cast of pointer to its first field. */
    if (type_->isPointer() && !type_->isStructurePointer()) {
        if (const PointerType *pointerType = operand()->getType()->as<PointerType>()) {
            if (const StructType *structType = pointerType->pointeeType()->as<StructType>()) {
                if (const MemberDeclaration *member = structType->getMember(0)) {
                    setOperand(std::make_unique<UnaryOperator>(tree(), UnaryOperator::REFERENCE,
                        std::make_unique<MemberAccessOperator>(tree(), MemberAccessOperator::ARROW, releaseOperand(), member)));
                }
            }
        }
    }

    /*
     * (int32_t*)(int64_t)expr -> (int32_t*)expr
     */
    if (type_->isScalar()) {
        if (Typecast *typecast = operand()->as<Typecast>()) {
            const Type *operandType = typecast->operand()->getType();

            if (typecast->type()->isScalar() &&
                operandType->isScalar() &&
                type_->size() == typecast->type()->size() &&
                typecast->type()->size() == operandType->size())
            {
                setOperand(typecast->releaseOperand());
            }
        }
    }

    /* This really must be the last rule. */
    if (type_ == operand()->getType()) {
        return releaseOperand().release();
    }

    return this;
}

void Typecast::doPrint(PrintContext &context) const {
    context.out() << '(' << *type() << ')';
    bool braces = operand()->is<BinaryOperator>();
    if (braces) {
        context.out() << '(';
    }
    operand()->print(context);
    if (braces) {
        context.out() << ')';
    }
}

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
