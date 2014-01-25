/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <vector>

#include <boost/unordered_map.hpp>

#include <nc/core/ir/MemoryLocation.h>

#include "CalleeId.h"

namespace nc {

class CancellationToken;

namespace core {

namespace image {
    class Image;
}

namespace ir {

class Functions;

namespace dflow {
    class Dataflows;
}

namespace calling {

class CalleeId;
class Hooks;
class Signatures;

/**
 * This class reconstructs signatures of functions.
 */
class SignatureAnalyzer {
    Signatures &signatures_;
    const image::Image &image_;
    const Functions &functions_;
    const dflow::Dataflows &dataflows_;
    Hooks &hooks_;

    /** Flag whether arguments were changed since last time. */
    bool changed_;

    /** Mapping from a callee id to the locations of the arguments. */
    boost::unordered_map<CalleeId, std::vector<MemoryLocation>> id2arguments_;

public:
    /**
     * Constructor.
     *
     * \param signatures An object where to store reconstructed signatures.
     * \param image Executable image.
     * \param functions Functions.
     * \param dataflows Dataflows.
     * \param hooks Calls data.
     */
    SignatureAnalyzer(Signatures &signatures, const image::Image &image, const Functions &functions,
        const dflow::Dataflows &dataflows, Hooks &hooks
    ):
        signatures_(signatures),
        image_(image),
        functions_(functions),
        dataflows_(dataflows),
        hooks_(hooks)
    {}

    /**
     * Reconstructs signatures of all functions and all called functions.
     *
     * \param[in] canceled Cancellation token.
     */
    void analyze(const CancellationToken &canceled);

private:
    /**
     * Computes locations of arguments for all functions.
     *
     * \param[in] canceled Cancellation token.
     */
    void computeArguments(const CancellationToken &canceled);

    /**
     * Computes locations of arguments for a given function.
     *
     * \param[in] canceled Cancellation token.
     */
    void computeArguments(const Function *function);

    /**
     * Adds given memory locations to the list of locations of arguments
     * of the given callee.
     *
     * \param calleeId Valid callee id.
     * \param arguments Argument locations to be added.
     */
    void addArguments(const CalleeId &calleeId, std::vector<MemoryLocation> arguments);

    /**
     * Sorts computed argument locations, so that they follow in the order
     * in which they are described in the calling convention.
     */
    void sortArguments();

    /**
     * Computes and sets signatures for all callee ids.
     *
     * \param[in] canceled Cancellation token.
     */
    void computeSignatures(const CancellationToken &canceled);

    /**
     * Reconstructs the signature of a function identified by a given callee id.
     *
     * \param calleeId Valid callee id.
     */
    void computeSignature(const CalleeId &calleeId);
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
