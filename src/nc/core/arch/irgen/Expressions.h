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

#include <QCoreApplication> /* For Q_DECLARE_TR_FUNCTIONS. */

#include <algorithm> /* std::max */

#include <boost/mpl/int.hpp>

#include <nc/common/Unused.h>
#include <nc/common/make_unique.h>

#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/Instruction.h>
#include <nc/core/arch/Operand.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/Statements.h>

#include "InstructionAnalyzer.h"
#include "InvalidInstructionException.h"

namespace nc { namespace core { namespace arch { namespace irgen { namespace expressions {

// -------------------------------------------------------------------------- //
// Statements
// -------------------------------------------------------------------------- //
/**
 * Base class for all instruction analyzer statements.
 */
template<class Derived>
class StatementBase {
public:
    Derived &derived() {
        return static_cast<Derived &>(*this);
    }

    const Derived &derived() const {
        return static_cast<const Derived &>(*this);
    }
};

/**
 * Base class for unary statements.
 */
template<class E, class Derived>
class UnaryStatementBase: public StatementBase<Derived> {
public:
    UnaryStatementBase(const E &expression): mExpression(expression) {}

    E &expression() {
        return mExpression;
    }

    const E &expression() const {
        return mExpression;
    }

private:
    E mExpression;
};

/**
 * Base class for binary statements.
 */
template<class L, class R, class Derived>
class BinaryStatementBase: public StatementBase<Derived> {
public:
    BinaryStatementBase(const L &left, const R &right):
        mLeft(left),
        mRight(right)
    {}

    L &left() {
        return mLeft;
    }

    const L &left() const {
        return mLeft;
    }

    R &right() {
        return mRight;
    }

    const R &right() const {
        return mRight;
    }

private:
    L mLeft;
    R mRight;
};

/**
 * Class for return statements.
 */
class ReturnStatement: public StatementBase<ReturnStatement> {};

/**
 * Class for kill statements.
 */
template<class E>
class KillStatement: public UnaryStatementBase<E, KillStatement<E> > {
    typedef UnaryStatementBase<E, KillStatement<E> > base_type;
public:
    KillStatement(const E &expression): base_type(expression) {}
};

/**
 * Class for jump statements.
 */
template<class C, class A>
class JumpStatement: public StatementBase<JumpStatement<C, A> > {
    typedef StatementBase<JumpStatement<C, A> > base_type;
    C mCondition;
    A mAddress;
    ir::BasicBlock *mThenBasicBlock;
    ir::BasicBlock *mElseBasicBlock;
public:
    JumpStatement(const C &condition, const A &address, ir::BasicBlock *thenBasicBlock, ir::BasicBlock *elseBasicBlock):
        mCondition(condition), mAddress(address), mThenBasicBlock(thenBasicBlock), mElseBasicBlock(elseBasicBlock)
    {}

    C &condition() { return mCondition; }
    const C &condition() const { return mCondition; }

    A &address() { return mAddress; }
    const A &address() const { return mAddress; }

    ir::BasicBlock *thenBasicBlock() const { return mThenBasicBlock; }
    ir::BasicBlock *elseBasicBlock() const { return mElseBasicBlock; }
};

/**
 * Class for call statements.
 */
template<class E>
class CallStatement: public UnaryStatementBase<E, CallStatement<E> > {
    typedef UnaryStatementBase<E, CallStatement<E> > base_type;
public:
    CallStatement(const E &expression): base_type(expression) {}
};

/**
 * Class for assignment statements.
 */
template<class L, class R>
class AssignmentStatement: public BinaryStatementBase<L, R, AssignmentStatement<L, R> > {
    typedef BinaryStatementBase<L, R, AssignmentStatement<L, R> > base_type;
public:
    AssignmentStatement(const L &left, const R &right): base_type(left, right) {}
};

/**
 * Class for sequential statements.
 */
template<class L, class R>
class SequenceStatement: public BinaryStatementBase<L, R, SequenceStatement<L, R> > {
    typedef BinaryStatementBase<L, R, SequenceStatement<L, R> > base_type;
public:
    SequenceStatement(const L &left, const R &right): base_type(left, right) {}
};


// -------------------------------------------------------------------------- //
// Expressions
// -------------------------------------------------------------------------- //
/**
 * Base class for all instruction analyzer expressions.
 */
template<class Derived>
class ExpressionBase {
public:
    typedef Derived derived_type;

    ExpressionBase(SmallBitSize size = 0): mSize(size) {}

    SmallBitSize size() const { return mSize; }

    void setSize(SmallBitSize size) { assert(!mSize); mSize = size; }

    Derived &derived() {
        return static_cast<Derived &>(*this);
    }

    const Derived &derived() const {
        return static_cast<const Derived &>(*this);
    }

    AssignmentStatement<Derived, Derived>
    operator=(const ExpressionBase &other) const {
        return AssignmentStatement<Derived, Derived>(derived(), other.derived());
    }

