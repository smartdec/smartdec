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

#include <nc/core/arch/disasm/InstructionDisassembler.h>

namespace nc {
namespace arch {
namespace intel {

class IntelArchitecture;
class IntelInstructionDisassemblerPrivate;

/**
 * Disassembler for x86 instructions.
 */
class IntelInstructionDisassembler: public core::arch::disasm::InstructionDisassembler {
    std::unique_ptr<IntelInstructionDisassemblerPrivate> impl_;

    public:

    /**
     * Constructor.
     *
     * \param architecture Valid pointer to Intel architecture.
     */
    IntelInstructionDisassembler(const IntelArchitecture *architecture);

    /**
     * Virtual destructor.
     */
    virtual ~IntelInstructionDisassembler();

    std::unique_ptr<core::arch::Instruction> disassemble(ByteAddr pc, const void *buffer, ByteSize size) const override;
};

} // namespace intel
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
