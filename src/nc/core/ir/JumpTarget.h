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

#include <memory>
#include <vector>

#include <nc/common/Printable.h>
#include <nc/common/Types.h>

namespace nc {
namespace core {
namespace ir {

class BasicBlock;
class Term;

/**
 * Entry in a jump table.
 */
class JumpTableEntry {
    /** Jump target address. */
    ByteAddr address_;

    /** Target basic block. */
    BasicBlock *basicBlock_;

    public:

    /**
     * Constructor.
     *
     * \param address    Jump target address.
     * \param basicBlock Pointer to the target basic block. Can be NULL.
     * */
    JumpTableEntry(ByteAddr address, BasicBlock *basicBlock = NULL):
        address_(address), basicBlock_(basicBlock)
    {}

    /**
     * \return Jump target address.
     */
    ByteAddr address() const { return address_; }

    /**
     * \return Target basic block.
     */
    BasicBlock *basicBlock() const { return basicBlock_; }

    /**
     * Sets the target basic block.
     *
     * \param basicBlock Pointer to the new target basic block. Can be NULL.
     */
    void setBasicBlock(BasicBlock *basicBlock) { basicBlock_ = basicBlock; }
};

typedef std::vector<JumpTableEntry> JumpTable;

/**
 * Jump target: an address, a basic block, or a vector of basic blocks (jump table).
 */
class JumpTarget: public PrintableBase<JumpTarget> {
    /** Term representing the jump address. */
    std::unique_ptr<Term> address_;

    /** Target basic block. */
    BasicBlock *basicBlock_;

    /** Jump table. */
    std::unique_ptr<JumpTable> table_;

    public:

    /**
     * Constructs an invalid jump target.
     */
    JumpTarget();

    /**
     * Constructor.
     *
     * \param address Valid pointer to the term representing the jump address.
     */
    JumpTarget(std::unique_ptr<Term> address);

    /**
     * Constructor.
     *
     * \param basicBlock Valid pointer to the target basic block.
     */
    JumpTarget(BasicBlock *basicBlock);

    /**
     * Copy constructor.
     *
     * \param[in] other Object to copy from.
     */
    JumpTarget(const JumpTarget &other);

    /**
     * Move constructor.
     *
     * \param[in] other Object to move from.
     */
    JumpTarget(JumpTarget &&other);

    /**
     * Destructor.
     */
    ~JumpTarget();

    /**
     * \return Pointer to the term representing the jump address. Can be NULL.
     */
    Term *address() { return address_.get(); }

    /**
     * \return Pointer to the term representing the jump address. Can be NULL.
     */
    const Term *address() const { return address_.get(); }

    /**
     * Sets the target address.
     *
     * \param address Valid pointer to the term representing the jump address.
     */
    void setAddress(std::unique_ptr<Term> address);

    /**
     * \return Pointer to the target basic block. Can be NULL.
     */
    BasicBlock *basicBlock() const { return basicBlock_; }

    /**
     * Sets the target basic block.
     *
     * \param basicBlock Pointer to the target basic block. Can be NULL.
     */
    void setBasicBlock(BasicBlock *basicBlock) { basicBlock_ = basicBlock; }

    /**
     * \return Pointer to the jump table. Can be NULL.
     */
    JumpTable *table() { return table_.get(); }

    /**
     * \return Pointer to the jump table. Can be NULL.
     */
    const JumpTable *table() const { return table_.get(); }

    /**
     * Sets the jump table.
     *
     * \param[in] table Pointer to the new jump table. Can be NULL.
     */
    void setTable(std::unique_ptr<JumpTable> table) { table_ = std::move(table); }

    /**
     * \return Non-null pointer is this is a valid jump target, NULL otherwise.
     */
    operator const void*() { return (address() || basicBlock() || table()) ? this : NULL; }

    /**
     * Prints the textual representation of the jump target to the stream.
     *
     * \param out Output stream.
     */
    void print(QTextStream &out) const;
};

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
