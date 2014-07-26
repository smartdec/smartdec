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

#include <cassert>
#include <memory> /* std::unique_ptr */

#include <nc/common/Types.h>

namespace nc {

class CancellationToken;

namespace core {

namespace image {
    class ByteSource;
}

namespace arch {

class Architecture;
class Instruction;
class Instructions;

namespace disasm {

/**
 * Disassembler for a sequence of instructions.
 */
class Disassembler {
    const Architecture *architecture_; ///< Architecture.
    Instructions *instructions_; ///< Instructions.

public:
    /**
     * Constructor.
     * 
     * \param[in] architecture Valid pointer to the architecture.
     * \param[out] instructions Pointer to the set of instructions where to add disassembled instructions.
     *                          Can be NULL, but then you must override addInstruction().
     */
    Disassembler(const Architecture *architecture, Instructions *instructions):
        architecture_(architecture),
        instructions_(instructions)
    {
        assert(architecture);
    }

    /**
     * \return Valid pointer to the architecture.
     */
    const Architecture *architecture() const { return architecture_; }

    /**
     * \return Pointer to the set of instructions. Can be NULL.
     */
    Instructions *instructions() const { return instructions_; }

    /**
     * Virtual destructor.
     */
    virtual ~Disassembler() {}

    /**
     * Disassembles all instructions in the given range of addresses.
     *
     * \param source Valid pointer to a byte source.
     * \param begin First address in the range.
     * \param end First address past the range.
     * \param canceled Cancellation token.
     */
    virtual void disassemble(const image::ByteSource *source, ByteAddr begin, ByteAddr end, const CancellationToken &canceled);

protected:
    /**
     * This method is called when a yet another instruction has been disassembled.
     * By default, the instruction is added to the set of instructions passed to the constructor.
     *
     * \param instruction Valid pointer to the disassembled instruction.
     */
    virtual void addInstruction(std::shared_ptr<Instruction> instruction);
};

} // namespace disasm
} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
