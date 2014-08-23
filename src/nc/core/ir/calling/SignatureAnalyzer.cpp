/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "SignatureAnalyzer.h"

#include <cstdint> /* uintptr_t */

#include <boost/range/adaptor/map.hpp>

#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/core/image/Image.h>
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

SignatureAnalyzer::SignatureAnalyzer(Signatures &signatures, const image::Image &image,
    const Functions &functions, const dflow::Dataflows &dataflows, const Hooks &hooks,
    const CancellationToken &canceled, const LogToken &log
):
    signatures_(signatures),
    image_(image),
    dataflows_(dataflows),
    hooks_(hooks),
    canceled_(canceled),
    log_(log)
{
    foreach (auto function, functions.list()) {
        auto &dataflow = *dataflows.at(function);

        id2referrers_[getCalleeId(function)].functions.push_back(function);

        foreach (auto basicBlock, function->basicBlocks()) {
            foreach (auto statement, basicBlock->statements()) {
                if (auto call = statement->asCall()) {
                    id2referrers_[getCalleeId(call, dataflow)].calls.push_back(call);
                    function2calls_[function].push_back(call);
                } else if (auto ret = statement->asReturn()) {
                    id2referrers_[getCalleeId(function)].returns.push_back(ret);
                    function2returns_[function].push_back(ret);
                }
            }
        }
    }

    foreach (const auto &functionAndDataflow, dataflows_) {
        function2uses_[functionAndDataflow.first] = std::make_unique<dflow::Uses>(*functionAndDataflow.second);
    }
}

SignatureAnalyzer::~SignatureAnalyzer() {}

void SignatureAnalyzer::analyze() {
    computeArgumentsAndReturnValues();
    computeSignatures();
}

void SignatureAnalyzer::computeArgumentsAndReturnValues() {
    int niterations = 0;

    bool changed;
    do {
        changed = false;

        computeArtificialTerms();

        foreach (const CalleeId &calleeId, id2referrers_ | boost::adaptors::map_keys) {
            if (computeArguments(calleeId)) {
                changed = true;
            }
            if (computeReturnValue(calleeId)) {
                changed = true;
            }
        }

        if (++niterations > 3) {
            log_.warning(tr("Fixpoint was not reached after %1 iterations while reconstructing arguments. Giving up.").arg(niterations));
            break;
        }

        canceled_.poll();
    } while (changed);
}

bool SignatureAnalyzer::isRealRead(const Term *term) {
    assert(term != NULL);
    assert(term->isRead());

    return !nc::contains(artificialTerms_, term);
}

bool SignatureAnalyzer::isRealWrite(const Term *term) {
    assert(term != NULL);
    assert(term->isWrite());

    return !nc::contains(artificialTerms_, term);
}

