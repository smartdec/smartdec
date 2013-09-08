/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

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
 * \param[in] nbits Number of bits.
 *
 * \return Value of type T with bits [0..nbits-1] set.
 * In case T is narrower than nbits, returned value has all bits set.
 */
template<class T>
T bitMask(unsigned nbits) {
    if (nbits < sizeof(T) * CHAR_BIT) {
        return T(1) << nbits;
    } else {
        return T(-1);
    }
}

/**
 * \tparam T Integer type.
 * \param[in] value Integer value.
 * \param[in] nbits Number of bits.
 *
 * \return Value shifted to the left by the given number of bits.
 */
template<class T>
T shiftLeft(T value, unsigned nbits) {
    if (nbits < sizeof(T) * CHAR_BIT) {
        return value << nbits;
    } else {
        return 0;
    }
}

/**
 * \tparam T Integer type.
 * \param[in] value Integer value.
 * \param[in] nbits Number of bits.
 *
 * \return Value shifted to the right by the given number of bits.
 */
template<class T>
T shiftRight(T value, unsigned nbits) {
    if (nbits < sizeof(T) * CHAR_BIT) {
        return value >> nbits;
    } else {
        return 0;
    }
}

/**
 * \tparam T Integer type.
 * \param[in] value Integer value.
 * \param[in] nbits Number of bits.
 *
 * \return The value shifted by the given number of bits.
 * If the number of bits is positive, the shift is to the left.
 * If the number of bits is negative, the shift is to the right.
 */
template<class T>
T bitShift(T value, int nbits) {
    if (nbits > 0) {
        return shiftLeft(value, nbits);
    } else if (nbits < 0) {
        return shiftRight(value, nbits);
    } else {
        return value;
    }
}

/**
 * \tparam T Integer type.
 * \param[in] value Integer value.
 * \param[in] nbits Number of bits.
 *
 * \return The input value truncated to the lowest size bits.
 */
template<class T>
T bitTruncate(T value, unsigned nbits) {
    return value & bitMask<T>(nbits);
}

/**
 * \tparam T Integer type.
 * \param[in] value Integer value with bits [nbits..\inf] cleared.
 * \param[in] nbits Number of bits.
 *
 * \return Negation of the value of the integer represented by bits [0..nbits-1] of value.
 */
template<class T>
T bitNegate(T value, unsigned nbits) {
    assert((value & bitMask<T>(nbits)) == value);

    return (value ^ bitMask<T>(nbits)) + 1;
}

/**
 * \tparam T Integer type.
 * \param[in] value Integer value with bits [nbits..\inf] cleared.
 * \param[in] nbits Number of bits.
 *
 * \return Absolute value of the integer represented by bits [0..nbits-1] of value.
 */
template<class T>
T bitAbs(T value, unsigned nbits) {
    assert((value & bitMask<T>(nbits)) == value);

    if (value & shiftLeft<T>(1, nbits - 1)) {
        return bitNegate(value, nbits);
    } else {
        return value;
    }
}

/**
 * \tparam T Integer type.
 * \param[in] value Integer value with bits [nbits..\inf] cleared.
 * \param[in] nbits Number of bits.
 *
 * \return Signed extension of the integer represented by bits [0..nbits-1] of value.
 */
template<class T>
T signExtend(T value, unsigned nbits) {
    assert((value & bitMask<T>(nbits)) == value);

    const T mask = shiftLeft<T>(1, nbits - 1);
    return (value ^ mask) - mask;
}

} // namespace nc

/* vim:set et sts=4 sw=4: */
