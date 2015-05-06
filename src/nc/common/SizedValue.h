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

#include <nc/common/BitTwiddling.h>
#include <nc/common/Types.h>

namespace nc {

/**
 * Integer value having a size and capable of returning its signed and unsigned value.
 */
class SizedValue {
    /** The value. All its bits from size_ and higher are zero. */
    ConstantValue value_;

    /** Size of the value. */
    SmallBitSize size_;

    public:

    /**
     * Constructor.
     *
     * \param[in] value A value.
     * \param[in] size A size.
     *
     * Given "value" is truncated to "size" bits.
     */
    SizedValue(ConstantValue value = 0, SmallBitSize size = sizeof(ConstantValue) * CHAR_BIT):
        value_(bitTruncate(value, size)),
        size_(size)
    {
        assert(0 <= size);
    }

    /**
     * \return Size of the value.
     */
    SmallBitSize size() const { return size_; }

    /**
     * \return Value in the unsigned sense (bits as they are, padded with zeroes).
     */
    ConstantValue value() const { return value_; }

    /**
     * \return Value in the signed sense (sign-extended value).
     */
    SignedConstantValue signedValue() const { return signExtend(value_, size_); }

    /**
     * \return Absolute value of the value.
     */
    ConstantValue absoluteValue() const { return bitAbs(value_, size_); }

    /**
     * \param[in] size A size.
     *
     * \return A copy of the value resized to given size.
     */
    SizedValue resized(SmallBitSize size) const { return SizedValue(value(), size); }
};

} // namespace nc

/* vim:set et sts=4 sw=4: */
