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

#include "Dataflow.h"

#include <nc/common/Range.h>
#include <nc/core/ir/Term.h>

#include "Value.h"

namespace nc {
namespace core {
namespace ir {
namespace dflow {

Value *Dataflow::getValue(const Term *term) {
    assert(term != NULL);

    if (term->assignee()) {
        term = term->assignee();
    }

    auto &result = values_[term];
    if (!result) {
        result.reset(new Value());
    }
    return result.get();
}

const Value *Dataflow::getValue(const Term *term) const {
    assert(term != NULL);

    if (term->assignee()) {
        term = term->assignee();
    }

    auto i = values_.find(term);

    if (i != values_.end()) {
        return i->second.get();
    } else {
        static const Value empty;
        return &empty;
    }
}

const ir::MemoryLocation &Dataflow::getMemoryLocation(const Term *term) const {
    assert(term != NULL);

    return nc::find(memoryLocations_, term);
}

void Dataflow::setMemoryLocation(const Term *term, const MemoryLocation &memoryLocation) {
    assert(term != NULL);

    if (!memoryLocation) {
        unsetMemoryLocation(term);
    } else {
        memoryLocations_[term] = memoryLocation;
    }
}

const ReachingDefinitions &Dataflow::getDefinitions(const Term *term) const {
    assert(term != NULL);
    assert(term->isRead());

    return nc::find(definitions_, term);
}

void Dataflow::setDefinitions(const Term *term, const ReachingDefinitions &definitions) {
    assert(term != NULL);
    assert(term->isRead());

    if (definitions.empty()) {
        clearDefinitions(term);
    } else {
        definitions_[term] = definitions;
    }
}

void Dataflow::clearDefinitions(const Term *term) {
    assert(term != NULL);
    assert(term->isRead());

    definitions_.erase(term);
}

const std::vector<const Term *> &Dataflow::getUses(const Term *term) const {
    assert(term != NULL);
    assert(term->isWrite());

    return nc::find(uses_, term);
}

void Dataflow::addUse(const Term *term, const Term *use) {
    assert(term != NULL);
    assert(term->isWrite());
    assert(use != NULL);
    assert(use->isRead());

    uses_[term].push_back(use);
}

void Dataflow::clearUses(const Term *term) {
    assert(term != NULL);
    assert(term->isWrite());

    uses_.erase(term);
}

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
