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

#include "FunctionDeclaration.h"

#include <nc/common/Foreach.h>

#include "Tree.h"
#include "PrintContext.h"

namespace nc {
namespace core {
namespace likec {

FunctionDeclaration::FunctionDeclaration(Tree &tree, const QString &identifier, const Type *returnType, bool variadic):
    Declaration(tree, FUNCTION_DECLARATION, identifier),
    type_(new FunctionPointerType(tree.pointerSize(), returnType, variadic))
{}

FunctionDeclaration::FunctionDeclaration(Tree &tree, int declarationKind, const QString &identifier, const Type *returnType, bool variadic):
    Declaration(tree, declarationKind, identifier),
    type_(new FunctionPointerType(tree.pointerSize(), returnType, variadic))
{}

void FunctionDeclaration::addArgument(ArgumentDeclaration *argument) {
    arguments_.push_back(std::unique_ptr<ArgumentDeclaration>(argument));
    type_->addArgumentType(argument->type());
}

void FunctionDeclaration::visitChildNodes(Visitor<TreeNode> &visitor) {
    Declaration::visitChildNodes(visitor);

    foreach (const auto &argument, arguments_) {
        visitor(argument.get());
    }
}

void FunctionDeclaration::doPrint(PrintContext &context) const {
    printComment(context);

    context.out() << *type()->returnType() << ' ' << identifier() << '(';

    bool comma = false;
    foreach (const auto &argument, arguments_) {
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
    
    context.out() << ");";
}

} /* namespace likec */
} // namespace core
} /* namespace nc */

/* vim:set et sts=4 sw=4: */
