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

#include <memory> /* std::unique_ptr */

#include <nc/common/Types.h>

namespace nc {
namespace core {
namespace arch {

class Instruction;

namespace disasm {

/**
 * Disassembler capable of parsing a single instruction.
 */
class InstructionDisassembler {
    public:

    /**
     * Virtual destructor.
     */
    virtual ~InstructionDisassembler() {}

    /**
     * \param[in] pc Program counter value at the beginning of the buffer.
     * \param[in] buffer Valid pointer to a buffer.
     * \param[in] size Buffer size.
     *
     * \return Pointer to the instruction disassembled from the buffer, if disassembling succeeded, or NULL otherwise.
     */
    virtual std::unique_ptr<Instruction> disassemble(ByteAddr pc, const void *buffer, ByteSize size) const = 0;
};

} // namespace disasm
} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
