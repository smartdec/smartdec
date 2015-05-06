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

#include <string>

#include <boost/array.hpp>

#include <nc/common/SizedValue.h>
#include <nc/common/Printable.h>
#include <nc/common/Types.h>
#include <nc/common/CheckedCast.h>

#include <nc/core/ir/MemoryLocation.h>

#include "Operand.h"
#include "Register.h"

namespace nc { namespace core { namespace arch {

/* TODO: move print() out of Operand. 
 * It is assembler- and architecture- dependent. */

/**
 * Cached operand.
 */
class CachedOperand: public Operand {
public:
    CachedOperand(int kind, SmallBitSize size): Operand(kind, true, size) {}
};


/**
 * Non-cached operand.
 */
class NonCachedOperand: public Operand {
public:
    NonCachedOperand(int kind, SmallBitSize size): Operand(kind, false, size) {}
};


/**
 * Operand with single argument.
 */
class UnaryOperand: public NonCachedOperand {
private:
    Operand *operand_; ///< Operand.

public:
    /**
     * Class constructor.
     *
     * \param[in] kind                 Kind of this operand.
     * \param[in] operand              Operand.
     * \param[in] size                 Size of this operand.
     */
    UnaryOperand(int kind, SmallBitSize size, Operand *operand):
        NonCachedOperand(kind, size), 
        operand_(operand)
    {
        assert(operand != NULL);
    }

    /**
     * Virtual destructor.
     */
    virtual ~UnaryOperand() {
        dispose(operand_);
    }

    /**
     * \return                         Operand.
     */
    const Operand *operand() const { return operand_; }
};


/**
 * Operand with two arguments.
 */
class BinaryOperand: public NonCachedOperand {
private:
    boost::array<Operand *, 2> operands_; ///< Operands.

public:
    /**
     * Class constructor.
     *
     * \param[in] kind                 Operand kind.
     * \param[in] size                 Size of addition result in bits.
     * \param[in] left                 Left summand.
     * \param[in] right                Right summand.
     */
    BinaryOperand(int kind, SmallBitSize size, Operand *left, Operand *right):
        NonCachedOperand(kind, size)
    {
        assert(left != NULL && right != NULL);

        operands_[0] = left;
        operands_[1] = right;
    }

    virtual ~BinaryOperand() {
        dispose(operands_[0]);
        dispose(operands_[1]);
    }

    /**
     * \return                         Left operand.
     */
    const Operand *left() const { return operands_[0]; }

    /**
     * \return                         Right operand.
     */
    const Operand *right() const { return operands_[1]; }
};


/**
 * Register operand.
 */
class RegisterOperand: public CachedOperand {
    const Register *register_; ///< Register.

public:
    /**
     * \returns                        Register that this operand corresponds to.
     */
    const Register *regizter() const {
        return register_;
    }

    /**
     * \returns                        Register number.
     */
    int number() const { return register_->number(); }

    /**
     * \return                         Corresponding abstract memory location of the register.
     */
    const ir::MemoryLocation &memoryLocation() const { return register_->memoryLocation(); }

    virtual void print(QTextStream &out) const override;

protected:
    friend class Architecture; /* Calls constructor. */

    /**
     * Class constructor.
     * 
     * Is not supposed to be called from outside IntelArchitecture.
     *
     * \param[in] regizter             Register.
     */
    RegisterOperand(const Register *regizter): 
        CachedOperand(REGISTER, regizter->memoryLocation().size<SmallBitSize>()),
        register_(regizter)
    {}
};


/**
 * Sum of two operands.
 */
class AdditionOperand: public BinaryOperand {
public:
    /**
     * Class constructor.
     *
     * \param[in] left                 Left summand.
     * \param[in] right                Right summand.
     * \param[in] size                 Size of addition result in bits.
     */
    AdditionOperand(Operand *left, Operand *right, SmallBitSize size):
        BinaryOperand(ADDITION, size, left, right)
    {}

    virtual void print(QTextStream &out) const override;
};


/**
 * Multiplication of two operands.
 */
class MultiplicationOperand: public BinaryOperand {
public:
    /**
     * Class constructor.
     *
     * \param[in] left                 Left multiplier.
     * \param[in] right                Right multiplier.
     * \param[in] size                 Size of multiplication result in bits.
     */
    MultiplicationOperand(Operand *left, Operand *right, SmallBitSize size):
        BinaryOperand(MULTIPLICATION, size, left, right) 
    {}

    virtual void print(QTextStream &out) const override;
};


/**
 * Dereference of a memory address.
 */
class DereferenceOperand: public UnaryOperand {
public:
    /**
     * Class constructor.
     *
     * \param[in] operand              Memory address dereferenced.
     * \param[in] size                 Size of memory access.
     */
    DereferenceOperand(Operand *operand, SmallBitSize size):
        UnaryOperand(DEREFERENCE, size, operand)
    {
        assert(operand != NULL);
    }

    virtual void print(QTextStream &out) const override;
};


/**
 * Operand that is a bit subrange of another operand.
 */
class BitRangeOperand: public UnaryOperand {
    SmallBitOffset offset_;

public:
    /**
     * Class constructor.
     *
     * For example, to get the operand that is the 3rd bit of some 
     * other operand, pass <tt>offset = 2</tt> and <tt>size = 1</tt>.
     *
     * \param[in] operand              Operand.
     * \param[in] offset               Bit offset.
     * \param[in] size                 Size of the bit range.
     */
    BitRangeOperand(Operand *operand, SmallBitOffset offset, SmallBitSize size):
        UnaryOperand(BIT_RANGE, size, operand),
        offset_(offset)
    {
        assert(operand != NULL);
        assert(size + offset <= operand->size());
    }

    /**
     * \returns                        Bit offset.
     */
    SmallBitOffset offset() const {
        return offset_;
    }

    virtual void print(QTextStream &out) const override;
};


/**
 * Constant operand.
 */
class ConstantOperand: public CachedOperand {
    SizedValue value_; ///< Constant value.

public:
    /**
     * \return                         Constant value.
     */
    SizedValue value() const { return value_; }

    virtual void print(QTextStream &out) const override;

protected:
    friend class Architecture; /* Calls constructor. */

    /**
     * Class constructor.
     * 
     * Is not supposed to be called from outside Architecture.
     *
     * \param[in] value                Constant value.
     */
    ConstantOperand(SizedValue value): CachedOperand(CONSTANT, value.size()), value_(value) {}
};


/*
 * Operand implementation follows.
 */

bool Operand::isRegister(int registerNumber) const {
    return isRegister() && as<RegisterOperand>()->number() == registerNumber;
}

const RegisterOperand *Operand::asRegister() const {
    return as<RegisterOperand>();
}

const AdditionOperand *Operand::asAddition() const {
    return as<AdditionOperand>();
}

const MultiplicationOperand *Operand::asMultiplication() const {
    return as<MultiplicationOperand>();
}

const DereferenceOperand *Operand::asDereference() const {
    return as<DereferenceOperand>();
}

const BitRangeOperand *Operand::asBitRange() const {
    return as<BitRangeOperand>();
}

const ConstantOperand *Operand::asConstant() const {
    return as<ConstantOperand>();
}

}}} // namespace nc::core::arch

/* vim:set et sts=4 sw=4: */

