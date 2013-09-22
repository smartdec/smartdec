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

#include "VariableAnalyzer.h"

#include <nc/common/DisjointSet.h>
#include <nc/common/Foreach.h>

#include <nc/core/ir/Term.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/misc/CensusVisitor.h>

#include "Variables.h"

namespace nc {
namespace core {
namespace ir {
namespace vars {

namespace {

class TermSet;
class TermSet: public DisjointSet<TermSet> {};

} // anonymous namespace

void VariableAnalyzer::analyze(const Function *function) {
    ir::misc::CensusVisitor census(callsData());
    census(function);

    boost::unordered_map<const Term *, std::unique_ptr<TermSet>> term2set;

    /*
     * Make a set for each read or write term.
     */
    foreach (const Term *term, census.terms()) {
        if (term->isRead() || term->isWrite()) {
            term2set[term] = std::make_unique<TermSet>();
        }
    }

    /*
     * Join sets of definitions and uses.
     */
    foreach (const Term *term, census.terms()) {
        if (term->isWrite()) {
            auto termSet = term2set[term].get();

            foreach (const Term *use, dataflow().getUses(term)) {
                assert(dataflow().getMemoryLocation(term).overlaps(dataflow().getMemoryLocation(use)));

                termSet->unionSet(term2set[use].get());
            }
        }
    }

    boost::unordered_map<TermSet *, Variable *> set2variable;

    /*
     * Make a variable for each set.
     */
    foreach (auto &pair, term2set) {
        auto set = pair.second->findSet();
        auto &variable = set2variable[set];
        if (!variable) {
            variable = variables().makeVariable();
        }
    }

    /*
     * Compute a memory location for each variable.
     */
    foreach (auto &pair, term2set) {
        auto set = pair.second->findSet();
        auto variable = set2variable[set];
        variables().setVariable(pair.first, variable);

        auto &termLocation = dataflow().getMemoryLocation(pair.first);
        assert(termLocation);

        auto &variableLocation = variable->memoryLocation();
        if (!variableLocation) {
            variable->setMemoryLocation(termLocation);
        } else {
            assert(termLocation.domain() == variableLocation.domain());
            auto addr = std::min(termLocation.addr(), variableLocation.addr());
            auto endAddr = std::max(termLocation.endAddr(), variableLocation.endAddr());
            variable->setMemoryLocation(MemoryLocation(termLocation.domain(), addr, endAddr - addr));
        }
    }
}

} // namespace vars
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
