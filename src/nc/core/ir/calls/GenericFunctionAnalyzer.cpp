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

#include "GenericFunctionAnalyzer.h"

#include <algorithm> /* std::transform() */

#include <nc/common/select1st.h>

#include <nc/core/ir/Function.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/DataflowAnalyzer.h>
#include <nc/core/ir/dflow/SimulationContext.h>
#include <nc/core/ir/misc/CensusVisitor.h>

#include <nc/common/Foreach.h>
#include <nc/common/Range.h> /* nc::contains() */

#include "GenericCallingConvention.h"
#include "GenericDescriptorAnalyzer.h"

namespace nc {
namespace core {
namespace ir {
namespace calls {

GenericFunctionAnalyzer::GenericFunctionAnalyzer(const Function *function, const GenericDescriptorAnalyzer *addressAnalyzer):
    FunctionAnalyzer(function), addressAnalyzer_(addressAnalyzer)
{
    stackPointer_.reset(new MemoryLocationAccess(convention()->stackPointer()));
    stackPointer_->initFlags(Term::WRITE);

    entryStatements_.reserve(convention()->entryStatements().size());
    foreach (const Statement *statement, convention()->entryStatements()) {
        entryStatements_.push_back(statement->clone().release());
    }

    /* Precompute the set of memory locations used for passing arguments. */
    foreach (const ArgumentGroup &group, convention()->argumentGroups()) {
        foreach (const Argument &argument, group.arguments()) {
            possibleArgumentLocations_.insert(argument.locations().begin(), argument.locations().end());
        }
    }
}

GenericFunctionAnalyzer::~GenericFunctionAnalyzer() {
    foreach (const Statement *statement, entryStatements_) {
        delete statement;
    }
}

inline const GenericCallingConvention *GenericFunctionAnalyzer::convention() const {
    return addressAnalyzer()->convention();
}

void GenericFunctionAnalyzer::simulateEnter(dflow::SimulationContext &context) {
    /*
     * Detect all stack arguments used.
     */
    if (context.fixpointReached()) {
        misc::CensusVisitor census(NULL);
        census(context.function());

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
            select1st<boost::unordered_map<MemoryLocation, std::unique_ptr<Term>>::value_type>());
    }

    /**
     * Set stack pointer offset to zero and simulate him.
     */
    context.analyzer().dataflow().getValue(stackPointer_.get())->makeStackOffset(0);
    context.analyzer().simulate(stackPointer_.get(), context);

    /*
     * Run simulation for "enter statements".
     */
    foreach (const auto &statement, entryStatements_) {
        context.analyzer().simulate(statement, context);
    }

    /*
     * Run simulation for all argument terms.
     */
    foreach (const auto &argument, arguments_) {
        context.analyzer().simulate(argument.second.get(), context);
    }
}

const Term *GenericFunctionAnalyzer::getArgumentTerm(const MemoryLocation &memoryLocation) {
    auto &result = arguments_[memoryLocation];
    if (!result) {
        result.reset(new MemoryLocationAccess(memoryLocation));
        result->initFlags(Term::WRITE);
    }
    return result.get();
}

void GenericFunctionAnalyzer::visitChildStatements(Visitor<const Statement> &visitor) const {
    foreach (const auto &statement, entryStatements_) {
        visitor(statement);
    }
}

void GenericFunctionAnalyzer::visitChildTerms(Visitor<const Term> &visitor) const {
    visitor(stackPointer_.get());

    foreach (const auto &argument, arguments_) {
        visitor(argument.second.get());
    }
}

} // namespace calls
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
