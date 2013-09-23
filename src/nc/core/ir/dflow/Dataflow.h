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
#include <memory>

#include <boost/unordered_map.hpp>

#include <nc/core/ir/MemoryLocation.h>

#include "ReachingDefinitions.h"
#include "Value.h"

namespace nc {
namespace core {
namespace ir {

class Term;

namespace dflow {

/**
 * This class contains results of dataflow and constant propagation and folding analysis.
 */
class Dataflow {
    /** Mapping from a term to a description of its value. */
    boost::unordered_map<const Term *, std::unique_ptr<Value>> values_;

    /** Mapping from a term to its location in memory. */
    boost::unordered_map<const Term *, MemoryLocation> memoryLocations_;

    /** Mapping from a term to the reaching definitions of the parts of its memory location. */
    boost::unordered_map<const Term *, ReachingDefinitions> definitions_;

    /** Mapping from a term to the list of terms reading its value. */
    boost::unordered_map<const Term *, std::vector<const Term *>> uses_;

    public:

    /**
     * \param[in] term Valid pointer to a term.
     *
     * \return Valid pointer to the value description for this term.
     *         If the term is being assigned to, the value description
     *         for the right hand side of the assignment is returned.
     */
    Value *getValue(const Term *term);

    /**
     * \param[in] term Valid pointer to a term.
     *
     * \return Valid pointer to the value description for this term.
     *         If the term is being assigned to, the value description
     *         for the right hand side of the assignment is returned.
     */
    const Value *getValue(const Term *term) const;

    /**
     * \param[in] term Valid pointer to a term.
     *
     * \return Memory location occupied by the term. If no memory location is associated
     *         with this term, an invalid MemoryLocation object is returned.
     */
    const ir::MemoryLocation &getMemoryLocation(const Term *term) const;

    /**
     * Associates a memory location with given term.
     *
     * \param[in] term Valid pointer to a term.
     * \param[in] memoryLocation Memory location.
     */
    void setMemoryLocation(const Term *term, const MemoryLocation &memoryLocation);

    /**
     * Marks given term as not associated with any memory location.
     *
     * \param[in] term Term.
     */
    void unsetMemoryLocation(const Term *term);

    /**
     * \param[in] term Valid pointer to a read term.
     *
     * \return List of term's definitions. If not set before, it is empty.
     */
    ReachingDefinitions &getDefinitions(const Term *term);

    /**
     * \param[in] term Valid pointer to a read term.
     *
     * \return List of term's definitions. If not set before, it is empty.
     */
    const ReachingDefinitions &getDefinitions(const Term *term) const;

    /**
     * Sets the list of term's definitions.
     *
     * \param[in] term Valid pointer to a read term.
     * \param[in] definitions Reaching definitions of this term.
     */
    void setDefinitions(const Term *term, ReachingDefinitions &&definitions);

    /**
     * Clears the set of term's definitions.
     *
     * \param[in] term Valid pointer to a read term.
     */
    void clearDefinitions(const Term *term);

    /**
     * \param[in] term Valid pointer to a read term.
     *
     * \return List of term's uses. If it has not been set before,
     *         an empty vector is returned.
     */
    const std::vector<const Term *> &getUses(const Term *term) const;

    /**
     * Adds a use of a term.
     *
     * \param[in] term Valid pointer to a write term.
     * \param[in] use  Valid pointer to a read term reading given write term.
     */
    void addUse(const Term *term, const Term *use);

    /**
     * Clears the set of term's definitions.
     *
     * \param[in] term Valid pointer to a write term.
     */
    void clearUses(const Term *term);
};

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
