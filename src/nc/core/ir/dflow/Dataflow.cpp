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

#include <nc/core/ir/Term.h>

namespace nc {
namespace core {
namespace ir {
namespace dflow {

Value *Dataflow::getValue(const Term *term) {
    auto &result = values_[term];
    if (!result) {
        result.reset(new Value(term->size()));
    }
    return result.get();
}

const Value *Dataflow::getValue(const Term *term) const {
    return const_cast<Dataflow *>(this)->getValue(term);
}

const ir::MemoryLocation &Dataflow::getMemoryLocation(const Term *term) const {
    auto i = memoryLocations_.find(term);
    if (i != memoryLocations_.end()) {
        return i->second;
    } else {
        static const MemoryLocation empty;
        return empty;
    }
}

void Dataflow::setMemoryLocation(const Term *term, const MemoryLocation &memoryLocation) {
    if (!memoryLocation) {
        unsetMemoryLocation(term);
    } else {
        memoryLocations_[term] = memoryLocation;
    }
}

const std::vector<const Term *> &Dataflow::getDefinitions(const Term *term) const {
    assert(term->isRead());

    auto i = definitions_.find(term);
    if (i != definitions_.end()) {
        return *i->second;
    } else {
        static const std::vector<const Term *> empty;
        return empty;
    }
}

void Dataflow::setDefinitions(const Term *term, const std::vector<const Term *> &definitions) {
    assert(term->isRead());

    if (definitions.empty()) {
        clearDefinitions(term);
    } else {
        auto &pointer = definitions_[term];
        if (pointer) {
            *pointer = definitions;
        } else {
            pointer.reset(new std::vector<const Term *>(definitions));
        }
    }
}

void Dataflow::clearDefinitions(const Term *term) {
    assert(term->isRead());

    definitions_.erase(term);
}

const std::vector<const Term *> &Dataflow::getUses(const Term *term) const {
    auto i = uses_.find(term);
    if (i != uses_.end()) {
        return *i->second;
    } else {
        static const std::vector<const Term *> empty;
        return empty;
    }
}

void Dataflow::addUse(const Term *term, const Term *use) {
    auto &pointer = uses_[term];
    if (!pointer) {
        pointer.reset(new std::vector<const Term *>(1, use));
    } else {
        pointer->push_back(use);
    }
}

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
