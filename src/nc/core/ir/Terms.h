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

#include <boost/optional.hpp>

#include <nc/common/SizedValue.h>

#include "Term.h"

namespace nc {

class SizedValue;

namespace core {
namespace ir {

/**
 * Constant integer.
 */
class Constant: public Term {
    SizedValue value_; ///< Value of the constant.

public:
    /**
     * Class constructor.
     *
     * \param[in] value                Value of the constant.
     */
    Constant(const SizedValue &value): Term(INT_CONST, value.size()), value_(value) {}

    /**
     * \return Value of the constant. Bits starting from size() and higher are zero.
     */
    const SizedValue &value() const { return value_; }

    /**
     * Sets the value of the constant.
     *
     * \param[in] value New value.
     */
    void setValue(const SizedValue &value) { value_ = SizedValue(value.value(), size()); }

    virtual void print(QTextStream &out) const override;

protected:
    virtual Constant *doClone() const { return new Constant(value()); }
};

/**
 * Value computed using built-in function.
 */
class Intrinsic: public Term {
    NC_CLASS_WITH_KINDS(Intrinsic, intrinsicKind)

public:
    enum {
        UNKNOWN,    ///< Unknown intrinsic.
        USER = 1000 ///< First user intrinsic.
    };

    /**
     * Constructor.
     *
     * \param[in] intrinsicKind         Kind of the convention.
     * \param[in] size                  Size of this term's value in bits.
     */
    Intrinsic(int intrinsicKind, SmallBitSize size):
        Term(INTRINSIC, size), intrinsicKind_(intrinsicKind)
    {}

    virtual void print(QTextStream &out) const override;

protected:
    virtual Intrinsic *doClone() const { return new Intrinsic(intrinsicKind(), size()); }
};

/**
 * Undefined.
 */
class Undefined: public Term {
public:
    /**
     * Constructor.
     * 
     * \param[in] size                 Size of this term's value in bits.
     */
    Undefined(SmallBitSize size): Term(UNDEFINED, size) {}

    virtual void print(QTextStream &out) const override;

protected:
    virtual Undefined *doClone() const { return new Undefined(size()); }
};

/**
 * Reference to some abstract memory location (e.g. register).
 */
class MemoryLocationAccess: public Term {
    MemoryLocation memoryLocation_; ///< Accessed memory location.

public:
    /**
     * Class constructor.
     *
     * \param[in] memoryLocation Referenced memory location.
     */
    MemoryLocationAccess(const MemoryLocation &memoryLocation);

    /**
     * \return Referenced memory location.
     */
    const MemoryLocation &memoryLocation() const { return memoryLocation_; }

    virtual void print(QTextStream &out) const override;

protected:
    virtual MemoryLocationAccess *doClone() const { return new MemoryLocationAccess(memoryLocation()); }
};

/**
 * Dereference of a memory address.
 */
class Dereference: public Term {
    Domain domain_; ///< Domain the address belongs to.
    std::unique_ptr<Term> address_; ///< Address to be dereferenced.

public:
    /**
     * Class constructor.
     *
     * \param[in] address              Memory address to be dereferenced.
     * \param[in] domain               Memory domain that the address belongs to.
     * \param[in] size                 Size of the value at address in bits.
     */
    Dereference(std::unique_ptr<Term> address, Domain domain, SmallBitSize size);

    /**
     * \return Domain the address belongs to.
     */
    Domain domain() const { return domain_; }

    /**
     * \return Address to be dereferenced.
     */
    Term *address() { return address_.get(); }

    /**
     * \return Address to be dereferenced.
     */
    const Term *address() const { return address_.get(); }

    virtual void visitChildTerms(Visitor<Term> &visitor) override;
    virtual void visitChildTerms(Visitor<const Term> &visitor) const override;

