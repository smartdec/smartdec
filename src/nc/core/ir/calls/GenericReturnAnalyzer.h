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
#include <vector>

#include <boost/unordered_map.hpp>

#include "ReturnAnalyzer.h"

namespace nc {
namespace core {
namespace ir {
namespace calls {

class GenericCallingConvention;
class GenericDescriptorAnalyzer;

/**
 * GenericReturnAnalyzer is a ReturnAnalyzer for a typical calling convention using registers and stack to pass arguments.
 */
class GenericReturnAnalyzer: public ReturnAnalyzer {
    /** Address analyzer. */
    const GenericDescriptorAnalyzer *addressAnalyzer_;

    /** Mapping of terms where return values may be kept to their clones. */
    boost::unordered_map<const Term *, std::unique_ptr<Term>> returnValues_;

    /** Computed set of terms where results are stored. */
    std::vector<const Term *> returnValueLocations_;

public:
    /**
     * Class constructor.
     *
     * \param[in] ret Valid pointer to a return statement to be analyzed.
     * \param[in] addressAnalyzer Parent DescriptorAnalyzer.
     */
    GenericReturnAnalyzer(const Return *ret, const GenericDescriptorAnalyzer *addressAnalyzer);

    /**
     * Destructor.
     */
    virtual ~GenericReturnAnalyzer();

    /**
     * \return Valid pointer to the address analyzer.
     */
    const GenericDescriptorAnalyzer *addressAnalyzer() const { return addressAnalyzer_; }

    /**
     * \return Valid pointer to the calling convention.
     */
    const GenericCallingConvention *convention() const;

    /**
     * \return Estimated list of terms describing locations where arguments are stored.
     */
    const std::vector<const Term *> &returnValueLocations() const { return returnValueLocations_; }

    virtual void simulateReturn(dflow::SimulationContext &context) override;
    virtual const Term *getReturnValueTerm(const Term *term) override;
    virtual void visitChildStatements(Visitor<const Statement> &visitor) const override;
    virtual void visitChildTerms(Visitor<const Term> &visitor) const override;
};

} // namespace calls
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
