/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <vector>

#include <nc/common/Printable.h>

#include <nc/core/ir/MemoryLocation.h>

namespace nc {
namespace core {
namespace ir {

class Term;

namespace dflow {

/**
 * Reaching definitions.
 */
class ReachingDefinitions: public PrintableBase<ReachingDefinitions> {
    /**
     * Pairs of memory locations and terms defining them.
     * The pairs are sorted by memory location.
     * Terms are sorted using default comparator.
     */
    std::vector<std::pair<MemoryLocation, std::vector<const Term *>>> definitions_;

    public:

    /**
     * \return Pairs of memory locations and terms defining them.
     *         The pairs are sorted by memory location.
     *         Terms are sorted using default comparator.
     */
    const decltype(definitions_) &definitions() const { return definitions_; }

    /**
     * Clears the reaching definitions.
     */
    void clear() { definitions_.clear(); }

    /**
     * Adds a definition of memory location, removing all previous definitions of overlapping memory locations.
     *
     * \param[in] memoryLocation Memory location.
     * \param[in] term Term which is the definition.
     */
    void addDefinition(const MemoryLocation &memoryLocation, const Term *term);

    /**
     * Kills definitions of given memory location.
     *
     * \param[in] memoryLocation Memory location.
     */
    void killDefinitions(const MemoryLocation &memoryLocation);

    /**
     * \param[in] memoryLocation Memory location.
     *
     * \return Definitions of the memory locations contained in the given one.
     */
    ReachingDefinitions getDefinitions(const MemoryLocation &memoryLocation) const;

    /**
     * \return All defined memory locations in the domain.
     *
     * \param[in] domain Domain.
     */
    std::vector<MemoryLocation> getDefinedMemoryLocationsWithin(Domain domain) const;

    /**
     * Adds given reaching definitions to the list of known reaching definitions.
     *
     * \param[in] those Reaching definitions.
     */
    void merge(const ReachingDefinitions &those);

    /**
     * \return True, if these and given reaching definitions are the same.
     *
     * \param[in] those Reaching definitions.
     */
    bool operator==(const ReachingDefinitions &those) { return definitions_ == those.definitions_; }

    /**
     * \return True, if these and given reaching definitions are different.
     *
     * \param[in] those Reaching definitions.
     */
    bool operator!=(const ReachingDefinitions &those) { return !(*this == those); }

    void print(QTextStream &out) const;
};

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
