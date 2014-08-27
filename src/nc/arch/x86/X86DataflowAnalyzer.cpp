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

#include "X86DataflowAnalyzer.h"

#include <nc/core/arch/Register.h>

#include <nc/core/ir/MemoryDomain.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/Value.h>

#include "X86Registers.h"

namespace nc {
namespace arch {
namespace x86 {

core::ir::dflow::Value *X86DataflowAnalyzer::computeValue(const core::ir::Term *term, const core::ir::dflow::ExecutionContext &context) {
    /* Do everything as usual. */
    auto value = DataflowAnalyzer::computeValue(term, context);

    /* But, if two values of fpu top register come here, force fpu top to zero. */
    if (const core::ir::MemoryLocationAccess *access = term->asMemoryLocationAccess()) {
        if (access->isRead()) {
            if (access->memoryLocation() == X86Registers::fpu_top()->memoryLocation()) {
                if (!value->abstractValue().isConcrete()) {
                    value->setAbstractValue(SizedValue(access->size(), 0));
                }
            }
        }
    }

    return value;
}

} // namespace x86
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
