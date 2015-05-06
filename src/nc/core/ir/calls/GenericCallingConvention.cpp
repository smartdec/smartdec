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

#include "GenericCallingConvention.h"

#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/core/arch/Operands.h>
#include <nc/core/ir/Statement.h>
#include <nc/core/ir/Term.h>

#include "GenericDescriptorAnalyzer.h"

namespace nc {
namespace core {
namespace ir {
namespace calls {

Argument &Argument::operator<<(const arch::RegisterOperand *reg) {
    assert(reg != NULL);

    return *this << reg->memoryLocation();
}

GenericCallingConvention::~GenericCallingConvention() {
    foreach (const Term *term, returnValues_) {
        delete term;
    }
    foreach (const Statement *statement, entryStatements_) {
        delete statement;
    }
}

std::unique_ptr<DescriptorAnalyzer> GenericCallingConvention::createDescriptorAnalyzer() const {
    return std::make_unique<GenericDescriptorAnalyzer>(this);
}

void GenericCallingConvention::addReturnValue(std::unique_ptr<Term> term) {
    assert(term != NULL);

    returnValues_.reserve(returnValues_.size() + 1);
    returnValues_.push_back(term.release());
}

void GenericCallingConvention::addEnterStatement(std::unique_ptr<Statement> statement) {
    assert(statement != NULL);

    entryStatements_.reserve(entryStatements_.size() + 1);
    entryStatements_.push_back(statement.release());
}

} // namespace calls
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
