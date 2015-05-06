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

#include "Statements.h"

#include <QTextStream>

#include <nc/core/arch/Instruction.h>

namespace nc {
namespace core {
namespace ir {

Comment *Comment::doClone() const {
    return new Comment(text());
}

void Comment::print(QTextStream &out) const {
    out << text_ << endl;
}

InlineAssembly *InlineAssembly::doClone() const {
    return new InlineAssembly();
}

void InlineAssembly::print(QTextStream &out) const {
    out << "asm { ";
    if (instruction()) {
        out << *instruction();
    }
    out << " }" << endl;
}

Assignment::Assignment(std::unique_ptr<Term> left, std::unique_ptr<Term> right):
    Statement(ASSIGNMENT), left_(std::move(left)), right_(std::move(right))
{
    assert(left_ && right_ && left_->size() == right_->size());

    left_->initFlags(Term::WRITE, right_.get());
    right_->initFlags(Term::READ);

    left_->setStatementRecursively(this);
    right_->setStatementRecursively(this);
}

void Assignment::visitChildTerms(Visitor<Term> &visitor) {
    visitor(left_.get());
    visitor(right_.get());
}

void Assignment::visitChildTerms(Visitor<const Term> &visitor) const {
    visitor(left_.get());
    visitor(right_.get());
}

Assignment *Assignment::doClone() const {
    return new Assignment(left()->clone(), right()->clone());
}

void Assignment::print(QTextStream &out) const {
    out << *left_ << " = " << *right_ << endl;
}

Kill::Kill(std::unique_ptr<Term> term):
    Statement(KILL), term_(std::move(term))
{
    assert(term_);

    term_->initFlags(Term::KILL);
    term_->setStatementRecursively(this);
}

void Kill::visitChildTerms(Visitor<Term> &visitor) {
    visitor(term_.get());
}

void Kill::visitChildTerms(Visitor<const Term> &visitor) const {
    visitor(term_.get());
}

Kill *Kill::doClone() const {
    return new Kill(term()->clone());
}

void Kill::print(QTextStream &out) const {
    out << "kill(" << *term_ << ")" << endl;
}

Call::Call(std::unique_ptr<Term> target):
    Statement(CALL), 
    target_(std::move(target))
{
    assert(target_ != NULL && "Jump target must be not NULL.");

    target_->initFlags(Term::READ);
    target_->setStatementRecursively(this);
}

void Call::visitChildTerms(Visitor<Term> &visitor) {
    visitor(target_.get());
}

void Call::visitChildTerms(Visitor<const Term> &visitor) const {
    visitor(target_.get());
}

void Call::print(QTextStream &out) const {
    out << "call " << *target_ << endl;
}

Return *Return::doClone() const {
    return new Return();
}

void Return::print(QTextStream &out) const {
    out << "return" << endl;
}

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
