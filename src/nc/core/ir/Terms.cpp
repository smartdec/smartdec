/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

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

#include <nc/common/Unreachable.h>

namespace nc {
namespace core {
namespace ir {

Constant *Constant::doClone() const {
    return new Constant(value());
}

void Constant::print(QTextStream &out) const {
    int integerBase = out.integerBase();
    hex(out) << "0x" << value().value();
    out.setIntegerBase(integerBase);
}

Intrinsic *Intrinsic::doClone() const {
    return new Intrinsic(intrinsicKind(), size());
}

void Intrinsic::print(QTextStream &out) const {
    out << "intrinsic(" << intrinsicKind() << ")";
}

MemoryLocationAccess::MemoryLocationAccess(const MemoryLocation &memoryLocation):
    Term(MEMORY_LOCATION_ACCESS, memoryLocation.size<SmallBitSize>()), memoryLocation_(memoryLocation)
{}

MemoryLocationAccess *MemoryLocationAccess::doClone() const {
    return new MemoryLocationAccess(memoryLocation());
}

void MemoryLocationAccess::print(QTextStream &out) const {
    out << memoryLocation_;
}

Dereference::Dereference(std::unique_ptr<Term> address, Domain domain, SmallBitSize size):
    Term(DEREFERENCE, size), domain_(domain), address_(std::move(address))
{
    address_->setAccessType(READ);
}

Dereference *Dereference::doClone() const {
    return new Dereference(address()->clone(), domain(), size());
}

void Dereference::setStatement(const Statement *statement) {
    Term::setStatement(statement);
    address()->setStatement(statement);
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

    operand_->setAccessType(READ);

    switch (operatorKind) {
        case NOT: case NEGATION:
            assert(size == operand_->size());
            break;
        case SIGN_EXTEND: case ZERO_EXTEND:
            assert(size > operand_->size());
            break;
        case TRUNCATE:
            assert(size < operand_->size());
            break;
    }
}

UnaryOperator *UnaryOperator::doClone() const {
    return new UnaryOperator(operatorKind(), operand()->clone(), size());
}

void UnaryOperator::setStatement(const Statement *statement) {
    Term::setStatement(statement);
    operand()->setStatement(statement);
}

void UnaryOperator::print(QTextStream &out) const {
    switch (operatorKind()) {
        case NOT:
            out << '~';
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
        case TRUNCATE:
            out << "truncate ";
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

    left_->setAccessType(READ);
    right_->setAccessType(READ);

    switch (operatorKind) {
        case AND: case OR: case XOR:
        case ADD: case SUB: case MUL:
        case SIGNED_DIV: case SIGNED_REM:
        case UNSIGNED_DIV: case UNSIGNED_REM:
            assert(left_->size() == right_->size());
            assert(size == left_->size());
            break;

        case SHL: case SHR: case SAR:
            assert(size == left_->size());
            break;

        case EQUAL:
        case SIGNED_LESS: case SIGNED_LESS_OR_EQUAL:
        case UNSIGNED_LESS: case UNSIGNED_LESS_OR_EQUAL:
            assert(left_->size() == right_->size());
            assert(size == 1);
            break;
    }
}

BinaryOperator *BinaryOperator::doClone() const {
    return new BinaryOperator(operatorKind(), left()->clone(), right()->clone(), size());
}

void BinaryOperator::setStatement(const Statement *statement) {
    Term::setStatement(statement);
    left()->setStatement(statement);
    right()->setStatement(statement);
}

void BinaryOperator::print(QTextStream &out) const {
    out << '(' << *left() << ' ';
    switch (operatorKind()) {
        case AND:
            out << '&';
            break;
        case OR:
            out << '|';
            break;
        case XOR:
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
        case SIGNED_REM:
            out << "(signed)%";
            break;
        case UNSIGNED_DIV:
            out << "(unsigned)/";
            break;
        case UNSIGNED_REM:
            out << "(unsigned)%";
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
        case UNSIGNED_LESS:
            out << "(unsigned)<";
            break;
        case UNSIGNED_LESS_OR_EQUAL:
            out << "(unsigned)<=";
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

    preferredTerm_->setAccessType(READ);
    defaultTerm_->setAccessType(READ);
}

Choice *Choice::doClone() const {
    return new Choice(preferredTerm()->clone(), defaultTerm()->clone());
}

void Choice::setStatement(const Statement *statement) {
    Term::setStatement(statement);
    preferredTerm()->setStatement(statement);
    defaultTerm()->setStatement(statement);
}

void Choice::print(QTextStream &out) const {
    out << "choice(" << *preferredTerm() << " over " << *defaultTerm() << ')';
}

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