void SignatureAnalyzer::computeArtificialTerms() {
    foreach (const auto &calleeAndReferrers, id2referrers_) {
        const auto &returnTerm = nc::find(id2returnValue_, calleeAndReferrers.first).first;

        foreach (const Call *call, calleeAndReferrers.second.calls) {
            if (auto hook = hooks_.getCallHook(call)) {
                foreach (const auto &termAndClone, hook->returnValueTerms()) {
                    artificialTerms_.insert(termAndClone.second);
                }
                if (returnTerm) {
                    artificialTerms_.erase(hook->getReturnValueTerm(returnTerm));
                }
            }
        }
        foreach (const Return *ret, calleeAndReferrers.second.returns) {
            if (auto hook = hooks_.getReturnHook(ret)) {
                foreach (const auto &termAndClone, hook->returnValueTerms()) {
                    artificialTerms_.insert(termAndClone.second);
                }
                if (returnTerm) {
                    artificialTerms_.erase(hook->getReturnValueTerm(returnTerm));
                }
            }
        }
    }
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

bool SignatureAnalyzer::computeArguments(const CalleeId &calleeId) {
    assert(calleeId);

    auto convention = hooks_.conventions().getConvention(calleeId);
    if (!convention) {
        return false;
    }

    const auto &referrers = nc::find(id2referrers_, calleeId);

    struct Placement {
        MemoryLocation inFunctions;
        boost::unordered_map<const Call *, MemoryLocation> inCalls;
    };

    boost::unordered_map<MemoryLocation, Placement> placements;

    foreach (auto function, referrers.functions) {
        foreach (const auto &memoryLocation, getUndefinedUses(function)) {
            if (auto argumentLocation = convention->getArgumentLocationCovering(memoryLocation)) {
                placements[argumentLocation].inFunctions.merge(memoryLocation);
            }
        }
    }

    foreach (auto call, referrers.calls) {
        foreach (const auto &memoryLocation, getUnusedDefines(call)) {
            if (auto argumentLocation = convention->getArgumentLocationCovering(memoryLocation)) {
                placements[argumentLocation].inCalls[call].merge(memoryLocation);
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

    std::vector<MemoryLocation> arguments;
    boost::unordered_map<const Call *, std::vector<MemoryLocation>> extraArguments;

    foreach (auto &locationAndPlacement, placements) {
        if (auto location = getArgumentLocation(locationAndPlacement.second)) {
            if (location.addr() == locationAndPlacement.first.addr()) {
                arguments.push_back(location);
            }
        } else {
            foreach (const auto &callAndLocation, locationAndPlacement.second.inCalls) {
                if (callAndLocation.second.addr() == locationAndPlacement.first.addr()) {
                    extraArguments[callAndLocation.first].push_back(callAndLocation.second);
                }
            }
        }
    }

    arguments = convention->sortArguments(std::move(arguments));

    foreach (auto &callAndLocations, extraArguments) {
        auto &callArguments = callAndLocations.second;
        callArguments.insert(callArguments.end(), arguments.begin(), arguments.end());
        callArguments = convention->sortArguments(std::move(callArguments));
        callArguments.erase(
            std::remove_if(callArguments.begin(), callArguments.end(),
                [&](const MemoryLocation &location) {
                    return !nc::contains(arguments, location);
                }),
            callArguments.end());
    }

    bool changed = false;

    auto &oldArguments = id2arguments_[calleeId];
    if (oldArguments != arguments) {
        oldArguments = std::move(arguments);
        changed = true;
    }

    foreach (auto &callAndLocations, extraArguments) {
        auto &oldExtraArguments = call2extraArguments_[callAndLocations.first];
        if (oldExtraArguments != callAndLocations.second) {
            oldExtraArguments = std::move(callAndLocations.second);
            changed = true;
        }
    }

    return changed;
}

bool SignatureAnalyzer::computeReturnValue(const CalleeId &calleeId) {
    assert(calleeId);

    const auto &referrers = nc::find(id2referrers_, calleeId);

    struct Placement {
        MemoryLocation location;
        std::size_t votes;

        Placement(): votes(0) {}
    };

    boost::unordered_map<const Term *, Placement> placements;

    foreach (auto call, referrers.calls) {
        foreach (const auto &termAndLocation, getUsedReturnValues(call)) {
            auto &placement = placements[termAndLocation.first];
            ++placement.votes;
            if (termAndLocation.first->asMemoryLocationAccess()) {
                placement.location = MemoryLocation::merge(placement.location, termAndLocation.second);
            }
        }
    }

    if (placements.empty()) {
        foreach (auto function, referrers.functions) {
            foreach (auto ret, nc::find(function2returns_, function)) {
                foreach (const auto &termAndLocation, getUnusedReturnValues(ret)) {
                    auto &placement = placements[termAndLocation.first];
                    ++placement.votes;
                    if (termAndLocation.first->asMemoryLocationAccess()) {
                        placement.location = MemoryLocation::merge(placement.location, termAndLocation.second);
                    }
                }
            }
        }
    }

    std::pair<const Term *, MemoryLocation> returnValue;

    if (!placements.empty()) {
        auto it = std::max_element(placements.begin(), placements.end(),
            [](const std::pair<const Term *, Placement> &a, const std::pair<const Term *, Placement> b){
                return a.second.votes < b.second.votes;
        });

        returnValue = std::make_pair(it->first, it->second.location);
    }

    auto &oldReturnValue = id2returnValue_[calleeId];
    if (oldReturnValue != returnValue) {
        oldReturnValue = returnValue;
        return true;
    } else {
        return false;
    }
}

namespace {

class StackOffsetFixer {
    boost::optional<BitSize> stackOffset_;

public:
    StackOffsetFixer(const Term *stackPointer, const dflow::Dataflow &dataflow) {
        if (stackPointer) {
            auto value = dataflow.getValue(stackPointer);
            if (value->isStackOffset()) {
                stackOffset_ = value->stackOffset() * CHAR_BIT;
            }
        }
    }

    MemoryLocation addStackOffset(const MemoryLocation &memoryLocation) {
        if (memoryLocation.domain() == MemoryDomain::STACK) {
            if (stackOffset_) {
                return memoryLocation.shifted(*stackOffset_);
            } else {
                return MemoryLocation();
            }
        } else {
            return memoryLocation;
        }
    }

    MemoryLocation removeStackOffset(const MemoryLocation &memoryLocation) {
        if (memoryLocation.domain() == MemoryDomain::STACK) {
            if (stackOffset_) {
                return memoryLocation.shifted(-*stackOffset_);
            } else {
                return MemoryLocation();
            }
        } else {
            return memoryLocation;
        }
    }
};

} // anonymous namespace

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

        if (term->isRead() && dataflow.getDefinitions(term).empty() && isRealRead(term)) {
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
        const auto &callArguments = nc::find(id2arguments_, getCalleeId(call, dataflow));
        if (callArguments.empty()) {
            continue;
        }

        auto callHook = hooks_.getCallHook(call);
        auto fixer = StackOffsetFixer(callHook->stackPointer(), dataflow);
        auto &reachingDefinitions = dataflow.getDefinitions(callHook->snapshotStatement());

        foreach (auto memoryLocation, callArguments) {
            memoryLocation = fixer.addStackOffset(memoryLocation);

            if (memoryLocation && reachingDefinitions.projected(memoryLocation).empty()) {
                result.push_back(memoryLocation);
            }
        }
    }

    return result;
}

std::vector<MemoryLocation> SignatureAnalyzer::getUnusedDefines(const Call *call) {
    assert(call != NULL);

    std::vector<MemoryLocation> result;

    auto callHook = hooks_.getCallHook(call);
    if (!callHook) {
        return result;
    }

    auto function = call->basicBlock()->function();
    auto &dataflow = *dataflows_.at(function);
    auto &uses = *function2uses_.at(function);
    auto fixer = StackOffsetFixer(callHook->stackPointer(), dataflow);

    foreach (const auto &chunk, dataflow.getDefinitions(callHook->snapshotStatement()).chunks()) {
        foreach (const Term *term, chunk.definitions()) {
            if (isRealWrite(term)) {
                bool used = false;
                foreach (const Term *use, uses.getUses(term)) {
                    if (isRealRead(use)) {
                        used = true;
                        break;
                    }
                }
                if (!used) {
                    if (auto memoryLocation = fixer.removeStackOffset(chunk.location())) {
                        result.push_back(memoryLocation);
                    }
                }
            }
        }
    }

    return result;
}

std::vector<std::pair<const Term *, MemoryLocation>> SignatureAnalyzer::getUsedReturnValues(const Call *call) {
    assert(call != NULL);

    std::vector<std::pair<const Term *, MemoryLocation>> result;

    auto callHook = hooks_.getCallHook(call);
    if (!callHook) {
        return result;
    }

    auto function = call->basicBlock()->function();
    auto &dataflow = *dataflows_.at(function);
    auto &uses = *function2uses_.at(function);

    foreach (const auto &termAndClone, callHook->returnValueTerms()) {
        MemoryLocation usedPart;

        foreach (const Term *use, uses.getUses(termAndClone.second)) {
            if (isRealRead(use)) {
                usedPart = MemoryLocation::merge(usedPart, dataflow.getMemoryLocation(use));
            }
        }

        if (usedPart && usedPart.addr() == dataflow.getMemoryLocation(termAndClone.second).addr()) {
            result.push_back(std::make_pair(termAndClone.first, usedPart));
        }
    }

    return result;
}

std::vector<std::pair<const Term *, MemoryLocation>> SignatureAnalyzer::getUnusedReturnValues(const Return *ret) {
    assert(ret != NULL);

    std::vector<std::pair<const Term *, MemoryLocation>> result;

    auto returnHook = hooks_.getReturnHook(ret);
    if (!returnHook) {
        return result;
    }

    auto function = ret->basicBlock()->function();
    auto &dataflow = *dataflows_.at(function);
    auto &uses = *function2uses_.at(function);

    foreach (const auto &termAndClone, returnHook->returnValueTerms()) {
        MemoryLocation unusedPart;

        foreach (const auto &chunk, dataflow.getDefinitions(termAndClone.second).chunks()) {
            bool used = false;
            bool defined = false;

            foreach (const Term *definition, chunk.definitions()) {
                if (isRealWrite(definition)) {
                    defined = true;

                    foreach (const Term *use, uses.getUses(definition)) {
                        if (use != termAndClone.second && isRealRead(use)) {
                            used = true;
                            break;
                        }
                    }
                }
            }

            if (defined && !used) {
                unusedPart = MemoryLocation::merge(unusedPart, chunk.location());
            }
        }

        if (unusedPart && unusedPart.addr() == dataflow.getMemoryLocation(termAndClone.second).addr()) {
            result.push_back(std::make_pair(termAndClone.first, unusedPart));
        }
    }

    return result;
}

void SignatureAnalyzer::computeSignatures() {
    foreach (const auto &calleeId, id2referrers_ | boost::adaptors::map_keys) {
        computeSignatures(calleeId);
        canceled_.poll();
    }
}

namespace {

class ArgumentFactory {
    MemoryLocation stackPointer_;
public:

    ArgumentFactory(const Convention *convention) {
        if (convention) {
            stackPointer_ = convention->stackPointer();
        }
    }

    std::shared_ptr<Term> operator()(const MemoryLocation &memoryLocation) const {
        if (memoryLocation.domain() == MemoryDomain::STACK) {
            if (stackPointer_) {
                return std::make_shared<Dereference>(
                    std::make_unique<BinaryOperator>(
                        BinaryOperator::ADD,
                        std::make_unique<MemoryLocationAccess>(stackPointer_),
                        std::make_unique<Constant>(SizedValue(
                            stackPointer_.size<SmallBitSize>(),
                            memoryLocation.addr() / CHAR_BIT)),
                        stackPointer_.size<SmallBitSize>()),
                    MemoryDomain::MEMORY,
                    memoryLocation.size<SmallBitSize>()
                );
            } else {
                return NULL;
            }
        } else {
            return std::make_shared<MemoryLocationAccess>(memoryLocation);
        }
    }
};

} // anonymous namespace

void SignatureAnalyzer::computeSignatures(const CalleeId &calleeId) {
    assert(calleeId);

    auto functionSignature = std::make_shared<FunctionSignature>();

    computeName(calleeId, *functionSignature);

    auto convention = hooks_.conventions().getConvention(calleeId);
    auto argumentFactory = ArgumentFactory(convention);

    foreach (const auto &memoryLocation, nc::find(id2arguments_, calleeId)) {
        if (auto term = argumentFactory(memoryLocation)) {
            functionSignature->arguments().push_back(term);
        }
    }

    auto &returnValue = nc::find(id2returnValue_, calleeId);
    if (returnValue.second) {
        functionSignature->setReturnValue(std::make_shared<MemoryLocationAccess>(returnValue.second));
    } else if (returnValue.first) {
        functionSignature->setReturnValue(returnValue.first->clone());
    }

    if (calleeId.entryAddress()) {
        signatures_.setSignature(*calleeId.entryAddress(), functionSignature);
    }

    const auto &referrers = nc::find(id2referrers_, calleeId);
    foreach (auto function, referrers.functions) {
        signatures_.setSignature(function, functionSignature);
    }

    foreach (auto call, referrers.calls) {
        auto callSignature = std::make_shared<CallSignature>();

        callSignature->arguments() = functionSignature->arguments();
        foreach (const auto &memoryLocation, nc::find(call2extraArguments_, call)) {
            if (auto term = argumentFactory(memoryLocation)) {
                callSignature->arguments().push_back(term);
                functionSignature->setVariadic();
            }
        }
        callSignature->setReturnValue(functionSignature->returnValue());

        signatures_.setSignature(call, callSignature);
    }
}

void SignatureAnalyzer::computeName(const CalleeId &calleeId, FunctionSignature &signature) {
    assert(calleeId);

    if (calleeId.entryAddress()) {
        /* Take the name of the corresponding symbol, if possible. */
        auto symbol = image_.getSymbol(*calleeId.entryAddress(), image::SymbolType::FUNCTION);
        if (!symbol) {
            symbol = image_.getSymbol(*calleeId.entryAddress(), image::SymbolType::NOTYPE);
        }
        if (symbol) {
            signature.setName(likec::Tree::cleanName(symbol->name()));

            if (signature.name() != symbol->name()) {
                signature.addComment(symbol->name());
            }

            QString demangledName = image_.demangler()->demangle(symbol->name());
            if (demangledName.contains('(')) {
                signature.addComment(demangledName);
            }
        }

        if (signature.name().isEmpty()) {
            /* Invent a name based on the entry address. */
            signature.setName(QString(QLatin1String("func_%1"))
                .arg(*calleeId.entryAddress(), 0, 16));
        }
    } else if (calleeId.function()) {
        signature.setName(QString(QLatin1String("func_noentry_%1"))
            .arg(reinterpret_cast<uintptr_t>(calleeId.function()), 0, 16));
    } else {
        /* Function is unknown, leave the name empty. */
    }
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
