/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "SignatureAnalyzer.h"

#include <nc/common/CancellationToken.h>
#include <nc/common/Foreach.h>

#include <nc/core/ir/Functions.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/misc/CensusVisitor.h>

#include "Hooks.h"
#include "DescriptorAnalyzer.h"
#include "Signatures.h"

namespace nc {
namespace core {
namespace ir {
namespace cconv {

void SignatureAnalyzer::analyze(const CancellationToken &canceled) {
    auto computeSignature = [&](const CalleeId &calleeId) {
        if (calleeId) {
            if (signatures_.getSignature(calleeId) == NULL) {
                if (auto analyzer = hooks_.getDescriptorAnalyzer(calleeId)) {
                    signatures_.setSignature(calleeId, analyzer->getSignature());
                }
            }
        }
    };

    foreach (auto function, functions_.functions()) {
        computeSignature(hooks_.getCalleeId(function));
        canceled.poll();
    }

    misc::CensusVisitor visitor(&hooks_);
    visitor(&functions_);

    foreach (auto statement, visitor.statements()) {
        if (auto call = statement->asCall()) {
            computeSignature(hooks_.getCalleeId(call));
        }
        canceled.poll();
    }
}

} // namespace cconv
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
