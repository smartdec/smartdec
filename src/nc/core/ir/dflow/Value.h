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

#include <boost/optional.hpp>

#include <nc/common/SizedValue.h>
#include <nc/common/Types.h>

namespace nc {
namespace core {
namespace ir {
namespace dflow {

/**
 * Traits of term's value.
 */
class Value {
    SmallBitSize size_; ///< Size of the value in bits.

    bool isConstant_; ///< Value is constant.
    bool isNonconstant_; ///< Value is nonconstant.

    SizedValue constantValue_; ///< The value of a constant, if the value is a constant.

    bool isStackOffset_; ///< Value is a pointer to stack.
    bool isNotStackOffset_; ///< Value is not a pointer to stack.

    SizedValue stackOffset_; ///< Offset to stack frame base (in bytes), if the value is a stack pointer.

    bool isMultiplication_; ///< Value has been computed via multiplication.
    bool isNotMultiplication_; ///< Value has not been computed via multiplication.

    public:

    static const SmallBitSize MAX_SIZE = sizeof(ConstantValue) * CHAR_BIT; ///< Max size of a value, in bits.

    /**
     * Class constructor.
     */
    Value(SmallBitSize size);

    /**
     * \return Size of the value in bits.
     */
    SmallBitSize size() const { return size_; }

    /**
     * \return True, if value is constant.
     */
    bool isConstant() const { return isConstant_ && !isNonconstant_; }

    /**
     * \return True, if value is nonconstant.
     */
    bool isNonconstant() const { return isNonconstant_; }

    /**
     * Mark the value as constant with given value.
     * If the value is already known to be a constant with another value, the value is marked as nonconstant.
     *
     * \param[in] value The value of a constant.
     */
    void makeConstant(const SizedValue &value);

    /**
     * Mark the value as constant with given value.
     * If the value is boost::none, the value is marked as nonconstant.
     *
     * \param[in] value The value of a constant.
     */
    void makeConstant(const boost::optional<SizedValue> &value) {
        if (value) {
            makeConstant(*value);
        } else {
            makeNonconstant();
        }
    }

    /**
     * Mark the value as nonconstant.
     */
    void makeNonconstant() { isNonconstant_ = true; }

    /**
     * Forcedly mark the value as constant with given value, even if the opposite was known before.
     *
     * \param[in] value The value of a constant.
     */
    void forceConstant(const SizedValue &value);

    /**
     * \return The value of a constant, if the value is constant.
     */
    const SizedValue &constantValue() const { assert(isConstant()); return constantValue_; }

    /**
     * \return True, if the value is a pointer to stack.
     */
    bool isStackOffset() const { return isStackOffset_ && !isNotStackOffset_; }

    /**
     * \return True, if the value is not a pointer to stack.
     */
    bool isNotStackOffset() const { return isNotStackOffset_; }

    /**
     * Merks the value as a stack pointer to given stack frame offset.
     *
     * \param[in] offset Offset to stack frame base.
     */
    void makeStackOffset(SizedValue offset);

    /**
     * Marks the value as not a pointer to stack.
     */
    void makeNotStackOffset() { isNotStackOffset_ = true; }

    /**
     * \return Offset to stack frame base (in bytes), if the value is a stack pointer.
     */
    const SizedValue &stackOffset() const { assert(isStackOffset()); return stackOffset_; }

    /**
     * \return True, if the value has been computed via multiplication.
     */
    bool isMultiplication() const { return isMultiplication_ && !isNotMultiplication_; }

    /**
     * \return True, if the value has not been computed via multiplication.
     */
    bool isNotMultiplication() const { return isNotMultiplication_; }

    /**
     * Marks the value as being computed via multiplication.
     */
    void makeMultiplication() { isMultiplication_ = true; }

    /**
     * Marks the value as being computed not via multiplication.
     */
    void makeNotMultiplication() { isNotMultiplication_ = true; }

    /**
     * Adds information about other value's traits to this value's traits.
     *
     * \param[in] value Another value.
     */
    void join(const Value &value);
};

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
