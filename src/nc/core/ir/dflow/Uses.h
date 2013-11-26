/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <vector>

#include <boost/unordered_map.hpp>

#include <nc/common/Range.h>

#include <nc/core/ir/Term.h>

namespace nc {
namespace core {
namespace ir {

class Term;

namespace dflow {

class Dataflow;

/**
 * Information about which term is being read by which terms.
 */
class Uses {
    /** Mapping from a write term to the list of terms using it. */
    boost::unordered_map<const Term *, std::vector<const Term *>> term2uses_;

public:
    /**
     * Constructs use information from dataflow information.
     *
     * \param dataflow Dataflow.
     */
    Uses(const Dataflow &dataflow);

    /**
     * \param[in] term Valid pointer to a write term.
     *
     * \return List of term's uses. If it has not been set before,
     *         an empty vector is returned.
     */
    std::vector<const Term *> &getUses(const Term *term) {
        assert(term != NULL);
        assert(term->isWrite());
        return term2uses_[term];
    }

    /**
     * \param[in] term Valid pointer to a write term.
     *
     * \return List of term's uses. If it has not been set before,
     *         an empty vector is returned.
     */
    const std::vector<const Term *> &getUses(const Term *term) const {
        assert(term != NULL);
        assert(term->isWrite());
        return nc::find(term2uses_, term);
    }
};

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
