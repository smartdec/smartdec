/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

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
    const Hooks &hooks_;

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
        const dflow::Dataflows &dataflows, const Hooks &hooks
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
     * Reconstructs the signature of a function identified by a given callee id.
     *
     * \param calleeId Valid callee id.
     */
    void analyze(const CalleeId &calleeId);
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
