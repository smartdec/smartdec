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

#include <numeric> /* std::accumulate */

#include <nc/common/DisjointSet.h>
#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

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
     * Make a set for each read or write term which has a memory location.
     */
    foreach (const Term *term, census.terms()) {
        if ((term->isRead() || term->isWrite()) && dataflow().getMemoryLocation(term)) {
            term2set[term] = std::make_unique<TermSet>();
        }
    }

    /*
     * Join sets of definitions and uses.
     */
    foreach (auto &pair, term2set) {
        auto term = pair.first;

        if (term->isWrite()) {
            auto termSet = pair.second.get();

            foreach (const Term *use, dataflow().getUses(term)) {
                assert(dataflow().getMemoryLocation(term).overlaps(dataflow().getMemoryLocation(use)));
                termSet->unionSet(term2set[use].get());
            }
        }
    }

    boost::unordered_map<TermSet *, std::vector<const Term *>> set2terms;

    /*
     * Compute the terms belonging to each set.
     */
    foreach (auto &pair, term2set) {
        set2terms[pair.second->findSet()].push_back(pair.first);
    }

    /*
     * Create variables.
     */
    foreach (auto &pair, set2terms) {
        auto &terms = pair.second;

        auto merge = [](const MemoryLocation &a, const MemoryLocation &b) -> MemoryLocation {
            if (!a) {
                return b;
            } else {
                assert(a.domain() == b.domain());
                auto addr = std::min(a.addr(), b.addr());
                auto endAddr = std::max(a.endAddr(), b.endAddr());
                return MemoryLocation(a.domain(), addr, endAddr - addr);
            }
        };

        auto memoryLocation = std::accumulate(terms.begin(), terms.end(), MemoryLocation(),
            [&](const MemoryLocation &a, const Term *term) { return merge(a, dataflow().getMemoryLocation(term)); });

        variables().addVariable(std::make_unique<Variable>(memoryLocation, std::move(terms)));
    }
}

} // namespace vars
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
