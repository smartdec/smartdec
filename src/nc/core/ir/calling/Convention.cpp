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

#include "Convention.h"

#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/core/arch/Operands.h>
#include <nc/core/ir/Statement.h>
#include <nc/core/ir/Term.h>

namespace nc {
namespace core {
namespace ir {
namespace calling {

Argument::Argument(const core::arch::Register *reg):
    location_(reg->memoryLocation())
{}

Convention::Convention(QString name): name_(std::move(name)), calleeCleanup_(false) {}

Convention::~Convention() {}

bool Convention::isArgumentLocation(const MemoryLocation &memoryLocation) const {
    if (memoryLocation.domain() == MemoryDomain::STACK &&
        memoryLocation.addr() >= firstArgumentOffset()
    ) {
        return true;
    }

    foreach (const auto &group, argumentGroups()) {
        foreach (const auto &argument, group.arguments()) {
            if (argument.location().covers(memoryLocation)) {
                return true;
            }
        }
    }

    return false;
}

void Convention::addReturnValue(std::unique_ptr<Term> term) {
    assert(term != NULL);
    returnValues_.push_back(std::move(term));
}

void Convention::addEnterStatement(std::unique_ptr<Statement> statement) {
    assert(statement != NULL);
    entryStatements_.push_back(std::move(statement));
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
