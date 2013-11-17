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

#include <cassert>
#include <vector>

#include <nc/common/Types.h>
#include <nc/common/Visitor.h>

namespace nc {
namespace core {
namespace ir {

class Function;
class MemoryLocation;
class Statement;
class Term;

namespace dflow {
    class ExecutionContext;
}

namespace cconv {

/**
 * EnterHook gets executed just before the function entry.
 */
class EnterHook {
    const Function *function_; ///< Function this callee convention object is related to.

    public:

    /**
     * Class constructor.
     *
     * \param function Valid pointer to the function to be analyzed.
     */
    EnterHook(const Function *function):
        function_(function)
    {
        assert(function != NULL);
    }

    /**
     * Virtual destructor.
     */
    virtual ~EnterHook() {}

    /**
     * \return Function this callee convention object is related to.
     */
    const Function *function() const { return function_; }

    /**
     * \return Statements that are executed behind the scence on function entry. Can be NULL.
     *
     * These statements will be actually used for generation of function->entry() basic block's code.
     */
    virtual const std::vector<const Statement *> &entryStatements() const;

    /**
     * This method is called just before function's entry node gets executed.
     * 
     * \param context Execution context.
     */
    virtual void execute(dflow::ExecutionContext &context) = 0;

    /**
     * Returns a valid pointer to the term representing the argument at given memory location.
     * The term is created when necessary and owned by this EnterHook.
     *
     * \param memoryLocation Memory location.
     */
    virtual const Term *getArgumentTerm(const MemoryLocation &memoryLocation) = 0;

    /**
     * Calls visitor for child statements.
     *
     * \param[in] visitor Visitor.
     */
    virtual void visitChildStatements(Visitor<const Statement> &visitor) const = 0;

    /**
     * Calls visitor for child terms.
     *
     * \param[in] visitor Visitor.
     */
    virtual void visitChildTerms(Visitor<const Term> &visitor) const = 0;
};

} // namespace cconv
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
