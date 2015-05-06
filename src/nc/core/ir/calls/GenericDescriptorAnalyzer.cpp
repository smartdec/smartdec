//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "GenericDescriptorAnalyzer.h"

#include <algorithm>

#include <nc/common/Foreach.h>
#include <nc/common/Range.h> /* nc::find */

#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Statement.h>

#include "FunctionSignature.h"
#include "GenericCallingConvention.h"
#include "GenericCallAnalyzer.h"
#include "GenericFunctionAnalyzer.h"
#include "GenericReturnAnalyzer.h"

namespace nc {
namespace core {
namespace ir {
namespace calls {

std::unique_ptr<CallAnalyzer> GenericDescriptorAnalyzer::createCallAnalyzer(const Call *call) {
    std::unique_ptr<GenericCallAnalyzer> result(new GenericCallAnalyzer(call, this));
    callAnalyzers_.push_back(result.get());
    return std::move(result);
}

std::unique_ptr<FunctionAnalyzer> GenericDescriptorAnalyzer::createFunctionAnalyzer(const Function *function) {
    std::unique_ptr<GenericFunctionAnalyzer> result(new GenericFunctionAnalyzer(function, this));
    functionAnalyzers_.push_back(result.get());
    return std::move(result);
}

std::unique_ptr<ReturnAnalyzer> GenericDescriptorAnalyzer::createReturnAnalyzer(const Return *ret) {
    std::unique_ptr<GenericReturnAnalyzer> result(new GenericReturnAnalyzer(ret, this));
    returnAnalyzers_.push_back(result.get());
    return std::move(result);
}

namespace {

/**
 * \param[in] function Valid pointer to a function.
 *
 * \return True if the function looks like a usual function, i.e. not a thunk or similar.
 */
bool isUsualFunction(const Function *function) {
    foreach (const BasicBlock *basicBlock, function->basicBlocks()) {
        if (basicBlock->getReturn()) {
            return true;
        }
    }
    return false;
}

} // anonymous namespace

FunctionSignature GenericDescriptorAnalyzer::getFunctionSignature() const {
    FunctionSignature signature;

    struct Counts {
        std::size_t defs;
        std::size_t uses;

        Counts(): defs(0), uses(0) {}
    };

    /*
     * Estimate the memory locations of arguments.
     */
    boost::unordered_map<MemoryLocation, Counts> argVotes;

    std::size_t callsCount = callAnalyzers_.size();
    std::size_t functionsCount = 0;

    foreach (GenericCallAnalyzer *callAnalyzer, callAnalyzers_) {
        foreach (const MemoryLocation &memoryLocation, callAnalyzer->argumentLocations()) {
            ++argVotes[memoryLocation].defs;
        }
    }

    foreach (GenericFunctionAnalyzer *functionAnalyzer, functionAnalyzers_) {
        if (isUsualFunction(functionAnalyzer->function())) {
            ++functionsCount;
        }
        foreach (const MemoryLocation &memoryLocation, functionAnalyzer->argumentLocations()) {
            ++argVotes[memoryLocation].uses;
        }
    }

    auto isRealArgument = [&](const MemoryLocation &memoryLocation) -> bool {
        const auto &counts = nc::find(argVotes, memoryLocation);

        if (functionsCount > 0) {
            return counts.uses > 0;
        } else {
            return counts.defs > callsCount / 2;
        }
    };

    /* Add register arguments. */
    bool considerStack = convention()->argumentGroups().empty();
    foreach (const ArgumentGroup &group, convention()->argumentGroups()) {
        bool groupIsFull = true;
        foreach (const Argument &argument, group.arguments()) {
            bool argumentIsUsed = false;
            foreach (const MemoryLocation &memoryLocation, argument.locations()) {
                if (isRealArgument(memoryLocation)) {
                    signature.addArgument(memoryLocation);
                    argumentIsUsed = true;
                }
            }
            if (!argumentIsUsed) {
                groupIsFull = false;
                break;
            }
        }
        if (groupIsFull) {
            considerStack = true;
        }
    }

    if (considerStack) {
        std::size_t firstStackArgument = signature.arguments().size();

        /* Add stack arguments. */
        foreach (const auto &pair, argVotes) {
            if (pair.first.domain() == MemoryDomain::STACK && isRealArgument(pair.first)) {
                signature.addArgument(pair.first);
            }
        }

        /* Sort stack arguments. */
        std::sort(signature.arguments().begin() + firstStackArgument, signature.arguments().end(),
                  [](const MemoryLocation &a, const MemoryLocation &b) { return a.addr() < b.addr(); });
    }

    /*
     * Estimate the term for returning a value.
     */
    boost::unordered_map<const Term *, int> retVotes;

    foreach (GenericCallAnalyzer *callAnalyzer, callAnalyzers_) {
        foreach (const Term *term, callAnalyzer->returnValueLocations()) {
            ++retVotes[term];
        }
    }
    foreach (GenericReturnAnalyzer *returnAnalyzer, returnAnalyzers_) {
        foreach (const Term *term, returnAnalyzer->returnValueLocations()) {
            ++retVotes[term];
        }
    }
    foreach (const Term *term, convention()->returnValues()) {
        if (find(retVotes, signature.returnValue()) < find(retVotes, term)) {
            signature.setReturnValue(term);
        }
    }

    return signature;
}

} // namespace calls
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
