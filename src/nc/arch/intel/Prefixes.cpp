//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "Prefixes.h"

#include <QHash>

#include <boost/preprocessor/stringize.hpp>
#include <boost/range/size.hpp>

namespace nc {
namespace arch {
namespace intel {
namespace prefixes {

const Prefix prefixes[] = {
#define PREFIX(lowercase, uppercase, bitmask, comment)      \
    uppercase,
#include "PrefixTable.i"
#undef PREFIX
};

const std::size_t prefixCount = boost::size(prefixes);

const char *getPrefixLowercaseName(int prefix) {
    switch (prefix) {
#define PREFIX(lowercase, uppercase, bitmask, comment)      \
        case uppercase:                                     \
            return BOOST_PP_STRINGIZE(lowercase);
#include "PrefixTable.i"
#undef PREFIX
        default:
            return 0;
    }
}

const char *getPrefixUppercaseName(int prefix) {
    switch (prefix) {
#define PREFIX(lowercase, uppercase, bitmask, comment)      \
        case uppercase:                                     \
            return BOOST_PP_STRINGIZE(uppercase);
#include "PrefixTable.i"
#undef PREFIX
        default:
            return 0;
    }
}

namespace {

QHash<QString, Prefix> makeStringToPrefixMap() {
    QHash<QString, Prefix> result;
#define PREFIX(lowercase, uppercase, bitmask, comment)          \
    result.insert(BOOST_PP_STRINGIZE(lowercase), uppercase);    \
    result.insert(BOOST_PP_STRINGIZE(uppercase), uppercase);
#include "PrefixTable.i"
#undef PREFIX
    return result;
}

} // anonymous namespace

Prefix getPrefix(const QString &name) {
    static const QHash<QString, Prefix> stringToPrefixMap = makeStringToPrefixMap();

    return stringToPrefixMap.value(name, INVALID);
}

} // namespace prefixes
} // namespace intel
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
