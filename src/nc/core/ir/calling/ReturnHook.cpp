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

#include "ReturnHook.h"

#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>

#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include "Convention.h"
#include "Signature.h"

namespace nc {
namespace core {
namespace ir {
namespace calling {

ReturnHook::ReturnHook(const Convention *convention, const Signature *signature):
    insertedStatementsCount_(0)
{
    assert(convention != NULL);

    auto createReturnValue = [&](const Term *term) {
        auto clone = term->clone();
        returnValueTerms_[term] = clone.get();
        statements_.push_back(std::make_unique<Touch>(
            std::move(clone),
            Term::READ
        ));
    };

    if (signature) {
        if (signature->returnValue()) {
            createReturnValue(signature->returnValue());
        }
    } else {
        foreach (auto term, convention->returnValueTerms()) {
            createReturnValue(term);
        }
    }
}

ReturnHook::~ReturnHook() {}

void ReturnHook::instrument(Return *ret) {
    while (!statements_.empty()) {
        ret->basicBlock()->insertAfter(ret, statements_.pop_back());
        ++insertedStatementsCount_;
    }
}

void ReturnHook::deinstrument(Return *ret) {
    while (insertedStatementsCount_ > 0) {
        statements_.push_back(ret->basicBlock()->statements().erase(++ret->basicBlock()->statements().get_iterator(ret)));
        --insertedStatementsCount_;
    }
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
