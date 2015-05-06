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

#include "CallsData.h"

#include <cassert>

#include <nc/common/Foreach.h>
#include <nc/common/Range.h>

#include <nc/core/arch/Instruction.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Statements.h>

#include "DescriptorAnalyzer.h"
#include "CallAnalyzer.h"
#include "CallingConvention.h"
#include "CallingConventionDetector.h"
#include "FunctionAnalyzer.h"
#include "FunctionSignature.h"
#include "ReturnAnalyzer.h"

namespace nc {
namespace core {
namespace ir {
namespace calls {

CallsData::CallsData(): callingConventionDetector_(NULL) {}

CallsData::~CallsData() {}

FunctionDescriptor CallsData::getDescriptor(const Function *function) const {
    assert(function != NULL);

    if (function->entry() && function->entry()->address()) {
        return FunctionDescriptor(FunctionDescriptor::ENTRY_ADDRESS, *function->entry()->address());
    } else {
        return FunctionDescriptor();
    }
}

FunctionDescriptor CallsData::getDescriptor(const Call *call) const {
    assert(call != NULL);

    if (auto addr = getCalledAddress(call)) {
        return FunctionDescriptor(FunctionDescriptor::ENTRY_ADDRESS, *addr);
    } else if (call->instruction()) {
        return FunctionDescriptor(FunctionDescriptor::CALL_ADDRESS, call->instruction()->addr());
    } else {
        return FunctionDescriptor();
    }
}

boost::optional<ByteAddr> CallsData::getCalledAddress(const Call *call) const {
    assert(call != NULL);

    return nc::find_optional(call2address_, call);
}

void CallsData::setCalledAddress(const Call *call, ByteAddr addr) {
    assert(call != NULL);

    call2address_[call] = addr;
}

void CallsData::setCallingConvention(const FunctionDescriptor &descriptor, const CallingConvention *convention) {
    assert(nc::find(descriptor2convention_, descriptor) == NULL && "Calling convention cannot be reset.");

    descriptor2convention_[descriptor] = convention;
}

const CallingConvention *CallsData::getCallingConvention(const FunctionDescriptor &descriptor) {
    if (!descriptor) {
        return NULL;
    }
    if (!nc::contains(descriptor2convention_, descriptor)) {
        if (callingConventionDetector()) {
            callingConventionDetector()->detectCallingConvention(descriptor);
        }
    }
    return nc::find(descriptor2convention_, descriptor);
}

DescriptorAnalyzer *CallsData::getDescriptorAnalyzer(const FunctionDescriptor &descriptor) {
    if (!descriptor) {
        return NULL;
    }
    if (!nc::contains(descriptor2analyzer_, descriptor)) {
        if (const CallingConvention *callingConvention = getCallingConvention(descriptor)) {
            descriptor2analyzer_[descriptor] = callingConvention->createDescriptorAnalyzer();
        }
    }
    return nc::find(descriptor2analyzer_, descriptor).get();
}

FunctionAnalyzer *CallsData::getFunctionAnalyzer(const Function *function) {
    assert(function != NULL);

    FunctionDescriptor descriptor = getDescriptor(function);
    if (!descriptor) {
        return NULL;
    }

    auto key = std::make_pair(descriptor, function);
    if (!nc::contains(function2analyzer_, key)) {
        if (DescriptorAnalyzer *descriptorAnalyzer = getDescriptorAnalyzer(descriptor)) {
            function2analyzer_[key] = descriptorAnalyzer->createFunctionAnalyzer(function);
        }
    }
    return nc::find(function2analyzer_, key).get();
}

CallAnalyzer *CallsData::getCallAnalyzer(const Call *call) {
    assert(call != NULL);

    FunctionDescriptor descriptor = getDescriptor(call);
    if (!descriptor) {
        return NULL;
    }

    auto key = std::make_pair(descriptor, call);
    if (!nc::contains(call2analyzer_, key)) {
        if (DescriptorAnalyzer *descriptorAnalyzer = getDescriptorAnalyzer(descriptor)) {
            call2analyzer_[key] = descriptorAnalyzer->createCallAnalyzer(call);
        }
    }
    return nc::find(call2analyzer_, key).get();
}

ReturnAnalyzer *CallsData::getReturnAnalyzer(const Function *function, const Return *ret) {
    assert(ret != NULL);

    FunctionDescriptor descriptor = getDescriptor(function);
    if (!descriptor) {
        return NULL;
    }

    auto key = std::make_pair(descriptor, ret);
    if (!nc::contains(return2analyzer_, key)) {
        if (DescriptorAnalyzer *addressAnalyzer = getDescriptorAnalyzer(descriptor)) {
            return2analyzer_[key] = addressAnalyzer->createReturnAnalyzer(ret);
        }
    }
    return nc::find(return2analyzer_, key).get();
}

const FunctionSignature *CallsData::getFunctionSignature(const FunctionDescriptor &descriptor) {
    if (!descriptor) {
        return NULL;
    }
    if (!nc::contains(descriptor2signature_, descriptor)) {
        if (DescriptorAnalyzer *analyzer = getDescriptorAnalyzer(descriptor)) {
            descriptor2signature_[descriptor].reset(new FunctionSignature(analyzer->getFunctionSignature()));
        }
    }
    return nc::find(descriptor2signature_, descriptor).get();
}

const FunctionSignature *CallsData::getFunctionSignature(const Function *function) {
    return getFunctionSignature(getDescriptor(function));
}

const FunctionSignature *CallsData::getFunctionSignature(const Call *call) {
    return getFunctionSignature(getDescriptor(call));
}

std::vector<const Return *> CallsData::getReturns(const Function *function) const {
    // TODO: not to recompute this every time. Move this function somewhere else, e.g. to ir::Functions?

    assert(function);

    std::vector<const Return *> result;

    foreach (const BasicBlock *basicBlock, function->basicBlocks()) {
        foreach (const Statement *statement, basicBlock->statements()) {
            if (const Return *ret = statement->as<Return>()) {
                result.push_back(ret);
            }
        }
    }

    return result;
}

} // namespace calls
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
