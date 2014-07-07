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

#include "DeclarationGenerator.h"

#include <nc/common/make_unique.h>

#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/Registers.h>
#include <nc/core/image/Image.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/calling/CallHook.h>
#include <nc/core/ir/calling/EntryHook.h>
#include <nc/core/ir/calling/FunctionSignature.h>
#include <nc/core/ir/calling/Hooks.h>
#include <nc/core/ir/calling/ReturnHook.h>
#include <nc/core/ir/calling/Signatures.h>
#include <nc/core/ir/types/Types.h>
#include <nc/core/likec/FunctionDeclaration.h>
#include <nc/core/likec/Tree.h>

namespace nc {
namespace core {
namespace ir {
namespace cgen {

DeclarationGenerator::DeclarationGenerator(CodeGenerator &parent, const calling::FunctionSignature *signature):
    parent_(parent),
    signature_(signature),
    declaration_(NULL)
{
    assert(signature != NULL);
}

void DeclarationGenerator::setDeclaration(likec::FunctionDeclaration *declaration) {
    assert(!declaration_); 
    declaration_ = declaration;
    parent().setFunctionDeclaration(signature_, declaration);
}

std::unique_ptr<likec::FunctionDeclaration> DeclarationGenerator::createDeclaration() {
    auto functionDeclaration = std::make_unique<likec::FunctionDeclaration>(tree(),
        signature()->name(), makeReturnType(), signature()->variadic());

    functionDeclaration->setComment(signature()->comment());

    setDeclaration(functionDeclaration.get());

    foreach (const auto &argument, signature()->arguments()) {
        makeArgumentDeclaration(argument.get());
    }

    return functionDeclaration;
}

const likec::Type *DeclarationGenerator::makeReturnType() {
    if (signature()->returnValue()) {
        return parent().makeType(parent().types().getType(signature()->returnValue().get()));
    }
    return tree().makeVoidType();
}

likec::ArgumentDeclaration *DeclarationGenerator::makeArgumentDeclaration(const Term *term) {
    QString name;

    if (auto access = term->asMemoryLocationAccess()) {
        if (auto reg = parent().image().architecture()->registers()->getRegister(access->memoryLocation())) {
            name = reg->lowercaseName();
        }
    }

    if (name.isEmpty()) {
        name = QString("a%1").arg(declaration()->arguments().size() + 1);
    }

    auto argumentDeclaration = std::make_unique<likec::ArgumentDeclaration>(tree(),
        name, parent().makeType(parent().types().getType(term)));
    auto result = argumentDeclaration.get();

    declaration()->addArgument(std::move(argumentDeclaration));

    return result;
}

} // namespace cgen
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
