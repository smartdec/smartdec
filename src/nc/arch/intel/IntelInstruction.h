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

#include <nc/core/arch/Instruction.h>
#include <nc/core/arch/Operands.h>

#include "IntelOperands.h"
#include "Prefixes.h"

namespace nc {
namespace arch {
namespace intel {

typedef int Prefixes; ///< Prefix bit mask.

/**
 * An instruction of Intel x86 platform.
 */
class IntelInstruction: public core::arch::Instruction {
    /** Bit mask of prefixes. */
    Prefixes prefixes_;

    /** Operand size of the instruction. */
    SmallBitSize operandSize_;

    /** Address size of the instruction. */
    SmallBitSize addressSize_;

public:
    /**
     * Class constructor.
     *
     * \param[in] mnemonic Instruction mnemoic.
     * \param[in] addr Instruction address in bytes.
     * \param[in] size Instruction size in bytes.
     * \param[in] prefixes Bit mask of instruction prefixes.
     */
    IntelInstruction(const core::arch::Mnemonic *mnemonic, ByteAddr addr = 0, SmallByteSize size = 0, Prefixes prefixes = 0):
        core::arch::Instruction(mnemonic, addr, size), 
        prefixes_(prefixes),
        operandSize_(0),
        addressSize_(0)
    {}

    /**
     * \return Bit mask of instruction prefixes.
     */
    Prefixes prefixes() const { return prefixes_; }

    /**
     * Sets instruction prefixes.
     *
     * \arg prefixes Bit mask of prefixes.
     */
    void setPrefixes(Prefixes prefixes) { prefixes_ = prefixes; }

    /**
     * \return Operand size.
     */
    SmallBitSize operandSize() const { return operandSize_; }

    /**
     * Sets the operand size.
     *
     * \param operandSize New operand size.
     */
    void setOperandSize(SmallBitSize operandSize) { operandSize_ = operandSize; }

    /**
     * \return Address size.
     */
    SmallBitSize addressSize() const { return addressSize_; }

    /**
     * Sets the address size.
     *
     * \param addressSize New address size.
     */
    void setAddressSize(SmallBitSize addressSize) { addressSize_ = addressSize; }

    virtual const QString &name() const override;

    virtual void print(QTextStream &out) const override;
};

} // namespace intel
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
