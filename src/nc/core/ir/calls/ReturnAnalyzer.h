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

#include <nc/common/Types.h>
#include <nc/common/Visitor.h>

namespace nc {
namespace core {
namespace ir {

namespace dflow {
    class SimulationContext;
}

class Return;
class Statement;
class Term;

namespace calls {

/**
 * ReturnAnalyzer extracts information about location of function's return value from a return site.
 */
class ReturnAnalyzer {
    const Return *return_; ///< Return statement.

    public:

    /**
     * Class constructor.
     *
     * \param ret Valid pointer to a return statement to be analyzed.
     */
    ReturnAnalyzer(const Return *ret):
        return_(ret)
    { assert(ret != NULL); }

    /**
     * Virtual destructor.
     */
    virtual ~ReturnAnalyzer() {}

    /**
     * \return Return statement for which the analyzer has been created.
     */
    const Return *ret() const { return return_; }

    /**
     * A method being called when specified return statement is simulated.
     * 
     * \param context Simulation context.
     */
    virtual void simulateReturn(dflow::SimulationContext &context) = 0;

    /**
     * Returns a valid pointer to the term representing the argument designated by given term.
     * The term is created when necessary and owned by this ReturnAnalyzer.
     *
     * \param term Valid pointer to a term.
     */
    virtual const Term *getReturnValueTerm(const Term *term) = 0;

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
