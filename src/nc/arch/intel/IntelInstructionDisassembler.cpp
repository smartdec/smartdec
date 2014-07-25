/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "IntelInstructionDisassembler.h"

#include <nc/common/CheckedCast.h>

#include "IntelArchitecture.h"
#include "IntelInstruction.h"

#include "udis86.h"

namespace nc {
namespace arch {
namespace intel {

std::shared_ptr<core::arch::Instruction> IntelInstructionDisassembler::disassemble(ByteAddr pc, const void *buffer, ByteSize size) const {
    ud_t ud_obj;
    ud_init(&ud_obj);
    ud_set_mode(&ud_obj, architecture_->bitness());
    ud_set_pc(&ud_obj, pc);
    ud_set_input_buffer(&ud_obj, const_cast<uint8_t *>(static_cast<const uint8_t *>(buffer)), checked_cast<std::size_t>(size));

    auto instructionSize = ud_disassemble(&ud_obj);
    if (!instructionSize || ud_obj.mnemonic == UD_Iinvalid) {
        return NULL;
    }

    return std::make_shared<IntelInstruction>(architecture_->bitness(), pc, instructionSize, buffer);
}

} // namespace intel
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
