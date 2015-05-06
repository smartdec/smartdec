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

#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>

#include <nc/core/ir/MemoryLocation.h>

namespace nc {

class CancellationToken;

namespace core {

class Context;

namespace likec {
    class FunctionDeclaration;
    class FunctionDefinition;
    class StructType;
    class Tree;
    class Type;
    class VariableDeclaration;
}

namespace ir {

class Function;
class Term;

namespace types {
    class Type;
}

namespace cgen {

/**
 * LikeC code generator.
 */
class CodeGenerator: boost::noncopyable {
    /** Context with the analyzed program. */
    core::Context &context_;

    /** Abstract syntax tree to generate code in. */
    likec::Tree &tree_;

    /** Types being translated to LikeC. */
    std::vector<const ir::types::Type *> typeCreationStack_;

    /** Structural types generated for IR types. */
    boost::unordered_map<const ir::types::Type *, const likec::StructType *> traits2structType_;

    /** Serial number for giving names to global variables. */
    int serial_;

    /** Already declared global variables. */
    boost::unordered_map<MemoryLocation, likec::VariableDeclaration *> variableDeclarations_;

    /** Mapping of functions to their declarations. */
    boost::unordered_map<const Function *, likec::FunctionDeclaration *> function2declaration_;

public:

    /*
     * Constructor.
     *
     * \param context Context with the analyzed program.
     * \param tree Abstract syntax tree to generate code in.
     */
    CodeGenerator(core::Context &context, likec::Tree &tree):
        context_(context), tree_(tree), serial_(0)
    {}

    /**
     * Virtual destructor.
     */
    virtual ~CodeGenerator() {}

    /**
     * \return Context with the analyzer program.
     */
    core::Context &context() const { return context_; }

    /**
     * \return Abstract syntax tree to generate code in.
     */
    likec::Tree &tree() const { return tree_; }

    /**
     * Translates input program into LikeC compilation unit.
     *
     * \param[in] canceled Cancellation token.
     */
    void makeCompilationUnit(const CancellationToken &canceled);

    /**
     * Creates high-level type object from given type traits.
     *
     * \param typeTraits Type traits.
     */
    const likec::Type *makeType(const types::Type *typeTraits);

#ifdef NC_STRUCT_RECOVERY
    /**
     * Creates high-level description of struct type from given type traits of a pointer to such struct.
     *
     * \param typeTraits Type traits.
     */
    const likec::StructType *makeStructuralType(const types::Type *typeTraits);
#endif

    /**
     * Returns a declaration of global variable for given term.
     * If necessary, the declaration is created and added to the code of current compilation unit.
     *
     * \param[in] memoryLocation Memory location of the global variable.
     * \param[in] type Valid pointer to the type traits of the global variable.
     *
     * \return Valid pointer to the global variable declaration.
     */
    likec::VariableDeclaration *makeGlobalVariableDeclaration(const MemoryLocation &memoryLocation, const types::Type *type);

    /**
     * \param[in] addr Entry address of a function.
     *
     * \return Declaration for function with this entry address, or 0 if no functions with such entry address are known.
     * Declaration is created when needed.
     */
    likec::FunctionDeclaration *makeFunctionDeclaration(ByteAddr addr);

    /**
     * Creates function's definition, if it wasn't yet, and adds it to the compilation unit.
     *
     * \param[in] function Function.
     *
     * \return Declaration for this function.
     */
    virtual likec::FunctionDeclaration *makeFunctionDeclaration(const Function *function);

    /**
     * Creates function's definition and adds it to the compilation unit.
     *
     * \param[in] function Function to create definition for.
     *
     * \return Created function definition.
     */
    virtual likec::FunctionDefinition *makeFunctionDefinition(const Function *function);

    /**
     * Registers a declaration of a function.
     *
     * \param[in] function Function.
     * \param[in] declaration Function declaration.
     */
    void setFunctionDeclaration(const Function *function, likec::FunctionDeclaration *declaration);
};

} // namespace cgen
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