    /**
     * MSVC chokes if this function is defined out-of-line, so here is a full definition.
     */
    template<class OtherDerived>
    AssignmentStatement<Derived, OtherDerived>
    operator=(const ExpressionBase<OtherDerived> &other) const {
        return AssignmentStatement<Derived, OtherDerived>(derived(), other.derived());
    }

private:
    SmallBitSize mSize;
};

/**
 * This macro is to be used inside a body of a leaf expression class to override
 * compiler-generated operator=.
 *
 * This macro expects a base_type typedef to be defined.
 *
 * @param CLASS                        Name of the class that this macro is
 *                                     used inside a body of.
 */
#define NC_EXPRESSION(CLASS)                                                    \
    template<class OtherE>                                                      \
    AssignmentStatement<typename base_type::derived_type, OtherE>               \
    operator=(const ExpressionBase<OtherE> &other) const {                      \
        return base_type::operator=(other);                                     \
    }                                                                           \
                                                                                \
    AssignmentStatement<typename base_type::derived_type, typename base_type::derived_type> \
    operator=(const CLASS &other) const {                                       \
        return base_type::operator=(other);                                     \
    }                                                                           \


/**
 * Base class for unary expressions.
 */
template<class E, class Derived>
class UnaryExpressionBase: public ExpressionBase<Derived> {
    typedef ExpressionBase<Derived> base_type;
public:
    NC_EXPRESSION(UnaryExpressionBase);

    UnaryExpressionBase(const E &operand, SmallBitSize size = 0): base_type(size), mOperand(operand) {}

    E &operand() {
        return mOperand;
    }

    const E &operand() const {
        return mOperand;
    }

private:
    E mOperand;
};

/**
 * Base class for binary expressions.
 */
template<class L, class R, class Derived>
class BinaryExpressionBase: public ExpressionBase<Derived> {
    typedef ExpressionBase<Derived> base_type;
public:
    NC_EXPRESSION(BinaryExpressionBase);

    BinaryExpressionBase(const L &left, const R &right, SmallBitSize size = 0):
        base_type(size),
        mLeft(left),
        mRight(right)
    {}

    L &left() {
        return mLeft;
    }

    const L &left() const {
        return mLeft;
    }

    R &right() {
        return mRight;
    }

    const R &right() const {
        return mRight;
    }

private:
    L mLeft;
    R mRight;
};


/**
 * Class for instrinsic expressions.
 */
class IntrinsicExpression: public ExpressionBase<IntrinsicExpression> {
    typedef ExpressionBase<IntrinsicExpression> base_type;
public:
    NC_EXPRESSION(IntrinsicExpression);
};

/**
 * Class for undefined expressions.
 */
class UndefinedExpression: public ExpressionBase<UndefinedExpression> {
    typedef ExpressionBase<UndefinedExpression> base_type;
public:
    NC_EXPRESSION(UndefinedExpression);
};

/**
 * Class for constant expressions.
 */
class ConstantExpression: public ExpressionBase<ConstantExpression> {
    typedef ExpressionBase<ConstantExpression> base_type;
public:
    NC_EXPRESSION(ConstantExpression);

    ConstantExpression(ConstantValue value, SmallBitSize size = 0): base_type(size), mValue(value) {}

    ConstantValue value() const {
        return mValue;
    }

private:
    ConstantValue mValue;
};

/**
 * Class for operand expressions.
 */
class OperandExpression: public ExpressionBase<OperandExpression> {
    typedef ExpressionBase<OperandExpression> base_type;
public:
    NC_EXPRESSION(OperandExpression);

    explicit OperandExpression(const arch::Operand *operand): mOperand(operand) {}

    const arch::Operand *operand() const {
        return mOperand;
    }

private:
    const arch::Operand *mOperand;
};

/**
 * Class for instruction operand expressions.
 */
class InstructionOperandExpression: public ExpressionBase<InstructionOperandExpression> {
    typedef ExpressionBase<InstructionOperandExpression> base_type;
public:
    NC_EXPRESSION(InstructionOperandExpression);

    /**
     * \param index                     Index of instruction's operand that this expression represents.
     */
    explicit InstructionOperandExpression(int index): mIndex(index) {}

    int index() const {
        return mIndex;
    }

private:
    int mIndex;
};

/**
 * Class for register expressions.
 */
class RegisterExpression: public ExpressionBase<RegisterExpression> {
    typedef ExpressionBase<RegisterExpression> base_type;
public:
    NC_EXPRESSION(RegisterExpression);

    explicit RegisterExpression(int number): mNumber(number) {}

    int number() const {
        return mNumber;
    }

private:
    int mNumber;
};

/**
 * Class for memory location expressions.
 */
class MemoryLocationExpression: public ExpressionBase<MemoryLocationExpression> {
    typedef ExpressionBase<MemoryLocationExpression> base_type;
public:
    NC_EXPRESSION(MemoryLocationExpression)

    MemoryLocationExpression(const ir::MemoryLocation &memoryLocation): mMemoryLocation(memoryLocation) {}

    const ir::MemoryLocation &memoryLocation() const {
        return mMemoryLocation;
    }
private:
    ir::MemoryLocation mMemoryLocation;
};

/**
 * Class for unary expressions.
 */
template<int subkind, class E>
class UnaryExpression: public UnaryExpressionBase<E, UnaryExpression<subkind, E> > {
    typedef UnaryExpressionBase<E, UnaryExpression<subkind, E> > base_type;
public:
    NC_EXPRESSION(UnaryExpression);

    UnaryExpression(const E &expression, SmallBitSize size = 0): base_type(expression, size) {}
};

/**
 * Class for dereference expressions.
 */
template<class E>
class DereferenceExpression: public UnaryExpressionBase<E, DereferenceExpression<E> > {
    typedef UnaryExpressionBase<E, DereferenceExpression<E> > base_type;
public:
    NC_EXPRESSION(DereferenceExpression);

