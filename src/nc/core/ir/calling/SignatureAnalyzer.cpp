/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "SignatureAnalyzer.h"

#include <algorithm>
#include <cstdint> /* uintptr_t */
#include <iterator> /* std::back_inserter */

#include <boost/range/adaptor/map.hpp>

#include <nc/common/CancellationToken.h>
#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/core/image/Image.h>
#include <nc/core/image/Symbols.h>
#include <nc/core/ir/Functions.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
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

namespace {

void mergeOverlapping(std::vector<MemoryLocation> &locations) {
    if (locations.empty()) {
        return;
    }

    auto begin = locations.begin();
    auto end = locations.end();

    std::sort(begin, end);

    for (auto i = std::next(begin); i != end; ++i) {
        if (begin->overlaps(*i)) {
            *begin = MemoryLocation::merge(*begin, *i);
        } else {
            *++begin = *i;
        }
    }

    locations.erase(std::next(begin), end);
}

} // anonymous namespace

void SignatureAnalyzer::analyze(const CancellationToken &canceled) {
    computeArguments(canceled);
    sortArguments();
    computeSignatures(canceled);
}

void SignatureAnalyzer::computeArguments(const CancellationToken &canceled) {
    do {
        changed_ = false;

        foreach (auto function, functions_.all()) {
            computeArguments(function);
            canceled.poll();
        }
    } while (changed_);
}

void SignatureAnalyzer::computeArguments(const Function *function) {
    auto calleeId = hooks_.getCalleeId(function);
    assert(calleeId);

    auto convention = hooks_.conventions().getConvention(calleeId);
    if (!convention) {
        return;
    }

    auto &dataflow = *dataflows_.at(function);

    misc::CensusVisitor visitor(&hooks_);
    visitor(function);

    std::vector<MemoryLocation> arguments;

    foreach (auto term, visitor.terms()) {
        if (term->isRead() && dataflow.getDefinitions(term).empty()) {
            if (const auto &memoryLocation = dataflow.getMemoryLocation(term)) {
                if (convention->isArgumentLocation(memoryLocation)) {
                    arguments.push_back(memoryLocation);
                }
            }
        }
    }

    addArguments(calleeId, arguments);
}

void SignatureAnalyzer::addArguments(const CalleeId &calleeId, std::vector<MemoryLocation> arguments) {
    assert(calleeId);

    auto &oldArguments = id2arguments_[calleeId];
    arguments.insert(arguments.end(), oldArguments.begin(), oldArguments.end());
    mergeOverlapping(arguments);

    if (arguments != id2arguments_[calleeId]) {
        id2arguments_[calleeId] = std::move(arguments);
        changed_ = true;
    }
}

void SignatureAnalyzer::sortArguments() {
    foreach (auto &idAndArguments, id2arguments_) {
        const auto &id = idAndArguments.first;
        auto &arguments = idAndArguments.second;

        auto convention = hooks_.conventions().getConvention(id);
        if (!convention) {
            continue;
        }

        std::vector<MemoryLocation> newArguments;
        newArguments.reserve(arguments.size());

        foreach (const auto &group, convention->argumentGroups()) {
            foreach (const auto &argument, group.arguments()) {
                auto predicate = [&argument](const MemoryLocation &location) { return argument.location().covers(location); };

                std::copy_if(arguments.begin(), arguments.end(), std::back_inserter(newArguments), predicate);
                arguments.erase(std::remove_if(arguments.begin(), arguments.end(), predicate), arguments.end());
            }
        }

        newArguments.insert(newArguments.end(), arguments.begin(), arguments.end());
        arguments = std::move(newArguments);
    }
}

void SignatureAnalyzer::computeSignatures(const CancellationToken &canceled) {
    foreach (auto function, functions_.all()) {
        computeSignature(hooks_.getCalleeId(function));
        canceled.poll();
    }

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
