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

#include <nc/core/ir/MemoryLocation.h>

namespace nc {
namespace core {
namespace ir {

class Term;

namespace calls {

/**
 * Signature of a function: a list of its arguments, and a return value.
 */
class FunctionSignature {
    std::vector<MemoryLocation> arguments_; ///< Function's arguments.
    bool variadic_; ///< True if a function is variadic.
    const Term *returnValue_; ///< Term containing the return value.

    public:

    FunctionSignature(): variadic_(false), returnValue_(NULL) {}

    /**
     * \return List of function's arguments.
     */
    std::vector<MemoryLocation> &arguments() { return arguments_; }

    /**
     * \return List of function's arguments.
     */
    const std::vector<MemoryLocation> &arguments() const { return arguments_; }

    /**
     * Adds a memory location to the list of function's arguments.
     *
     * \param memoryLocation Memory location to add.
     */
    void addArgument(const MemoryLocation &memoryLocation) { arguments_.push_back(memoryLocation); }

    /**
     * \return True if the function is variadic.
     */
    bool variadic() const { return variadic_; }

    /**
     * Sets whether the function is variadic.
     *
     * \param value Flag whether the function is variadic.
     */
    void setVariadic(bool value = true) { variadic_ = value; }

    /**
     * \return Pointer to the term containing the return value. Can be NULL.
     */
    const Term *returnValue() const { return returnValue_; }

    /**
     * Sets the pointer to the term containing the return value.
     *
     * \param term Pointer to the term. Can be NULL.
     */
    void setReturnValue(const Term *term) { returnValue_ = term; }
};

} // namespace calls
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
