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

#include <nc/core/Context.h>
#include <nc/core/Module.h>
#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/Registers.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/calls/CallsData.h>
#include <nc/core/ir/calls/FunctionAnalyzer.h>
#include <nc/core/ir/calls/FunctionSignature.h>
#include <nc/core/ir/calls/ReturnAnalyzer.h>
#include <nc/core/ir/types/Types.h>
#include <nc/core/likec/Tree.h>
#include <nc/core/likec/FunctionDeclaration.h>

namespace nc {
namespace core {
namespace ir {
namespace cgen {

DeclarationGenerator::DeclarationGenerator(CodeGenerator &parent, const Function *function):
    parent_(parent),
    function_(function),
    types_(parent.context().getTypes(function)),
    declaration_(NULL)
{}

void DeclarationGenerator::setDeclaration(likec::FunctionDeclaration *declaration) {
    assert(!declaration_); 
    declaration_ = declaration;
}

std::unique_ptr<likec::FunctionDeclaration> DeclarationGenerator::createDeclaration() {
    auto functionDeclaration = std::make_unique<likec::FunctionDeclaration>(tree(),
        function()->name(), makeReturnType(), variadic());

    functionDeclaration->setComment(function()->comment().text());

    setDeclaration(functionDeclaration.get());

    if (const calls::FunctionSignature *signature = parent().context().callsData()->getFunctionSignature(function())) {
        if (calls::FunctionAnalyzer *functionAnalyzer = parent().context().callsData()->getFunctionAnalyzer(function())) {
            foreach (const MemoryLocation &memoryLocation, signature->arguments()) {
                makeArgumentDeclaration(functionAnalyzer->getArgumentTerm(memoryLocation));
            }
        }
    }

    return functionDeclaration;
}

const likec::Type *DeclarationGenerator::makeReturnType() {
    if (const calls::FunctionSignature *signature = parent().context().callsData()->getFunctionSignature(function())) {
        if (signature->returnValue()) {
            foreach (const Return *ret, parent().context().callsData()->getReturns(function())) {
                if (calls::ReturnAnalyzer *returnAnalyzer = parent().context().callsData()->getReturnAnalyzer(function(), ret)) {
                    return parent().makeType(types().getType(returnAnalyzer->getReturnValueTerm(signature->returnValue())));
                }
            }
        }
    }
    return tree().makeVoidType();
}

bool DeclarationGenerator::variadic() const {
    if (const calls::FunctionSignature *signature = parent().context().callsData()->getFunctionSignature(function())) {
        return signature->variadic();
    } else {
        return false;
    }
}

likec::ArgumentDeclaration *DeclarationGenerator::makeArgumentDeclaration(const Term *term) {
    QString name;

#ifdef NC_REGISTER_VARIABLE_NAMES
    if (term->kind() == Term::MEMORY_LOCATION_ACCESS) {
        if (const arch::Register *regizter = parent().context().module()->architecture()->registers()->regizter(term->asMemoryLocationAccess()->memoryLocation())) {
            name = regizter->lowercaseName();
        }
    }
#endif

    if (name.isEmpty()) {
        name = QString("arg%1").arg(declaration()->arguments().size() + 1);
    }

    likec::ArgumentDeclaration *argumentDeclaration = new likec::ArgumentDeclaration(
        tree(), name, parent().makeType(types().getType(term)));

    declaration()->addArgument(argumentDeclaration);

    return argumentDeclaration;
}

} // namespace cgen
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
