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

#include "CallInliner.h"

#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/find.hpp>

#include <nc/common/Foreach.h>
#include <nc/common/Range.h>
#include <nc/common/make_unique.h>

#include <nc/core/arch/Instruction.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/FunctionsGenerator.h>
#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Statements.h>

namespace nc { namespace core { namespace ir { namespace inlining {

void CallInliner::perform(Function *receivingFunction, const Call *call, const Function *inlinedFunction) const {
    /* Split call->basicBlock(). */
    BasicBlock *leadIn = call->basicBlock();
    auto leadOut = leadIn->split(boost::find(leadIn->statements(), call) - leadIn->statements().begin() + 1, boost::none);

    /* Clone the basic blocks of inlined function. */
    auto clones = FunctionsGenerator::cloneIntoFunction(inlinedFunction->basicBlocks(), receivingFunction);

    /* Replace the call by the jump to the cloned entry. */
    leadIn->popBack();
    leadIn->addStatement(std::make_unique<Jump>(JumpTarget(nc::find(clones, inlinedFunction->entry()))));

    /* Replace returns by the jumps to 'lead out'. */
    foreach (BasicBlock *basicBlock, clones | boost::adaptors::map_values) {
        if (basicBlock->getReturn()) {
            basicBlock->popBack();
            basicBlock->addStatement(std::make_unique<Jump>(leadOut.get()));
        }
    }

    receivingFunction->addBasicBlock(std::move(leadOut));
}

}}}} // namespace nc::core::ir::inlining

/* vim:set et sts=4 sw=4: */
