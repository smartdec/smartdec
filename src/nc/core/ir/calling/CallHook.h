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

#include <memory> /* unique_ptr */

#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>

#include <nc/common/Visitor.h>

#include <nc/core/ir/MemoryLocation.h>
#include <nc/core/ir/dflow/ReachingDefinitions.h>

namespace nc {
namespace core {
namespace ir {

class Call;
class Statement;
class Term;

namespace dflow {
    class ExecutionContext;
}

namespace calling {

class Convention;
class Signature;

/**
 * Hook being executed after a return is executed after a call is executed.
 */
class CallHook {
    /** Mapping of argument memory locations to corresponding terms. */
    boost::unordered_map<const Term *, std::unique_ptr<Term>> arguments_;

    /** Mapping of terms where return values may be kept to their clones. */
    boost::unordered_map<const Term *, std::unique_ptr<Term>> returnValues_;

    /** Term for tracking stack pointer. */
    std::unique_ptr<Term> stackPointer_;

    /** Statement to change stack pointer by the amendment constant. */
    std::unique_ptr<Statement> cleanupStatement_;

    /** Definitions reaching this hook. */
    dflow::ReachingDefinitions reachingDefinitions_;

public:
    /**
     * Class constructor.
     *
     * \param[in] call Valid pointer to the call statement being hooked.
     * \param[in] convention Valid pointer to the calling convention.
     * \param[in] signature Pointer to the function's signature. Can be NULL.
     * \param[in] stackArgumentsSize Size of arguments passed on the stack.
     */
    CallHook(const Call *call, const Convention *convention, const Signature *signature, boost::optional<ByteSize> stackArgumentsSize);

    /**
     * Destructor.
     */
    ~CallHook();

    /**
     * A method being called when specified call statement is executed.
     *
     * \param context Execution context.
     */
    void execute(dflow::ExecutionContext &context);

    /**
     * \return Definitions reaching this hook.
     */
    const dflow::ReachingDefinitions &reachingDefinitions() const { return reachingDefinitions_; }

    /**
     * \param term Valid pointer to a term representing the argument
     *             in the signature.
     *
     * \return Pointer to the term representing this argument in the hook.
     *         Will be NULL, if signature does not include such an argument.
     */
    const Term *getArgumentTerm(const Term *term) const;

    /**
     * \param term Valid pointer to a term representing the return value
     *             in the signature.
     *
     * \return Pointer to the term representing the return value in the hook.
     *         Will be NULL, if signature does not include such an argument.
     */
    const Term *getReturnValueTerm(const Term *term) const;

    /**
     * Calls visitor for child statements.
     *
     * \param[in] visitor Visitor.
     */
    void visitChildStatements(Visitor<const Statement> &visitor) const;

    /**
     * Calls visitor for child terms.
     *
     * \param[in] visitor Visitor.
     */
    void visitChildTerms(Visitor<const Term> &visitor) const;
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
