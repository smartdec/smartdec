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
#include <string>
#include <QString>

namespace nc {
namespace arch {
namespace intel {
namespace prefixes {

/**
 * Constants for bit masks of instruction prefixes.
 */
enum Prefix {
    INVALID = 0,
#define PREFIX(lowercase, uppercase, bitmask, comment)  \
    uppercase = bitmask,
#include "PrefixTable.i"
#undef PREFIX
};

extern const Prefix prefixes[]; ///< Array containing all prefixes.
extern const std::size_t prefixCount; ///< Number of prefixes.

/**
 * \param[in] prefix Prefix bit mask.
 *
 * \return Lowercase string representation for prefix with given bit mask or 0, if such prefix isn't known.
 */
const char *getPrefixLowercaseName(int prefix);

/**
 * \param[in] prefix Prefix bit mask.
 *
 * \return Uppercase string representation for prefix with given bit mask or 0, if such prefix isn't known.
 */
const char *getPrefixUppercaseName(int prefix);

/**
 * \param[in] name String representation for prefix.
 *
 * \return Prefix bit mask.
 */
Prefix getPrefix(const QString &name);

} // mamespace prefixes
} // namespace intel
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
