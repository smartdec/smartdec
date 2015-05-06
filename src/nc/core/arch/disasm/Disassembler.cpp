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

#include "Disassembler.h"

#include <algorithm> /* std::max() */
#include <memory>

#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/Instructions.h>
#include <nc/core/image/ByteSource.h>

#include <nc/common/CancellationToken.h>
#include <nc/common/Warnings.h>

#include "InstructionDisassembler.h"

namespace nc {
namespace core {
namespace arch {
namespace disasm {

void Disassembler::disassemble(const image::ByteSource *source, ByteAddr begin, ByteAddr end, const CancellationToken &canceled) {
    assert(source != NULL);

    SmallByteSize maxInstructionSize = architecture()->maxInstructionSize();

    const ByteSize bufferCapacity = std::max(4096, maxInstructionSize);
    std::unique_ptr<char[]> buffer(new char[bufferCapacity]);

    ByteOffset bufferOffset = 0;
    ByteSize bufferSize = 0;

    while (begin < end && !canceled) {
        if (bufferOffset + maxInstructionSize > bufferSize && end - begin > bufferSize - bufferOffset) {
            bufferOffset = 0;
            bufferSize = source->readBytes(begin, buffer.get(), std::min(end - begin, bufferCapacity));
        }
        if (bufferOffset >= bufferSize) {
            break;
        }

        auto instruction = disassembleInstruction(begin, buffer.get() + bufferOffset, bufferSize - bufferOffset);

        ByteSize instructionSize = 1;
        if (instruction) {
            if (!instruction->size()) {
                ncWarning("Size of instruction at address %1 is undefined. Aborting.", instruction->addr());
                return;
            }
            instructionSize = instruction->size();
            addInstruction(std::move(instruction));
        }

        begin += instructionSize;
        bufferOffset += instructionSize;
    }
}

std::unique_ptr<Instruction> Disassembler::disassembleInstruction(ByteAddr pc, const void *buffer, SmallByteSize size) {
    if (!architecture()->instructionDisassembler()) {
        ncWarning("Architecture does not define a disassembler for a single instruction.");
        return NULL;
    }

    return architecture()->instructionDisassembler()->disassemble(pc, buffer, size);
}

void Disassembler::addInstruction(std::unique_ptr<Instruction> instruction) {
    assert(instruction);
    assert(instructions());

    instructions()->add(std::move(instruction));
}

} // namespace disasm
} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
