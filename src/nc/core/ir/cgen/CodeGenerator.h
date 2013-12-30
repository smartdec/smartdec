/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

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

class Module;

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
class Functions;
class Term;

namespace calling {
    class CalleeId;
    class Hooks;
    class Signatures;
}

namespace cflow {
    class Graphs;
}

namespace dflow {
    class Dataflows;
}

namespace liveness {
    class Livenesses;
}

namespace types {
    class Type;
    class Types;
}

namespace vars {
    class Variable;
    class Variables;
}

namespace cgen {

/**
 * LikeC code generator.
 */
class CodeGenerator: boost::noncopyable {
    /** Abstract syntax tree to generate code in. */
    likec::Tree &tree_;

    /** Module being decompiled. */
    const Module &module_;

    /** Intermediate representation of functions. */
    const Functions &functions_;

    /** Hooks of calling conventions. */
    calling::Hooks &hooks_;

    /** Signatures of the functions. */
    const calling::Signatures &signatures_;

    /** Dataflow information for all functions. */
    const dflow::Dataflows &dataflows_;

    /** Reconstructed variables. */
    const vars::Variables &variables_;

    /** Reduced control-flow graphs. */
    const cflow::Graphs &graphs_;

    /** Liveness information for all functions. */
    const liveness::Livenesses &livenesses_;

    /** Information about types. */
    const types::Types &types_;

    /** Cancellation token. */
    const CancellationToken &cancellationToken_;

    /** Types being translated to LikeC. */
    std::vector<const ir::types::Type *> typeCreationStack_;

    /** Structural types generated for IR types. */
    boost::unordered_map<const ir::types::Type *, const likec::StructType *> traits2structType_;

    /** Already declared global variables. */
    boost::unordered_map<const vars::Variable *, likec::VariableDeclaration *> variableDeclarations_;

    /** Mapping of functions to their declarations. */
    boost::unordered_map<const Function *, likec::FunctionDeclaration *> function2declaration_;

public:

    /**
     * Constructor.
     *
     * \param[out] tree Abstract syntax tree to generate code in.
     * \param[in] module Module being decompiled.
     * \param[in] functions Intermediate representation of functions.
     * \param[in] hooks Hooks of calling conventions.
     * \param[in] signatures Signatures of functions.
     * \param[in] dataflows Dataflow information for all functions.
     * \param[in] graphs Reduced control-flow graphs.
     * \param[in] liveness Liveness information for all functions.
     * \param[in] types Information about types.
     * \param[in] cancellationToken Cancellation token.
     */
    CodeGenerator(likec::Tree &tree, const Module &module, const Functions &functions, calling::Hooks &hooks,
        const calling::Signatures &signatures, const dflow::Dataflows &dataflows, const vars::Variables &variables,
        const cflow::Graphs &graphs, const liveness::Livenesses &livenesses, const types::Types &types,
        const CancellationToken &cancellationToken
    ):
        tree_(tree), module_(module), functions_(functions), hooks_(hooks), signatures_(signatures),
        dataflows_(dataflows), variables_(variables), graphs_(graphs), livenesses_(livenesses),
        types_(types), cancellationToken_(cancellationToken)
    {}

    /**
     * Virtual destructor.
     */
    virtual ~CodeGenerator() {}

    /**
     * \return Abstract syntax tree to generate code in.
     */
    likec::Tree &tree() const { return tree_; }

    /**
     * \return Module being decompiled.
     */
    const Module &module() const { return module_; }

    /**
     * \return Intermediate representation of functions.
     */
    const Functions &functions() const { return functions_; }

    /**
     * \return Hooks of calling conventions.
     */
    calling::Hooks &hooks() const { return hooks_; }

    /**
     * \return Signatures of functions.
     */
    const calling::Signatures &signatures() const { return signatures_; }

    /**
     * \return Dataflow information for all functions.
     */
    const ir::dflow::Dataflows &dataflows() const { return dataflows_; }

    /**
     * \return Reconstructed variables.
     */
    const vars::Variables &variables() const { return variables_; }

    /**
     * \return Reduced control-flow graphs.
     */
    const cflow::Graphs &graphs() const { return graphs_; }

    /**
     * \return Liveness information for all functions.
     */
    const liveness::Livenesses &livenesses() const { return livenesses_; }

    /**
     * \return Information about types.
     */
    const types::Types &types() const { return types_; }

    /**
     * \return Cancellation token.
     */
    const CancellationToken &cancellationToken() const { return cancellationToken_; }

    /**
     * Translates input program into LikeC compilation unit.
     */
    void makeCompilationUnit();

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
     * \param variable Valid pointer to a variable.
     *
     * \return Valid pointer to the LikeC type of this variable.
     */
    const likec::Type *makeVariableType(const vars::Variable *variable);

    /**
     * \param[in] variable Valid pointer to a global variable.
     *
     * \return Valid pointer to corresponding global variable declaration.
     */
    likec::VariableDeclaration *makeGlobalVariableDeclaration(const vars::Variable *variable);

    /**
     * \param[in] calleeId Id of a called function.
     *
     * \return Declaration for this function, or NULL if it was impossible to create one.
     */
    likec::FunctionDeclaration *makeFunctionDeclaration(const calling::CalleeId &calleeId);

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
