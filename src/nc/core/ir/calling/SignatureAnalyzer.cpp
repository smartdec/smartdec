/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "SignatureAnalyzer.h"

#include <cstdint> /* uintptr_t */

#include <boost/range/adaptor/map.hpp>

#include <nc/common/CancellationToken.h>
#include <nc/common/Foreach.h>
#include <nc/common/Warnings.h>
#include <nc/common/make_unique.h>

#include <nc/core/image/Image.h>
#include <nc/core/image/Symbols.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Functions.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/dflow/Dataflows.h>
#include <nc/core/ir/dflow/Uses.h>
#include <nc/core/ir/dflow/Value.h>
#include <nc/core/likec/Tree.h>
#include <nc/core/mangling/Demangler.h>

#include "CallHook.h"
#include "Convention.h"
#include "Conventions.h"
#include "EntryHook.h"
#include "Hooks.h"
#include "ReturnHook.h"
#include "Signatures.h"

namespace nc {
namespace core {
namespace ir {
namespace calling {

SignatureAnalyzer::SignatureAnalyzer(Signatures &signatures, const image::Image &image, const Functions &functions,
    const dflow::Dataflows &dataflows, Hooks &hooks
):
    signatures_(signatures),
    image_(image),
    dataflows_(dataflows),
    hooks_(hooks)
{
    foreach (auto function, functions.all()) {
        foreach (auto basicBlock, function->basicBlocks()) {
            foreach (auto statement, basicBlock->statements()) {
                if (auto call = statement->asCall()) {
                    call2function_[call] = function;
                    function2calls_[function].push_back(call);
                }
            }
        }
    }

    foreach (const auto &functionAndDataflow, dataflows_) {
        function2uses_[functionAndDataflow.first] = std::make_unique<dflow::Uses>(*functionAndDataflow.second);
    }
}

SignatureAnalyzer::~SignatureAnalyzer() {}

void SignatureAnalyzer::analyze(const CancellationToken &canceled) {
    computeArguments(canceled);
    computeSignatures(canceled);
}

void SignatureAnalyzer::computeArguments(const CancellationToken &canceled) {
    int niterations = 0;

    bool changed;
    do {
        changed = false;

        foreach (const CalleeId &calleeId, hooks_.map() | boost::adaptors::map_keys) {
            auto arguments = computeArguments(calleeId);
            auto &oldArguments = id2arguments_[calleeId];

            if (arguments != oldArguments) {
                oldArguments = std::move(arguments);
                changed = true;
            }

            canceled.poll();
        }

        if (++niterations > 5) {
            ncWarning("Didn't reach a fixpoint after %1 iterations while reconstructing arguments. Giving up.", niterations);
            break;
        }
    } while (changed);
}

namespace {

template<class Container>
bool isHomogeneous(const Container &container) {
    if (container.empty()) {
        return true;
    }

    const auto &first = *container.begin();
    foreach (const auto &value, container) {
        if (value != first) {
            return false;
        }
    }

    return true;
}

} // anonymous namespace

std::vector<MemoryLocation> SignatureAnalyzer::computeArguments(const CalleeId &calleeId) {
    assert(calleeId);

    std::vector<MemoryLocation> result;

    auto convention = hooks_.conventions().getConvention(calleeId);
    if (!convention) {
        return result;
    }

    const auto &calleeHooks = nc::find(hooks_.map(), calleeId);

    struct Placement {
        MemoryLocation inFunctions;
        boost::unordered_map<const Call *, MemoryLocation> inCalls;
    };

    boost::unordered_map<MemoryLocation, Placement> placements;

    foreach (const auto &functionAndHook, calleeHooks.entryHooks) {
        foreach (const auto &memoryLocation, getUndefinedUses(functionAndHook.first)) {
            if (auto argumentLocation = convention->getArgumentLocationCovering(memoryLocation)) {
                placements[argumentLocation].inFunctions.merge(memoryLocation);
            }
        }
    }

    foreach (const auto &callAndHook, calleeHooks.callHooks) {
        foreach (const auto &memoryLocation, getUnusedDefines(callAndHook.first, callAndHook.second.get())) {
            if (auto argumentLocation = convention->getArgumentLocationCovering(memoryLocation)) {
                placements[argumentLocation].inCalls[callAndHook.first].merge(memoryLocation);
            }
        }
    }

    auto getArgumentLocation = [](const Placement &placement) -> MemoryLocation {
        if (placement.inFunctions) {
            return placement.inFunctions;
        } else if (!placement.inCalls.empty() && isHomogeneous(placement.inCalls | boost::adaptors::map_values)) {
            return placement.inCalls.begin()->second;
        } else {
            return MemoryLocation();
        }
    };

    foreach (auto &locationAndPlacement, placements) {
        if (auto location = getArgumentLocation(locationAndPlacement.second)) {
            if (location.addr() == locationAndPlacement.first.addr()) {
                result.push_back(location);
            }
        }
    }

    return convention->sortArguments(result);
}

std::vector<MemoryLocation> SignatureAnalyzer::getUndefinedUses(const Function *function) {
    assert(function != NULL);

    auto &dataflow = *dataflows_.at(function);

    std::vector<MemoryLocation> result;

    /*
     * If a term reads a memory location through which an argument
     * can be passed, and nobody defines this memory location, this
     * location is likely to be actually used for passing an argument.
     */
    foreach (const auto &termAndLocation, dataflow.term2location()) {
        auto term = termAndLocation.first;
        const auto &memoryLocation = termAndLocation.second;

        if (term->isRead() && dataflow.getDefinitions(term).empty()) {
            result.push_back(memoryLocation);
        }
    }

    /*
     * If a call has an argument whose memory location can be used
     * for passing arguments to this function, and this memory location
     * is not defined in the function, this memory location is likely
     * to be used for passing an argument.
     */
    foreach (auto call, nc::find(function2calls_, function)) {
        const auto &callArguments = nc::find(id2arguments_, hooks_.getCalleeId(call));
        if (callArguments.empty()) {
            continue;
        }

        auto callHook = hooks_.getCallHook(call);
        auto stackPointerValue = dataflow.getValue(callHook->stackPointer());

        auto fixup = [&](const MemoryLocation &memoryLocation) -> MemoryLocation {
            if (memoryLocation.domain() == MemoryDomain::STACK) {
                if (stackPointerValue->isStackOffset()) {
                    return memoryLocation.shifted(stackPointerValue->stackOffset() * CHAR_BIT);
                } else {
                    return MemoryLocation();
                }
            } else {
                return memoryLocation;
            }
        };

        foreach (auto memoryLocation, callArguments) {
            memoryLocation = fixup(memoryLocation);

            if (memoryLocation && callHook->reachingDefinitions().projected(memoryLocation).empty()) {
                result.push_back(memoryLocation);
            }
        }
    }

    return result;
}

std::vector<MemoryLocation> SignatureAnalyzer::getUnusedDefines(const Call *call, const CallHook *callHook) {
    assert(call != NULL);
    assert(callHook != NULL);

    std::vector<MemoryLocation> result;

    auto function = nc::find(call2function_, call);
    assert(function != NULL);

    auto &dataflow = *dataflows_.at(function);
    auto &uses = *function2uses_.at(function);
    auto stackPointerValue = dataflow.getValue(callHook->stackPointer());

    auto fixup = [&](const MemoryLocation &memoryLocation) -> MemoryLocation {
        if (memoryLocation.domain() == MemoryDomain::STACK) {
            if (stackPointerValue->isStackOffset()) {
                return memoryLocation.shifted(-stackPointerValue->stackOffset() * CHAR_BIT);
            } else {
                return MemoryLocation();
            }
        } else {
            return memoryLocation;
        }
    };

    /*
     * Count as an argument everything that reaches the call, can be used to
     * pass an argument, and not used somewhere else.
     */
    foreach (const auto &chunk, callHook->reachingDefinitions().chunks()) {
        if (auto memoryLocation = fixup(chunk.location())) {
            foreach (const Term *term, chunk.definitions()) {
                if (uses.getUses(term).empty()) {
                    result.push_back(memoryLocation);
                    break;
                }
            }
        }
    }

    return result;
}

void SignatureAnalyzer::computeSignatures(const CancellationToken &canceled) {
    foreach (const auto &calleeId, hooks_.map() | boost::adaptors::map_keys) {
        computeSignature(calleeId);
        canceled.poll();
    }
}

void SignatureAnalyzer::computeSignature(const CalleeId &calleeId) {
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
            if (demangledName.contains('(') && demangledName != symbol->name()) {
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
            .arg(reinterpret_cast<uintptr_t>(calleeId.function()), 0, 16));
    } else {
        /* Function is unknown, leave the name empty. */
    }

    /*
     * Set arguments.
     */
    if (auto convention = hooks_.conventions().getConvention(calleeId)) {
        foreach (const auto &memoryLocation, nc::find(id2arguments_, calleeId)) {
            if (memoryLocation.domain() == MemoryDomain::STACK) {
                signature->addArgument(std::make_unique<Dereference>(
                    std::make_unique<BinaryOperator>(
                        BinaryOperator::ADD,
                        std::make_unique<MemoryLocationAccess>(convention->stackPointer()),
                        std::make_unique<Constant>(SizedValue(
                            convention->stackPointer().size<SmallBitSize>(),
                            memoryLocation.addr() / CHAR_BIT)),
                        convention->stackPointer().size()),
                    MemoryDomain::MEMORY,
                    memoryLocation.size<SmallBitSize>()
                ));
            } else {
                signature->addArgument(std::make_unique<MemoryLocationAccess>(memoryLocation));
            }
        }
    }

    signatures_.setSignature(calleeId, std::move(signature));
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
