/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

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

#include <array>
#include <cassert>
#include <cstring>

#include <nc/core/arch/Instruction.h>
#include <nc/core/arch/Operands.h>

#include "IntelOperands.h"

namespace nc {
namespace arch {
namespace intel {

/**
 * An instruction of Intel x86 platform.
 */
class IntelInstruction: public core::arch::Instruction {
public:
    /** Max size of an instruction. */
    static const SmallByteSize MAX_SIZE = 15;

private:
    /** Copy of architecture's bitness value. */
    SmallBitSize bitness_;

    /** Binary representation of the instruction. */
    std::array<uint8_t, MAX_SIZE> bytes_;

public:
    /**
     * Class constructor.
     *
     * \param[in] bytes Valid pointer to the bytes of the instruction.
     * \param[in] mnemonic Instruction mnemonic.
     * \param[in] addr Instruction address in bytes.
     * \param[in] size Instruction size in bytes.
     */
    IntelInstruction(SmallBitSize bitness, ByteAddr addr, SmallByteSize size, const void *bytes):
        core::arch::Instruction(addr, size), bitness_(bitness)
    {
        assert(size > 0);
        assert(size <= MAX_SIZE);
        memcpy(&bytes_, bytes, size);
    }

    /**
     * \return Valid pointer to the buffer containing the binary
     *         representation of the instruction.
     */
    const uint8_t *bytes() const { return &bytes_[0]; }

    virtual void print(QTextStream &out) const override;
};

} // namespace intel
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
