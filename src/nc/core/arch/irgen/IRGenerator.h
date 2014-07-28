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
#include <vector>

#include <nc/common/CancellationToken.h>
#include <nc/common/Types.h>

namespace nc {
namespace core {

namespace image {
    class Image;
}

namespace ir {
    class BasicBlock;
    class JumpTarget;
    class Program;
    class Term;

    namespace dflow {
        class Dataflow;
    }
}

namespace arch {

class Disassembler;
class Instructions;

namespace irgen {

/**
 * Class for translating assembler programs into intermediate representation.
 */
class IRGenerator {
    const image::Image *image_; ///< Executable image.
    const Instructions *instructions_; ///< Instructions.
    ir::Program *program_; ///< Program.
    std::unique_ptr<Disassembler> disassembler_; ///< Disassembler.

public:
    /**
     * Constructor.
     *
     * \param[in] image Valid pointer to the executable image.
     * \param[in] instructions Valid pointer to the set of instructions.
     * \param[out] program Valid pointer to the program.
     */
    IRGenerator(const image::Image *image, const Instructions *instructions, ir::Program *program);

    /**
     * Destructor.
     */
    ~IRGenerator();

    /**
     * Builds a program control flow graph from the instructions
     * given to the constructor.
     */
    void generate(const CancellationToken &canceled);

private:
    /**
     * Computes jump targets in the basic block.
     *
     * \param basicBlock Valid pointer to a basic block.
     */
    void computeJumpTargets(ir::BasicBlock *basicBlock);

    /**
     * Sets the basic block or jump table fields in the jump target,
     * based on the address expression and some guessing.
     *
     * \param[in,out] target   Jump target.
     * \param[in]     dataflow Dataflow information collected up to the point where jump has been met.
     */
    void computeJumpTarget(ir::JumpTarget &target, const ir::dflow::Dataflow &dataflow);

    /**
     * Determines jump table address and recovers its entries in a form of a vector of addresses.
     *
     * \param[in] target Valid pointer to a term representing the jump target.
     * \param[in] dataflow Dataflow information collected up to the point where jump has been met.
     *
     * \returns The entries of the jump table.
     */
    std::vector<ByteAddr> getJumpTableEntries(const ir::Term *target, const ir::dflow::Dataflow &dataflow);

    /**
     * \param address A virtual address.
     *
     * \return True if the address seems to be an instruction address, false otherwise.
     */
    bool isInstructionAddress(ByteAddr address);

    /**
     * Adds a jump to direct successor to given basic block if the latter
     * does not have a terminator yet.
     *
     * \param basicBlock Valid pointer to a basic block.
     */
    void addJumpToDirectSuccessor(ir::BasicBlock *basicBlock);
};

} // namespace irgen
} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
