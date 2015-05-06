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
#include <boost/noncopyable.hpp>
#include <QString>

namespace nc { namespace core { namespace arch {

/**
 * Instruction mnemonic.
 * 
 * Mnemonics are immutable.
 */
class Mnemonic: public boost::noncopyable {
public:
    Mnemonic(int number, const QString &name, const QString &description):
        mNumber(number),
        mLowercaseName(name.toLower()),
        mUppercaseName(name.toUpper()),
        mDescription(description)
    {
        assert(number >= 0);
    }

    /**
     * \returns                         Mnemonic number.
     */
    int number() const {
        return mNumber;
    }

    /**
     * \returns                         Mnemonic as an uppercase string.
     */
    const QString &uppercaseName() const {
        return mUppercaseName;
    }

    /**
     * \returns                         Mnemonic as a lowercase string.
     */
    const QString &lowercaseName() const {
        return mLowercaseName;
    }

    /**
     * \returns                         Textual description of the mnemonic.
     */
    const QString &description() const {
        return mDescription;
    }

private:
    int mNumber;
    QString mLowercaseName;
    QString mUppercaseName;
    QString mDescription;
};

}}} // namespace nc::core::arch
