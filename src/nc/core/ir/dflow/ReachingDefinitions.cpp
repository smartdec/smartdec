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
#include <iterator>

#include <QTextStream>

#include <nc/common/Foreach.h>

#include <nc/core/ir/Term.h>

namespace nc {
namespace core {
namespace ir {
namespace dflow {

void ReachingDefinitions::addDefinition(const MemoryLocation &mloc, const Term *term) {
    assert(mloc);

    killDefinitions(mloc);
    
    auto i = std::lower_bound(pairs_.begin(), pairs_.end(), mloc,
        [](decltype(pairs_)::value_type &a, const MemoryLocation &b) -> bool {
            return a.first < b;
        });

    pairs_.insert(i, std::make_pair(mloc, std::vector<const Term *>(1, term)));
}

void ReachingDefinitions::killDefinitions(const MemoryLocation &mloc) {
    assert(mloc);

    if (pairs_.empty()) {
        return;
    }

    std::vector<std::pair<MemoryLocation, std::vector<const Term *>>> result;
    result.reserve(pairs_.size() + 1);

    foreach (auto &pair, pairs_) {
        if (!mloc.overlaps(pair.first)) {
            result.push_back(std::move(pair));
        } else {
            if (pair.first.addr() < mloc.addr()) {
                if (mloc.endAddr() < pair.first.endAddr()) {
                    result.push_back(std::make_pair(
                        MemoryLocation(mloc.domain(), pair.first.addr(), mloc.addr() - pair.first.addr()),
                        pair.second));
                } else {
                    result.push_back(std::make_pair(
                        MemoryLocation(mloc.domain(), pair.first.addr(), mloc.addr() - pair.first.addr()),
                        std::move(pair.second)));
                }
            }
            if (mloc.endAddr() < pair.first.endAddr()) {
                result.push_back(std::make_pair(
                    MemoryLocation(mloc.domain(), mloc.endAddr(), pair.first.endAddr() - mloc.endAddr()),
                    std::move(pair.second)));
            }
        }
    }

    pairs_ = std::move(result);
}

void ReachingDefinitions::project(const MemoryLocation &mloc, ReachingDefinitions &result) const {
    assert(mloc);

    result.clear();

    foreach (const auto &pair, pairs_) {
        if (pair.first.domain() == mloc.domain()) {
            auto addr = std::max(pair.first.addr(), mloc.addr());
            auto endAddr = std::min(pair.first.endAddr(), mloc.endAddr());

            if (addr < endAddr) {
                result.pairs_.push_back(
                    std::make_pair(MemoryLocation(mloc.domain(), addr, endAddr - addr),
                    pair.second));
            }
        }
    }
}

std::vector<MemoryLocation> ReachingDefinitions::getDefinedMemoryLocationsWithin(Domain domain) const {
    std::vector<MemoryLocation> result;
    result.reserve(pairs_.size());

    foreach (const auto &pair, pairs_) {
        if (pair.first.domain() == domain) {
            result.push_back(pair.first);
        }
    }

    return result;
}

void ReachingDefinitions::merge(const ReachingDefinitions &those) {
    std::vector<std::pair<MemoryLocation, std::vector<const Term *>>> result;
    result.reserve(pairs_.size() + those.pairs_.size());

    auto i = pairs_.begin();
    auto iend = pairs_.end();

    auto j = those.pairs_.begin();
    auto jend = those.pairs_.end();

    while (i != iend || j != jend) {
        auto a = i != iend ? i->first : MemoryLocation();
        auto b = j != jend ? j->first : MemoryLocation();

        if (!result.empty()) {
            const auto &c = result.back().first;
            if (c.domain() == a.domain() && c.endAddr() > a.addr()) {
                a = MemoryLocation(a.domain(), c.endAddr(), a.endAddr() - c.endAddr());
            }
            if (c.domain() == b.domain() && c.endAddr() > b.addr()) {
                b = MemoryLocation(b.domain(), c.endAddr(), b.endAddr() - c.endAddr());
            }
        }

        if (!b) {
            result.push_back(std::make_pair(a, i->second));
            ++i;
        } else if (!a) {
            result.push_back(std::make_pair(b, j->second));
            ++j;
        } else if (a.domain() < b.domain()) {
            result.push_back(std::make_pair(a, i->second));
            ++i;
        } else if (b.domain() < a.domain()) {
            result.push_back(std::make_pair(b, j->second));
            ++j;
        } else if (a.endAddr() <= b.addr()) {
            result.push_back(std::make_pair(a, i->second));
            ++i;
        } else if (b.endAddr() <= a.addr()) {
            result.push_back(std::make_pair(b, j->second));
            ++j;
        } else if (a.addr() < b.addr()) {
            result.push_back(std::make_pair(MemoryLocation(a.domain(), a.addr(), b.addr() - a.addr()), i->second));
        } else if (b.addr() < a.addr()) {
            result.push_back(std::make_pair(MemoryLocation(b.domain(), b.addr(), a.addr() - b.addr()), j->second));
        } else {
            std::vector<const Term *> merged;
            merged.reserve(i->second.size() + j->second.size());
            std::set_union(i->second.begin(), i->second.end(), j->second.begin(), j->second.end(), std::back_inserter(merged));

            if (a.size() < b.size()) {
                result.push_back(std::make_pair(a, std::move(merged)));
                ++i;
            } else if (b.size() < a.size()) {
                result.push_back(std::make_pair(b, std::move(merged)));
                ++j;
            } else {
                result.push_back(std::make_pair(a, std::move(merged)));
                ++i;
                ++j;
            }
        }
    }

    pairs_ = std::move(result);
}

void ReachingDefinitions::print(QTextStream &out) const {
    out << '{';
    foreach (const auto &pair, pairs_) {
        out << pair.first << ':';
        foreach (const Term *term, pair.second) {
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