    DereferenceExpression(const E &expression, SmallBitSize size = 0): base_type(expression, size) {}
};

/**
 * Class for binary expressions.
 */
template<int subkind, class L, class R>
class BinaryExpression: public BinaryExpressionBase<L, R, BinaryExpression<subkind, L, R> > {
    typedef BinaryExpressionBase<L, R, BinaryExpression<subkind, L, R> > base_type;
public:
    NC_EXPRESSION(BinaryExpression);

    BinaryExpression(const L &left, const R &right, SmallBitSize size = 0): base_type(left, right, size) {}
};

/**
 * Class for choice expressions.
 */
template<class L, class R>
class ChoiceExpression: public BinaryExpressionBase<L, R, ChoiceExpression<L, R> > {
    typedef BinaryExpressionBase<L, R, ChoiceExpression<L, R> > base_type;
public:
    NC_EXPRESSION(ChoiceExpression);

    ChoiceExpression(const L &left, const R &right): base_type(left, right) {}
};

/**
 * Class for sign expressions.
 *
 * These expressions have no direct counterpart in IR. Instead, they are used
 * to mark arithmetic operations as signed or unsigned.
 *
 * \see nc::core::expressions::ExpressionSignedness
 */
template<int signedness, class E>
class SignExpression: public UnaryExpressionBase<E, SignExpression<signedness, E> > {
    typedef UnaryExpressionBase<E, SignExpression<signedness, E> > base_type;
public:
    NC_EXPRESSION(SignExpression);

