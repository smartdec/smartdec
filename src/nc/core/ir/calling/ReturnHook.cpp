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

#include "ReturnHook.h"

#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/DataflowAnalyzer.h>
#include <nc/core/ir/dflow/ExecutionContext.h>

#include <nc/common/Foreach.h>
#include <nc/common/Range.h>

#include "Convention.h"
#include "Signature.h"

namespace nc {
namespace core {
namespace ir {
namespace calling {

ReturnHook::ReturnHook(const Return *ret, const Convention *convention, const Signature *signature) {
    assert(ret != NULL);
    assert(convention != NULL);

    if (signature) {
        if (signature->returnValue()) {
            returnValues_[signature->returnValue()] = signature->returnValue()->clone();
        }
    } else {
        foreach (auto term, convention->returnValues()) {
            returnValues_[signature->returnValue()] = term->clone();
        }
    }

    foreach (const auto &pair, returnValues_) {
        pair.second->setAccessType(Term::READ);
        pair.second->setStatementRecursively(ret);
    }
}

ReturnHook::~ReturnHook() {}

void ReturnHook::execute(dflow::ExecutionContext &context) {
    foreach (const auto &pair, returnValues_) {
        context.analyzer().execute(pair.second.get(), context);
    }
    
    // TODO: move
#if 0
    /* 
     * Compute the candidate terms containing return value.
     *
     * If a reaching definition comes, and it comes not from a call (which
     * isn't sure himself, how he returns values), it is the candidate.
     */
    returnValueLocations_.clear();

    // FIXME: adapt to new dflow
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

const Term *ReturnHook::getReturnValueTerm(const Term *term) {
    return nc::find(returnValues_, term).get();
}

void ReturnHook::visitChildStatements(Visitor<const Statement> & /*visitor*/) const {
    /* Nothing to do. */
}

void ReturnHook::visitChildTerms(Visitor<const Term> &visitor) const {
    foreach (const auto &returnValue, returnValues_) {
        visitor(returnValue.second.get());
    }
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
