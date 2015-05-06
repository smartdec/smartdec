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
#include <memory> /* unique_ptr */
#include <vector>

#include <QHash>
#include <QString>

#include "Mnemonic.h"

namespace nc { namespace core { namespace arch {

/**
 * Base class for mnemonic containers.
 */
class Mnemonics {
public:
    /**
     * Virtual destructor.
     */
    virtual ~Mnemonics() {}

    /**
     * \param[in] name                 Mnemonic name, uppercase or lowercase.
     * \returns                        Mnemonic for the given name, or NULL
     *                                 if no such mnemonic exists.
     */
    const Mnemonic *mnemonic(const QString &name) const {
        return mMnemonicByName.value(name, NULL);
    }

    /**
     * \param[in] number               Mnemonic number.
     * \returns                        Mnemonic for the given number, or NULL
     *                                 if no such mnemonic exists.
     */
    const Mnemonic *mnemonic(int number) const {
        if(number < 0 || static_cast<std::size_t>(number) >= mMnemonicByNumber.size())
            return NULL;

        return mMnemonicByNumber[number].get();
    }

protected:
    /**
     * Registers the given mnemonic. This mnemonic container takes ownership
     * of the given mnemonic.
     * 
     * \param[in] mnemonic             Mnemonic to register.
     */
    void registerMnemonic(Mnemonic *mnemonic) {
        assert(mnemonic != NULL);
        assert(!mMnemonicByName.contains(mnemonic->lowercaseName())); /* Re-registration not allowed. */

        mMnemonicByName[mnemonic->lowercaseName()] = mnemonic;
        mMnemonicByName[mnemonic->uppercaseName()] = mnemonic;

        if(static_cast<std::size_t>(mnemonic->number()) >= mMnemonicByNumber.size())
            mMnemonicByNumber.resize((mnemonic->number() + 1) * 2);
        mMnemonicByNumber[mnemonic->number()].reset(mnemonic);
    }

private:
    /** Mnemonic name (uppercase or lowercase) to mnemonic map. */
    QHash<QString, Mnemonic *> mMnemonicByName;

    /** Mnemonic number to mnemonic map. */
    std::vector<std::unique_ptr<Mnemonic>> mMnemonicByNumber;
};


/**
 * Base class for static mnemonic containers.
 */
template<class Derived>
class StaticMnemonics: public Mnemonics {
public:
    static const Mnemonic *mnemonic(const QString &name) {
        return instance()->Mnemonics::mnemonic(name);
    }

    static const Mnemonic *mnemonic(int number) {
        return instance()->Mnemonics::mnemonic(number);
    }

    static Derived *instance() {
        return &sInstance;
    }

private:
    static Derived sInstance;
};

template<class Derived>
Derived StaticMnemonics<Derived>::sInstance;

}}} // namespace nc::core::arch

/* vim:set et sts=4 sw=4: */