    explicit SignExpression(const E &expression): base_type(expression) {}
};

/**
 * Class for no expression.
 */
class NullExpression: public ExpressionBase<NullExpression> {};

// -------------------------------------------------------------------------- //
// Expression signedness
// -------------------------------------------------------------------------- //
class ExpressionSignedness {
public:
    enum {
        SIGNED,
        UNSIGNED,
        UNKNOWN
    };
};

/**
 * Metafunction that returns expression's signedness.
 */
template<class Expression>
struct expression_signedness: boost::mpl::int_<ExpressionSignedness::UNKNOWN> {};

template<class Derived>
struct expression_signedness<ExpressionBase<Derived> >: expression_signedness<Derived> {};

template<int signedness, class E>
struct expression_signedness<SignExpression<signedness, E> >: boost::mpl::int_<signedness> {};

/**
 * Metafuction that returns binary expression subkind based on signedness.
 */
template<int unsigned_subkind, int signedness>
struct binary_subkind;

template<int unsigned_subkind>
struct binary_subkind<unsigned_subkind, ExpressionSignedness::UNSIGNED>: boost::mpl::int_<unsigned_subkind> {};

template<>
struct binary_subkind<ir::BinaryOperator::UNSIGNED_DIV, ExpressionSignedness::SIGNED>: boost::mpl::int_<ir::BinaryOperator::SIGNED_DIV> {};

template<>
struct binary_subkind<ir::BinaryOperator::UNSIGNED_REM, ExpressionSignedness::SIGNED>: boost::mpl::int_<ir::BinaryOperator::SIGNED_REM> {};

template<>
struct binary_subkind<ir::BinaryOperator::UNSIGNED_LESS, ExpressionSignedness::SIGNED>: boost::mpl::int_<ir::BinaryOperator::SIGNED_LESS> {};

template<>
struct binary_subkind<ir::BinaryOperator::UNSIGNED_LESS_OR_EQUAL, ExpressionSignedness::SIGNED>: boost::mpl::int_<ir::BinaryOperator::SIGNED_LESS_OR_EQUAL> {};

template<>
struct binary_subkind<ir::BinaryOperator::UNSIGNED_GREATER, ExpressionSignedness::SIGNED>: boost::mpl::int_<ir::BinaryOperator::SIGNED_GREATER> {};

template<>
struct binary_subkind<ir::BinaryOperator::UNSIGNED_GREATER_OR_EQUAL, ExpressionSignedness::SIGNED>: boost::mpl::int_<ir::BinaryOperator::SIGNED_GREATER_OR_EQUAL> {};

template<>
struct binary_subkind<ir::BinaryOperator::SHR, ExpressionSignedness::SIGNED>: boost::mpl::int_<ir::BinaryOperator::SAR> {};


/**
 * Metafunction that returns binary expression subkind based on signedness of the given expression.
 */
template<int unsigned_subkind, class E>
struct binary_expression_subkind: public binary_subkind<unsigned_subkind, expression_signedness<E>::value> {};


// -------------------------------------------------------------------------- //
// Shortcuts
// -------------------------------------------------------------------------- //
inline IntrinsicExpression
intrinsic() {
    return IntrinsicExpression();
}

inline UndefinedExpression
undefined() {
    return UndefinedExpression();
}

inline ConstantExpression
constant(ConstantValue value, SmallBitSize size = 0) {
    return ConstantExpression(value, size);
}

inline OperandExpression
operand(const arch::Operand *operand) {
    return OperandExpression(operand);
}

inline InstructionOperandExpression
operand(int index) {
    return InstructionOperandExpression(index);
}

inline RegisterExpression
regizter(int number) {
    return RegisterExpression(number);
}

template<class E>
inline SignExpression<ExpressionSignedness::SIGNED, E>
signed_(const ExpressionBase<E> &expression) {
    return SignExpression<ExpressionSignedness::SIGNED, E>(expression.derived());
}

template<class E>
inline SignExpression<ExpressionSignedness::UNSIGNED, E>
unsigned_(const ExpressionBase<E> &expression) {
    return SignExpression<ExpressionSignedness::UNSIGNED, E>(expression.derived());
}

template<class E>
inline DereferenceExpression<E>
operator*(const ExpressionBase<E> &expression) {
    return DereferenceExpression<E>(expression.derived());
}

template<class E>
inline DereferenceExpression<E>
dereference(const ExpressionBase<E> &expression, SmallBitSize size) {
    DereferenceExpression<E> result(expression.derived());
    result.setSize(size);
    return result;
}

template<class E>
inline UnaryExpression<ir::UnaryOperator::BITWISE_NOT, E>
operator~(const ExpressionBase<E> &expression) {
    return UnaryExpression<ir::UnaryOperator::BITWISE_NOT, E>(expression.derived());
}

template<class E>
inline UnaryExpression<ir::UnaryOperator::LOGICAL_NOT,  E>
operator!(const ExpressionBase<E> &expression) {
    return UnaryExpression<ir::UnaryOperator::LOGICAL_NOT, E>(expression.derived(), 1);
}

template<class E>
inline UnaryExpression<ir::UnaryOperator::NEGATION,  E>
operator-(const ExpressionBase<E> &expression) {
    return UnaryExpression<ir::UnaryOperator::NEGATION, E>(expression.derived());
}

template<class E>
inline UnaryExpression<ir::UnaryOperator::SIGN_EXTEND, E>
sign_extend(const ExpressionBase<E> &expression) {
    return UnaryExpression<ir::UnaryOperator::SIGN_EXTEND, E>(expression.derived());
}

template<class E>
inline UnaryExpression<ir::UnaryOperator::ZERO_EXTEND, E>
zero_extend(const ExpressionBase<E> &expression) {
    return UnaryExpression<ir::UnaryOperator::ZERO_EXTEND, E>(expression.derived());
}

template<class E>
inline UnaryExpression<ir::UnaryOperator::RESIZE, E>
resize(const ExpressionBase<E> &expression, SmallBitSize size = 0) {
    return UnaryExpression<ir::UnaryOperator::RESIZE, E>(expression.derived(), size);
}

template<class L, class R>
inline BinaryExpression<ir::BinaryOperator::ADD, L, R>
operator+(const ExpressionBase<L> &left, const ExpressionBase<R> &right) {
    return BinaryExpression<ir::BinaryOperator::ADD, L, R>(left.derived(), right.derived());
}

template<class L, class R>
inline BinaryExpression<ir::BinaryOperator::SUB, L, R>
operator-(const ExpressionBase<L> &left, const ExpressionBase<R> &right) {
    return BinaryExpression<ir::BinaryOperator::SUB, L, R>(left.derived(), right.derived());
}

template<class L, class R>
inline BinaryExpression<ir::BinaryOperator::MUL, L, R>
operator*(const ExpressionBase<L> &left, const ExpressionBase<R> &right) {
    return BinaryExpression<ir::BinaryOperator::MUL, L, R>(left.derived(), right.derived());
}

template<class L, class R>
inline BinaryExpression<binary_expression_subkind<ir::BinaryOperator::UNSIGNED_DIV, L>::value, L, R>
operator/(const ExpressionBase<L> &left, const ExpressionBase<R> &right) {
    return BinaryExpression<binary_expression_subkind<ir::BinaryOperator::UNSIGNED_DIV, L>::value, L, R>(left.derived(), right.derived());
}

template<class L, class R>
inline BinaryExpression<binary_expression_subkind<ir::BinaryOperator::UNSIGNED_REM, L>::value, L, R>
operator%(const ExpressionBase<L> &left, const ExpressionBase<R> &right) {
    return BinaryExpression<binary_expression_subkind<ir::BinaryOperator::UNSIGNED_REM, L>::value, L, R>(left.derived(), right.derived());
}

template<class L, class R>
inline BinaryExpression<ir::BinaryOperator::BITWISE_AND, L, R>
operator&(const ExpressionBase<L> &left, const ExpressionBase<R> &right) {
    return BinaryExpression<ir::BinaryOperator::BITWISE_AND, L, R>(left.derived(), right.derived());
}

template<class L, class R>
inline BinaryExpression<ir::BinaryOperator::LOGICAL_AND, L, R>
operator&&(const ExpressionBase<L> &left, const ExpressionBase<R> &right) {
    return BinaryExpression<ir::BinaryOperator::LOGICAL_AND, L, R>(left.derived(), right.derived(), 1);
}

template<class L, class R>
inline BinaryExpression<ir::BinaryOperator::BITWISE_OR, L, R>
operator|(const ExpressionBase<L> &left, const ExpressionBase<R> &right) {
    return BinaryExpression<ir::BinaryOperator::BITWISE_OR, L, R>(left.derived(), right.derived());
}

template<class L, class R>
inline BinaryExpression<ir::BinaryOperator::LOGICAL_OR, L, R>
operator||(const ExpressionBase<L> &left, const ExpressionBase<R> &right) {
    return BinaryExpression<ir::BinaryOperator::LOGICAL_OR, L, R>(left.derived(), right.derived(), 1);
}

template<class L, class R>
inline BinaryExpression<ir::BinaryOperator::BITWISE_XOR, L, R>
operator^(const ExpressionBase<L> &left, const ExpressionBase<R> &right) {
    return BinaryExpression<ir::BinaryOperator::BITWISE_XOR, L, R>(left.derived(), right.derived());
}

template<class L, class R>
inline BinaryExpression<ir::BinaryOperator::SHL, L, R>
operator<<(const ExpressionBase<L> &left, const ExpressionBase<R> &right) {
    return BinaryExpression<ir::BinaryOperator::SHL, L, R>(left.derived(), right.derived());
}

template<class L, class R>
inline BinaryExpression<binary_expression_subkind<ir::BinaryOperator::SHR, L>::value, L, R>
operator>>(const ExpressionBase<L> &left, const ExpressionBase<R> &right) {
    return BinaryExpression<binary_expression_subkind<ir::BinaryOperator::SHR, L>::value, L, R>(left.derived(), right.derived());
}

template<class L, class R>
inline BinaryExpression<ir::BinaryOperator::EQUAL, L, R>
operator==(const ExpressionBase<L> &left, const ExpressionBase<R> &right) {
    return BinaryExpression<ir::BinaryOperator::EQUAL, L, R>(left.derived(), right.derived(), 1);
}

template<class L, class R>
inline BinaryExpression<binary_expression_subkind<ir::BinaryOperator::UNSIGNED_LESS, L>::value, L, R>
operator<(const ExpressionBase<L> &left, const ExpressionBase<R> &right) {
    return BinaryExpression<binary_expression_subkind<ir::BinaryOperator::UNSIGNED_LESS, L>::value, L, R>(left.derived(), right.derived(), 1);
}

template<class L, class R>
inline BinaryExpression<binary_expression_subkind<ir::BinaryOperator::UNSIGNED_LESS_OR_EQUAL, L>::value, L, R>
operator<=(const ExpressionBase<L> &left, const ExpressionBase<R> &right) {
    return BinaryExpression<binary_expression_subkind<ir::BinaryOperator::UNSIGNED_LESS_OR_EQUAL, L>::value, L, R>(left.derived(), right.derived(), 1);
}

template<class L, class R>
inline BinaryExpression<binary_expression_subkind<ir::BinaryOperator::UNSIGNED_GREATER, L>::value, L, R>
operator>(const ExpressionBase<L> &left, const ExpressionBase<R> &right) {
    return BinaryExpression<binary_expression_subkind<ir::BinaryOperator::UNSIGNED_GREATER, L>::value, L, R>(left.derived(), right.derived(), 1);
}

template<class L, class R>
inline BinaryExpression<binary_expression_subkind<ir::BinaryOperator::UNSIGNED_GREATER_OR_EQUAL, L>::value, L, R>
operator>=(const ExpressionBase<L> &left, const ExpressionBase<R> &right) {
    return BinaryExpression<binary_expression_subkind<ir::BinaryOperator::UNSIGNED_GREATER_OR_EQUAL, L>::value, L, R>(left.derived(), right.derived(), 1);
}

template<class L, class R>
inline ChoiceExpression<L, R>
choice(const ExpressionBase<L> &first, const ExpressionBase<R> &second) {
    return ChoiceExpression<L, R>(first.derived(), second.derived());
}

template<class E>
inline KillStatement<E>
kill(const ExpressionBase<E> &expression) {
    return KillStatement<E>(expression.derived());
}

template<class E>
inline JumpStatement<NullExpression, E>
jump(const ExpressionBase<E> &thenAddress) {
    return JumpStatement<NullExpression, E>(NullExpression(), thenAddress.derived(), NULL, NULL);
}

inline JumpStatement<NullExpression, NullExpression>
jump(ir::BasicBlock *targetBasicBlock) {
    return JumpStatement<NullExpression, NullExpression>(NullExpression(), NullExpression(), targetBasicBlock, NULL);
}

template<class L, class R>
inline JumpStatement<L, R>
jump(const ExpressionBase<L> &condition, const ExpressionBase<R> &thenAddress, ir::BasicBlock *elseTarget) {
    return JumpStatement<L, R>(condition.derived(), thenAddress.derived(), NULL, elseTarget);
}

template<class E>
inline JumpStatement<E, NullExpression>
jump(const ExpressionBase<E> &condition, ir::BasicBlock *thenBasicBlock, ir::BasicBlock *elseTarget) {
    return JumpStatement<E, NullExpression>(condition.derived(), NullExpression(), thenBasicBlock, elseTarget);
}

template<class E>
inline CallStatement<E>
call(const ExpressionBase<E> &target) {
    return CallStatement<E>(target.derived());
}

inline ReturnStatement
return_() {
    return ReturnStatement();
}

template<class L, class R>
inline SequenceStatement<L, R>
operator,(const StatementBase<L> &first, const StatementBase<R> &second) {
    return SequenceStatement<L, R>(first.derived(), second.derived());
}


// -------------------------------------------------------------------------- //
// ExpressionFactory
// -------------------------------------------------------------------------- //
/**
 * Factory class that converts instruction analyzer expressions into terms and
 * statements.
 */
template<class Derived>
class ExpressionFactory {
    Q_DECLARE_TR_FUNCTIONS(ExpressionFactory)
public:
    ExpressionFactory(arch::Architecture *architecture, const arch::Instruction *instruction):
        mArchitecture(architecture),
        mInstructionAnalyzer(architecture->instructionAnalyzer()),
        mInstruction(instruction)
    {}

