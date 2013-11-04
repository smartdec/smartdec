/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

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
    boost::unordered_map<CalleeId, Signature> id2signature_;

public:
    /**
     * \param calleeId Valid callee id.
     *
     * \return Signature for this callee id.
     *         If none was set, a default-constructed signature is returned.
     */
    const Signature &getSignature(const CalleeId &calleeId) const {
        assert(calleeId);
        return nc::find(id2signature_, calleeId);
    }

    /**
     * Sets signature for a given callee id.
     *
     * \param calleeId Valid callee id.
     * \param signature Signature.
     */
    void setSignature(const CalleeId &calleeId, Signature signature) {
        assert(calleeId);
        id2signature_[calleeId] = std::move(signature);
    }

    /**
     * \param calleeId Valid callee id.
     *
     * \return True if a signature for this callee id was set, false otherwise.
     */
    bool isSet(const CalleeId &calleeId) const {
        assert(calleeId);
        return nc::contains(id2signature_, calleeId);
    }
};

} // namespace cconv
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
