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

#include "ReachingDefinitions.h"

#include <algorithm>

#include <QTextStream>

#include <nc/common/Foreach.h>

#include <nc/core/ir/Term.h>

namespace nc {
namespace core {
namespace ir {
namespace dflow {

namespace {

/**
 * \param a A memory location.
 * \param b A memory location.
 *
 * \return True if the memory locations overlap, false otherwise.
 */
inline bool overlap(const MemoryLocation &a, const MemoryLocation &b) {
    return a.domain() == b.domain() && a.addr() < b.endAddr() && b.addr() < a.endAddr();
}

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
    assert(memoryLocation);

    killDefinitions(memoryLocation);
    
    definitions_.push_back(ReachingDefinition(memoryLocation, std::vector<const Term *>(1, term)));
}

void ReachingDefinitions::killDefinitions(const MemoryLocation &mloc) {
    assert(mloc);

    if (definitions_.empty()) {
        return;
    }

    std::vector<ReachingDefinition> newDefinitions;
    newDefinitions.reserve(definitions_.size() + 1);

    foreach (auto &def, definitions_) {
        if (!overlap(def.first, mloc)) {
            newDefinitions.push_back(std::move(def));
        } else {
            if (def.first.addr() < mloc.addr()) {
                if (mloc.endAddr() < def.first.endAddr()) {
                    newDefinitions.push_back(std::make_pair(
                        MemoryLocation(mloc.domain(), def.first.addr(), mloc.addr() - def.first.addr()),
                        def.second));
                } else {
                    newDefinitions.push_back(std::make_pair(
                        MemoryLocation(mloc.domain(), def.first.addr(), mloc.addr() - def.first.addr()),
                        std::move(def.second)));
                }
            }
            if (mloc.endAddr() < def.first.endAddr()) {
                newDefinitions.push_back(std::make_pair(
                    MemoryLocation(mloc.domain(), mloc.endAddr(), def.first.endAddr() - mloc.endAddr()),
                    std::move(def.second)));
            }
        }
    }

    definitions_ = std::move(newDefinitions);
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