    arch::Architecture *architecture() const {
        return mArchitecture;
    }

    const InstructionAnalyzer *instructionAnalyzer() const {
        return mInstructionAnalyzer;
    }

    const Instruction *instruction() const {
        return mInstruction;
    }

    /**
     * \param expression               Expression to create term from.
     * \returns                        Newly created term for the given expression,
     *                                 with expression size determined automatically.
     */
    template<class E>
    std::unique_ptr<ir::Term> createTerm(const ExpressionBase<E> &expression) const {
        auto result(derived().doCreateTerm(expression.derived()));

        if (result && result->size() != expression.size()) {
            throw InvalidInstructionException(tr("term %1 created from expression of size %2 has completely different size %3")
                .arg(result->toString()).arg(expression.size()).arg(result->size()));
        }

        return result;
    }

    /**
     * \param statement                Statement to create IR statement from.
     * \returns                        Newly created ir statement for the given statement.
     */
    template<class E>
    std::unique_ptr<ir::Statement> createStatement(StatementBase<E> &statement) const {
        return derived().doCreateStatement(statement.derived());
    }

    /**
     * \param expression               Expression.
     * \param suggestedSize            Suggested size of the expression.
     */
    template<class E>
    void computeSize(ExpressionBase<E> &expression, SmallBitSize suggestedSize) const {
        derived().doComputeSize(expression.derived(), suggestedSize);
    }

protected:
    /* Overload-based dispatch functions follow.
     *
     * They can be overridden and extended in derived class thanks to CRTP
     * (http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern). */

