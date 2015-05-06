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

#include "GenericReturnAnalyzer.h"

#include <algorithm> /* std::transform() */

#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/DataflowAnalyzer.h>
#include <nc/core/ir/dflow/SimulationContext.h>

#include <nc/common/Foreach.h>

#include "GenericCallingConvention.h"
#include "GenericDescriptorAnalyzer.h"

namespace nc {
namespace core {
namespace ir {
namespace calls {

GenericReturnAnalyzer::GenericReturnAnalyzer(const Return *ret, const GenericDescriptorAnalyzer *addressAnalyzer):
	ReturnAnalyzer(ret), addressAnalyzer_(addressAnalyzer)
{
    foreach (const Term *sample, convention()->returnValues()) {
        getReturnValueTerm(sample);
    }
}

GenericReturnAnalyzer::~GenericReturnAnalyzer() {}

inline const GenericCallingConvention *GenericReturnAnalyzer::convention() const {
    return addressAnalyzer()->convention();
}

void GenericReturnAnalyzer::simulateReturn(dflow::SimulationContext &context) {
    foreach (const auto &pair, returnValues_) {
        context.analyzer().simulate(pair.second.get(), context);
    }
    
    /* 
     * Compute the candidate terms containing return value.
     *
     * If a reaching definition comes, and it comes not from a call (which
     * isn't sure himself, how he returns values), it is the candidate.
     */
    returnValueLocations_.clear();
    foreach (const auto &pair, returnValues_) {
        foreach (const Term *definition, context.analyzer().dataflow().getDefinitions(pair.second.get())) {
            if (definition->statement() && !definition->statement()->isCall()) {
                returnValueLocations_.push_back(pair.first);
                break;
            }
        }
    }
}

const Term *GenericReturnAnalyzer::getReturnValueTerm(const Term *term) {
    auto &result = returnValues_[term];
    if (!result) {
        result = term->clone();
        result->initFlags(Term::READ);
        result->setStatement(ret());
    }
    return result.get();
}

void GenericReturnAnalyzer::visitChildStatements(Visitor<const Statement> & /*visitor*/) const {
    /* Nothing to do. */
}

void GenericReturnAnalyzer::visitChildTerms(Visitor<const Term> &visitor) const {
    foreach (const auto &returnValue, returnValues_) {
        visitor(returnValue.second.get());
    }
}

} // namespace calls
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
