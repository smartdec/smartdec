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
class Return;

namespace dflow {
    class Dataflows;
    class Uses;
}

namespace calling {

class FunctionSignature;
class Hooks;
class Signatures;

/**
 * This class reconstructs signatures of functions.
 */
class SignatureAnalyzer {
    Signatures &signatures_;
    const image::Image &image_;
    const dflow::Dataflows &dataflows_;
    const Hooks &hooks_;

    struct Referrers {
        std::vector<const Function *> functions;
        std::vector<const Call *> calls;
        std::vector<const Return *> returns;
    };

    /** Mapping from a callee id to the functions, calls, and returns with this id. */
    boost::unordered_map<CalleeId, Referrers> id2referrers_;

    /** Mapping from a function to the list of calls in it.*/
    boost::unordered_map<const Function *, std::vector<const Call *>> function2calls_;

    /** Mapping from a function to the term use information for this function. */
    boost::unordered_map<const Function *, std::unique_ptr<dflow::Uses>> function2uses_;

    /** Mapping from a callee id to the list of its formal arguments. */
    boost::unordered_map<CalleeId, std::vector<MemoryLocation>> id2arguments_;

    /** Mapping from a call to the list of factual arguments not included into the list of
     *  formal arguments of the respective callee id. */
    boost::unordered_map<const Call *, std::vector<MemoryLocation>> call2extraArguments_;

public:
    /**
     * Constructor.
     *
     * \param signatures An object where to store reconstructed signatures.
     * \param image Executable image.
     * \param functions Functions.
     * \param dataflows Dataflows.
     * \param hooks Hooks manager.
     */
    SignatureAnalyzer(Signatures &signatures, const image::Image &image, const Functions &functions,
        const dflow::Dataflows &dataflows, const Hooks &hooks);

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
     * Recomputes arguments of the function with the given callee id
     * by looking at the function's body and calls to it.
     *
     * \param[in] calleeId Valid callee id.
     *
     * \return True if something has changed, false otherwise.
     */
    bool computeArguments(const CalleeId &calleeId);

    /**
     * \param[in] function Valid pointer to a function.
     *
     * \return Memory locations in the function that are read in the function,
     *         but whose value is not defined before it is read.
     */
    std::vector<MemoryLocation> getUndefinedUses(const Function *function);

    /**
     * \param[in] call Valid pointer to a call statement.
     *
     * \return Memory locations that are defined before the call, but never used.
     *         Stack offsets are fixed up in accordance to the reaching stack pointer value.
     */
    std::vector<MemoryLocation> getUnusedDefines(const Call *call);

    /**
     * Computes and sets signatures for all callee ids.
     *
     * \param[in] canceled Cancellation token.
     */
    void computeSignatures(const CancellationToken &canceled);

    /**
     * Reconstructs the signatures of the functions and calls
     * with the given callee id.
     *
     * \param calleeId Valid callee id.
     */
    void computeSignatures(const CalleeId &calleeId);

    /**
     * Chooses a name for a function with the given callee id and
     * sets it in the signature.
     *
     * \param calleeId Valid callee id.
     * \param signature Signature.
     */
    void computeName(const CalleeId &calleeId, FunctionSignature &signature);
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
