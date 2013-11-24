/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

namespace nc {

class CancellationToken;

namespace core {
namespace ir {

namespace dflow {
    class Dataflows;
}

namespace calling {

class Hooks;
class Signatures;

/**
 * This class reconstructs signatures of functions.
 */
class SignatureAnalyzer {
    Signatures &signatures_;
    const dflow::Dataflows &dataflows_;
    const Hooks &hooks_;

public:
    /**
     * Constructor.
     *
     * \param signatures An object where to store reconstructed signatures.
     * \param dataflows Dataflows.
     * \param hooks Calls data.
     */
    SignatureAnalyzer(Signatures &signatures, const dflow::Dataflows &dataflows, const Hooks &hooks):
        signatures_(signatures),
        dataflows_(dataflows),
        hooks_(hooks)
    {}

    /**
     * Reconstructs signatures of all functions and all called functions.
     *
     * \param[in] canceled Cancellation token.
     */
    void analyze(const CancellationToken &canceled);
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
