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
#include <climits>

namespace nc {

/**
 * \tparam T Integer type.
 * \param nbits Number of bits.
 *
 * \return Value of type T with bits [0..nbits-1] set.
 * In case T is narrower than nbits, returned value has all bits set.
 */
template<class T>
T bitMask(unsigned nbits) {
    return nbits < sizeof(T) * CHAR_BIT ? (T(1) << nbits) - 1 : T(-1);
}

/**
 * \tparam T Integer type.
 * \param[in] value Integer value.
 * \param[in] nbits Number of bits.
 *
 * \return The input value truncated to the lowest nbits bits.
 */
template<class T>
T bitTruncate(T value, unsigned nbits) {
    return nbits < sizeof(T) * CHAR_BIT ? value & ((T(1) << nbits) - 1) : value;
}

/**
 * \tparam T Integer type.
 * \param[in] value Integer value with bits [nbits..\inf] cleared.
 * \param[in] nbits Number of bits, must not exceed the width of T.
 *
 * \return Absolute value of the integer represented by bits [0..nbits-1] of value.
 */
template<class T>
T bitAbs(T value, unsigned nbits) {
    assert((value & bitMask<T>(nbits)) == value);
    assert(nbits <= sizeof(T) * CHAR_BIT);

    return value & (T(1) << (nbits - 1)) ? (value ^ ((T(1) << nbits) - 1)) + 1 : value;
}

/**
 * \tparam T Integer type.
 * \param[in] value Integer value with bits [nbits..\inf] cleared.
 * \param[in] nbits Number of bits, must not exceed the width of T.
 *
 * \return Signed extension of the integer represented by bits [0..nbits-1] of value.
 */
template<class T>
T signExtend(T value, unsigned nbits) {
    assert((value & bitMask<T>(nbits)) == value);
    assert(nbits <= sizeof(T) * CHAR_BIT);

    const T mask = T(1) << (nbits - 1);
    return (value ^ mask) - mask;
}

} // namespace nc

/* vim:set et sts=4 sw=4: */
