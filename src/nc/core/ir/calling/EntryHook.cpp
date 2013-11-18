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

#include <nc/common/make_unique.h>

#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/DataflowAnalyzer.h>
#include <nc/core/ir/dflow/ExecutionContext.h>
#include <nc/core/ir/dflow/Value.h>

#include <nc/common/Foreach.h>

#include "CallingConvention.h"
#include "Signature.h"

namespace nc {
namespace core {
namespace ir {
namespace calling {

EntryHook::EntryHook(const CallingConvention *convention, const Signature *signature) {
    assert(convention != NULL);

    stackPointer_ = std::make_unique<MemoryLocationAccess>(convention->stackPointer());
    stackPointer_->setAccessType(Term::WRITE);

    entryStatements_.reserve(convention->entryStatements().size());
    foreach (const Statement *statement, convention->entryStatements()) {
        entryStatements_.push_back(statement->clone());
    }

    if (signature) {
        foreach (const auto &location, signature->arguments()) {
            arguments_[location] = std::make_unique<MemoryLocationAccess>(location);
        }
    }

    // TODO: remove
#if 0
    /* Precompute the set of memory locations used for passing arguments. */
    foreach (const ArgumentGroup &group, convention->argumentGroups()) {
        foreach (const Argument &argument, group.arguments()) {
            possibleArgumentLocations_.insert(argument.locations().begin(), argument.locations().end());
        }
    }
#endif
}

EntryHook::~EntryHook() {}

void EntryHook::execute(dflow::ExecutionContext &context) {
    // TODO: move
#if 0
    /*
     * Detect all stack arguments used.
     */
    if (context.fixpointReached()) {
        misc::CensusVisitor census(NULL);
        census(context.analyzer().function());

        /* Estimate memory locations of arguments. */
        foreach (const Term *term, census.terms()) {
            MemoryLocation memoryLocation = context.analyzer().dataflow().getMemoryLocation(term);

            /*
             * If one reads a register that has not been defined
             * and the register can be used for passing arguments,
             * then this register is used for passing an argument.
             *
             * If one reads or writes a location on the stack
             * which lies above the call return address,
             * then this location is used for passing an argument.
             */
            if (memoryLocation && term->isRead() &&
                ((contains(possibleArgumentLocations_, memoryLocation) &&
                  context.analyzer().dataflow().getDefinitions(term).empty()) ||
                 (memoryLocation.domain() == MemoryDomain::STACK &&
                  memoryLocation.addr() >= convention()->firstArgumentOffset())))
            {
                getArgumentTerm(memoryLocation);
            }
        }

        argumentLocations_.clear();
        std::transform(arguments_.begin(), arguments_.end(), std::back_inserter(argumentLocations_),
            [](std::pair<const MemoryLocation, std::unique_ptr<Term>> &x) { return x.first; });
    }
#endif

    /**
     * Set stack pointer offset to zero and execute the term.
     */
    context.analyzer().dataflow().getValue(stackPointer_.get())->makeStackOffset(0);
    context.analyzer().execute(stackPointer_.get(), context);

    /*
     * Execute entry statements.
     */
    foreach (auto &statement, entryStatements_) {
        context.analyzer().execute(statement.get(), context);
    }

    /*
     * Execute all arguments terms.
     */
    foreach (const auto &argument, arguments_) {
        context.analyzer().execute(argument.second.get(), context);
    }
}

const Term *EntryHook::getArgumentTerm(const MemoryLocation &memoryLocation) {
    return nc::find(arguments_, memoryLocation).get();
}

void EntryHook::visitChildStatements(Visitor<const Statement> &visitor) const {
    foreach (const auto &statement, entryStatements_) {
        visitor(statement.get());
    }
}

void EntryHook::visitChildTerms(Visitor<const Term> &visitor) const {
    visitor(stackPointer_.get());

    foreach (const auto &argument, arguments_) {
        visitor(argument.second.get());
    }
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
