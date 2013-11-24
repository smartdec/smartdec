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

// TODO: remove
#if 0
namespace {

class CompareAddress {
    public:

    bool operator()(const MemoryLocation &a, const MemoryLocation &b) {
        return a.addr() < b.addr();
    }
};

} // anonymous namespace
#endif

CallHook::CallHook(const Call *call, const Convention *convention, const Signature *signature,
    boost::optional<ByteSize> stackArgumentsSize)
{
    assert(call != NULL);
    assert(convention != NULL);

    if (signature) {
        foreach (const auto &location, signature->arguments()) {
            if (location.domain() == MemoryDomain::STACK) {
                assert(location.addr() % CHAR_BIT == 0);
                arguments_[location] = std::make_unique<Dereference>(
                    std::make_unique<BinaryOperator>(BinaryOperator::ADD,
                        std::make_unique<MemoryLocationAccess>(convention->stackPointer()),
                        std::make_unique<Constant>(SizedValue(convention->stackPointer().size(), location.addr() / CHAR_BIT)),
                        convention->stackPointer().size<SmallBitSize>()),
                    MemoryDomain::MEMORY,
                    location.size());
            } else {
                arguments_[location] = std::make_unique<MemoryLocationAccess>(location);
            }
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
}

CallHook::~CallHook() {}

void CallHook::execute(dflow::ExecutionContext &context) {
    /* Execute all argument terms. */
    foreach (const auto &pair, arguments_) {
        context.analyzer().execute(pair.second.get(), context);
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

    // FIXME
#if 0
    argumentLocations_.clear();

    /*
     * Estimate which registers are used for passing arguments.
     */
    bool someGroupIsFull = convention()->argumentGroups().empty();

    foreach (const ArgumentGroup &group, convention()->argumentGroups()) {
        bool groupIsFull = true;

        foreach (const Argument &argument, group.arguments()) {
            bool argumentFound = false;

            foreach (const MemoryLocation &location, argument.locations()) {
                foreach (const Term *definition, context.definitions().getDefinitions(location)) {
                    if (definition->statement() && !definition->statement()->isCall()) {
                        getArgumentTerm(location);
                        argumentLocations_.push_back(location);
                        argumentFound = true;
                        break;
                    }
                }
                if (argumentFound) {
                    break;
                }
            }
            if (!argumentFound) {
                groupIsFull = false;
                break;
            }
        }
        someGroupIsFull = someGroupIsFull || groupIsFull;
    }

    /*
     * Estimate which stack memory locations are used for passing arguments.
     */
    if (someGroupIsFull && convention()->stackPointer()) {
        /*
         * Detect the value of the stack pointer.
         */
        if (!stackPointer_) {
            stackPointer_.reset(new MemoryLocationAccess(convention()->stackPointer()));
            stackPointer_->setAccessType(Term::READ);
            stackPointer_->setStatementRecursively(call());
        }
        context.analyzer().execute(stackPointer_.get(), context);

        const dflow::Value *stackPointerValue = context.analyzer().dataflow().getValue(stackPointer_.get());

        /*
         * If the stack pointer is valid, guess the arguments passed on the stack.
         */
        if (stackPointerValue->isStackOffset()) {
            stackTop_ = stackPointerValue->stackOffset() * CHAR_BIT;

            /* Let's examine reaching definition of stack memory locations... */
            std::vector<MemoryLocation> stackLocations = context.definitions().getDefinedMemoryLocationsWithin(MemoryDomain::STACK);
            std::sort(stackLocations.begin(), stackLocations.end(), CompareAddress());

            auto i = stackLocations.begin();
            auto iend = stackLocations.end();

            /* Position of the first stack argument. */
            BitAddr nextArgumentOffset = stackTop_ + convention()->firstArgumentOffset();

            /* First argument must be located exactly at the computed position. */
            for (; i != iend && i->addr() != nextArgumentOffset; ++i);

            /* Each next argument must go not too far from the previous. */
            for (; i != iend && i->addr() <= nextArgumentOffset; ++i) {
                /*
                 * We use shifted() in order to make so that the same arguments have
                 * matching locations when found by CallHook and by EntryHook.
                 */
                MemoryLocation argumentLocation = i->shifted(-stackTop_);
                getArgumentTerm(argumentLocation);
                argumentLocations_.push_back(argumentLocation);
                nextArgumentOffset = i->addr() + i->size() + convention()->argumentAlignment() - 1;
            }
        }
    }
#endif

    // TODO: remove
#if 0
    /* If return values can overlap, they kill each other and the following hack is necessary. */
    foreach (const auto &pair, returnValues_) {
        if (const MemoryLocation &memoryLocation = context.analyzer().dataflow().getMemoryLocation(pair.second.get())) {
            dflow::ReachingDefinitions definitions;
            definitions.addDefinition(memoryLocation, pair.second.get());
            context.definitions().join(definitions);
        }
    }
#endif

    // FIXME
#if 0
    /* Remember possible return values being used. */
    returnValueLocations_.clear();
    foreach (const auto &pair, returnValues_) {
        foreach (const Term *use, context.analyzer().dataflow().getUses(pair.second.get())) {
            if (use->statement() && !use->statement()->isCall() && !use->statement()->isReturn()) {
                returnValueLocations_.push_back(pair.first);
                break;
            }
        }
    }
#endif
}

const Term *CallHook::getArgumentTerm(const MemoryLocation &memoryLocation) const {
    return nc::find(arguments_, memoryLocation).get();

// TODO: remove
#if 0
    auto &result = arguments_[memoryLocation];
    if (!result) {
        result.reset(new MemoryLocationAccess(
            memoryLocation.domain() == MemoryDomain::STACK ?
                memoryLocation.shifted(stackTop_) :
                memoryLocation));
        result->setAccessType(Term::READ);
        result->setStatementRecursively(call());
    }
    return result.get();
#endif
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
