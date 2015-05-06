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

#include "CodeGenerator.h"

#include <nc/core/Module.h>
#include <nc/core/Context.h>
#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/Registers.h>

#include <nc/common/CancellationToken.h>
#include <nc/common/make_unique.h>

#include <nc/core/ir/Function.h>
#include <nc/core/ir/Functions.h>
#include <nc/core/ir/calls/CallsData.h>
#include <nc/core/ir/types/Type.h>

#include <nc/core/likec/FunctionDefinition.h>
#include <nc/core/likec/StructType.h>
#include <nc/core/likec/StructTypeDeclaration.h>
#include <nc/core/likec/Tree.h>

#include "DefinitionGenerator.h"

namespace nc {
namespace core {
namespace ir {
namespace cgen {

void CodeGenerator::makeCompilationUnit(const CancellationToken &canceled) {
    ir::Functions *functions = context().functions();

    tree().setPointerSize(context().module()->architecture()->bitness());
    tree().setRoot(std::make_unique<likec::CompilationUnit>(tree()));
    tree().root()->setComment(functions->comment().text());

    foreach (const Function *function, functions->functions()) {
        if (canceled) {
            break;
        }
        makeFunctionDefinition(function);
    }

    tree().rewriteRoot();
}

const likec::Type *CodeGenerator::makeType(const types::Type *typeTraits) {
    assert(!typeTraits || typeTraits->findSet() == typeTraits);

    if (!typeTraits) {
        return tree().makeVoidType();
    } else if (typeTraits->isPointer()) {
        if (std::find(typeCreationStack_.begin(), typeCreationStack_.end(), typeTraits) != typeCreationStack_.end()) {
            /* Circular dependency. */
            return tree().makePointerType(typeTraits->size(), tree().makeVoidType());
#ifdef NC_STRUCT_RECOVERY
        } else if (const likec::Type *structuralType = makeStructuralType(typeTraits)) {
            return tree().makePointerType(typeTraits->size(), structuralType);
#endif
        } else {
            typeCreationStack_.push_back(typeTraits);
            const likec::Type *pointee = makeType(typeTraits->pointee());
            typeCreationStack_.pop_back();

            return tree().makePointerType(typeTraits->size(), pointee);
        }
    } else if (typeTraits->isFloat()) {
        return tree().makeFloatType(typeTraits->size());
    } else {
        return tree().makeIntegerType(typeTraits->size(), typeTraits->isUnsigned());
    }
}

#ifdef NC_STRUCT_RECOVERY
const likec::StructType *CodeGenerator::makeStructuralType(const types::Type *typeTraits) {
    assert(typeTraits->findSet() == typeTraits);

    if (!typeTraits->isPointer()) {
        return NULL;
    }

    if (typeTraits->offsets().size() < 2) {
        return NULL;
    }

    auto i = traits2structType_.find(typeTraits);
    if (i != traits2structType_.end()) {
        return i->second;
    }

    bool isStruct = false;
    foreach (auto offset, typeTraits->offsets()) {
        ByteOffset offsetValue = offset.first;
        const types::Type *offsetType = offset.second->findSet();

        if (offsetValue > 0 && offsetType == typeTraits) {
            break;
        }

        if (offsetValue > 0 && offsetType->pointee() && offsetType->pointee()->size()) {
            isStruct = true;
            break;
        }
    }
    if (!isStruct) {
        return NULL;
    }

    auto typeDeclaration = std::make_unique<likec::StructTypeDeclaration>(tree_, QString("s%1").arg(traits2structType_.size()));

    likec::StructType *type = typeDeclaration->type();
    traits2structType_[typeTraits] = type;

    foreach (auto offset, typeTraits->offsets()) {
        ByteOffset offsetValue = offset.first;
        const types::Type *offsetType = offset.second->findSet();

        if (offsetValue > 0 && offsetType == typeTraits) {
            break;
        }

        if (offsetValue >= 0 && offsetType->pointee() && offsetType->pointee()->size()) {
            if (offsetValue > type->size() / CHAR_BIT) {
                typeDeclaration->type()->addMember(new likec::MemberDeclaration(
                    tree_, QString("pad%1").arg(offsetValue),
                    tree_.makeArrayType(tree_.makeIntegerType(CHAR_BIT, false), offsetValue - type->size() / CHAR_BIT)));
            }
            typeDeclaration->type()->addMember(new likec::MemberDeclaration(
                tree_, QString("f%1").arg(offsetValue), makeType(offsetType->pointee())));
        }
    }

    tree().root()->addDeclaration(std::move(typeDeclaration));

    return type;
}
#endif

likec::VariableDeclaration *CodeGenerator::makeGlobalVariableDeclaration(const MemoryLocation &memoryLocation, const types::Type *type) {
    if (likec::VariableDeclaration *result = nc::find(variableDeclarations_, memoryLocation)) {
        return result;
    } else {
        QString name;
        QString comment;

        if (memoryLocation.domain() == MemoryDomain::MEMORY) {
            ByteAddr addr = memoryLocation.addr() / CHAR_BIT;
            comment = context().module()->getName(addr);
            if (!comment.isEmpty()) {
                name = likec::Tree::cleanName(context().module()->getName(addr));
                if (name == comment) {
                    comment = QString();
                }
            }
        }

#ifdef NC_REGISTER_VARIABLE_NAMES
        if (name.isEmpty()) {
            if (const arch::Register *reg = context().module()->architecture()->registers()->regizter(memoryLocation)) {
                name = reg->lowercaseName();
            }
        }
#endif

        if (name.isEmpty()) {
            name = QString("g%1").arg(++serial_);
        }

        auto declaration = std::make_unique<likec::VariableDeclaration>(tree(), name, makeType(type));
        declaration->setComment(comment);
        result = declaration.get();

        tree().root()->addDeclaration(std::move(declaration));
        variableDeclarations_[memoryLocation] = result;

        return result;
    }
}

likec::FunctionDeclaration *CodeGenerator::makeFunctionDeclaration(const Function *function) {
    if (likec::FunctionDeclaration *result = nc::find(function2declaration_, function)) {
        return result;
    } else {
        DeclarationGenerator generator(*this, function);

        tree().root()->addDeclaration(generator.createDeclaration());
        setFunctionDeclaration(function, generator.declaration());

        return generator.declaration();
    }
}

// TODO: make this function succeed even when there are no Function objects associated with the address.
likec::FunctionDeclaration *CodeGenerator::makeFunctionDeclaration(ByteAddr addr) {
    foreach (const Function *function, context().functions()->getFunctionsAtAddress(addr)) {
        return makeFunctionDeclaration(function);
    }
    return NULL;
}

likec::FunctionDefinition *CodeGenerator::makeFunctionDefinition(const Function *function) {
    DefinitionGenerator generator(*this, function);

    tree().root()->addDeclaration(generator.createDefinition());
    setFunctionDeclaration(function, generator.definition());

    return generator.definition();
}

void CodeGenerator::setFunctionDeclaration(const Function *function, likec::FunctionDeclaration *declaration) {
    function2declaration_[function] = declaration;
}

} // namespace cgen
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
