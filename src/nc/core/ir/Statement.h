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

#include <boost/noncopyable.hpp>

#include <QString>

#include <nc/common/Kinds.h>
#include <nc/common/Printable.h>
#include <nc/common/Visitor.h>

namespace nc {
namespace core {

namespace arch {
    class Instruction;
}

namespace ir {

class BasicBlock;
class Term;

class Comment;
class InlineAssembly;
class Assignment;
class Kill;
class Jump;
class Call;
class Return;

/**
 * Base class for different kinds of statements of intermediate representation.
 * 
 * Statements are supposed to be immutable <i>at the interface level</i>.
 */
class Statement: public Printable, boost::noncopyable {
    NC_CLASS_WITH_KINDS(Statement, kind)

public:
    /**
     * Statement kind.
     */
    enum {
        COMMENT,        ///< Comment.
        INLINE_ASSEMBLY,///< Inline assembly.
        ASSIGNMENT,     ///< Assignment.
        KILL,           ///< Killing of a reaching definition.
        JUMP,           ///< Jump to an address.
        CALL,           ///< Function call.
        RETURN,         ///< Return from function call.
        USER = 1000     ///< Base for user-defined statements.
    };

private:
    BasicBlock *basicBlock_; ///< Basic block this statement is a part of.
    const arch::Instruction *instruction_; ///< Instruction from which this statement was generated.

public:
    /**
     * Class constructor.
     *
     * \param[in] kind Kind of the statement.
     */
    Statement(int kind): kind_(kind), basicBlock_(NULL), instruction_(NULL) {}

    /**
     * \return Basic block that this statement is a part of.
     */
    BasicBlock *basicBlock() const { return basicBlock_; }

    /**
     * Sets basic block that this statement is a part of.
     *
     * \param[in] basicBlock Basic block.
     */
    void setBasicBlock(BasicBlock *basicBlock) { basicBlock_ = basicBlock; }

    /**
     * \param[in] instruction Instruction that this statement was generated from.
     */
    void setInstruction(const arch::Instruction *instruction) {
        assert(instruction);
        assert(!instruction_); /* Must be used for initialization only. */

        instruction_ = instruction;
    }

    /**
     * \return Instruction that this statement was generated from. Can be NULL.
     */
    const arch::Instruction *instruction() const { return instruction_; }

    /**
     * Calls visitor for statement's terms.
     *
     * \param[in] visitor Visitor for terms.
     */
    virtual void visitChildTerms(Visitor<Term> &visitor);
    virtual void visitChildTerms(Visitor<const Term> &visitor) const;

    /**
     * Clones the statement via doClone() and copies properties common to all statements.
     *
     * \returns                        Valid pointer to the clone.
     */
    std::unique_ptr<Statement> clone() const;

    inline bool isComment() const;
    inline bool isInlineAssembly() const;
    inline bool isAssignment() const;
    inline bool isKill() const;
    inline bool isJump() const;
    inline bool isCall() const;
    inline bool isReturn() const;

    /* The following functions are defined in Statements.h. */

    inline const Comment *asComment() const;
    inline const Assignment *asAssignment() const;
    inline const Kill *asKill() const;
    inline const Jump *asJump() const;
    inline const Call *asCall() const;
    inline const Return *asReturn() const;
    
protected:
    /**
     * Actually clones the statement.
     *
     * \return Valid pointer to the clone.
     */
    virtual Statement *doClone() const = 0;
};

}}} // namespace nc::core::ir

/**
 * Defines a compile-time mapping from statement class to statement kind.
 * Makes it possible to use the given class as an argument to <tt>Statement::as</tt>
 * and <tt>Statement::is</tt> template functions.
 * 
 * Must be used at global namespace.
 * 
 * \param CLASS                        Statement class.
 * \param KIND                         Statement kind.
 */
#define NC_REGISTER_STATEMENT_CLASS(CLASS, KIND)                                \
    NC_REGISTER_CLASS_KIND(nc::core::ir::Statement, CLASS, KIND)

NC_REGISTER_STATEMENT_CLASS(nc::core::ir::Comment,        nc::core::ir::Statement::COMMENT)
NC_REGISTER_STATEMENT_CLASS(nc::core::ir::InlineAssembly, nc::core::ir::Statement::INLINE_ASSEMBLY)
NC_REGISTER_STATEMENT_CLASS(nc::core::ir::Assignment,     nc::core::ir::Statement::ASSIGNMENT)
NC_REGISTER_STATEMENT_CLASS(nc::core::ir::Kill,           nc::core::ir::Statement::KILL)
NC_REGISTER_STATEMENT_CLASS(nc::core::ir::Jump,           nc::core::ir::Statement::JUMP)
NC_REGISTER_STATEMENT_CLASS(nc::core::ir::Call,           nc::core::ir::Statement::CALL)
NC_REGISTER_STATEMENT_CLASS(nc::core::ir::Return,         nc::core::ir::Statement::RETURN)

namespace nc { namespace core { namespace ir {

bool Statement::isComment() const {
    return is<Comment>();
}

bool Statement::isInlineAssembly() const {
    return is<InlineAssembly>();
}

bool Statement::isAssignment() const {
    return is<Assignment>();
}

bool Statement::isKill() const {
    return is<Kill>();
}

bool Statement::isJump() const {
    return is<Jump>();
}

bool Statement::isCall() const {
    return is<Call>();
}

bool Statement::isReturn() const {
    return is<Return>();
}

}}} // namespace nc::core::ir

/* vim:set et sts=4 sw=4: */
