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

#include "EntryHook.h"

#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/DataflowAnalyzer.h>
#include <nc/core/ir/dflow/ExecutionContext.h>
#include <nc/core/ir/dflow/Value.h>

#include "Convention.h"
#include "Signature.h"

namespace nc {
namespace core {
namespace ir {
namespace calling {

EntryHook::EntryHook(const Convention *convention, const Signature *signature) {
    assert(convention != NULL);

    stackPointer_ = std::make_unique<MemoryLocationAccess>(convention->stackPointer());
    stackPointer_->setAccessType(Term::WRITE);

    entryStatements_.reserve(convention->entryStatements().size());

    foreach (const Statement *statement, convention->entryStatements()) {
        entryStatements_.push_back(statement->clone());
    }

    if (signature) {
        foreach (const Term *argument, signature->arguments()) {
            auto term = argument->clone();
            term->setAccessType(Term::WRITE);
            argumentsSet_.insert(term.get());
            arguments_[argument] = std::move(term);
        }
    }
}

EntryHook::~EntryHook() {}

void EntryHook::execute(dflow::ExecutionContext &context) {
    /**
     * Set stack pointer offset to zero and execute the term.
     */
    auto value = context.analyzer().dataflow().getValue(stackPointer_.get());
    value->makeStackOffset(0);
    value->setAbstractValue(dflow::AbstractValue(stackPointer_->size(), -1, -1));
    context.analyzer().execute(stackPointer_.get(), context);

    /*
     * Execute all arguments terms.
     */
    foreach (const auto &argument, arguments_) {
        context.analyzer().execute(argument.second.get(), context);
    }

    /*
     * Execute entry statements.
     */
    foreach (auto &statement, entryStatements_) {
        context.analyzer().execute(statement.get(), context);
    }
}

const Term *EntryHook::getArgumentTerm(const Term *term) const {
    return nc::find(arguments_, term).get();
}

bool EntryHook::isArgumentTerm(const Term *term) const {
    return nc::contains(argumentsSet_, term);
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
