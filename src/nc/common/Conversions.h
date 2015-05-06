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
#include <QString>

namespace nc {

/**
 * \param[in] s                        String.
 *
 * \return                             Whether the given string consists of hex digits only.
 */
bool isHexString(const QString &s);

/**
 * Converts hex string to integer.
 *
 * \tparam Integer                     Integer class to convert to.
 * \param[in] s                        Hex string.
 * 
 * \return                             Converted number.
 * \exception                          nc::Exception in case of failure.
 */
template<class Integer>
Integer hexStringToInt(const QString &s);

/**
 * Converts hex string to integer. Doesn't throw.
 *
 * \tparam Integer                     Integer class to convert to.
 * \param[in] s                        Hex string.
 * \param[out] target                  Pointer to target integer. May be NULL.
 * 
 * \return                             Whether conversion was successful.
 */
template<class Integer>
bool hexStringToInt(const QString &s, Integer *target);

/**
 * Converts hex string representation of a byte (hex string of length 2) 
 * to unsigned char.
 * 
 * \param[in] s                        Hex string.
 * \param[out] target                  Pointer to target byte.
 * 
 * \return                             Whether conversion was successful.
 */
bool hexStringToByte(const QString &s, unsigned char *target = NULL);

/**
 * Converts C-like string representation of an integer to an integer.
 *
 * \tparam Integer                     Integer class to convert to.
 * \param[in] s                        String containing a number.
 * \param[out] target                  Pointer to target integer. May be NULL.
 * 
 * \return                             Converted number.
 */
template<class Integer>
bool stringToInt(const QString &s, Integer *target);

/**
 * Converts C-like string representation of an integer to an integer.
 *
 * \tparam Integer                     Integer class to convert to.
 * \param[in] s                        String containing a number.
 * \return                             Converted number.
 * \exception                          nc::Exception in case of failure.
 */
template<class Integer>
Integer stringToInt(const QString &s);

} // namespace nc

/* vim:set et sts=4 sw=4: */
