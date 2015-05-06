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

namespace nc {
namespace core {
namespace ir {

class Call;
class Function;
class Return;

namespace calls {

class CallAnalyzer;
class FunctionAnalyzer;
class ReturnAnalyzer;
class FunctionSignature;

/**
 * This class is responsible for aggregating information about the function's argument
 * list and its return value from call sites and function's implementations.
 */
class DescriptorAnalyzer {
    public:

    /**
     * Virtual destructor.
     */
    virtual ~DescriptorAnalyzer() {}

    /**
     * \param call                      Valid pointer to a call statement.
     *
     * \return Pointer to a new instance of CallAnalyzer for handling given call statement. Can be NULL.
     */
    virtual std::unique_ptr<CallAnalyzer> createCallAnalyzer(const Call *call) = 0;

    /**
     * \param function                  Valid pointer to a function.
     *
     * \return Pointer to a new instance of FunctionAnalyzer for handling given function. Can be NULL.
     */
    virtual std::unique_ptr<FunctionAnalyzer> createFunctionAnalyzer(const Function *function) = 0;

    /**
     * \param ret                       Valid pointer to a return statement.
     *
     * \return Pointer to a new instance of ReturnAnalyzer for handling given function. Can be NULL.
     */
    virtual std::unique_ptr<ReturnAnalyzer> createReturnAnalyzer(const Return *ret) = 0;

    /**
     * \return Signature of the function at the analyzed address.
     */
    virtual FunctionSignature getFunctionSignature() const = 0;
};

} // namespace calls
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
