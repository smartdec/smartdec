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

#include <nc/core/arch/Architecture.h>
#include <nc/core/ir/Term.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/Dataflows.h>

#include "Variables.h"

namespace nc {
namespace core {
namespace ir {
namespace vars {

namespace {

class TermSet;
class TermSet: public DisjointSet<TermSet> {};

} // anonymous namespace

void VariableAnalyzer::analyze() {
    std::vector<std::pair<MemoryLocation, const Term *>> globalMemoryAccesses;

    /*
     * Reconstruct local variables.
     */
    foreach (const auto &functionAndDataflow, dataflows_) {
        const auto &dataflow = *functionAndDataflow.second;

        boost::unordered_map<const Term *, std::unique_ptr<TermSet>> term2set;

        /*
         * Make a set for each read or write term which has a memory location.
         */
        foreach (const auto &termAndLocation, dataflow.term2location()) {
            const auto &term = termAndLocation.first;
            const auto &location = termAndLocation.second;

            if ((term->isRead() || term->isWrite()) && location) {
                if (architecture_->isGlobalMemory(location)) {
                    globalMemoryAccesses.emplace_back(location, term);
                } else {
                    term2set[term] = std::make_unique<TermSet>();
                }
            }
        }

        /*
         * Join sets of definitions and uses.
         */
        foreach (auto &pair, term2set) {
            auto term = pair.first;

            if (term->isRead()) {
                auto termSet = pair.second.get();

                foreach (const auto &chunk, dataflow.getDefinitions(term).chunks()) {
                    foreach (const Term *def, chunk.definitions()) {
                        assert(dataflow.getMemoryLocation(term).overlaps(dataflow.getMemoryLocation(def)));
                        termSet->unionSet(term2set[def].get());
                    }
                }
            }
        }

        /*
         * Compute the terms belonging to each set.
         */
        boost::unordered_map<TermSet *, std::vector<const Term *>> set2terms;
        foreach (auto &pair, term2set) {
            set2terms[pair.second->findSet()].push_back(pair.first);
        }

        /*
         * Create local variables.
         */
        foreach (auto &pair, set2terms) {
            auto &terms = pair.second;

            auto variableLocation = std::accumulate(terms.begin(), terms.end(), MemoryLocation(),
                [&](const MemoryLocation &a, const Term *term) {
                    return MemoryLocation::merge(a, dataflow.getMemoryLocation(term));
                });

            variables_.addVariable(std::make_unique<Variable>(variableLocation, std::move(terms)));
        }
    }

    /*
     * Reconstruct global variables.
     */
    std::sort(globalMemoryAccesses.begin(), globalMemoryAccesses.end(),
        [](const std::pair<MemoryLocation, const Term *> &a, const std::pair<MemoryLocation, const Term *> &b) {
            return a.first < b.first;
        });

    MemoryLocation variableLocation;
    std::vector<const Term *> terms;

    foreach (const auto &locationAndTerm, globalMemoryAccesses) {
        if (variableLocation && !variableLocation.overlaps(locationAndTerm.first)) {
            variables_.addVariable(std::make_unique<Variable>(variableLocation, terms));
            variableLocation = MemoryLocation();
            terms.clear();
        }

        terms.push_back(locationAndTerm.second);
        variableLocation = MemoryLocation::merge(variableLocation, locationAndTerm.first);
    }

    if (variableLocation) {
        variables_.addVariable(std::make_unique<Variable>(variableLocation, std::move(terms)));
    }
}

} // namespace vars
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
