/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "SignatureAnalyzer.h"

#include <nc/common/CancellationToken.h>
#include <nc/common/Foreach.h>

#include <nc/core/ir/Functions.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/misc/CensusVisitor.h>

#include "CallsData.h"
#include "DescriptorAnalyzer.h"
#include "Signatures.h"

namespace nc {
namespace core {
namespace ir {
namespace cconv {

void SignatureAnalyzer::analyze(const CancellationToken &canceled) {
    auto computeSignature = [&](const CalleeId &calleeId) {
        if (calleeId) {
            if (!signatures_.isSet(calleeId)) {
                if (auto analyzer = callsData_.getDescriptorAnalyzer(calleeId)) {
                    signatures_.setSignature(calleeId, analyzer->getSignature());
                }
            }
        }
    };

    foreach (auto function, functions_.functions()) {
        computeSignature(callsData_.getCalleeId(function));
        if (canceled) {
            return;
        }
    }

    misc::CensusVisitor visitor(&callsData_);
    visitor(&functions_);

    foreach (auto statement, visitor.statements()) {
        if (auto call = statement->asCall()) {
            computeSignature(callsData_.getCalleeId(call));
        }
        if (canceled) {
            return;
        }
    }
}

} // namespace cconv
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
