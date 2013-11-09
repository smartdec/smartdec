/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory>

#include <boost/unordered_map.hpp>

#include <nc/common/Range.h>

#include "CalleeId.h"
#include "Signature.h"

namespace nc {
namespace core {
namespace ir {
namespace cconv {

class Signatures {
    /** Mapping from a callee id to a signature. */
    boost::unordered_map<CalleeId, std::unique_ptr<Signature>> id2signature_;

public:
    /**
     * \param calleeId Valid callee id.
     *
     * \return Pointer to the signature assigned for this callee id.
     *         Returns NULL if no signature was assigned.
     */
    const Signature *getSignature(const CalleeId &calleeId) const {
        assert(calleeId);
        return nc::find(id2signature_, calleeId).get();
    }

    /**
     * Sets signature for a given callee id.
     *
     * \param calleeId Valid callee id.
     * \param signature Pointer to the signature. Can be NULL.
     */
    void setSignature(const CalleeId &calleeId, std::unique_ptr<Signature> signature) {
        assert(calleeId);
        id2signature_[calleeId] = std::move(signature);
    }
};

} // namespace cconv
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
