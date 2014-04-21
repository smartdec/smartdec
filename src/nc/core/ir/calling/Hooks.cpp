/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

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

#include "Hooks.h"

#include <cassert>

#include <nc/common/Foreach.h>
#include <nc/common/Range.h>
#include <nc/common/make_unique.h>

#include <nc/core/arch/Instruction.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/Value.h>

#include "Conventions.h"
#include "CallHook.h"
#include "Convention.h"
#include "EntryHook.h"
#include "Signatures.h"
#include "ReturnHook.h"

namespace nc {
namespace core {
namespace ir {
namespace calling {

Hooks::Hooks(const Conventions &conventions, const Signatures &signatures):
    conventions_(conventions), signatures_(signatures)
{}

Hooks::~Hooks() {}

const Convention *Hooks::getConvention(const CalleeId &calleeId) const {
    if (!calleeId) {
        return NULL;
    }
    if (auto result = conventions_.getConvention(calleeId)) {
        return result;
    } else {
        conventionDetector_(calleeId);
        return conventions_.getConvention(calleeId);
    }
}

const EntryHook *Hooks::getEntryHook(const Function *function) const {
    assert(function != NULL);

    return nc::find(lastEntryHooks_, function);
}

const CallHook *Hooks::getCallHook(const Call *call) const {
    assert(call != NULL);

    return nc::find(lastCallHooks_, call);
}

const ReturnHook *Hooks::getReturnHook(const Return *ret) const {
    assert(ret != NULL);

    return nc::find(lastReturnHooks_, ret);
}

void Hooks::instrument(Function *function, const dflow::Dataflow *dataflow) {
    assert(function != NULL);
    assert(dataflow != NULL);

    auto &hooks = insertedHooks_[function];

    if (!hooks.empty()) {
        deinstrument(function);
    }

    hooks.push_back(function->entry()->pushFront(std::make_unique<Callback>([=](){
        instrumentEntry(function);
    })));

    foreach (auto basicBlock, function->basicBlocks()) {
        foreach (auto statement, basicBlock->statements()) {
            if (auto call = statement->as<Call>()) {
                hooks.push_back(basicBlock->insertAfter(call, std::make_unique<Callback>([=](){
                    instrumentCall(call, *dataflow);
                })));
            } else if (auto ret = statement->as<Return>()) {
                hooks.push_back(basicBlock->insertBefore(ret, std::make_unique<Callback>([=](){
                    instrumentReturn(ret);
                })));
            }
        }
    }
}

void Hooks::deinstrument(Function *function) {
    assert(function != NULL);

    auto &hooks = insertedHooks_[function];

    foreach (auto hook, hooks) {
        hook->basicBlock()->statements().erase(hook);
    }

    insertedHooks_.erase(function);

    foreach (auto basicBlock, function->basicBlocks()) {
        foreach (auto statement, basicBlock->statements()) {
            if (auto call = statement->as<Call>()) {
                deinstrumentCall(call);
            } else if (auto ret = statement->as<Return>()) {
                deinstrumentReturn(ret);
            }
        }
    }
}

void Hooks::deinstrumentAll() {
    /* deinstrument() can break iterators. */
    while (!insertedHooks_.empty()) {
        deinstrument(insertedHooks_.begin()->first);
    }
}

void Hooks::instrumentEntry(Function *function) {
    auto convention = getConvention(getCalleeId(function));
    auto signature = signatures_.getSignature(function).get();
    auto &entryHook = entryHooks_[std::make_tuple(function, convention, signature)];

    if (!entryHook) {
        entryHook = std::make_unique<EntryHook>(convention, signature);
    }

    auto &lastEntryHook = lastEntryHooks_[function];

    if (entryHook.get() != lastEntryHook) {
        if (lastEntryHook) {
            lastEntryHook->deinstrument(function);
        }
        if (entryHook) {
            entryHook->instrument(function);
        }
        lastEntryHook = entryHook.get();
    }
}

void Hooks::deinstrumentEntry(Function *function) {
    auto &lastEntryHook = lastEntryHooks_[function];

    if (lastEntryHook) {
        lastEntryHook->deinstrument(function);
        lastEntryHook = NULL;
    }
}

void Hooks::instrumentCall(Call *call, const dflow::Dataflow &dataflow) {
    auto calleeId = getCalleeId(call, dataflow);
    auto convention = getConvention(calleeId);
    auto signature = signatures_.getSignature(call).get();
    auto stackArgumentsSize = conventions_.getStackArgumentsSize(calleeId);
    auto &callHook = callHooks_[std::make_tuple(call, convention, signature, stackArgumentsSize)];

    if (!callHook) {
        callHook = std::make_unique<CallHook>(convention, signature, stackArgumentsSize);
    }

    auto &lastCallHook = lastCallHooks_[call];

    if (callHook.get() != lastCallHook) {
        if (lastCallHook) {
            lastCallHook->deinstrument(call);
        }
        if (callHook) {
            callHook->instrument(call);
        }
        lastCallHook = callHook.get();
    }
}

void Hooks::deinstrumentCall(Call *call) {
    auto &lastCallHook = lastCallHooks_[call];

    if (lastCallHook) {
        lastCallHook->deinstrument(call);
        lastCallHook = NULL;
    }
}

void Hooks::instrumentReturn(Return *ret) {
    auto function = ret->basicBlock()->function();
    auto convention = getConvention(getCalleeId(function));
    auto signature = signatures_.getSignature(function).get();
    auto &returnHook = returnHooks_[std::make_tuple(ret, convention, signature)];

    if (!returnHook) {
        returnHook = std::make_unique<ReturnHook>(convention, signature);
    }

    auto &lastReturnHook = lastReturnHooks_[ret];

    if (returnHook.get() != lastReturnHook) {
        if (lastReturnHook) {
            lastReturnHook->deinstrument(ret);
        }
        if (returnHook) {
            returnHook->instrument(ret);
        }
        lastReturnHook = returnHook.get();
    }
}

void Hooks::deinstrumentReturn(Return *ret) {
    auto &lastReturnHook = lastReturnHooks_[ret];

    if (lastReturnHook) {
        lastReturnHook->deinstrument(ret);
        lastReturnHook = NULL;
    }
}

CalleeId getCalleeId(const Function *function) {
    assert(function != NULL);

    if (function->entry() && function->entry()->address()) {
        return CalleeId(*function->entry()->address(), CalleeId::entryAddr);
    } else {
        return CalleeId(function);
    }
}

CalleeId getCalleeId(const Call *call, const dflow::Dataflow &dataflow) {
    assert(call != NULL);

    auto targetValue = dataflow.getValue(call->target());
    if (targetValue->abstractValue().isConcrete()) {
        return CalleeId(targetValue->abstractValue().asConcrete().value(), CalleeId::entryAddr);
    } else if (call->instruction()) {
        return CalleeId(call->instruction()->addr(), CalleeId::callAddr);
    } else {
        return CalleeId();
    }
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
