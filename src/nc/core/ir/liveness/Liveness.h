/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <boost/unordered_set.hpp>

namespace nc {
namespace core {
namespace ir {

class Term;

namespace liveness {

/**
 * Set of terms producing actual high-level code.
 */
class Liveness {
    boost::unordered_set<const Term *> liveTerms_; ///< Set of live terms.

    public:

    /**
     * \param[in] term Term.
     *
     * \return True if term is live.
     */
    bool isLive(const Term *term) const { return liveTerms_.find(term) != liveTerms_.end(); }

    /**
     * Marks a term as live.
     *
     * \param[in] term Term.
     */
    void makeLive(const Term *term) { liveTerms_.insert(term); }
};

} // namespace liveness
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
