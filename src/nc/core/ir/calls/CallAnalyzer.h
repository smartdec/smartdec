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

class Call;
class Statement;
class Term;

namespace calls {

/**
 * CallAnalyzer extracts the information about location of arguments and return values from a call site.
 */
class CallAnalyzer {
    const Call *call_; ///< Call statement for which the analyzer has been created.

public:
    /**
     * Class constructor.
     *
     * \param call Valid pointer to a call statement to be analyzed.
     */
    CallAnalyzer(const Call *call):
        call_(call)
    { assert(call != NULL); }

    /**
     * Virtual destructor.
     */
    virtual ~CallAnalyzer() {}

    /**
     * \return Call statement for which the analyzer has been created. 
     */
    const Call *call() const { return call_; }

    /**
     * A method being called when specified call statement is simulated.
     * 
     * \param context                  Simulation context.
     */
    virtual void simulateCall(dflow::SimulationContext &context) = 0;

    /**
     * \param memoryLocation Memory location.
     *
     * \return A valid pointer to the term representing the argument at given memory location.
     * The term is created when necessary and owned by this CallAnalyzer.
     */
    virtual const Term *getArgumentTerm(const MemoryLocation &memoryLocation) = 0;

    /**
     * \param term Valid pointer to a term.
     *
     * \return A valid pointer to the term representing the argument designated by given term.
     * The former term is created when necessary and owned by this CallAnalyzer.
     */
    virtual const Term *getReturnValueTerm(const Term *term) = 0;

    /**
     * Calls visitor for child statements.
     *
     * \param[in] visitor              Visitor.
     */
    virtual void visitChildStatements(Visitor<const Statement> &visitor) const = 0;

    /**
     * Calls visitor for child terms.
     *
     * \param[in] visitor              Visitor.
     */
    virtual void visitChildTerms(Visitor<const Term> &visitor) const = 0;
};

}}}} // namespace nc::core::ir::calls

/* vim:set et ts=4 sw=4: */
