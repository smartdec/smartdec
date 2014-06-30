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

#include "CodeGenerator.h"

#include <nc/common/CancellationToken.h>
#include <nc/common/Range.h>
#include <nc/common/make_unique.h>

#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/Registers.h>
#include <nc/core/image/Image.h>
#include <nc/core/image/Symbols.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Functions.h>
#include <nc/core/ir/calling/Hooks.h>
#include <nc/core/ir/calling/Signatures.h>
#include <nc/core/ir/types/Type.h>
#include <nc/core/ir/types/Types.h>
#include <nc/core/ir/vars/Variable.h>
#include <nc/core/likec/FunctionDefinition.h>
#include <nc/core/likec/StructType.h>
#include <nc/core/likec/StructTypeDeclaration.h>
#include <nc/core/likec/Tree.h>

#include "DefinitionGenerator.h"

namespace nc {
namespace core {
namespace ir {
namespace cgen {

void CodeGenerator::makeCompilationUnit() {
    tree().setPointerSize(image().architecture()->bitness());
    tree().setRoot(std::make_unique<likec::CompilationUnit>(tree()));

    foreach (const Function *function, functions().list()) {
        makeFunctionDefinition(function);
        cancellationToken().poll();
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
        ByteSize offsetValue = offset.first;
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
        ByteSize offsetValue = offset.first;
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

const likec::Type *CodeGenerator::makeVariableType(const vars::Variable *variable) {
    assert(variable != NULL);

    foreach (auto termAndLocation, variable->termsAndLocations()) {
        if (termAndLocation.location == variable->memoryLocation()) {
            return makeType(types().getType(termAndLocation.term));
        }
    }

    return tree().makeIntegerType(variable->memoryLocation().size(), true);
}

likec::VariableDeclaration *CodeGenerator::makeGlobalVariableDeclaration(const vars::Variable *variable) {
    assert(variable != NULL);
    assert(variable->isGlobal());

    if (likec::VariableDeclaration *result = nc::find(variableDeclarations_, variable)) {
        return result;
    } else {
        QString name;
        QString comment;

        if (variable->memoryLocation().domain() == MemoryDomain::MEMORY) {
            ByteAddr addr = variable->memoryLocation().addr() / CHAR_BIT;
            auto symbol = image().symbols()->find(image::Symbol::Data, addr);
            if (!symbol) {
                symbol = image().symbols()->find(image::Symbol::None, addr);
            }
            if (symbol) {
                name = likec::Tree::cleanName(symbol->name());
                if (name != symbol->name()) {
                    comment = symbol->name();
                }
            }
        }

#ifdef NC_REGISTER_VARIABLE_NAMES
        if (name.isEmpty()) {
            if (auto reg = image().architecture()->registers()->getRegister(variable->memoryLocation())) {
                name = reg->lowercaseName();
            }
        }
#endif

        if (name.isEmpty()) {
            name = QString("g%1").arg(variable->memoryLocation().addr() / CHAR_BIT, 0, 16);
        }

        auto declaration = std::make_unique<likec::VariableDeclaration>(tree(), name, makeVariableType(variable));
        declaration->setComment(comment);
        result = declaration.get();

        tree().root()->addDeclaration(std::move(declaration));
        variableDeclarations_[variable] = result;

        return result;
    }
}

likec::FunctionDeclaration *CodeGenerator::makeFunctionDeclaration(const calling::FunctionSignature *signature) {
    assert(signature != NULL);

    if (auto declaration = nc::find(signature2declaration_, signature)) {
        return declaration;
    }

    DeclarationGenerator generator(*this, signature);
    tree().root()->addDeclaration(generator.createDeclaration());
    return generator.declaration();
}

likec::FunctionDefinition *CodeGenerator::makeFunctionDefinition(const Function *function) {
    DefinitionGenerator generator(*this, function, cancellationToken());
    tree().root()->addDeclaration(generator.createDefinition());
    return generator.definition();
}

void CodeGenerator::setFunctionDeclaration(const calling::FunctionSignature *signature, likec::FunctionDeclaration *declaration) {
    assert(signature != NULL);
    assert(declaration != NULL);

    signature2declaration_[signature] = declaration;
}

} // namespace cgen
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
