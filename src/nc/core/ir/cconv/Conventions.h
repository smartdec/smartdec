/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>

#include "CalleeId.h"

namespace nc {
namespace core {
namespace ir {
namespace cconv {

class CallingConvention;

/**
 * Information about assigned calling conventions and their parameters.
 */
class Conventions {
    /** Mapping from a callee id to the associated calling convention. */
    boost::unordered_map<CalleeId, const CallingConvention *> id2convention_;

    /** Mapping from a callee id to its arguments size. */
    boost::unordered_map<CalleeId, ByteSize> id2argumentsSize_;

public:

    /**
     * Assigns a calling convention to a callee id.
     *
     * \param calleeId Callee id.
     * \param convention Pointer to a calling convention of the function. Can be NULL.
     */
    void setCallingConvention(const CalleeId &calleeId, const CallingConvention *convention) {
        id2convention_[calleeId] = convention;
    }

    /**
     * \param calleeId Callee id.
     *
     * \return Pointer to the calling convention used for calls to given address. Can be NULL.
     */
    const CallingConvention *getCallingConvention(const CalleeId &calleeId) const {
        return nc::find(id2convention_, calleeId);
    }

    /**
     * Sets the stack arguments size for the given callee id.
     *
     * \param calleeId Callee id.
     * \param size Size of the arguments passed on the stack, or boost::none if unknown.
     */
    void setArgumentsSize(const CalleeId &calleeId, const boost::optional<ByteSize> &size) {
        if (size) {
            id2argumentsSize_[calleeId] = *size;
        } else {
            id2argumentsSize_.erase(calleeId);
        }
    }

    /**
     * \param calleeId Callee id.
     *
     * \return Size of the arguments passed on the stack, or boost::none if unknown.
     */
    boost::optional<ByteSize> getArgumentsSize(const CalleeId &calleeId) {
        return nc::find_optional(id2argumentsSize_, calleeId);
    }
};

} // namespace cconv
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
