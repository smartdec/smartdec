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

#include "ReachingDefinitions.h"

#include <algorithm>
#include <functional>

#include <QTextStream>

#include <nc/common/Foreach.h>

#include <nc/core/ir/Term.h>

namespace nc {
namespace core {
namespace ir {
namespace dflow {

namespace {

/**
 * Predicate functor returning true iff the memory location of a reaching
 * definition overlaps the memory location passed to the constructor.
 */
class Overlap: public std::unary_function<const ReachingDefinition &, bool> {
    MemoryLocation sample_;

    public:

    Overlap(const MemoryLocation &sample): sample_(sample) {}

    bool operator()(const MemoryLocation &mloc) const {
        return sample_.domain() == mloc.domain() &&
               mloc.addr() < sample_.addr() + sample_.size() &&
               sample_.addr() < mloc.addr() + mloc.size();
    }

    bool operator()(const ReachingDefinition &def) const {
        return operator()(def.first);
    }
};

/**
 * Predicate returning true iff the memory location of a definition
 * is within the domain passed to the constructor.
 */
class WithinDomain: public std::unary_function<const ReachingDefinition &, bool> {
    Domain domain_;

    public:

    WithinDomain(Domain domain): domain_(domain) {}

    bool operator()(const ReachingDefinition &def) const {
        return def.first.domain() == domain_;
    }
};

/**
 * Comparator establishing a total order on ReachingDefinition objects.
 */
class Before: public std::binary_function<const ReachingDefinition &, const ReachingDefinition &, bool> {
    public:

    bool operator()(const ReachingDefinition &a, const ReachingDefinition &b) const {
        return a.first < b.first;
    }
};

} // anonymous namespace

void ReachingDefinitions::addDefinition(const MemoryLocation &memoryLocation, const Term *term) {
    assert(memoryLocation.domain() != MemoryDomain::UNKNOWN);

    killDefinitions(memoryLocation);
    
    definitions_.push_back(ReachingDefinition(memoryLocation, std::vector<const Term *>(1, term)));
}

void ReachingDefinitions::killDefinitions(const MemoryLocation &memoryLocation) {
    assert(memoryLocation.domain() != MemoryDomain::UNKNOWN);

    definitions_.erase(std::remove_if(definitions_.begin(), definitions_.end(), Overlap(memoryLocation)), definitions_.end());
}

const std::vector<const Term *> &ReachingDefinitions::getDefinitions(const MemoryLocation &memoryLocation) const {
    foreach (const ReachingDefinition &definition, definitions_) {
        if (definition.first == memoryLocation) {
            return definition.second;
        }
    }

    static const std::vector<const Term *> empty;
    return empty;
}

std::vector<MemoryLocation> ReachingDefinitions::getDefinedMemoryLocationsWithin(Domain domain) const {
    std::vector<MemoryLocation> result;
    result.reserve(definitions_.size());

    foreach (const ReachingDefinition &definition, definitions_) {
        if (definition.first.domain() == domain) {
            result.push_back(definition.first);
        }
    }

    return result;
}

void ReachingDefinitions::join(ReachingDefinitions &those) {
    std::sort(definitions_.begin(), definitions_.end(), Before());
    std::sort(those.definitions_.begin(), those.definitions_.end(), Before());

    std::vector<ReachingDefinition> result;
    result.reserve(definitions_.size() + those.definitions_.size());

    std::vector<ReachingDefinition>::iterator i = definitions_.begin();
    std::vector<ReachingDefinition>::iterator iend = definitions_.end();

    std::vector<ReachingDefinition>::iterator j = those.definitions_.begin();
    std::vector<ReachingDefinition>::iterator jend = those.definitions_.end();

    while (i != iend && j != jend) {
        if (Before()(*i, *j)) {
            result.push_back(*i++);
        } else if (Before()(*j, *i)) {
            result.push_back(*j++);
        } else {
            result.push_back(*i++);
            std::vector<const Term *> &terms(result.back().second);
            terms.insert(terms.end(), j->second.begin(), j->second.end());
            std::sort(terms.begin(), terms.end());
            terms.erase(std::unique(terms.begin(), terms.end()), terms.end());
            ++j;
        }
    }
    while (i != iend) {
        result.push_back(*i++);
    }
    while (j != jend) {
        result.push_back(*j++);
    }

    definitions_.swap(result);
}

void ReachingDefinitions::sort() {
    std::sort(definitions_.begin(), definitions_.end(), Before());
    foreach (ReachingDefinition &definition, definitions_) {
        std::sort(definition.second.begin(), definition.second.end());
    }
}

bool ReachingDefinitions::operator==(ReachingDefinitions &those) {
    sort();
    those.sort();

    return definitions_ == those.definitions_;
}

void ReachingDefinitions::print(QTextStream &out) const {
    out << '{';
    foreach (const ReachingDefinition &definition, definitions_) {
        out << definition.first << ':';
        foreach (const Term *term, definition.second) {
            out << ' ' << *term;
        }
        out << ';';
    }
    out << '}';
}

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
