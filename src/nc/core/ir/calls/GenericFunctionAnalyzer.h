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

#include <memory> /* std::unique_ptr */

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <nc/core/ir/MemoryLocation.h>

#include "FunctionAnalyzer.h"

namespace nc {
namespace core {
namespace ir {
namespace calls {

class GenericCallingConvention;
class GenericDescriptorAnalyzer;

/**
 * GenericFunctionAnalyzer is a FunctionAnalyzer for a typical calling convention using registers and stack to pass arguments.
 */
class GenericFunctionAnalyzer: public FunctionAnalyzer {
    /** Address analyzer. */
    const GenericDescriptorAnalyzer *addressAnalyzer_;

    /** Precomputed set of locations where arguments can be stored. */
    boost::unordered_set<MemoryLocation> possibleArgumentLocations_;

    /** Term for initializing stack pointer. */
    std::unique_ptr<Term> stackPointer_;

    /** Statements executed when a function is entered. */
    std::vector<const Statement *> entryStatements_;

    /** Mapping of argument memory locations to corresponding terms. */
    boost::unordered_map<MemoryLocation, std::unique_ptr<Term>> arguments_;

    /** Computed set of memory locations where arguments are stored. */
    std::vector<MemoryLocation> argumentLocations_;

public:
    /**
     * Class constructor.
     *
     * \param[in] function Valid pointer to the function to be analyzed.
     * \param[in] addressAnalyzer Parent DescriptorAnalyzer.
     */
    GenericFunctionAnalyzer(const Function *function, const GenericDescriptorAnalyzer *addressAnalyzer);

    /**
     * Destructor.
     */
    virtual ~GenericFunctionAnalyzer();

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

    virtual const std::vector<const Statement *> &entryStatements() const override { return entryStatements_; }
    virtual void simulateEnter(dflow::SimulationContext &context) override;
    virtual const Term *getArgumentTerm(const MemoryLocation &memoryLocation) override;
    virtual void visitChildStatements(Visitor<const Statement> &visitor) const override;
    virtual void visitChildTerms(Visitor<const Term> &visitor) const override;
};

} // namespace calls
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