    /**
     * \param expression               Intrinsic expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    std::unique_ptr<ir::Term> doCreateTerm(const IntrinsicExpression &expression) const {
        NC_UNUSED(expression);

        if (!expression.size()) {
            throw InvalidInstructionException(tr("size of an intrinsic expression is unknown"));
        }

        return std::make_unique<ir::Intrinsic>(ir::Intrinsic::UNKNOWN, expression.size());
    }

    /**
     * \param expression               Undefined expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    std::unique_ptr<ir::Term> doCreateTerm(const UndefinedExpression &expression) const {
        NC_UNUSED(expression);

        if (!expression.size()) {
            throw InvalidInstructionException(tr("size of an undefined expression is unknown"));
        }

        return std::make_unique<ir::Undefined>(expression.size());
    }

    /**
     * \param expression               Constant expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    std::unique_ptr<ir::Term> doCreateTerm(const ConstantExpression &expression) const {
        if (!expression.size()) {
            throw InvalidInstructionException(tr("size of a constant expression is unknown"));
        }

        return std::make_unique<ir::Constant>(SizedValue(expression.value(), expression.size()));
    }

    /**
     * \param expression               Operand expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    std::unique_ptr<ir::Term> doCreateTerm(const OperandExpression &expression) const {
        return mInstructionAnalyzer->createTerm(expression.operand());
    }

    /**
     * \param expression               Instruction operand expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    std::unique_ptr<ir::Term> doCreateTerm(const InstructionOperandExpression &expression) const {
        return mInstructionAnalyzer->createTerm(mInstruction->operand(expression.index()));
    }

    /**
     * \param expression               Register expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    std::unique_ptr<ir::Term> doCreateTerm(const RegisterExpression &expression) const {
        return mInstructionAnalyzer->createTerm(mArchitecture->registerOperand(expression.number()));
    }

    /**
     * \param expression               Memory location expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    std::unique_ptr<ir::Term> doCreateTerm(const MemoryLocationExpression &expression) const {
        return std::make_unique<ir::MemoryLocationAccess>(expression.memoryLocation());
    }

    /**
     * \param expression               Unary expression.
     * \returns                        Newly created term for the given expression.
     */
    template<int subkind, class E>
    std::unique_ptr<ir::Term> doCreateTerm(const UnaryExpression<subkind, E> &expression) const {
        if (!expression.size()) {
            throw InvalidInstructionException(tr("size of a unary expression is unknown"));
        }

        return std::make_unique<ir::UnaryOperator>(subkind, createTerm(expression.operand()), expression.size());
    }

    /**
     * \param expression               Dereference expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    template<class E>
    std::unique_ptr<ir::Term> doCreateTerm(const DereferenceExpression<E> &expression) const {
        if (!expression.size()) {
            throw InvalidInstructionException(tr("size of a dereference expression is unknown"));
        }

        return std::make_unique<ir::Dereference>(createTerm(expression.operand()), ir::MemoryDomain::MEMORY, expression.size());
    }

    /**
     * \param expression               Dereference expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    template<int signedness, class E>
    std::unique_ptr<ir::Term> doCreateTerm(const SignExpression<signedness, E> &expression) const {
        return createTerm(expression.operand());
    }

    /**
     * \param expression               Binary expression that change size to create term from.
     * \returns                        Newly created term for the given expression.
     */
    template<int subkind, class L, class R>
    std::unique_ptr<ir::Term> doCreateTerm(const BinaryExpression<subkind, L, R> &expression) const {
        return std::make_unique<ir::BinaryOperator>(subkind, createTerm(expression.left()), createTerm(expression.right()), expression.size());
    }

