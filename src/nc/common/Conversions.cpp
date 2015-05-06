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

#include "Conversions.h"
#include <cassert>
#include <boost/preprocessor/seq/for_each.hpp>
#include <QCoreApplication>
#include <nc/common/Exception.h>
#include <nc/common/Foreach.h>

namespace nc {
namespace {
#define NC_DEFINE_QSTRING_CONVERSION_ROUTINE(TYPE, METHOD)                      \
bool stringToInt(const QString &s, int base, TYPE *target) {                    \
    bool ok;                                                                    \
    *target = s.METHOD(&ok, base);                                              \
    return ok;                                                                  \
}
NC_DEFINE_QSTRING_CONVERSION_ROUTINE(short,              toShort)
NC_DEFINE_QSTRING_CONVERSION_ROUTINE(unsigned short,     toUShort)
NC_DEFINE_QSTRING_CONVERSION_ROUTINE(int,                toInt)
NC_DEFINE_QSTRING_CONVERSION_ROUTINE(unsigned int,       toUInt)
NC_DEFINE_QSTRING_CONVERSION_ROUTINE(long,               toLong)
NC_DEFINE_QSTRING_CONVERSION_ROUTINE(unsigned long,      toULong)
NC_DEFINE_QSTRING_CONVERSION_ROUTINE(long long,          toLongLong)
NC_DEFINE_QSTRING_CONVERSION_ROUTINE(unsigned long long, toULongLong)
#undef NC_DEFINE_QSTRING_CONVERSION_ROUTINE

bool stringToInt(const QString &s, int base, char *target) {
    bool ok;
    int result = s.toInt(&ok, base);
    if(result < std::numeric_limits<char>::min() || result > std::numeric_limits<char>::max())
        return false;
    *target = static_cast<char>(result);
    return ok;
}

bool stringToInt(const QString &s, int base, unsigned char *target) {
    bool ok;
    unsigned int result = s.toUInt(&ok, base);
    if(/* result < std::numeric_limits<unsigned char>::min() || */ result > std::numeric_limits<unsigned char>::max())
        return false;
    *target = static_cast<unsigned char>(result);
    return ok;
}
} // namespace `anonymous-namespace`


bool isHexString(const QString &s) {
    if (s.isEmpty())
        return false;

    foreach (QChar c, s) {
        switch(c.unicode()) {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': 
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': 
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            continue;
        default:
            return false;
        }
    }
    return true;
}

template<class Integer>
Integer hexStringToInt(const QString &s) {
    Integer result;
    if(!hexStringToInt(s, &result))
        throw nc::Exception(qApp->translate("hexStringToInt", "Can't convert hex string \"%1\" to integer.").arg(s));
    return result;
}

template<class Integer>
bool hexStringToInt(const QString &s, Integer *target) {
    if(s.startsWith(QLatin1String("0x")))
        return hexStringToInt(s.mid(2), target);

    Integer result;
    if(!stringToInt(s, 16, &result))
        return false;
    if(target != NULL)
        *target = result;
    return true;
}

template<class Integer>
Integer stringToInt(const QString &s) {
    Integer result;
    if(!stringToInt(s, &result))
        throw nc::Exception(qApp->translate("stringToInt", "Can't convert string \"%1\" to integer.").arg(s));
    return result;
}

template<class Integer>
bool stringToInt(const QString &s, Integer *target) {
    if (s.startsWith(QLatin1String("0x"))) {
        return hexStringToInt(s.mid(2), target);
    }

    Integer result;
    if (!stringToInt(s, 10, &result)) {
        return false;
    }
    if (target != NULL) {
        *target = result;
    }
    return true;
}

bool hexStringToByte(const QString &s, unsigned char *target) {
    if(s.size() != 2)
        return false;

    return hexStringToInt(s, target);
}

/* Generate explicit instantiations. */

#define FOR_ALL_INTEGER_TYPES_I(r, data, elem)                                  \
    data(elem)

#define FOR_ALL_INTEGER_TYPES(MACRO)                                            \
    BOOST_PP_SEQ_FOR_EACH(FOR_ALL_INTEGER_TYPES_I, MACRO, (char)(unsigned char)(short)(unsigned short)(int)(unsigned int)(long)(unsigned long)(long long)(unsigned long long))

#define INSTANTIATE_FUNCTIONS(TYPE)                                             \
template TYPE hexStringToInt<TYPE>(const QString &);                            \
template bool hexStringToInt<TYPE>(const QString &, TYPE *);                    \
template TYPE stringToInt<TYPE>(const QString &);                               \
template bool stringToInt<TYPE>(const QString &, TYPE *);
FOR_ALL_INTEGER_TYPES(INSTANTIATE_FUNCTIONS)
#undef INSTANTIATE_FUNCTIONS

#ifndef NDEBUG
namespace {
    using nc::stringToInt;

    bool test() {
        int result;
        assert(hexStringToInt("0x1", &result));
        assert(result == 1);

        assert(hexStringToInt("0xFF", &result));
        assert(result == 255);

        assert(hexStringToInt("100", &result));
        assert(result == 256);

        assert(stringToInt("100", &result));
        assert(result == 100);

        assert(stringToInt("0x101", &result));
        assert(result == 257);

        return true;
    }
    
    static bool success = test();
} // namespace `anonymous-namespace`
#endif

} // namespace nc

/* vim:set et sts=4 sw=4: */
