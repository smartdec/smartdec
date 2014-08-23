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

#include "TypeAnalyzer.h"

#include <nc/common/CancellationToken.h>
#include <nc/common/Foreach.h>

#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Functions.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/calling/CallHook.h>
#include <nc/core/ir/calling/EntryHook.h>
#include <nc/core/ir/calling/Hooks.h>
#include <nc/core/ir/calling/ReturnHook.h>
#include <nc/core/ir/calling/Signatures.h>
#include <nc/core/ir/dflow/Dataflows.h>
#include <nc/core/ir/liveness/Livenesses.h>
#include <nc/core/ir/vars/Variables.h>

#include "FunctionAnalyzer.h"
#include "Type.h"
#include "Types.h"

namespace nc {
namespace core {
namespace ir {
namespace types {

void TypeAnalyzer::analyze(const CancellationToken &canceled) {
    joinVariableTypes();
    joinArgumentTypes();

    std::vector<FunctionAnalyzer> analyzers;
    analyzers.reserve(functions_.list().size());

    foreach (auto function, functions_.list()) {
        analyzers.push_back(FunctionAnalyzer(types_, *dataflows_.at(function), *livenesses_.at(function)));
    }

    /*
     * Recompute types until reaching fixpoint.
     */
    bool changed;
    do {
        changed = false;

        foreach (auto &analyzer, analyzers) {
            while (analyzer.analyze()) {
                changed = true;
                canceled.poll();
            }
            canceled.poll();
        }
    } while (changed);
}

namespace {

void uniteTypes(Type *&a, Type *b) {
    if (a == NULL) {
        a = b;
    } else {
        a->unionSet(b);
    }
};

} // anonymous namespace

void TypeAnalyzer::joinVariableTypes() {
    foreach (const vars::Variable *variable, variables_.list()) {
        boost::unordered_map<MemoryLocation, Type *> location2type;

        foreach (const auto &termAndLocation, variable->termsAndLocations()) {
            uniteTypes(location2type[termAndLocation.location], types_.getType(termAndLocation.term));
        }
    }
}

void TypeAnalyzer::joinArgumentTypes() {
    foreach (auto function, functions_.list()) {
        if (auto entryHook = hooks_.getEntryHook(function)) {
            if (auto signature = signatures_.getSignature(function)) {
                foreach (const auto &term, signature->arguments()) {
                    types_.getType(term.get())->unionSet(types_.getType(entryHook->getArgumentTerm(term.get())));
                }
            }
        }

        foreach (auto basicBlock, function->basicBlocks()) {
            foreach (auto statement, basicBlock->statements()) {
                if (auto call = statement->asCall()) {
                    if (auto callHook = hooks_.getCallHook(call)) {
                        if (auto signature = signatures_.getSignature(call)) {
                            foreach (const auto &term, signature->arguments()) {
                                types_.getType(term.get())->unionSet(types_.getType(callHook->getArgumentTerm(term.get())));
                            }
                            if (auto returnValue = signature->returnValue().get()) {
                                types_.getType(returnValue)->unionSet(types_.getType(callHook->getReturnValueTerm(returnValue)));
                            }
                        }
                    }
                } else if (auto ret = statement->asReturn()) {
                    if (auto returnHook = hooks_.getReturnHook(ret)) {
                        if (auto signature = signatures_.getSignature(function)) {
                            if (auto returnValue = signature->returnValue().get()) {
                                types_.getType(returnValue)->unionSet(types_.getType(returnHook->getReturnValueTerm(returnValue)));
                            }
                        }
                    }
                }
            }
        }
    }
}

}}}} // namespace nc::core::ir::types

/* vim:set et sts=4 sw=4: */
