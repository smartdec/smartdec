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

#include "Conventions.h"
#include "DescriptorAnalyzer.h"
#include "CallHook.h"
#include "Convention.h"
#include "EntryHook.h"
#include "Signatures.h"
#include "ReturnHook.h"
#include "GenericDescriptorAnalyzer.h"

namespace nc {
namespace core {
namespace ir {
namespace calling {

Hooks::Hooks(const Conventions &conventions, const Signatures &signatures):
    conventions_(conventions), signatures_(signatures)
{}

Hooks::~Hooks() {}

CalleeId Hooks::getCalleeId(const Function *function) const {
    assert(function != NULL);

    if (function->entry() && function->entry()->address()) {
        return CalleeId(CalleeId::ENTRY_ADDRESS, *function->entry()->address());
    } else {
        return CalleeId();
    }
}

CalleeId Hooks::getCalleeId(const Call *call) const {
    assert(call != NULL);

    if (auto addr = getCalledAddress(call)) {
        return CalleeId(CalleeId::ENTRY_ADDRESS, *addr);
    } else if (call->instruction()) {
        return CalleeId(CalleeId::CALL_ADDRESS, call->instruction()->addr());
    } else {
        return CalleeId();
    }
}

boost::optional<ByteAddr> Hooks::getCalledAddress(const Call *call) const {
    assert(call != NULL);

    return nc::find_optional(call2address_, call);
}

void Hooks::setCalledAddress(const Call *call, ByteAddr addr) {
    assert(call != NULL);

    call2address_[call] = addr;
}

const Convention *Hooks::getConvention(const CalleeId &calleeId) {
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

DescriptorAnalyzer *Hooks::getDescriptorAnalyzer(const CalleeId &calleeId) {
    if (!calleeId) {
        return NULL;
    }
    if (!nc::contains(id2analyzer_, calleeId)) {
        if (auto convention = getConvention(calleeId)) {
            id2analyzer_[calleeId] = std::make_unique<GenericDescriptorAnalyzer>(convention);
        }
    }
    return nc::find(id2analyzer_, calleeId).get();
}

EntryHook *Hooks::getEntryHook(const Function *function) {
    assert(function != NULL);

    auto calleeId = getCalleeId(function);

    if (!calleeId) {
        return NULL;
    }

    auto key = std::make_pair(calleeId, function);

    if (auto result = nc::find(entryHooks_, key).get()) {
        return result;
    }
    if (auto convention = getConvention(calleeId)) {
        return (entryHooks_[key] = std::make_unique<EntryHook>(convention, signatures_.getSignature(calleeId))).get();
    }
    return NULL;
}

ReturnHook *Hooks::getReturnHook(const Function *function, const Return *ret) {
    assert(ret != NULL);

    auto calleeId = getCalleeId(function);
    if (!calleeId) {
        return NULL;
    }

    auto key = std::make_pair(calleeId, ret);

    if (auto result = nc::find(returnHooks_, key).get()) {
        return result;
    }
    if (auto convention = getConvention(calleeId)) {
        return (returnHooks_[key] = std::make_unique<ReturnHook>(convention, signatures_.getSignature(calleeId))).get();
    }
    return NULL;
}

CallHook *Hooks::getCallHook(const Call *call) {
    assert(call != NULL);

    auto calleeId = getCalleeId(call);
    if (!calleeId) {
        return NULL;
    }

    auto key = std::make_pair(calleeId, call);
    if (!nc::contains(call2analyzer_, key)) {
        if (DescriptorAnalyzer *descriptorAnalyzer = getDescriptorAnalyzer(calleeId)) {
            call2analyzer_[key] = descriptorAnalyzer->createCallHook(call);
        }
    }
    return nc::find(call2analyzer_, key).get();
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
