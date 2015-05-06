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

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

#include <nc/common/Printable.h>
#include <nc/common/Types.h>

namespace nc {
namespace core {
namespace ir {

class Jump;
class Return;
class Statement;

/**
 * Basic block of a function.
 */
class BasicBlock: public PrintableBase<BasicBlock>, boost::noncopyable {
    boost::optional<ByteAddr> address_; ///< Address of basic block.
    boost::optional<ByteAddr> successorAddress_; ///< Address of the end of the basic block.
    std::vector<std::unique_ptr<Statement>> statements_; ///< Statements.

    public:

    /**
     * Constructor.
     *
     * \param[in] address Address of the basic block.
     */
    BasicBlock(const boost::optional<ByteAddr> &address = boost::none);

    /**
     * Destructor.
     */
    ~BasicBlock();

    /**
     * \return Address of the basic block.
     */
    const boost::optional<ByteAddr> &address() const { return address_; }

    /**
     * \return Address of the basic block's successor.
     */
    const boost::optional<ByteAddr> &successorAddress() const { return successorAddress_; }

    /**
     * Sets the address of the basic block's successor.
     *
     * \param[in] successorAddress New successor address.
     */
    void setSuccessorAddress(const boost::optional<ByteAddr> &successorAddress);

    /**
     * \return Statements of the basic block.
     */
    const std::vector<Statement *> &statements() {
        return reinterpret_cast<const std::vector<Statement *> &>(statements_);
    }

    /**
     * \return Statements of the basic block.
     */
    const std::vector<const Statement *> &statements() const {
        return reinterpret_cast<const std::vector<const Statement *> &>(statements_);
    }

    /**
     * Adds a statement to the end of the basic block.
     *
     * \param statement Valid pointer to the statement being added.
     */
    void addStatement(std::unique_ptr<Statement> statement);

    /**
     * Adds given statements after given existing statements.
     *
     * \param addedStatements   List of pairs (existing statement, added statement).
     *                          First components of the pairs must appear in the basic
     *                          block in the same order as they appear in this list.
     */
    void addStatements(std::vector<std::pair<const Statement *, std::unique_ptr<Statement>>> &&addedStatements);

    /**
     * Removes the last statement of the basic block.
     */
    void popBack();

    /**
     * \return Valid pointer to the last statement in the basic block if this
     *         is a jump or return, NULL otherwise.
     */
    const Statement *getTerminator() const;

    /**
     * \return Valid pointer to the last statement in the basic block if this
     *         is a jump, NULL otherwise.
     */
    Jump *getJump();

    /**
     * \return Valid pointer to the last statement in the basic block if this
     *         is a jump, NULL otherwise.
     */
    const Jump *getJump() const;

    /**
     * \return Valid pointer to the last statement in the basic block if this
     *         is a return, NULL otherwise.
     */
    const Return *getReturn() const;

    /**
     * Breaks the basic block into two parts. First part contains first 'index'
     * statements and remains in the original basic block. Second part
     * containing all other statements is moved to newly created
     * basic block.
     *
     * \param[in] index Index of the first statements of the second part.
     * \param[in] address Start address to give to the newly created basic block.
     *
     * \return Valid pointer to the newly created basic block containing the
     * second part of statements.
     */
    std::unique_ptr<BasicBlock> split(std::size_t index, const boost::optional<ByteAddr> &address);

    /**
     * \return A valid pointer to the basic block which is a copy of this one.
     */
    std::unique_ptr<BasicBlock> clone() const;

    /**
     * Prints the basic block and its incoming edges into a stream in DOT format.
     *
     * \param out Output stream.
     */
    void print(QTextStream &out) const;
};

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
