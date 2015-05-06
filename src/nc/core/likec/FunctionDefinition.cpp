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

#include "FunctionDefinition.h"

#include <nc/common/Foreach.h>

#include "PrintContext.h"

namespace nc {
namespace core {
namespace likec {

void FunctionDefinition::visitChildNodes(Visitor<TreeNode> &visitor) {
    FunctionDeclaration::visitChildNodes(visitor);

    visitor(block_.get());

    foreach (const auto &label, labels_) {
        visitor(label.get());
    }
}

FunctionDefinition *FunctionDefinition::rewrite() {
    rewriteChild(block_);
    rewriteChildren(labels_);
    return this;
}

void FunctionDefinition::doPrint(PrintContext &context) const {
    printComment(context);

    context.out() << *type()->returnType() << " " << identifier() << '(';

    bool comma = false;
    foreach (const auto &argument, arguments()) {
        if (comma) {
            context.out() << ", ";
        } else {
            comma = true;
        }
        argument->print(context);
    }

    if (type()->variadic()) {
        if (comma) {
            context.out() << ", ";
        }
        context.out() << "...";
    }
    
    context.out() << ") ";
    block_->print(context);
}

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