    /**
     * \param expression               Choice expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    template<class L, class R>
    std::unique_ptr<ir::Term> doCreateTerm(const ChoiceExpression<L, R> &expression) const {
        return std::make_unique<ir::Choice>(createTerm(expression.left()), createTerm(expression.right()));
    }

    /**
     * \param expression               Null expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    std::unique_ptr<ir::Term> doCreateTerm(const NullExpression &expression) const {
        NC_UNUSED(expression);

        return std::unique_ptr<ir::Term>();
    }

    /**
     * \param statement                Return statement to create IR statement from.
     * \returns                        Newly created ir statement for the given statement.
     */
    std::unique_ptr<ir::Statement> doCreateStatement(ReturnStatement &statement) const {
        NC_UNUSED(statement);

        return std::make_unique<ir::Return>();
    }

    /**
     * \param statement                Kill statement to create IR statement from.
     * \returns                        Newly created ir statement for the given statement.
     */
    template<class E>
    std::unique_ptr<ir::Statement> doCreateStatement(KillStatement<E> &statement) const {
        computeSize(statement.expression(), 0);

        return std::make_unique<ir::Kill>(createTerm(statement.expression()));
    }

    /**
     * \param statement                Jump statement to create IR statement from.
     * \returns                        Newly created ir statement for the given statement.
     */
    template<class C, class A>
    std::unique_ptr<ir::Statement> doCreateStatement(JumpStatement<C, A> &statement) const {
        computeSize(statement.condition(), 1);
        computeSize(statement.address(), mArchitecture->bitness());

        auto condition = createTerm(statement.condition());
        auto address = createTerm(statement.address());

        ir::JumpTarget thenTarget;
        if (address) {
            thenTarget.setAddress(std::move(address));
        } else {
            thenTarget.setBasicBlock(statement.thenBasicBlock());
        }

        ir::JumpTarget elseTarget;
        if (statement.elseBasicBlock()) {
            elseTarget.setBasicBlock(statement.elseBasicBlock());
        }

        if (condition) {
            return std::make_unique<ir::Jump>(std::move(condition), std::move(thenTarget), std::move(elseTarget));
        } else {
            return std::make_unique<ir::Jump>(std::move(thenTarget));
        }
    }

    /**
     * \param statement                Call statement to create IR statement from.
     * \returns                        Newly created ir statement for the given statement.
     */
    template<class E>
    std::unique_ptr<ir::Statement> doCreateStatement(CallStatement<E> &statement) const {
        computeSize(statement.expression(), mArchitecture->bitness());

        return std::make_unique<ir::Call>(createTerm(statement.expression()));
    }

    /**
     * \param statement                Assignment statement to create IR statement from.
     * \returns                        Newly created ir statement for the given statement.
     */
    template<class L, class R>
    std::unique_ptr<ir::Statement> doCreateStatement(AssignmentStatement<L, R> &statement) const {
        computeSize(statement.left(), 0);
        computeSize(statement.right(), 0);

        if (!statement.left().size() && statement.right().size()) {
            computeSize(statement.left(), statement.right().size());
        } else if (statement.left().size() && !statement.right().size()) {
            computeSize(statement.right(), statement.left().size());
        }

        if (statement.left().size() != statement.right().size()) {
            throw core::arch::irgen::InvalidInstructionException(tr("assigning expression of different sizes: %1 and %2")
                .arg(statement.left().size()).arg(statement.right().size()));
        }

        return std::make_unique<ir::Assignment>(createTerm(statement.left()), createTerm(statement.right()));
    }

    /**
     * A 'catch-all' implementation of the size calculation. This is the function
     * that will be called if no suitable overload is found.
     *
     * \param expression               Expression.
     * \param suggestedSize            Suggested size of the expression.
     */
    template<class E>
    void doComputeSize(ExpressionBase<E> &expression, SmallBitSize suggestedSize) const {
        NC_UNUSED(expression);

        if (!expression.size() && suggestedSize) {
            expression.setSize(suggestedSize);
        }
    }

    /**
     * \param expression               Operand expression.
     * \param suggestedSize            Suggested size of the expression.
     */
    void doComputeSize(OperandExpression &expression, SmallBitSize suggestedSize) const {
        NC_UNUSED(suggestedSize);

        if (!expression.size()) {
            expression.setSize(expression.operand()->size());
        }
    }

    /**
     * \param expression               Instruction operand expression.
     * \param suggestedSize            Suggested size of the expression.
     */
    void doComputeSize(InstructionOperandExpression &expression, SmallBitSize suggestedSize) const {
        NC_UNUSED(suggestedSize);

        if (!expression.size()) {
            expression.setSize(mInstruction->operand(expression.index())->size());
        }
    }

    /**
     * \param expression               Register expression.
     * \param suggestedSize            Suggested size of the expression.
     */
    void doComputeSize(RegisterExpression &expression, SmallBitSize suggestedSize) const {
        NC_UNUSED(suggestedSize);

        if (!expression.size()) {
            expression.setSize(mArchitecture->registerOperand(expression.number())->size());
        }
    }

