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

#include <boost/optional.hpp>

#include <nc/common/Types.h>

#include "DescriptorAnalyzer.h"

namespace nc {
namespace core {
namespace ir {
namespace calls {

class GenericCallAnalyzer;
class GenericCallingConvention;
class GenericFunctionAnalyzer;
class GenericReturnAnalyzer;

/**
 * Address analyzer for a typical calling convention using registers and stack for passing arguments.
 */
class GenericDescriptorAnalyzer: public DescriptorAnalyzer {
    const GenericCallingConvention *convention_; ///< Calling convention.

    std::vector<GenericCallAnalyzer *> callAnalyzers_; ///< All created call analyzers.
    std::vector<GenericFunctionAnalyzer *> functionAnalyzers_; ///< All created function analyzers.
    std::vector<GenericReturnAnalyzer *> returnAnalyzers_; ///< All created return analyzers.

    boost::optional<ByteSize> argumentsSize_; ///< Size of arguments on the stack in bytes.

    public:

    /**
     * Constructor.
     *
     * \param convention Valid pointer to the calling convention.
     */
    GenericDescriptorAnalyzer(const GenericCallingConvention *convention):
        convention_(convention)
    {
        assert(convention);
    }

    /**
     * \return Valid pointer to the calling convention.
     */
    const GenericCallingConvention *convention() const { return convention_; }

    /**
     * \return Size of arguments in bytes, if known.
     */
    boost::optional<ByteSize> argumentsSize() const { return argumentsSize_; }

    /**
     * Sets size of arguments.
     *
     * \param[in] size                 Size of arguments in bytes.
     */
    void setArgumentsSize(boost::optional<ByteSize> size) { argumentsSize_ = size; }

    virtual std::unique_ptr<CallAnalyzer> createCallAnalyzer(const Call *call) override;
    virtual std::unique_ptr<FunctionAnalyzer> createFunctionAnalyzer(const Function *function) override;
    virtual std::unique_ptr<ReturnAnalyzer> createReturnAnalyzer(const Return *function) override;
    virtual FunctionSignature getFunctionSignature() const override;
};

} // namespace calls
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
