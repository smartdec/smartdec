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
    class SimulationContext;
}

namespace calls {

/**
 * FunctionAnalyzer extracts the information about location of function's arguments from function's implementation.
 */
class FunctionAnalyzer {
    const Function *function_; ///< Function this callee convention object is related to.

    public:

    /**
     * Class constructor.
     *
     * \param function Valid pointer to the function to be analyzed.
     */
    FunctionAnalyzer(const Function *function):
        function_(function)
    { assert(function != NULL); }

    /**
     * Virtual destructor.
     */
    virtual ~FunctionAnalyzer() {}

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
     * Method run when function's entry node is being simulated.
     * 
     * \param context Simulation context.
     */
    virtual void simulateEnter(dflow::SimulationContext &context) = 0;

    /**
     * Returns a valid pointer to the term representing the argument at given memory location.
     * The term is created when necessary and owned by this FunctionAnalyzer.
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

} // namespace calls
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
