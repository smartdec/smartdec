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

#include <cassert>

#include <nc/core/ir/MemoryLocation.h>

namespace nc {
namespace core {
namespace ir {

class Term;

namespace vars {

/**
 * Class containing information about a reconstructed variable.
 */
class Variable {
    /** Memory location of the variable. */
    MemoryLocation memoryLocation_;

    /** Terms belonging to the variable. */
    std::vector<const Term *> terms_;

    public:

    /**
     * Constructor.
     *
     * \param memoryLocation Valid memory location of the variable.
     * \param terms Terms belonging to the variable.
     */
    Variable(const MemoryLocation &memoryLocation, std::vector<const Term *> terms):
        memoryLocation_(memoryLocation), terms_(std::move(terms))
    {
        assert(memoryLocation);
    }

    /**
     * \return Memory location of the variable.
     */
    const MemoryLocation &memoryLocation() const { return memoryLocation_; }

    /**
     * \return Terms belonging to the variable.
     */
    const std::vector<const Term *> &terms() const { return terms_; }
};

} // namespace vars
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
