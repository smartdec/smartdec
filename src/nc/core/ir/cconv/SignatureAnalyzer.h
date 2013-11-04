/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

namespace nc {

class CancellationToken;

namespace core {
namespace ir {

class Functions;

namespace cconv {

class CallsData;
class Signatures;

/**
 * This class reconstructs signatures of functions.
 */
class SignatureAnalyzer {
    Signatures &signatures_;
    const Functions &functions_;
    CallsData &callsData_;

public:
    /**
     * Constructor.
     *
     * \param signatures An object where to store reconstructed signatures.
     * \param functions Functions.
     * \param callsData Calls data.
     */
    SignatureAnalyzer(Signatures &signatures, const Functions &functions, CallsData &callsData):
        signatures_(signatures),
        functions_(functions),
        callsData_(callsData)
    {}

    /**
     * Reconstructs signatures of all functions and all called functions.
     *
     * \param[in] canceled Cancellation token.
     */
    void analyze(const CancellationToken &canceled);
};

} // namespace cconv
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