    virtual void print(QTextStream &out) const override;

protected:
    virtual Dereference *doClone() const { return new Dereference(address()->clone(), domain(), size()); }
};

/**
 * Base class for unary operators.
 */
class UnaryOperator: public Term {
    NC_CLASS_WITH_KINDS(UnaryOperator, operatorKind)

    std::unique_ptr<Term> operand_; ///< The operand of the unary operator.

public:
    enum {
        BITWISE_NOT, ///< Bitwise NOT.
        LOGICAL_NOT, ///< Logical NOT.
        NEGATION, ///< Negation.
        SIGN_EXTEND, ///< Sign extend.
        ZERO_EXTEND, ///< Zero extend.
        RESIZE, ///< Just copy bits. Zero extend or truncate as necessary.
        USER = 1000 ///< Base for user-defined operators.
    };

    /**
     * Class constructor.
     *
     * \param[in] operatorKind         Subkind of the unary operator.
     * \param[in] operand              The operand of the unary operator.
     * \param[in] size                 Size of this term's value in bits.
     */
    UnaryOperator(int operatorKind, std::unique_ptr<Term> operand, SmallBitSize size);

    /**
     * Class constructor. 
     * 
     * Size is taken from the given operand.
     * 
     * \param[in] operatorKind         Subkind of the unary operator.
     * \param[in] operand              The operand of the unary operator.
     */
    UnaryOperator(int operatorKind, std::unique_ptr<Term> operand);

    /**
     * \return                         The operand of the unary operator.
     */
    Term *operand() { return operand_.get(); }

    /**
     * \return                         The operand of the unary operator.
     */
    const Term *operand() const { return operand_.get(); }

    virtual void visitChildTerms(Visitor<Term> &visitor) override;
    virtual void visitChildTerms(Visitor<const Term> &visitor) const override;

    /**
     * Applies the unary operator to the integer argument.
     *
     * \param a                        Argument.
     * \return                         Result of application of this unary operator to the given argument,
     *                                 or boost::none when the operator is not applicable.
     */
    virtual boost::optional<SizedValue> apply(const SizedValue &a) const;

    virtual void print(QTextStream &out) const override;

protected:
    virtual UnaryOperator *doClone() const { return new UnaryOperator(operatorKind(), operand()->clone(), size()); }
};

/**
 * Base class for binary operators.
 */
class BinaryOperator: public Term {
    NC_CLASS_WITH_KINDS(BinaryOperator, operatorKind)

    std::unique_ptr<Term> left_; ///< Left operand.
    std::unique_ptr<Term> right_; ///< Right operand.

public:
    enum {
        ADD, ///< Integer addition.
        SUB, ///< Integer subtraction.
        MUL, ///< Integer multiplication.
        SIGNED_DIV, ///< Signed integer division.
        UNSIGNED_DIV, ///< Unsigned integer division.
        SIGNED_REM, ///< Signed integer remainder.
        UNSIGNED_REM, ///< Unsigned integer remainder.
        BITWISE_AND, ///< Bitwise AND.
        LOGICAL_AND, ///< Logical AND.
        BITWISE_OR, ///< Bitwise OR.
        LOGICAL_OR, ///< Logical OR.
        BITWISE_XOR, ///< Bitwise XOR.
        SHL, ///< Bit shift left.
        SHR, ///< Bit shift right.
        SAR, ///< Arithmetic bit shift right.
        EQUAL, ///< Equality operator for integers.
        SIGNED_LESS, ///< Integer signed less.
        SIGNED_LESS_OR_EQUAL, ///< Integer signed less or equal.
        SIGNED_GREATER, ///< Integer signed greater.
        SIGNED_GREATER_OR_EQUAL, ///< Signed greater or equal for integer.
        UNSIGNED_LESS, ///< Integer unsigned less.
        UNSIGNED_LESS_OR_EQUAL, ///< Integer unsigned less or equal.
        UNSIGNED_GREATER, ///< Integer unsigned greater.
        UNSIGNED_GREATER_OR_EQUAL, ///< Integer unsigned greater or equal.
        USER = 1000 ///< Base for user-defined operators.
    };

