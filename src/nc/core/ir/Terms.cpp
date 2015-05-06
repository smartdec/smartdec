//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "Terms.h"

#include <QTextStream>

#include <nc/common/Foreach.h>
#include <nc/common/SizedValue.h>
#include <nc/common/Unreachable.h>

namespace nc {
namespace core {
namespace ir {

void Constant::print(QTextStream &out) const {
    int integerBase = out.integerBase();
    hex(out) << "0x" << value().value();
    out.setIntegerBase(integerBase);
}

void Intrinsic::print(QTextStream &out) const {
    out << "intrinsic";
}

void Undefined::print(QTextStream &out) const {
    out << "undefined";
}

MemoryLocationAccess::MemoryLocationAccess(const MemoryLocation &memoryLocation):
    Term(MEMORY_LOCATION_ACCESS, memoryLocation.size<SmallBitSize>()), memoryLocation_(memoryLocation)
{}

void MemoryLocationAccess::print(QTextStream &out) const {
    out << memoryLocation_;
}

Dereference::Dereference(std::unique_ptr<Term> address, Domain domain, SmallBitSize size):
    Term(DEREFERENCE, size), domain_(domain), address_(std::move(address))
{
    address_->initFlags(READ);
}

void Dereference::visitChildTerms(Visitor<Term> &visitor) {
    visitor(address_.get());
}

void Dereference::visitChildTerms(Visitor<const Term> &visitor) const {
    visitor(address_.get());
}

void Dereference::print(QTextStream &out) const {
    out << "[" << *address() << "]";
}

UnaryOperator::UnaryOperator(int operatorKind, std::unique_ptr<Term> operand, SmallBitSize size):
    Term(UNARY_OPERATOR, size), 
    operatorKind_(operatorKind), 
    operand_(std::move(operand))
{
    assert(operand_ != NULL);

    operand_->initFlags(READ);
}

UnaryOperator::UnaryOperator(int operatorKind, std::unique_ptr<Term> operand):
    Term(UNARY_OPERATOR, operand->size()), 
    operatorKind_(operatorKind), 
    operand_(std::move(operand))
{
    operand_->initFlags(READ);
}

void UnaryOperator::visitChildTerms(Visitor<Term> &visitor) {
    visitor(operand_.get());
}

void UnaryOperator::visitChildTerms(Visitor<const Term> &visitor) const {
    visitor(operand_.get());
}

boost::optional<SizedValue> UnaryOperator::apply(const SizedValue &a) const {
    switch (operatorKind()) {
        case BITWISE_NOT:
            return SizedValue(~a.value(), size());
        case LOGICAL_NOT:
            return SizedValue(!a.value(), size());
        case NEGATION:
            return SizedValue(-a.signedValue(), size());
        case SIGN_EXTEND:
            return SizedValue(a.signedValue(), size());
        case ZERO_EXTEND:
            return SizedValue(a.value(), size());
        case RESIZE:
            return SizedValue(a.value(), size());
        default:
            unreachable();
            return boost::none;
    }
}

void UnaryOperator::print(QTextStream &out) const {
    switch (operatorKind()) {
        case BITWISE_NOT:
            out << '~';
            break;
        case LOGICAL_NOT:
            out << '!';
            break;
        case NEGATION:
            out << '-';
            break;
        case SIGN_EXTEND:
            out << "sign_extend ";
            break;
        case ZERO_EXTEND:
            out << "zero_extend ";
            break;
        case RESIZE:
            out << "resize ";
            break;
        default:
            unreachable();
            break;
    }
    out << *operand();
}

BinaryOperator::BinaryOperator(int operatorKind, std::unique_ptr<Term> left, std::unique_ptr<Term> right, SmallBitSize size):
    Term(BINARY_OPERATOR, size), operatorKind_(operatorKind), left_(std::move(left)), right_(std::move(right))
{
    assert(left_ != NULL);
    assert(right_ != NULL);

    left_->initFlags(READ);
    right_->initFlags(READ);
}

BinaryOperator::BinaryOperator(int operatorKind, std::unique_ptr<Term> left, std::unique_ptr<Term> right):
    Term(BINARY_OPERATOR, left->size()), operatorKind_(operatorKind), left_(std::move(left)), right_(std::move(right))
{
    left_->initFlags(READ);
    right_->initFlags(READ);
}

void BinaryOperator::visitChildTerms(Visitor<Term> &visitor) {
    visitor(left_.get());
    visitor(right_.get());
}

void BinaryOperator::visitChildTerms(Visitor<const Term> &visitor) const {
    visitor(left_.get());
    visitor(right_.get());
}

boost::optional<SizedValue> BinaryOperator::apply(const SizedValue &a, const SizedValue &b) const {
    switch (operatorKind()) {
        case ADD:
            return SizedValue(a.value() + b.value(), size());
        case SUB:
            return SizedValue(a.value() - b.value(), size());
        case MUL:
            return SizedValue(a.value() * b.value(), size());
        case SIGNED_DIV:
            if (b.value() != 0) {
                return SizedValue(a.signedValue() / b.signedValue(), size());
            } else {
                return boost::none;
            }
        case UNSIGNED_DIV:
            if (b.value() != 0) {
                return SizedValue(a.value() / b.value(), size());
            } else {
                return boost::none;
            }
        case SIGNED_REM:
            if (b.value() != 0) {
                return SizedValue(a.signedValue() % b.signedValue(), size());
            } else {
                return boost::none;
            }
        case UNSIGNED_REM:
            if (b.value() != 0) {
                return SizedValue(a.value() % b.value(), size());
            } else {
                return boost::none;
            }
        case BITWISE_AND:
            return SizedValue(a.value() & b.value(), size());
        case LOGICAL_AND:
            return SizedValue(a.value() && b.value(), size());
        case BITWISE_OR:
            return SizedValue(a.value() | b.value(), size());
        case LOGICAL_OR:
            return SizedValue(a.value() || b.value(), size());
        case BITWISE_XOR:
            return SizedValue(a.value() ^ b.value(), size());
        case SHL:
            return SizedValue(a.value() << b.value(), size());
        case SHR:
            return SizedValue(a.value() >> b.value(), size());
        case SAR:
            return SizedValue(a.signedValue() >> b.value(), size());
        case EQUAL:
            return SizedValue(a.value() == b.value(), size());
        case SIGNED_LESS:
            return SizedValue(a.signedValue() < b.signedValue(), size());
        case SIGNED_LESS_OR_EQUAL:
            return SizedValue(a.signedValue() <= b.signedValue(), size());
        case SIGNED_GREATER:
            return SizedValue(a.signedValue() > b.signedValue(), size());
        case SIGNED_GREATER_OR_EQUAL:
            return SizedValue(a.signedValue() >= b.signedValue(), size());
        case UNSIGNED_LESS:
            return SizedValue(a.value() < b.value(), size());
        case UNSIGNED_LESS_OR_EQUAL:
            return SizedValue(a.value() <= b.value(), size());
        case UNSIGNED_GREATER:
            return SizedValue(a.value() > b.value(), size());
        case UNSIGNED_GREATER_OR_EQUAL:
            return SizedValue(a.value() >= b.value(), size());
        default:
            unreachable();
            return boost::none;
    }
}

BinaryOperator *BinaryOperator::doClone() const {
    return new BinaryOperator(operatorKind(), left()->clone(), right()->clone(), size());
}

void BinaryOperator::print(QTextStream &out) const {
    out << '(' << *left() << ' ';
    switch (operatorKind()) {
        case ADD:
            out << '+';
            break;
        case SUB:
            out << '-';
            break;
        case MUL:
            out << '*';
            break;
        case SIGNED_DIV:
            out << "(signed)/";
            break;
        case UNSIGNED_DIV:
            out << "(unsigned)/";
            break;
        case SIGNED_REM:
            out << "(signed)%";
            break;
        case UNSIGNED_REM:
            out << "(unsigned)%";
            break;
        case BITWISE_AND:
            out << '&';
            break;
        case LOGICAL_AND:
            out << "&&";
            break;
        case BITWISE_OR:
            out << '|';
            break;
        case LOGICAL_OR:
            out << "||";
            break;
        case BITWISE_XOR:
            out << '^';
            break;
        case SHL:
            out << "<<";
            break;
        case SHR:
            out << ">>>";
            break;
        case SAR:
            out << ">>";
            break;
        case EQUAL:
            out << "==";
            break;
        case SIGNED_LESS:
            out << "(signed)<";
            break;
        case SIGNED_LESS_OR_EQUAL:
            out << "(signed)<=";
            break;
        case SIGNED_GREATER:
            out << "(signed)>";
            break;
        case SIGNED_GREATER_OR_EQUAL:
            out << "(signed)>=";
            break;
        case UNSIGNED_LESS:
            out << "(unsigned)<";
            break;
        case UNSIGNED_LESS_OR_EQUAL:
            out << "(unsigned)<=";
            break;
        case UNSIGNED_GREATER:
            out << "(unsigned)>";
            break;
        case UNSIGNED_GREATER_OR_EQUAL:
            out << "(unsigned)>=";
            break;
        default:
            unreachable();
            break;
    }
    out << ' ' << *right() << ')';
}


Choice::Choice(std::unique_ptr<Term> preferredTerm, std::unique_ptr<Term> defaultTerm):
    Term(CHOICE, preferredTerm->size()), preferredTerm_(std::move(preferredTerm)), defaultTerm_(std::move(defaultTerm))
{
    assert(preferredTerm_ != NULL);
    assert(defaultTerm_ != NULL);
    assert(preferredTerm_->size() == defaultTerm_->size());

    preferredTerm_->initFlags(READ);
    defaultTerm_->initFlags(READ);
}

Choice *Choice::doClone() const {
    return new Choice(preferredTerm()->clone(), defaultTerm()->clone());
}

void Choice::visitChildTerms(Visitor<Term> &visitor) {
    visitor(preferredTerm_.get());
    visitor(defaultTerm_.get());
}

void Choice::visitChildTerms(Visitor<const Term> &visitor) const {
    visitor(preferredTerm_.get());
    visitor(defaultTerm_.get());
}

void Choice::print(QTextStream &out) const {
    out << "choice(" << *preferredTerm() << " over " << *defaultTerm() << ')';
}

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
