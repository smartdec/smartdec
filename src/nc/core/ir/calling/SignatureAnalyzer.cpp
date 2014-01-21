/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "SignatureAnalyzer.h"

#include <cstdint> /* uintptr_t */

#include <boost/range/adaptor/map.hpp>

#include <nc/common/CancellationToken.h>
#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/core/image/Image.h>
#include <nc/core/image/Symbols.h>
#include <nc/core/ir/Functions.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/dflow/Dataflows.h>
#include <nc/core/ir/misc/CensusVisitor.h>
#include <nc/core/likec/Tree.h>
#include <nc/core/mangling/Demangler.h>

#include "CallHook.h"
#include "Convention.h"
#include "Conventions.h"
#include "Hooks.h"
#include "Signatures.h"

namespace nc {
namespace core {
namespace ir {
namespace calling {

void SignatureAnalyzer::analyze(const CancellationToken & canceled) {
    foreach (auto function, functions_.all()) {
        analyze(hooks_.getCalleeId(function));
        canceled.poll();
    }

    foreach (const auto &calleeId, hooks_.map() | boost::adaptors::map_keys) {
        analyze(calleeId);
        canceled.poll();
    }

// TODO
#if 0
    /*
     * Gets the argument locations read but not defined in a function.
     */
    auto getUsedArguments = [&](const Function *function, const Convention *convention) {
        assert(function != NULL);
        assert(convention != NULL);

        auto &dataflow = *dataflows_.at(function);

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
                if (const auto &memoryLocation = dataflow.getMemoryLocation(term)) {
                    if (convention->isArgumentLocation(memoryLocation)) {
                        if (dataflow.getDefinitions(term).empty()) {
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
#endif
}

void SignatureAnalyzer::analyze(const CalleeId &calleeId) {
    assert(calleeId);

    if (signatures_.getSignature(calleeId)) {
        return;
    }

    auto signature = std::make_unique<Signature>();

    /*
     * Pick up a name for the function.
     */
    if (calleeId.entryAddress()) {
        /* Take the name of the corresponding symbol, if possible. */
        if (auto symbol = image_.symbols()->find(image::Symbol::Function, *calleeId.entryAddress())) {
            signature->setName(likec::Tree::cleanName(symbol->name()));

            if (signature->name() != symbol->name()) {
                signature->addComment(symbol->name());
            }

            QString demangledName = image_.demangler()->demangle(symbol->name());
            if (demangledName != symbol->name()) {
                signature->addComment(demangledName);
            }
        }

        if (signature->name().isEmpty()) {
            /* Invent a name based on the entry address. */
            signature->setName(QString(QLatin1String("func_%1"))
                .arg(*calleeId.entryAddress(), 0, 16));
        }
    } else if (calleeId.function()) {
        signature->setName(QString(QLatin1String("func_noentry_%1"))
            .arg(reinterpret_cast<std::uintptr_t>(calleeId.function()), 0, 16));
    } else {
        /* Function is unknown, leave the name empty. */
    }

    signatures_.setSignature(calleeId, std::move(signature));
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
