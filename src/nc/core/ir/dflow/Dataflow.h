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

#include <nc/common/Range.h>

#include <nc/core/ir/MemoryLocation.h>
#include <nc/core/ir/Term.h>

#include "ReachingDefinitions.h"

namespace nc {
namespace core {
namespace ir {

class Term;

namespace dflow {

class Value;

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
     * Constructor.
     */
    Dataflow();

    /**
     * Destructor.
     */
    ~Dataflow();

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
    const ir::MemoryLocation &getMemoryLocation(const Term *term) const {
        assert(term != NULL);
        return nc::find(memoryLocations_, term);
    }

    /**
     * Associates a memory location with given term.
     *
     * \param[in] term Valid pointer to a term.
     * \param[in] memoryLocation Memory location.
     */
    void setMemoryLocation(const Term *term, const MemoryLocation &memoryLocation) {
        assert(term != NULL);
        memoryLocations_[term] = memoryLocation;
    }

    /**
     * \param[in] term Valid pointer to a read term.
     *
     * \return List of term's definitions. If not set before, it is empty.
     */
    ReachingDefinitions &getDefinitions(const Term *term) {
        assert(term != NULL);
        assert(term->isRead());
        return definitions_[term];
    }

    /**
     * \param[in] term Valid pointer to a read term.
     *
     * \return List of term's definitions. If not set before, it is empty.
     */
    const ReachingDefinitions &getDefinitions(const Term *term) const {
        assert(term != NULL);
        assert(term->isRead());
        return nc::find(definitions_, term);
    }

    /**
     * \param[in] term Valid pointer to a read term.
     *
     * \return List of term's uses. If it has not been set before,
     *         an empty vector is returned.
     */
    std::vector<const Term *> &getUses(const Term *term) {
        assert(term != NULL);
        assert(term->isWrite());
        return uses_[term];
    }

    /**
     * \param[in] term Valid pointer to a read term.
     *
     * \return List of term's uses. If it has not been set before,
     *         an empty vector is returned.
     */
    const std::vector<const Term *> &getUses(const Term *term) const {
        assert(term != NULL);
        assert(term->isWrite());
        return nc::find(uses_, term);
    }
};

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
