/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <vector>

#include <QCoreApplication>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <nc/common/CancellationToken.h>
#include <nc/common/LogToken.h>

#include <nc/core/ir/MemoryLocation.h>

#include "CalleeId.h"

namespace nc {

namespace core {

namespace image {
    class Image;
}

namespace ir {

class Call;
class Functions;
class Return;
class Term;

namespace dflow {
    class Dataflows;
    class Uses;
}

namespace calling {

class FunctionSignature;
class Hooks;
class Signatures;
class ReturnHook;

/**
 * This class reconstructs signatures of functions.
 */
class SignatureAnalyzer {
    Q_DECLARE_TR_FUNCTIONS(SignatureAnalyzer)

    Signatures &signatures_;
    const image::Image &image_;
    const dflow::Dataflows &dataflows_;
    const Hooks &hooks_;
    const CancellationToken &canceled_;
    const LogToken &log_;

    struct Referrers {
        std::vector<const Function *> functions;
        std::vector<const Call *> calls;
        std::vector<const Return *> returns;
    };

    /** Mapping from a callee id to the functions, calls, and returns with this id. */
    boost::unordered_map<CalleeId, Referrers> id2referrers_;

    /** Mapping from a function to the list of calls in it.*/
    boost::unordered_map<const Function *, std::vector<const Call *>> function2calls_;

    /** Mapping from a function to the list of returns in it.*/
    boost::unordered_map<const Function *, std::vector<const Return *>> function2returns_;

    /** Mapping from a function to the term use information for this function. */
    boost::unordered_map<const Function *, std::unique_ptr<dflow::Uses>> function2uses_;

    /** Mapping from a callee id to the list of its formal arguments. */
    boost::unordered_map<CalleeId, std::vector<MemoryLocation>> id2arguments_;

    /** Mapping from a call to the list of factual arguments not included into the list of
     *  formal arguments of the respective callee id. */
    boost::unordered_map<const Call *, std::vector<MemoryLocation>> call2extraArguments_;

    /** Mapping from a callee id to the estimated return value term and
     * (if it is a MemoryLocationAccess) its part used for passing the return value. */
    boost::unordered_map<CalleeId, std::pair<const Term *, MemoryLocation>> id2returnValue_;

    /** Terms that do not belong to the program, but to the hooks. */
    boost::unordered_set<const Term *> artificialTerms_;

public:
    /**
     * Constructor.
     *
     * \param signatures An object where to store reconstructed signatures.
     * \param image Executable image.
     * \param functions Functions.
     * \param dataflows Dataflows.
     * \param hooks Hooks manager.
     * \param canceled Cancellation token.
     * \param log Log token.
     */
    SignatureAnalyzer(Signatures &signatures, const image::Image &image, const Functions &functions,
        const dflow::Dataflows &dataflows, const Hooks &hooks, const CancellationToken &canceled,
        const LogToken &log);

    /**
     * Destructor.
     */
    ~SignatureAnalyzer();

    void analyze();

private:
    /**
     * Computes locations of arguments for all functions.
     */
    void computeArgumentsAndReturnValues();

    /**
     * \param term Valid pointer to a read term.
     *
     * \return True if the term is a read that belongs to some instruction
     *         or is a read of a return value created by ReturnHook, and
     *         this term belongs to the signature.
     */
    bool isRealRead(const Term *term);

    /**
     * \param term Valid pointer to a write term.
     *
     * \return True if the term is a write that belongs to some instruction
     *         or is a write of return value created by CallHook, and
     *         this term belongs to the signature.
     */
    bool isRealWrite(const Term *term);

    /**
     * Computes the terms representing potential arguments inserted by hooks.
     */
    void computeArtificialTerms();

    /**
     * Recomputes arguments of the function with the given callee id
     * by looking at the function's body and calls to it.
     *
     * \param[in] calleeId Valid callee id.
     *
     * \return True if the arguments have changed, false otherwise.
     */
    bool computeArguments(const CalleeId &calleeId);

    /**
     * Recomputes the return value of the function with the given callee id
     * by looking at the call and return sites.
     *
     * \param[in] calleeId Valid callee id.
     *
     * \return True if the return value has changed, false otherwise.
     */
    bool computeReturnValue(const CalleeId &calleeId);

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
     * \param[in] call Valid pointer to a call statement.
     *
     * \return List of terms identifying the locations that can be used
     *         to pass return values and that are read after the call,
     *         together with the corresponding memory locations being read.
     */
    std::vector<std::pair<const Term *, MemoryLocation>> getUsedReturnValues(const Call *call);

    /**
     * \param[in] ret Valid pointer to a return statement.
     *
     * \return List of terms identifying the locations that can be used
     *         to pass return values, values in which are defined before
     *         the return, however, not used before it.
     */
    std::vector<std::pair<const Term *, MemoryLocation>> getUnusedReturnValues(const Return *ret);

    /**
     * Computes and sets signatures for all callee ids.
     */
    void computeSignatures();

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
