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

#include "Utils.h"

#include <boost/unordered_set.hpp>

#include <nc/common/Range.h>
#include <nc/common/Unreachable.h>
#include <nc/core/ir/Terms.h>

#include "Dataflow.h"

namespace nc {
namespace core {
namespace ir {

class Term;

namespace dflow {

const Term *getFirstCopy(const Term *term, const Dataflow &dataflow) {
    assert(term != NULL);

    /* Terms that were already seen. */
    boost::unordered_set<const Term *> visited;

    do {
        visited.insert(term);

        if (term->isWrite()) {
            if (term->assignee()) {
                term = term->assignee();
            } else {
                break;
            }
        } else if (term->isRead()) {
            const std::vector<const Term *> &definitions = dataflow.getDefinitions(term);

            if (definitions.size() == 1) {
                term = definitions.front();
            } else if (const Choice *choice = term->as<Choice>()) {
                if (!dataflow.getDefinitions(choice->preferredTerm()).empty()) {
                    term = choice->preferredTerm();
                } else {
                    term = choice->defaultTerm();
                }
            } else {
                break;
            }
        } else {
            unreachable();
        }
    } while (!nc::contains(visited, term));

    return term;
}

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
