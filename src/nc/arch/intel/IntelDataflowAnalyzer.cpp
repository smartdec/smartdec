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

#include "IntelDataflowAnalyzer.h"

#include <nc/core/arch/Register.h>

#include <nc/core/ir/MemoryDomain.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/SimulationContext.h>

#include "IntelRegisters.h"

namespace nc {
namespace arch {
namespace intel {

void IntelDataflowAnalyzer::simulate(const core::ir::Term *term, core::ir::dflow::SimulationContext &context) {
    /* Do everything as usual. */
    DataflowAnalyzer::simulate(term, context);

    /* but... */

    /* If two values of fpu top register come here, force fpu top to zero. */
    if (const core::ir::MemoryLocationAccess *access = term->asMemoryLocationAccess()) {
        if (access->isRead()) {
            if (access->memoryLocation() == IntelRegisters::fpu_top()->memoryLocation()) {
                core::ir::dflow::Value *value = dataflow().getValue(term);
                if (!value->isConstant()) {
                    value->forceConstant(0);
                }
            }
        }
    }
}

} // namespace intel
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
