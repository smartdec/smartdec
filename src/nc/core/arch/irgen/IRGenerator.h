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

namespace ir {
    class BasicBlock;
    class JumpTarget;
    class Program;
    class Term;

    namespace dflow {
        class Dataflow;
    }
}

class Module;

namespace arch {

class Instructions;

namespace irgen {

/**
 * Class for translating assembler programs into intermediate representation.
 */
class IRGenerator {
    const Module *module_; ///< Module.
    const Instructions *instructions_; ///< Module.
    ir::Program *program_; ///< Program.

public:

    /**
     * Constructor.
     *
     * \param[in] module Valid pointer to the module.
     * \param[in] instructions Valid pointer to the set of instructions.
     * \param[out] program Valid pointer to the program.
     */
    IRGenerator(const Module *module, const Instructions *instructions, ir::Program *program):
        module_(module), instructions_(instructions), program_(program)
    {
        assert(module);
        assert(instructions);
        assert(program);
    }

    /**
     * Virtual destructor.
     */
    virtual ~IRGenerator() {}

    /**
     * \return Valid pointer to the module.
     */
    const Module *module() const { return module_; }

    /**
     * \return Valid pointer to the instructions.
     */
    const Instructions *instructions() const { return instructions_; }

    /**
     * \return Output control flow graph.
     */
    ir::Program *program() const { return program_; }

    /**
     * Builds a control flow graph from the set of instructions given in the constructor.
     */
    virtual void generate(const CancellationToken &canceled);

protected:

    /**
     * Computes jump targets in the basic block.
     *
     * \param basicBlock Valid pointer to a basic block.
     */
    virtual void computeJumpTargets(ir::BasicBlock *basicBlock);

    /**
     * Sets the basic block or jump table fields in the jump target,
     * based on the address expression and some guessing.
     *
     * \param[in,out] target   Jump target.
     * \param[in]     dataflow Dataflow information collected up to the point where jump has been met.
     */
    virtual void computeJumpTarget(ir::JumpTarget &target, const ir::dflow::Dataflow &dataflow);

    /**
     * Determines jump table address and recovers its entries in a form of a vector of addresses.
     *
     * \param[in] target Valid pointer to a term representing the jump target.
     * \param[in] dataflow Dataflow information collected up to the point where jump has been met.
     *
     * \returns The entries of the jump table.
     */
    virtual std::vector<ByteAddr> getJumpTableEntries(const ir::Term *target, const ir::dflow::Dataflow &dataflow);

    /**
     * Adds a jump to direct successor to given basic block if the latter
     * does not have a terminator yet.
     *
     * \param basicBlock Valid pointer to a basic block.
     */
    virtual void addJumpToDirectSuccessor(ir::BasicBlock *basicBlock);
};

} // namespace irgen
} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
