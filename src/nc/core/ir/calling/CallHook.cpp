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

#include "CallHook.h"

#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/MemoryDomain.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>

#include "CallSignature.h"
#include "Convention.h"

namespace nc {
namespace core {
namespace ir {
namespace calling {

CallHook::CallHook(const Convention *convention, const CallSignature *signature,
    const boost::optional<ByteSize> &stackArgumentsSize):
    stackPointer_(NULL), snapshotStatement_(NULL), insertedStatementsCount_(0)
{
    assert(convention != NULL);

    if (convention->stackPointer()) {
        auto stackPointer = std::make_unique<MemoryLocationAccess>(convention->stackPointer());
        stackPointer_ = stackPointer.get();

        statements_.push_back(std::make_unique<Touch>(
            std::move(stackPointer),
            Term::READ
        ));
    }

    auto createArgument = [&](const Term *term) {
        auto clone = term->clone();
        argumentTerms_[term] = clone.get();

        statements_.push_back(std::make_unique<Touch>(
            std::move(clone),
            Term::READ
        ));
    };

    auto createReturnValue = [&](const Term *term) {
        auto clone = term->clone();
        returnValueTerms_[term] = clone.get();

        statements_.push_back(std::make_unique<Assignment>(
            std::move(clone),
            std::make_unique<Intrinsic>(Intrinsic::UNDEFINED, term->size())
        ));
    };

    if (signature) {
        foreach (const auto &term, signature->arguments()) {
            createArgument(term.get());
        }

        if (signature->returnValue()) {
            createReturnValue(signature->returnValue().get());
        }
    } else {
        auto snapshotStatement = std::make_unique<RememberReachingDefinitions>();
        snapshotStatement_ = snapshotStatement.get();
        statements_.push_back(std::move(snapshotStatement));

        foreach (auto term, convention->returnValueTerms()) {
            createReturnValue(term);
        }
    }

    if (convention->calleeCleanup() && stackArgumentsSize) {
        assert(convention->stackPointer());

        statements_.push_back(std::make_unique<Assignment>(
            std::make_unique<MemoryLocationAccess>(convention->stackPointer()),
            std::make_unique<BinaryOperator>(
                BinaryOperator::ADD,
                std::make_unique<MemoryLocationAccess>(convention->stackPointer()),
                std::make_unique<Constant>(SizedValue(convention->stackPointer().size(), *stackArgumentsSize)),
                convention->stackPointer().size<SmallBitSize>())));
    }
}

CallHook::~CallHook() {}

void CallHook::instrument(Call *call) {
    while (!statements_.empty()) {
        call->basicBlock()->insertAfter(call, statements_.pop_back());
        ++insertedStatementsCount_;
    }
}

void CallHook::deinstrument(Call *call) {
    while (insertedStatementsCount_ > 0) {
        statements_.push_back(call->basicBlock()->erase(*++call->basicBlock()->statements().get_iterator(call)));
        --insertedStatementsCount_;
    }
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
