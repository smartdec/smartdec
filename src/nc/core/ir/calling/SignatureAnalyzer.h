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

class Call;
class Functions;

namespace dflow {
    class Dataflows;
    class Uses;
}

namespace calling {

class CallHook;
class CalleeId;
class Hooks;
class Signatures;

/**
 * This class reconstructs signatures of functions.
 */
class SignatureAnalyzer {
    Signatures &signatures_;
    const image::Image &image_;
    const dflow::Dataflows &dataflows_;
    Hooks &hooks_;
    boost::unordered_map<const Call *, const Function *> call2function_;
    boost::unordered_map<const Function *, std::vector<const Call *>> function2calls_;
    boost::unordered_map<const Function *, std::unique_ptr<dflow::Uses>> function2uses_;
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
        const dflow::Dataflows &dataflows, Hooks &hooks);

    /**
     * Destructor.
     */
    ~SignatureAnalyzer();

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
     * Computes arguments of the function with the given callee id
     * by looking at the function's body and calls to it.
     *
     * \param[in] calleeId Valid callee id.
     *
     * \return Computed locations of arguments.
     */
    std::vector<MemoryLocation> computeArguments(const CalleeId &calleeId);

    /**
     * \param[in] function Valid pointer to a function.
     *
     * \return Memory locations in the function that are read in the function,
     *         but whose value is not defined before it is read.
     */
    std::vector<MemoryLocation> getUndefinedUses(const Function *function);

    /**
     * \param[in] call Valid pointer to a call statement.
     * \param[in] callHook Valid pointer to its call hook.
     *
     * \return Memory locations that are defined before the call, but never used.
     *         Stack offsets are fixed up in accordance to the reaching stack pointer value.
     */
    std::vector<MemoryLocation> getUnusedDefines(const Call *call, const CallHook *callHook);

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
