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

#include <boost/mpl/int.hpp>

#include <nc/common/Kinds.h>
#include <nc/common/Printable.h>
#include <nc/common/Types.h>

namespace nc { namespace core { namespace arch {

class RegisterOperand;
class AdditionOperand;
class MultiplicationOperand;
class DereferenceOperand;
class BitRangeOperand;
class ConstantOperand;

/**
 * Base class for instruction operands.
 * 
 * Operands are supposed to be immutable. That is, once created and initialized, 
 * they cannot be changed.
 */
class Operand: public Printable {
    NC_CLASS_WITH_KINDS(Operand, kind)

public:
    /** 
     * Operand kind. 
     */
    enum {
        REGISTER,
        ADDITION,
        MULTIPLICATION,
        DEREFERENCE,
        BIT_RANGE,
        CONSTANT,
        USER = 1000
    };

    /**
     * Constructor.
     *
     * \param[in] kind                 Operand kind.
     * \param[in] isCached             Whether this operand is cached.
     * \param[in] size                 Operand size in bits.
     */
    Operand(int kind, bool isCached, SmallBitSize size): 
        kind_(kind), 
        isCached_(isCached), 
        size_(size) 
    {
        /*
         * Zero size can sometimes be valid.
         * An example is a dereference in x86 LEA instruction.
         */
        assert(size >= 0);
    }

    /**
     * \return                         Operand size in bits.
     */
    SmallBitSize size() const { return size_; }

    /**
     * \return                         Whether this operand is cached.
     */
    bool isCached() const { return isCached_; }

    inline bool isRegister() const;
    inline bool isAddition() const;
    inline bool isMultiplication() const;
    inline bool isDereference() const;
    inline bool isBitRange() const;
    inline bool isConstant() const;

    /* The following functions are defined in Operands.h. */

    inline const RegisterOperand *asRegister() const;
    inline const AdditionOperand *asAddition() const;
    inline const MultiplicationOperand *asMultiplication() const;
    inline const DereferenceOperand *asDereference() const;
    inline const BitRangeOperand *asBitRange() const;
    inline const ConstantOperand *asConstant() const;
    inline bool isRegister(int registerNumber) const;

protected:
    friend class Architecture; /* Architecture can delete operands directly. */
    friend class Instruction; /* Instruction calls dispose(). */

    /**
     * Protected destructor.
     * 
     * Operands are to be deleted with a call to <tt>dispose()</tt>.
     */
    ~Operand() {}

    /**
     * Deallocates this operand if it is owned by the enclosing instruction.
     * Does nothing otherwise.
     */
    void dispose() {
        if(!isCached_)
            delete this;
    }

    /**
     * Static version of <tt>dispose()</tt> function, to make it available to derived classes.
     * 
     * \param[in] operand              Operand to dispose.
     */
    static void dispose(Operand *operand) {
        operand->dispose();
    }

    /**
     * \param[in] isCached             Whether this operand is cached.
     */
    void setCached(bool isCached) {
        isCached_ = isCached;
    }

private:
    bool isCached_; ///< Whether this operand is cached.
    SmallBitSize size_; ///< Operand size in bits.
};

}}} // namespace nc::core::arch

/**
 * Defines a compile-time mapping from operand class to operand kind.
 * Makes it possible to use the given class as an argument to <tt>Operand::as</tt>
 * and <tt>Operand::is</tt> template functions.
 * 
 * Must be used at global namespace.
 * 
 * \param CLASS                        Operand class.
 * \param KIND                         Operand kind.
 */
#define NC_REGISTER_OPERAND_CLASS(CLASS, KIND)                                  \
    NC_REGISTER_CLASS_KIND(nc::core::arch::Operand, CLASS, KIND)

NC_REGISTER_OPERAND_CLASS(nc::core::arch::RegisterOperand,       nc::core::arch::Operand::REGISTER)
NC_REGISTER_OPERAND_CLASS(nc::core::arch::AdditionOperand,       nc::core::arch::Operand::ADDITION)
NC_REGISTER_OPERAND_CLASS(nc::core::arch::MultiplicationOperand, nc::core::arch::Operand::MULTIPLICATION)
NC_REGISTER_OPERAND_CLASS(nc::core::arch::DereferenceOperand,    nc::core::arch::Operand::DEREFERENCE)
NC_REGISTER_OPERAND_CLASS(nc::core::arch::BitRangeOperand,       nc::core::arch::Operand::BIT_RANGE)
NC_REGISTER_OPERAND_CLASS(nc::core::arch::ConstantOperand,       nc::core::arch::Operand::CONSTANT)

namespace nc { namespace core { namespace arch {

bool Operand::isRegister() const {
    return is<RegisterOperand>();
}

bool Operand::isAddition() const {
    return is<AdditionOperand>();
}

bool Operand::isMultiplication() const {
    return is<MultiplicationOperand>();
}

bool Operand::isDereference() const {
    return is<DereferenceOperand>();
}

bool Operand::isBitRange() const {
    return is<BitRangeOperand>();
}

bool Operand::isConstant() const {
    return is<ConstantOperand>();
}

}}} // namespace nc::core::arch

/* vim:set et sts=4 sw=4: */
