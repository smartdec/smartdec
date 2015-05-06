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
 * Pair of memory location and its definitions.
 */
typedef std::pair<MemoryLocation, std::vector<const Term *> > ReachingDefinition;

/**
 * Reaching definitions.
 */
class ReachingDefinitions: public PrintableBase<ReachingDefinitions> {
    std::vector<ReachingDefinition> definitions_; ///< The definitions: pairs of memory locations and sets of terms defining it.

    public:

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
     * \return Definitions of given memory location.
     *
     * \param[in] memoryLocation Memory location.
     */
    const std::vector<const Term *> &getDefinitions(const MemoryLocation &memoryLocation) const;

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
    void join(ReachingDefinitions &those);

    /**
     * Sorts reaching definitions so that two sorted instances of reaching definitions can be compared for equality.
     */
    void sort();

    /**
     * \return True, if these and given reaching definitions are the same.
     *
     * \param[in] those Reaching definitions.
     */
    bool operator==(ReachingDefinitions &those);

    /**
     * \return True, if these and given reaching definitions are different.
     *
     * \param[in] those Reaching definitions.
     */
    bool operator!=(ReachingDefinitions &those) { return !(*this == those); }

    void print(QTextStream &out) const;
};

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
