/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "SignatureAnalyzer.h"

#include <nc/common/CancellationToken.h>
#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/core/ir/Statements.h>
#include <nc/core/ir/dflow/Dataflows.h>
#include <nc/core/ir/misc/CensusVisitor.h>

#include "CallHook.h"
#include "Convention.h"
#include "Conventions.h"
#include "Hooks.h"
#include "Signatures.h"

namespace nc {
namespace core {
namespace ir {
namespace calling {

void SignatureAnalyzer::analyze(const CancellationToken &canceled) {
    /*
     * Gets the argument locations read but not defined in a function.
     */
    auto getUsedArguments = [&](const Function *function, const Convention *convention) {
        assert(function != NULL);
        assert(convention != NULL);

        auto dataflow = dataflows_.getDataflow(function);
        assert(dataflow);

        misc::CensusVisitor census(NULL);
        census(function);

        /*
         * If one reads a register or a stack location that can be used
         * for passing arguments, and there are no definitions of this
         * memory location in the function itself, then this location is
         * likely used for passing an argument.
         */
        std::vector<MemoryLocation> result;

        foreach (const Term *term, census.terms()) {
            if (term->isRead()) {
                if (const auto &memoryLocation = dataflow->getMemoryLocation(term)) {
                    if (convention->isArgumentLocation(memoryLocation)) {
                        if (dataflow->getDefinitions(term).empty()) {
                            result.push_back(memoryLocation);
                        }
                    }
                }
            }
        }

        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());

        return result;
    };

    auto getDefinedArguments = [](CallHook *callHook, const Convention *convention) {
        assert(callHook);
        assert(convention);

        std::vector<MemoryLocation> result;
        foreach (const auto &group, convention->argumentGroups()) {
            foreach (const auto &argument, group.arguments()) {
                if (callHook->reachingDefinitions().projected(argument.location()).empty()) {
                    result.push_back(argument.location());
                }
            }
        }
        return result;
    };

    auto getSignature = [&](const CalleeId &calleeId, const Hooks::CalleeHooks &calleeHooks) {
        auto signature = std::make_unique<Signature>();

        auto convention = hooks_.conventions().getConvention(calleeId);
        assert(convention);

        foreach (const auto &pair, calleeHooks.entryHooks) {
            foreach (const auto &location, getUsedArguments(pair.first, convention)) {
                signature->addArgument(location);
            }
        }

        return signature;
    };

    foreach (const auto &pair, hooks_.calleeHooks()) {
        signatures_.setSignature(pair.first, getSignature(pair.first, pair.second));
        canceled.poll();
    }
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
