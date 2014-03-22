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

#include <memory>

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

#include <nc/common/Printable.h>
#include <nc/common/Types.h>
#include <nc/common/ilist.h>

namespace nc {
namespace core {
namespace ir {

class Function;
class Jump;
class Return;
class Statement;

/**
 * Basic block.
 */
class BasicBlock: public PrintableBase<BasicBlock>, boost::noncopyable {
    boost::optional<ByteAddr> address_; ///< Address of basic block.
    boost::optional<ByteAddr> successorAddress_; ///< Address of the end of the basic block.
    ilist<Statement> statements_; ///< Statements.
    const Function *function_; ///< Function this basic block belongs to.

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
     * \return Pointer to the function this basic block belongs to. Can be NULL.
     */
    const Function *function() const { return function_; }

    /**
     * Sets the function this basic block belongs to.
     *
     * \param function Pointer to the function. Can be NULL.
     */
    void setFunction(const Function *function) { function_ = function; }

    /**
     * \return Statements of the basic block.
     */
    ilist<Statement> &statements() { return statements_; }

    /**
     * \return Statements of the basic block.
     */
    const ilist<Statement> &statements() const { return statements_; }

    /**
     * Inserts a statement at the given position.
     *
     * \param position Position where the statement must be inserted.
     * \param statement Valid pointer to the statement being inserted.
     */
    void insertStatement(ilist<Statement>::const_iterator position, std::unique_ptr<Statement> statement);

    /**
     * Adds a statement to the end of the basic block.
     *
     * \param statement Valid pointer to the statement being added.
     */
    void addStatement(std::unique_ptr<Statement> statement);

    /**
     * Inserts a statement after a given one.
     *
     * \param after Valid pointer to the statement after which to insert.
     * \param statement Valid pointer to the statement being inserted.
     */
    void insertStatementAfter(const Statement *after, std::unique_ptr<Statement> statement);

    /**
     * Removes the last statement in the basic block.
     * The basic block must be not empty.
     *
     * \return Valid pointer to the removed statement.
     */
    std::unique_ptr<Statement> popBack();

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
     * Breaks the basic block into two parts. The first part contains the
     * statements before the given position and remains in the original basic
     * block. The second part includes remaining statements and is moved to a
     * newly created basic block.
     *
     * \param[in] position Iterator identifying the first statement of the second part.
     * \param[in] address Start address to give to the newly created basic block.
     *
     * \return Valid pointer to the newly created basic block containing the
     *         second part of statements.
     */
    std::unique_ptr<BasicBlock> split(ilist<Statement>::const_iterator position, const boost::optional<ByteAddr> &address);

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