    /**
     * \param expression               Memory location expression.
     * \param suggestedSize            Suggested size of the expression.
     */
    void doComputeSize(MemoryLocationExpression &expression, SmallBitSize suggestedSize) const {
        NC_UNUSED(suggestedSize);

        if (!expression.size()) {
            expression.setSize(expression.memoryLocation().size());
        }
    }

    /**
     * \param expression               Unary expression.
     * \param suggestedSize            Suggested size of the expression.
     */
    template<int subkind, class E>
    void doComputeSize(UnaryExpression<subkind, E> &expression, SmallBitSize suggestedSize) const {
        if (expression.size()) {
            computeSize(expression.operand(), expression.size());
        } else {
            computeSize(expression.operand(), suggestedSize);
            switch (subkind) {
                case ir::UnaryOperator::SIGN_EXTEND:
                case ir::UnaryOperator::ZERO_EXTEND:
                case ir::UnaryOperator::RESIZE:
                    if (suggestedSize) {
                        expression.setSize(suggestedSize);
                    }
                    break;
                default:
                    if (expression.operand().size()) {
                        expression.setSize(expression.operand().size());
                    }
                    break;
            }
        }
    }

    /**
     * \param expression               Dereference.
     * \param suggestedSize            Suggested size of the expression.
     */
    template<class E>
    void doComputeSize(DereferenceExpression<E> &expression, SmallBitSize suggestedSize) const {
        computeSize(expression.operand(), suggestedSize);

        if (!expression.size() && suggestedSize) {
            expression.setSize(suggestedSize);
        }
    }

    /**
     * \param expression               Sign expression.
     * \param suggestedSize            Suggested size of the expression.
     */
    template<int signedness, class E>
    void doComputeSize(SignExpression<signedness, E> &expression, SmallBitSize suggestedSize) const {
        if (expression.size()) {
            computeSize(expression.operand(), expression.size());
        } else {
            computeSize(expression.operand(), suggestedSize);
            expression.setSize(expression.operand().size());
        }
    }

    /**
     * \param expression               Binary expression (Binary operation or Choice).
     * \param suggestedSize            Suggested size of the expression.
     */
    template<class L, class R, class D>
    void doComputeSize(BinaryExpressionBase<L, R, D> &expression, SmallBitSize suggestedSize) const {
        computeSize(expression.left(), 0);
        computeSize(expression.right(), 0);

        if (!expression.left().size() || !expression.right().size()) {
            /* If at least one of the operands knows its size by itself, take it as suggested. */
            if (expression.left().size() || expression.right().size()) {
                suggestedSize = std::max(expression.left().size(), expression.right().size());
            }

            /* If there are still no ideas, use my own size. */
            if (!suggestedSize) {
                suggestedSize = expression.size();
            }

            if (suggestedSize) {
                if (!expression.left().size()) {
                    computeSize(expression.left(), suggestedSize);
                }
                if (!expression.right().size()) {
                    computeSize(expression.right(), suggestedSize);
                }
            }
        }

        if (!expression.size()) {
            expression.setSize(std::max(expression.left().size(), expression.right().size()));
        }
    }

private:
    const Derived &derived() const {
        return static_cast<const Derived &>(*this);
    }

    Architecture *mArchitecture;
    const InstructionAnalyzer *mInstructionAnalyzer;
    const Instruction *mInstruction;
};


// -------------------------------------------------------------------------- //
// ExpressionFactoryCallback
// -------------------------------------------------------------------------- //
/**
 * Convenience class that handles sequential statements by adding them to
 * the given basic block.
 */
template<class ExpressionFactory>
class ExpressionFactoryCallback {
public:
    /**
     * Constructor.
     *
     * \param factory                   Expression factory to use to create separate statements.
     * \param basicBlock                Basic block to add statements to.
     */
    ExpressionFactoryCallback(ExpressionFactory &factory, ir::BasicBlock *basicBlock):
        mFactory(factory),
        mBasicBlock(basicBlock)
    {
        assert(basicBlock != NULL);
    }

    /**
     * \return Basic block to which statements are added.
     */
    ir::BasicBlock *basicBlock() const { return mBasicBlock; }

    /**
     * \param statement                 Call statement to create IR statement from.
     *                                  Created statement will be added to the basic block.
     */
    template<class E>
    void operator[](StatementBase<E> &statement) const {
        doCallback(statement.derived());
    }

    /**
     * \param statement                 Call statement to create IR statement from.
     *                                  Created statement will be added to the basic block.
     */
    template<class E>
    void operator[](StatementBase<E> &&statement) const {
        doCallback(statement.derived());
    }

    void operator()(std::unique_ptr<ir::Statement> statement) const {
        statement->setInstruction(mFactory.instruction());
        mBasicBlock->addStatement(std::move(statement));
    }

protected:
    /* Overload-based dispatch functions follow. */

    template<class E>
    void doCallback(StatementBase<E> &statement) const {
        this->operator()(mFactory.createStatement(statement));
    }

    template<class L, class R>
    void doCallback(SequenceStatement<L, R> &statement) const {
        this->operator[](statement.left());
        this->operator[](statement.right());
    }

private:
    ExpressionFactory &mFactory;
    ir::BasicBlock *mBasicBlock;
};


}}}}} // namespace nc::core::arch::irgen::expressions

/* vim:set et sts=4 sw=4: */
