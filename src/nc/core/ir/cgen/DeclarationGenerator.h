/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <boost/noncopyable.hpp>

#include "CodeGenerator.h"

namespace nc {
namespace core {

namespace likec {
    class ArgumentDeclaration;
    class FunctionDeclaration;
    class Tree;
}

namespace ir {

class Function;
class Term;

namespace types {
    class Types;
}

namespace cgen {

/**
 * Generator of function declarations.
 */
class DeclarationGenerator: boost::noncopyable {
    CodeGenerator &parent_; ///< Parent code generator.
    const Function *function_; ///< Function under consideration.
    const types::Types *types_; ///< Reconstructed types.
    likec::FunctionDeclaration *declaration_; ///< Function's declaration.

    public:

    /**
     * Constructor.
     *
     * \param parent Parent code generator.
     * \param function Valid pointer to the function being declared.
     */
    DeclarationGenerator(CodeGenerator &parent, const Function *function);

    /**
     * Virtual destructor.
     */
    virtual ~DeclarationGenerator() {}

    /**
     * \return Parent code generator.
     */
    CodeGenerator &parent() const { return parent_; }

    /**
     * \return LikeC tree.
     */
    likec::Tree &tree() const { return parent().tree(); }

    /**
     * \return Function being translated.
     */
    const Function *function() const { return function_; }

    /**
     * \return Reconstructed types.
     */
    const types::Types &types() const { return *types_; }

    /**
     * \return Function's declaration.
     */
    likec::FunctionDeclaration *declaration() const { return declaration_; }

    /**
     * Sets function's declaration without taking ownership of it.
     *
     * \param[in] declaration Function's declaration.
     */
    void setDeclaration(likec::FunctionDeclaration *declaration);

    /**
     * Creates function's declaration and sets function's declaration to it.
     *
     * \return Valid pointer to the created declaration.
     */
    std::unique_ptr<likec::FunctionDeclaration> createDeclaration();

    /**
     * \return Type of function's return value.
     */
    const likec::Type *makeReturnType();

    /**
     * \return Whether the function has variable number of arguments.
     */
    bool variadic() const;

    /**
     * Creates a declaration of function argument for given term.
     * Declaration is automatically added to the list of formal arguments
     * of current function's declaration.
     *
     * \param[in] term Term.
     *
     * \return Created declaration of function's formal argument.
     */
    likec::ArgumentDeclaration *makeArgumentDeclaration(const Term *term);
};

} // namespace cgen
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
