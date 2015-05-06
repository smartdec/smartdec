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
#include <vector>

#include <QString>

#include <nc/common/Printable.h>
#include <nc/common/Types.h>

#include "Mnemonic.h"

namespace nc {
namespace core {
namespace arch {

class Operand;
class Mnemonic;

/**
 * Base class for instructions.
 */
class Instruction: public Printable {
    /** Mnemonic of this instruction. */
    const Mnemonic *mnemonic_;

    /** Instruction's address in bytes. */
    ByteAddr addr_;

    /** Instruction's size in bytes. */
    SmallByteSize size_;

    /** Instruction's operands. */
    std::vector<Operand *> operands_;

public:
    /**
     * Constructor.
     *
     * \param[in] mnemonic  Valid pointer to the mnemonic of this instruction.
     * \param[in] addr      Instruction's address in bytes.
     * \param[in] size      Instruction's size in bytes.
     */
    Instruction(const Mnemonic *mnemonic, ByteAddr addr = 0, SmallByteSize size = 0):
        mnemonic_(mnemonic), addr_(addr), size_(size)
    {
        assert(mnemonic != NULL);
        assert(size >= 0);
    }

    /**
     * Virtual destructor
     */
    virtual ~Instruction();

    /**
     * \return Valid pointer to the mnemonic of this instruction.
     */
    const Mnemonic *mnemonic() const { return mnemonic_; }

    /**
     * Sets mnemonic of this instruction.
     *
     * \param[in] mnemonic Instruction mnemonic.
     */
    void setMnemonic(const Mnemonic *mnemonic) { assert(mnemonic != NULL); mnemonic_ = mnemonic; }

    /**
     * \return Instruction's operands.
     */
    const std::vector<const Operand *> &operands() const {
        return reinterpret_cast<const std::vector<const Operand *> &>(operands_);
    }

    /**
     * \param[in] index Index of an operand.
     *
     * \return Operand for the given index.
     */
    const Operand *operand(std::size_t index) const { assert(index < operands_.size()); return operands_[index]; }

    /**
     * Adds the given operand to the list of operands for this instruction.
     * 
     * \param[in] operand Valid pointer to the operand to add.
     */
    void addOperand(Operand *operand) {
        assert(operand != NULL);
        operands_.push_back(operand);
    }

    /**
     * Inserts the given operand into the list of operands for this instruction
     * at given position.
     * 
     * \param[in] index                Position to insert operand at.
     * \param[in] operand              Operand to insert.
     */
    void addOperand(std::size_t index, Operand *operand) {
        assert(operand != NULL);
        operands_.insert(operands_.begin() + index, operand);
    }

    /**
     * Removes operand at given position.
     *
     * \param[in] index                 Position to remove operand at.
     */
    void removeOperand(std::size_t index);

    /**
     * Replaces an operand at given position.
     * 
     * \param[in] index                Position to replace operand at.
     * \param[in] operand              Operand to replace by.
     */
    void replaceOperand(std::size_t index, Operand *operand);

    /**
     * Clears the operands of this instruction.
     */
    void clearOperands();

    /**
     * \return Name of this instruction.
     */
    virtual const QString &name() const = 0;

    /**
     * \return Instruction's address in bytes.
     */
    ByteAddr addr() const { return addr_; }

    /**
     * Sets this instruction's address.
     *
     * \param[in] addr Address in bytes.
     */
    void setAddr(ByteAddr addr) { addr_ = addr; }

    /**
     * \return Instruction's size in bytes.
     */
    SmallByteSize size() const { return size_; }

    /**
     * Sets the size of this instruction.
     *
     * \param[in] size Size in bytes.
     */
    void setSize(SmallByteSize size) { assert(size > 0); size_ = size; }

    /**
     * \return Address of the next instruction in memory.
     */ 
    ByteAddr endAddr() const { return addr_ + size_; }
};

} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