    /**
     * Class constructor.
     * 
     * Size of left and right operands is expected to be equal.
     *
     * \param[in] operatorKind         Subkind of the binary operator.
     * \param[in] left                 Left operand.
     * \param[in] right                Right operand.
     * \param[in] size                 Size of this term's value in bits.
     */
    BinaryOperator(int operatorKind, std::unique_ptr<Term> left, std::unique_ptr<Term> right, SmallBitSize size);

    /**
     * Class constructor.
     * 
     * Size is taken from the given operands.
     *
     * \param[in] operatorKind         Subkind of the binary operator.
     * \param[in] left                 Left operand.
     * \param[in] right                Right operand.
     */
    BinaryOperator(int operatorKind, std::unique_ptr<Term> left, std::unique_ptr<Term> right);

    /**
     * \return                         Left operand.
     */
    Term *left() { return left_.get(); }

    /**
     * \return                         Left operand.
     */
    const Term *left() const { return left_.get(); }

    /**
     * \return                         Right operand.
     */
    Term *right() { return right_.get(); }

    /**
     * \return                         Right operand.
     */
    const Term *right() const { return right_.get(); }

    virtual void visitChildTerms(Visitor<Term> &visitor) override;
    virtual void visitChildTerms(Visitor<const Term> &visitor) const override;

    /**
     * Applies the binary operator to two integer arguments.
     *
     * \param a                        Left argument.
     * \param b                        Right argument.
     * \return                         a op b, or boost::none if result is undefined.
     */
    virtual boost::optional<SizedValue> apply(const SizedValue &a, const SizedValue &b) const;

    virtual void print(QTextStream &out) const override;

protected:
    virtual BinaryOperator *doClone() const;
};

/**
 * Special kind of binary operator which is equal to its first argument, if the latter
 * is defined, and equal to the second argument otherwise. Just like a Phi-function.
 */
class Choice: public Term {
    std::unique_ptr<Term> preferredTerm_; ///< Preferred term (used if defined).
    std::unique_ptr<Term> defaultTerm_; ///< Default term (used if preferred term is not defined).

public:
    /**
     * Class constructor.
     *
     * \param preferredTerm Preferred term (used if defined).
     * \param defaultTerm Default term (used if preferred term is not defined).
     */
    Choice(std::unique_ptr<Term> preferredTerm, std::unique_ptr<Term> defaultTerm);

    /**
     * \return Preferred term.
     */
    Term *preferredTerm() { return preferredTerm_.get(); }

    /**
     * \return Preferred term.
     */
    const Term *preferredTerm() const { return preferredTerm_.get(); }

    /**
     * \return Default term.
     */
    Term *defaultTerm() { return defaultTerm_.get(); }

    /**
     * \return Default term.
     */
    const Term *defaultTerm() const { return defaultTerm_.get(); }

    virtual void visitChildTerms(Visitor<Term> &visitor) override;
    virtual void visitChildTerms(Visitor<const Term> &visitor) const override;

    virtual void print(QTextStream &out) const override;

protected:
    virtual Choice *doClone() const;
};


/*
 * Term implementation follows.
 */

const Constant *Term::asConstant() const { return as<Constant>(); }
const Intrinsic *Term::asIntrinsic() const { return as<Intrinsic>(); }
const MemoryLocationAccess *Term::asMemoryLocationAccess() const { return as<MemoryLocationAccess>(); }
const Dereference *Term::asDereference() const { return as<Dereference>(); }
const UnaryOperator *Term::asUnaryOperator() const { return as<UnaryOperator>(); }
const BinaryOperator *Term::asBinaryOperator() const { return as<BinaryOperator>(); }
const Choice *Term::asChoice() const { return as<Choice>(); }

}}} // namespace nc::core::ir

/* vim:set et sts=4 sw=4: */
