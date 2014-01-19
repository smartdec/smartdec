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
#include <memory>
#include <vector>

#include <boost/noncopyable.hpp>

#include <nc/common/Printable.h>

namespace nc {
namespace core {
namespace ir {

class BasicBlock;

/**
 * Intermediate representation of a function.
 */
class Function: public PrintableBase<Function>, boost::noncopyable {
    BasicBlock *entry_; ///< Entry basic block.
    std::vector<std::unique_ptr<BasicBlock>> basicBlocks_; ///< All basic blocks of the function.

public:
    /**
     * Class constructor.
     */
    Function();

    /**
     * Class destructor.
     */
    ~Function();

    /**
     * \return Pointer to the entry basic block. Can be NULL.
     */
    BasicBlock *entry() const { return entry_; }

    /**
     * Sets the function's entry.
     *
     * \param entry Valid pointer to the new entry basic block.
     */
    void setEntry(BasicBlock *entry) {
        assert(entry != NULL && "Function's entry must be not NULL.");
        entry_ = entry;
    }

    /**
     * \return All basic blocks of the function.
     */
    const std::vector<BasicBlock *> &basicBlocks() {
        return reinterpret_cast<const std::vector<BasicBlock *> &>(basicBlocks_);
    }

    /**
     * \return All basic blocks of the function.
     */
    const std::vector<const BasicBlock *> &basicBlocks() const {
        return reinterpret_cast<const std::vector<const BasicBlock *> &>(basicBlocks_);
    }

    /**
     * Adds basic block to the function. The function doesn't have ownership of basic blocks.
     *
     * \param basicBlock Basic block.
     */
    void addBasicBlock(std::unique_ptr<BasicBlock> basicBlock);

    /**
     * \return True iff this function has no statements in its basic blocks.
     */
    bool isEmpty() const;

    /**
     * Prints the representation of the function in DOT format into a stream.
     *
     * \param[in] out Output stream.
     */
    void print(QTextStream &out) const;
};

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
