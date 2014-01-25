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

#include "CallHook.h"

#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/core/ir/MemoryDomain.h>
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

CallHook::CallHook(const Call *call, const Convention *convention, const Signature *signature,
    boost::optional<ByteSize> stackArgumentsSize)
{
    assert(call != NULL);
    assert(convention != NULL);

    if (signature) {
        foreach (const Term *argument, signature->arguments()) {
            arguments_[argument] = argument->clone();
        }
        if (signature->returnValue()) {
            returnValues_[signature->returnValue()] = signature->returnValue()->clone();
        }
    } else {
        foreach (auto term, convention->returnValues()) {
            returnValues_[term] = term->clone();
        }
    }

    foreach (const auto &pair, arguments_) {
        pair.second->setAccessType(Term::READ);
        pair.second->setStatementRecursively(call);
    }
    foreach (const auto &pair, returnValues_) {
        pair.second->setAccessType(Term::WRITE);
        pair.second->setStatementRecursively(call);
    }

    if (convention->calleeCleanup() && stackArgumentsSize) {
        cleanupStatement_ = std::make_unique<Assignment>(
            std::make_unique<MemoryLocationAccess>(convention->stackPointer()),
            std::make_unique<BinaryOperator>(
                BinaryOperator::ADD,
                std::make_unique<MemoryLocationAccess>(convention->stackPointer()),
                std::make_unique<Constant>(SizedValue(convention->stackPointer().size(), *stackArgumentsSize)),
                convention->stackPointer().size<SmallBitSize>()));
    }

    stackPointer_ = std::make_unique<MemoryLocationAccess>(convention->stackPointer());
    stackPointer_->setAccessType(Term::READ);
}

CallHook::~CallHook() {}

void CallHook::execute(dflow::ExecutionContext &context) {
    /* Remember reaching definitions. */
    reachingDefinitions_ = context.definitions();

    /* Execute all argument terms. */
    foreach (const auto &pair, arguments_) {
        context.analyzer().execute(pair.second.get(), context);
    }

    /* Execute the stack pointer. */
    if (stackPointer_) {
        context.analyzer().execute(stackPointer_.get(), context);
    }

    /* Execute the cleanup statement. */
    if (cleanupStatement_) {
        context.analyzer().execute(cleanupStatement_.get(), context);
    }

    /* Execute all return value terms. */
    foreach (const auto &pair, returnValues_) {
        auto value = context.analyzer().dataflow().getValue(pair.second.get());
        value->setAbstractValue(dflow::AbstractValue(pair.second->size(), -1, -1));
        value->makeNotStackOffset();

        context.analyzer().execute(pair.second.get(), context);
    }
}

const Term *CallHook::getArgumentTerm(const Term *term) const {
    return nc::find(arguments_, term).get();
}

const Term *CallHook::getReturnValueTerm(const Term *term) const {
    assert(term != NULL);
    return nc::find(returnValues_, term).get();
}

void CallHook::visitChildStatements(Visitor<const Statement> &visitor) const {
    if (cleanupStatement_) {
        visitor(cleanupStatement_.get());
    }
}

void CallHook::visitChildTerms(Visitor<const Term> &visitor) const {
    foreach (const auto &pair, arguments_) {
        visitor(pair.second.get());
    }
    foreach (const auto &pair, returnValues_) {
        visitor(pair.second.get());
    }
    if (stackPointer_) {
        visitor(stackPointer_.get());
    }
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
