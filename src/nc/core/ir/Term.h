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

#include <nc/common/Kinds.h>
#include <nc/common/Printable.h>
#include <nc/common/Types.h>
#include <nc/common/Visitor.h>

#include "MemoryLocation.h"

namespace nc {
namespace core {
namespace ir {

class Constant;
class Intrinsic;
class Undefined;
class BinaryOperator;
class Choice;
class Dereference;
class MemoryLocationAccess;
class UnaryOperator;

class Statement;

/**
 * Base class for different kinds of expressions of intermediate representation.
 * 
 * Terms are supposed to be immutable <i>at the interface level</i>.
 * That is, once created and initialized, they cannot be changed.
 * This is why there is no point in using <tt>const Term</tt> type.
 */
class Term: public Printable, boost::noncopyable {
    NC_CLASS_WITH_KINDS(Term, kind)

public:
    /**
     * Term kind.
     */
    enum {
        INT_CONST, ///< Integer constant.
        INTRINSIC, ///< Intrinsic function call.
        UNDEFINED, ///< Undefined value.
        MEMORY_LOCATION_ACCESS, ///< Access to an abstract memory location.
        DEREFERENCE, ///< Dereference.
        UNARY_OPERATOR, ///< Unary operator.
        BINARY_OPERATOR, ///< Binary operator.
        CHOICE, ///< Choice between two terms.
        USER = 1000 ///< Base for user-defined terms.
    };

    enum Flags {
        WRITE = 0x1,
        READ  = 0x2,
        KILL  = 0x4,
        FLAGS_MASK = 0x7
    };

private:
    SmallBitSize size_; ///< Size of this term's value in bits.

    bool isRead_; ///< Term is read.
    bool isWrite_; ///< Term is written.
    bool isKill_; ///< Term is killed.

    Term *assignee_; ///< RHS of assignment operator, whose LHS is this term.

    const Statement *statement_; ///< Statement that this term belongs to.

public:
    /**
     * Class constructor.
     *
     * \param[in] kind                 Kind of this term.
     * \param[in] size                 Size of this term's value in bits.
     */
    Term(int kind, SmallBitSize size):
        kind_(kind), size_(size),
        isRead_(false), isWrite_(false), isKill_(false),
        assignee_(0), statement_(NULL)
    {
        assert(size != 0);
    }

    /**
     * Initializes this term's flags.
     * 
     * \param[in] flags                Term flags.
     * \param[in] assignee             Expression that is assigned to this term, if any.
     */
    void initFlags(int flags, Term *assignee = NULL) {
        assert(!hasFlags());
        assert((flags & ~FLAGS_MASK) == 0);

        isRead_ = (flags & READ) != 0;
        isWrite_ = (flags & WRITE) != 0;
        isKill_ = (flags & KILL) != 0;
        assignee_ = assignee;

        assert(hasFlags());
    }

    /**
     * \returns                        Size of this term's value in bits.
     */
    SmallBitSize size() const { return size_; }

    /**
     * \return                         True, if term is used for reading.
     */
    bool isRead() const { assert(hasFlags()); return isRead_; }

    /**
     * \return                         True, if term is used for writing.
     */
    bool isWrite() const { assert(hasFlags()); return isWrite_; }

    /**
     * \return                         True, if term is used for killing.
     */
    bool isKill() const { assert(hasFlags()); return isKill_; }

    /**
     * \return                         Expression that this term is assigned to.
     */
    Term *assignee() const { assert(isWrite()); return assignee_; }

    /**
     * \return                         Statement this term belongs to.
     */
    const Statement *statement() const { return statement_; }

    /**
     * \param[in] statement            Statement this term belongs to.
     */
    void setStatement(const Statement *statement) { statement_ = statement; }

    /**
     * Sets the statement that this term and child terms belong to.
     *
     * \param[in] statement            Statement this term and child terms belong to.
     */
    void setStatementRecursively(const Statement *statement);

    /**
     * Calls visitor for term's child terms.
     *
     * \param[in] visitor              Visitor.
     */
    virtual void visitChildTerms(Visitor<Term> &visitor);
    virtual void visitChildTerms(Visitor<const Term> &visitor) const;

    /**
     * \return Clone of the term.
     */
    std::unique_ptr<Term> clone() const { return std::unique_ptr<Term>(doClone()); }

    inline bool isConstant() const;
    inline bool isIntrinsic() const;
    inline bool isMemoryLocationAccess() const;
    inline bool isDereference() const;
    inline bool isUnaryOperator() const;
    inline bool isBinaryOperator() const;
    inline bool isChoice() const;

    /* The following functions are defined in Terms.h. */

    inline const Constant *asConstant() const;
    inline const Intrinsic *asIntrinsic() const;
    inline const MemoryLocationAccess *asMemoryLocationAccess() const;
    inline const Dereference *asDereference() const;
    inline const UnaryOperator *asUnaryOperator() const;
    inline const BinaryOperator *asBinaryOperator() const;
    inline const Choice *asChoice() const;

protected:
    /**
     * \returns                        Whether this term's flags were initialized.
     */
    bool hasFlags() const {
        return isRead_ || isWrite_ || isKill_;
    }

    /**
     * Actually clones the term.
     *
     * \return Clone of the term.
     */
    virtual Term *doClone() const = 0;
};

}}} // namespace nc::core::ir

/**
 * Defines a compile-time mapping from term class to term kind.
 * Makes it possible to use the given class as an argument to <tt>Term::as</tt>
 * and <tt>Term::is</tt> template functions.
 * 
 * Must be used at global namespace.
 * 
 * \param CLASS                        Term class.
 * \param KIND                         Term kind.
 */
#define NC_REGISTER_TERM_CLASS(CLASS, KIND)                                     \
    NC_REGISTER_CLASS_KIND(nc::core::ir::Term, CLASS, KIND)

NC_REGISTER_TERM_CLASS(nc::core::ir::Constant,             nc::core::ir::Term::INT_CONST)
NC_REGISTER_TERM_CLASS(nc::core::ir::Intrinsic,            nc::core::ir::Term::INTRINSIC)
NC_REGISTER_TERM_CLASS(nc::core::ir::Undefined,            nc::core::ir::Term::UNDEFINED)
NC_REGISTER_TERM_CLASS(nc::core::ir::MemoryLocationAccess, nc::core::ir::Term::MEMORY_LOCATION_ACCESS)
NC_REGISTER_TERM_CLASS(nc::core::ir::Dereference,          nc::core::ir::Term::DEREFERENCE)
NC_REGISTER_TERM_CLASS(nc::core::ir::UnaryOperator,        nc::core::ir::Term::UNARY_OPERATOR)
NC_REGISTER_TERM_CLASS(nc::core::ir::BinaryOperator,       nc::core::ir::Term::BINARY_OPERATOR)
NC_REGISTER_TERM_CLASS(nc::core::ir::Choice,               nc::core::ir::Term::CHOICE)


namespace nc { namespace core { namespace ir {

bool Term::isConstant() const { return is<Constant>(); } 
bool Term::isIntrinsic() const { return is<Intrinsic>(); } 
bool Term::isMemoryLocationAccess() const { return is<MemoryLocationAccess>(); }
bool Term::isDereference() const { return is<Dereference>(); }
bool Term::isUnaryOperator() const { return is<UnaryOperator>(); }
bool Term::isBinaryOperator() const { return is<BinaryOperator>(); }
bool Term::isChoice() const { return is<Choice>(); }

}}} // namespace nc::core::ir

/* vim:set et sts=4 sw=4: */
