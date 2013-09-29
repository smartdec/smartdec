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

    /**
     * Types of term's access.
     */
    enum AccessType {
        NO_ACCESS, ///< Unknown, not set.
        READ,      ///< Term is being read from.
        WRITE,     ///< Term is being assigned to.
        KILL       ///< Term is being killed.
    };

private:
    SmallBitSize size_; ///< Size of this term's value in bits.
    AccessType accessType_; ///< Type of term's use.
    const Statement *statement_; ///< Statement that this term belongs to.

public:
    /**
     * Class constructor.
     *
     * \param[in] kind                 Kind of this term.
     * \param[in] size                 Size of this term's value in bits.
     */
    Term(int kind, SmallBitSize size):
        kind_(kind), size_(size), accessType_(NO_ACCESS), statement_(NULL)
    {
        assert(size != 0);
    }

    /**
     * \returns Size of this term's value in bits.
     */
    SmallBitSize size() const { return size_; }

    /**
     * Sets the type of the term's use: read, write, or kill.
     *
     * \param[in] accessType Access type.
     *
     * \note Must be called only once for each term.
     */
    void setAccessType(AccessType accessType) {
        assert(accessType_ == NO_ACCESS);
        assert(accessType == READ || accessType == WRITE || accessType == KILL);

        accessType_ = accessType;
    }

    /**
     * \return True if term is used for reading, false otherwise.
     */
    bool isRead() const { assert(accessType_ != NO_ACCESS); return accessType_ == READ; }

    /**
     * \return True if term is used for writing, false otherwise.
     */
    bool isWrite() const { assert(accessType_ != NO_ACCESS); return accessType_ == WRITE; }

    /**
     * \return True if term is used for killing, false otherwise.
     */
    bool isKill() const { assert(accessType_ != NO_ACCESS); return accessType_ == KILL; }

    /**
     * \return Pointer to the statement this term belongs to. Can be NULL.
     */
    const Statement *statement() const { return statement_; }

    /**
     * Sets the statement this term belongs to.
     *
     * \param[in] statement Valid pointer to the statement.
     *
     * \note Must be called only once for each term.
     */
    void setStatement(const Statement *statement) {
        assert(statement_ == NULL);
        assert(statement != NULL);

        statement_ = statement;
    }

    /**
     * Sets the statement that this term and child terms belong to.
     *
     * \param[in] statement            Statement this term and child terms belong to.
     */
    void setStatementRecursively(const Statement *statement);

    /**
     * \return If the term stands in the left hand side of an assignment,
     *         returns the right hand size of this assignment. Otherwise,
     *         NULL is returned.
     */
    const Term *assignee() const;

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
