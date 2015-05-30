/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

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

#include "X86Disassembler.h"

#include <nc/common/CheckedCast.h>
#include <nc/common/make_unique.h>
#include <nc/common/Unreachable.h>

#include "X86Architecture.h"
#include "X86Instruction.h"

namespace nc {
namespace arch {
namespace x86 {

X86Disassembler::X86Disassembler(const X86Architecture *architecture): core::arch::Disassembler(architecture) {
    if (architecture->bitness() == 16) {
        mode_ = CS_MODE_16;
    } else if (architecture->bitness() == 32) {
        mode_ = CS_MODE_32;
    } else if (architecture->bitness() == 64) {
        mode_ = CS_MODE_64;
    } else {
        unreachable();
    }
    capstone_ = std::make_unique<core::arch::Capstone>(CS_ARCH_X86, mode_);
}

std::shared_ptr<core::arch::Instruction> X86Disassembler::disassembleSingleInstruction(ByteAddr pc, const void *buffer, ByteSize size) {
    if (auto instr = capstone_->disassemble(pc, buffer, size, 1)) {
        return std::make_shared<X86Instruction>(mode_, instr->address, instr->size, buffer);
    }
    return nullptr;
}

} // namespace x86
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
