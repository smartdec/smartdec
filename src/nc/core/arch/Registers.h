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
#include <nc/common/Foreach.h>
#include <nc/core/ir/MemoryLocation.h>

#include <vector>

#include <QString>
#include <QHash>
#include <QSet>

#include "Register.h"

namespace nc { namespace core { namespace arch {

class Register;

/**
 * Register container.
 */
class Registers {
public:
    enum {
        /** Invalid register. */
        INVALID = -1
    };

    /**
     * Virtual destructor.
     */
    virtual ~Registers() {
        foreach(const core::arch::Register *regizter, mRegisters)
            delete regizter;
    }

    /**
     * \returns                        All registers.
     */
    const std::vector<const Register *> &registers() const {
        return mRegisters;
    }

    /**
     * \param number                   Register number.
     * \returns                        Register for the given number, or NULL if
     *                                 no such register exists.
     */
    const Register *regizter(int number) const {
        if(number < 0 || static_cast<std::size_t>(number) >= mRegisterByNumber.size())
            return NULL;

        return mRegisterByNumber[number];
    }

    /**
     * \param location                 Register's memory location.
     * \returns                        Register for the given memory location,
     *                                 or NULL if there is no such register.
     */
    const Register *regizter(const ir::MemoryLocation &location) const {
        return mRegisterByLocation.value(location, NULL);
    }

    /**
     * \param[in] name                 Register name.
     * \return                         Corresponding register, or NULL if 
     *                                 register with such name doesn't exist.
     */
    const Register *regizter(const QString &name) const {
        return mRegisterByName.value(name, NULL);
    }

    /**
     * \param[in] number               Register number.
     * \return                         True if register is a stack pointer.
     */
    bool isStackPointer(int number) const {
        return mStackPointerNumbers.contains(number);
    }

    /**
     * \param[in] memoryLocation       Memory location.
     * \return                         True if memory location stores a stack pointer.
     */
    bool isStackPointer(const ir::MemoryLocation &memoryLocation) const {
        return mStackPointerLocations.contains(memoryLocation);
    }

protected:
    /**
     * Registers the given register. This register container takes ownership of
     * the given register.
     * 
     * \param[in] regizter             Register to register.
     */
    void registerRegister(Register *regizter) {
        assert(regizter != NULL);
        assert(this->regizter(regizter->number()) == NULL); /* Re-registration not allowed. */

        mRegisters.push_back(regizter);

        mRegisterByName[regizter->lowercaseName()] = regizter;
        mRegisterByName[regizter->uppercaseName()] = regizter;

        if(static_cast<std::size_t>(regizter->number()) >= mRegisterByNumber.size())
            mRegisterByNumber.resize((regizter->number() + 1) * 2);
        mRegisterByNumber[regizter->number()] = regizter;

        mRegisterByLocation[regizter->memoryLocation()] = regizter;
    }

    /**
     * Registers the given register as stack pointer.
     * 
     * \param[in] number               Register number.
     */
    void registerStackPointer(int number) {
        assert(regizter(number) != NULL); /* Register number must be registered. */
        assert(!mStackPointerNumbers.contains(number)); /* Re-registration not allowed. */

        mStackPointerNumbers.insert(number);
        mStackPointerLocations.insert(regizter(number)->memoryLocation());
    }

private:
    /** All registers. */
    std::vector<const Register *> mRegisters;

    /** Register name (uppercase or lowercase) to register number map. */
    QHash<QString, Register *> mRegisterByName;

    /** Register number to register map. */
    std::vector<Register *> mRegisterByNumber;

    /** Map from memory location to register. */
    QHash<ir::MemoryLocation, Register *> mRegisterByLocation;

    /** Set of register numbers that are stack pointers. */
    QSet<int> mStackPointerNumbers;

    /** Set of memory locations that are stack pointers. */
    QSet<ir::MemoryLocation> mStackPointerLocations;

    /** Null string to return reference to in case number was not found. */
    QString mNullString;
};


/**
 * Base class for static register containers.
 */
template<class Derived>
class StaticRegisters: public Registers {
public:
    static const std::vector<const Register *> &registers() {
        return instance()->Registers::registers();
    }

    static const Register *regizter(int number) {
        return instance()->Registers::regizter(number);
    }

    static const Register *regizter(const ir::MemoryLocation &location) {
        return instance()->Registers::regizter(location);
    }

    static const Register *regizter(const QString &name) {
        return instance()->Registers::regizter(name);
    }

    static bool isStackPointer(int number) {
        return instance()->Registers::isStackPointer(number);
    }

    static bool isStackPointer(const ir::MemoryLocation &memoryLocation) {
        return instance()->Registers::isStackPointer(memoryLocation);
    }

    static Derived *instance() {
        return &sInstance;
    }

private:
    static Derived sInstance;
};

template<class Derived>
Derived StaticRegisters<Derived>::sInstance;

}}} // namespace nc::core::arch
