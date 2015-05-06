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

#include "GenericCallAnalyzer.h"

#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/DataflowAnalyzer.h>
#include <nc/core/ir/dflow/SimulationContext.h>
#include <nc/core/ir/MemoryDomain.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>

#include "GenericCallingConvention.h"
#include "GenericDescriptorAnalyzer.h"

namespace nc {
namespace core {
namespace ir {
namespace calls {

namespace {

class CompareAddress {
    public:

    bool operator()(const MemoryLocation &a, const MemoryLocation &b) {
        return a.addr() < b.addr();
    }
};

} // anonymous namespace

GenericCallAnalyzer::GenericCallAnalyzer(const Call *call, const GenericDescriptorAnalyzer *addressAnalyzer):
    CallAnalyzer(call), addressAnalyzer_(addressAnalyzer), stackTop_(0)
{
    auto stackAmendmentConstant = std::make_unique<Constant>(SizedValue(0, convention()->stackPointer().size()));
    stackAmendmentConstant_ = stackAmendmentConstant.get();

    stackAmendmentStatement_.reset(
        new Assignment(
            std::make_unique<MemoryLocationAccess>(convention()->stackPointer()),
            std::make_unique<BinaryOperator>(
                BinaryOperator::ADD, 
                std::make_unique<MemoryLocationAccess>(convention()->stackPointer()), 
                std::move(stackAmendmentConstant))));

    foreach (const Term *returnValue, convention()->returnValues()) {
        getReturnValueTerm(returnValue);
    }
}

GenericCallAnalyzer::~GenericCallAnalyzer() {}

inline const GenericCallingConvention *GenericCallAnalyzer::convention() const {
    return addressAnalyzer()->convention();
}

void GenericCallAnalyzer::simulateCall(dflow::SimulationContext &context) {
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
            stackPointer_->initFlags(Term::READ);
            stackPointer_->setStatement(call());
        }
        context.analyzer().simulate(stackPointer_.get(), context);

        const dflow::Value *stackPointerValue = context.analyzer().dataflow().getValue(stackPointer_.get());

        /*
         * If the stack pointer is valid, guess the arguments passed on the stack.
         */
        if (stackPointerValue->isStackOffset()) {
            stackTop_ = stackPointerValue->stackOffset().signedValue() * CHAR_BIT;

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
                 * matching locations when found by CallAnalyzer and by FunctionAnalyzer.
                 */
                MemoryLocation argumentLocation = i->shifted(-stackTop_);
                getArgumentTerm(argumentLocation);
                argumentLocations_.push_back(argumentLocation);
                nextArgumentOffset = i->addr() + i->size() + convention()->argumentAlignment() - 1;
            }
        }
    }

    /* Run simulation for all argument terms. */
    foreach (const auto &argument, arguments_) {
        context.analyzer().simulate(argument.second.get(), context);
    }

#if 0
    /* Kill all estimated arguments. */
    foreach (const MemoryLocation &memoryLocation, argumentLocations_) {
        context.definitions().killDefinitions(
            memoryLocation.domain() == MemoryDomain::STACK ?
                memoryLocation.shifted(stackTop_) :
                memoryLocation);
    }
#endif

    /* Run simulation for all return value terms. */
    foreach (const auto &pair, returnValues_) {
        dflow::Value *value = context.analyzer().dataflow().getValue(pair.second.get());
        value->makeNonconstant();
        value->makeNotStackOffset();

        context.analyzer().simulate(pair.second.get(), context);
    }

    /* If return values can overlap, they kill each other and the following hack is necessary. */
    foreach (const auto &pair, returnValues_) {
        if (const MemoryLocation &memoryLocation = context.analyzer().dataflow().getMemoryLocation(pair.second.get())) {
            dflow::ReachingDefinitions definitions;
            definitions.addDefinition(memoryLocation, pair.second.get());
            context.definitions().join(definitions);
        }
    }

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

    /* Compute the stack pointer amendment. */
    ByteSize amendment = convention()->firstArgumentOffset() / 8;
    if (convention()->calleeCleanup() && addressAnalyzer()->argumentsSize()) {
        amendment += *addressAnalyzer()->argumentsSize();
    }
    stackAmendmentConstant_->setValue(amendment);
    
    /* Simulate the amendment statement. */
    context.analyzer().simulate(stackAmendmentStatement_.get(), context);
}

const Term *GenericCallAnalyzer::getArgumentTerm(const MemoryLocation &memoryLocation) {
    auto &result = arguments_[memoryLocation];
    if (!result) {
        result.reset(new MemoryLocationAccess(
            memoryLocation.domain() == MemoryDomain::STACK ?
                memoryLocation.shifted(stackTop_) :
                memoryLocation));
        result->initFlags(Term::READ);
        result->setStatement(call());
    }
    return result.get();
}

const Term *GenericCallAnalyzer::getReturnValueTerm(const Term *term) {
    assert(term != NULL);

    auto &result = returnValues_[term];
    if (!result) {
        result = term->clone();
        result->initFlags(Term::WRITE);
        result->setStatement(call());
    }
    return result.get();
}

void GenericCallAnalyzer::visitChildStatements(Visitor<const Statement> &visitor) const {
    visitor(stackAmendmentStatement_.get());
}

void GenericCallAnalyzer::visitChildTerms(Visitor<const Term> &visitor) const {
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

} // namespace calls
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
