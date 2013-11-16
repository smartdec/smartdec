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

#include "GenericReturnHook.h"

#include <algorithm> /* std::transform() */

#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/DataflowAnalyzer.h>
#include <nc/core/ir/dflow/ExecutionContext.h>

#include <nc/common/Foreach.h>

#include "GenericCallingConvention.h"
#include "GenericDescriptorAnalyzer.h"

namespace nc {
namespace core {
namespace ir {
namespace cconv {

GenericReturnHook::GenericReturnHook(const Return *ret, const GenericDescriptorAnalyzer *addressAnalyzer):
	ReturnHook(ret), addressAnalyzer_(addressAnalyzer)
{
    foreach (const Term *sample, convention()->returnValues()) {
        getReturnValueTerm(sample);
    }
}

GenericReturnHook::~GenericReturnHook() {}

inline const GenericCallingConvention *GenericReturnHook::convention() const {
    return addressAnalyzer()->convention();
}

void GenericReturnHook::executeReturn(dflow::ExecutionContext &context) {
    foreach (const auto &pair, returnValues_) {
        context.analyzer().execute(pair.second.get(), context);
    }
    
    /* 
     * Compute the candidate terms containing return value.
     *
     * If a reaching definition comes, and it comes not from a call (which
     * isn't sure himself, how he returns values), it is the candidate.
     */
    returnValueLocations_.clear();

    // FIXME: adapt to new dflow
#if 0
    foreach (const auto &pair, returnValues_) {
        foreach (const Term *definition, context.analyzer().dataflow().getDefinitions(pair.second.get())) {
            if (definition->statement() && !definition->statement()->isCall()) {
                returnValueLocations_.push_back(pair.first);
                break;
            }
        }
    }
#endif
}

const Term *GenericReturnHook::getReturnValueTerm(const Term *term) {
    auto &result = returnValues_[term];
    if (!result) {
        result = term->clone();
        result->setAccessType(Term::READ);
        result->setStatementRecursively(ret());
    }
    return result.get();
}

void GenericReturnHook::visitChildStatements(Visitor<const Statement> & /*visitor*/) const {
    /* Nothing to do. */
}

void GenericReturnHook::visitChildTerms(Visitor<const Term> &visitor) const {
    foreach (const auto &returnValue, returnValues_) {
        visitor(returnValue.second.get());
    }
}

} // namespace cconv
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
