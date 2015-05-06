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

#include "VariableAnalyzer.h"

#include <nc/common/Foreach.h>

#include <nc/core/ir/Term.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/misc/CensusVisitor.h>

#include "Variables.h"

namespace nc {
namespace core {
namespace ir {
namespace vars {

void VariableAnalyzer::analyze(const Function *function) {
    ir::misc::CensusVisitor census(callsData());
    census(function);

    foreach (const Term *term, census.terms()) {
        if (term->isRead()) {
            Variable *variable = variables().getVariable(term);
            
            foreach (const Term *definition, dataflow().getDefinitions(term)) {
                assert(dataflow().getMemoryLocation(term) == dataflow().getMemoryLocation(definition));
                variable->unionSet(variables().getVariable(definition));
            }
        }
    }
}

} // namespace vars
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
