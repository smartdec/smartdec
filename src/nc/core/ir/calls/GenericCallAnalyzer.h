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
#include <vector>

#include <boost/unordered_map.hpp>

#include <nc/core/ir/MemoryLocation.h>

#include "CallAnalyzer.h"

namespace nc {
namespace core {
namespace ir {

class Constant;
class Statement;
class Term;

namespace calls {

class GenericCallingConvention;
class GenericDescriptorAnalyzer;

/**
 * GenericCallAnalyzer is a CallAnalyzer for a typical calling convention using registers and stack to pass arguments.
 */
class GenericCallAnalyzer: public CallAnalyzer {
    /** Address analyzer. */
    const GenericDescriptorAnalyzer *addressAnalyzer_;

    /** Mapping of argument memory locations to corresponding terms. */
    boost::unordered_map<MemoryLocation, std::unique_ptr<Term>> arguments_;

    /** Mapping of terms where return values may be kept to their clones. */
    boost::unordered_map<const Term *, std::unique_ptr<Term>> returnValues_;

    /** Term for tracking stack pointer. */
    std::unique_ptr<Term> stackPointer_;

    /** Detected value of stack pointer. */
    BitOffset stackTop_;

    /** Integer number added to the stack pointer inside called function. */
    Constant *stackAmendmentConstant_;

    /** Statement to change stack pointer by the amendment constant. */
    std::unique_ptr<Statement> stackAmendmentStatement_;

    /** Computed set of memory locations where arguments are stored. */
    std::vector<MemoryLocation> argumentLocations_;

    /** Computed set of terms where results are stored. */
    std::vector<const Term *> returnValueLocations_;

public:
    /**
     * Class constructor.
     *
     * \param[in] call Valid pointer to a call statement.
     * \param[in] addressAnalyzer Parent DescriptorAnalyzer.
     */
    GenericCallAnalyzer(const Call *call, const GenericDescriptorAnalyzer *addressAnalyzer);

    /**
     * Destructor.
     */
    virtual ~GenericCallAnalyzer();

    /**
     * \return Valid pointer to the address analyzer.
     */
    const GenericDescriptorAnalyzer *addressAnalyzer() const { return addressAnalyzer_; }

    /**
     * \return Valid pointer to the calling convention.
     */
    const GenericCallingConvention *convention() const;

    /**
     * \return Estimated list of locations where arguments are stored.
     */
    const std::vector<MemoryLocation> &argumentLocations() const { return argumentLocations_; }

    /**
     * \return Estimated list of terms describing locations where arguments are stored.
     */
    const std::vector<const Term *> &returnValueLocations() const { return returnValueLocations_; }

    virtual void simulateCall(dflow::SimulationContext &context) override;
    virtual const Term *getArgumentTerm(const MemoryLocation &memoryLocation) override;
    virtual const Term *getReturnValueTerm(const Term *term) override;
    virtual void visitChildStatements(Visitor<const Statement> &visitor) const override;
    virtual void visitChildTerms(Visitor<const Term> &visitor) const override;
};

} // namespace calls
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
