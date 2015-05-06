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

#include "BasicBlock.h"

#include <QTextStream>

#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Term.h>

#include <nc/common/Escaping.h>
#include <nc/common/Foreach.h>

#include <boost/range/algorithm/find.hpp>

namespace nc {
namespace core {
namespace ir {

BasicBlock::BasicBlock(const boost::optional<ByteAddr> &address):
    address_(address), successorAddress_(address)
{}

BasicBlock::~BasicBlock() {}

void BasicBlock::setSuccessorAddress(const boost::optional<ByteAddr> &successorAddress) {
    assert((!successorAddress || address()) && "A non-memory-bound basic block cannot have a successor address.");
    successorAddress_ = successorAddress;
}

void BasicBlock::addStatement(std::unique_ptr<Statement> statement) {
    assert(statement != NULL);

    statement->setBasicBlock(this);
    statements_.push_back(std::move(statement));
}

void BasicBlock::addStatements(std::vector<std::pair<const Statement *, std::unique_ptr<Statement>>> &&addedStatements) {
    std::vector<std::unique_ptr<Statement>> newStatements;

    newStatements.reserve(statements_.size() + addedStatements.size());

    auto i    = addedStatements.begin();
    auto iend = addedStatements.end();

    foreach (auto &statement, statements_) {
        Statement *lastStatement = statement.get();
        newStatements.push_back(std::move(statement));

        while (i != iend && i->first == lastStatement) {
            assert(i->second != NULL);

            i->second->setBasicBlock(this);
            newStatements.push_back(std::move(i->second));

            ++i;
        }
    }

    assert(i == iend);

    statements_.swap(newStatements);
}

void BasicBlock::popBack() {
    assert(!statements_.empty());
    statements_.pop_back();
}

const Statement *BasicBlock::getTerminator() const {
    if (statements().empty()) {
        return NULL;
    }

    const Statement *terminator = statements().back();
    if (terminator->isJump() || terminator->isReturn()) {
        return terminator;
    }

    return NULL;
}

Jump *BasicBlock::getJump() {
    if (statements().empty()) {
        return NULL;
    } else {
        return statements().back()->as<Jump>();
    }
}

const Jump *BasicBlock::getJump() const {
    if (statements().empty()) {
        return NULL;
    } else {
        return statements().back()->as<Jump>();
    }
}

const Return *BasicBlock::getReturn() const {
    if (statements().empty()) {
        return NULL;
    } else {
        return statements().back()->as<Return>();
    }
}

std::unique_ptr<BasicBlock> BasicBlock::split(std::size_t index, const boost::optional<ByteAddr> &address) {
    assert(index <= statements_.size());

    /* Create a new basic block. */
    std::unique_ptr<BasicBlock> result(new BasicBlock(address));
    result->setSuccessorAddress(this->successorAddress());
    this->setSuccessorAddress(address);

    /* Move statements to it. */
    std::size_t size = statements_.size();
    result->statements_.reserve(size - index);
    for (std::size_t i = index; i < size; ++i) {
        result->addStatement(std::move(statements_[i]));
    }
    statements_.resize(index);

    return result;
}

std::unique_ptr<BasicBlock> BasicBlock::clone() const {
    std::unique_ptr<BasicBlock> result(new BasicBlock(address()));
    result->setSuccessorAddress(successorAddress());

    foreach (const Statement *statement, statements()) {
        result->addStatement(statement->clone());
    }

    return result;
}

void BasicBlock::print(QTextStream &out) const {
    out << "basicBlock" << this << " [shape=box,label=\"";

    QString label;
    QTextStream ls(&label);

    ls << "Address: ";
    if (address()) {
        int integerBase = ls.integerBase();
        hex(ls) << "0x" << *address();
        ls.setIntegerBase(integerBase);
    } else {
        ls << "None";
    }
    ls << endl;

    foreach (const Statement *statement, statements()) {
        ls << *statement;
    }

    out << escapeDotString(label) << "\"];" << endl;
}

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
