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

#include "IRGenerator.h"

#include <nc/core/Module.h>
#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/Instructions.h>
#include <nc/core/image/Image.h>

#include <nc/common/Foreach.h>
#include <nc/common/Range.h>
#include <nc/common/Warnings.h>
#include <nc/common/make_unique.h>

#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Program.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/DataflowAnalyzer.h>
#include <nc/core/ir/dflow/SimulationContext.h>
#include <nc/core/ir/misc/ArrayAccess.h>
#include <nc/core/ir/misc/PatternRecognition.h>

#include "InstructionAnalyzer.h"
#include "InvalidInstructionException.h"

namespace nc {
namespace core {
namespace arch {
namespace irgen {

void IRGenerator::generate(const CancellationToken &canceled) {
    /* Generate statements. */
    foreach (const auto &instr, instructions()->all()) {
        if (canceled) {
            return;
        }
        try {
            module()->architecture()->instructionAnalyzer()->createStatements(instr.get(), program());
        } catch (const InvalidInstructionException &e) {
            /* Note: this is an AntiIdiom: http://c2.com/cgi/wiki?LoggingDiscussion */
            ncWarning(e.unicodeWhat());
        }
    }

    /* Compute jump targets. */
    for (std::size_t i = 0; i < program()->basicBlocks().size(); ++i) {
        if (canceled) {
            return;
        }
        computeJumpTargets(program()->basicBlocks()[i]);
    }

    /* Add jumps to direct successors where necessary. */
    for (std::size_t i = 0; i < program()->basicBlocks().size(); ++i) {
        if (canceled) {
            return;
        }
        addJumpToDirectSuccessor(program()->basicBlocks()[i]);
    }
}

void IRGenerator::computeJumpTargets(ir::BasicBlock *basicBlock) {
    assert(basicBlock != NULL);

    /* Prepare context for quick and dirty dataflow analysis. */
    ir::dflow::Dataflow dataflow;
    ir::dflow::DataflowAnalyzer analyzer(dataflow, module()->architecture(), NULL);
    ir::dflow::SimulationContext context(analyzer);

    for (std::size_t i = 0; i < basicBlock->statements().size(); ++i) {
        ir::Statement *statement = basicBlock->statements()[i];

        /* Simulate another statement. */
        analyzer.simulate(statement, context);

        if (statement->isInlineAssembly()) {
            /*
             * Inline assembly can do unpredictable things.
             * Therefore, clear the reaching definitions.
             */
            context.definitions().clear();
        } else if (statement->isCall()) {
            const ir::Call *call = statement->asCall();
            const ir::dflow::Value *addressValue = dataflow.getValue(call->target());

            /* Record information about the function entry. */
            if (addressValue->isConstant()) {
                program()->addCalledAddress(addressValue->constantValue().value());
                program()->createBasicBlock(addressValue->constantValue().value());
            } else {
                foreach (ByteAddr address, getJumpTableEntries(call->target(), dataflow)) {
                    program()->addCalledAddress(address);
                    program()->createBasicBlock(address);
                }
            }

            /*
             * A call can do unpredictable things.
             * Therefore, clear the reaching definitions.
             */
            context.definitions().clear();
        } else if (statement->isJump()) {
            ir::Jump *jump = statement->as<ir::Jump>();

            /* If target basic block is unknown, try to guess it. */
            computeJumpTarget(jump->thenTarget(), dataflow);
            computeJumpTarget(jump->elseTarget(), dataflow);

            /* Current basic block ends here. */
            if (jump->basicBlock()->address() && jump->instruction()) {
                program()->createBasicBlock(jump->instruction()->endAddr());
            }
        } else if (statement->isReturn()) {
            /* Current basic block ends here. */
            if (statement->basicBlock()->address()) {
                program()->createBasicBlock(statement->instruction()->endAddr());
            }
        }
    }
}

void IRGenerator::computeJumpTarget(ir::JumpTarget &target, const ir::dflow::Dataflow &dataflow) {
    if (target.address() && !target.basicBlock() && !target.table()) {
        const ir::dflow::Value *addressValue = dataflow.getValue(target.address());
        if (addressValue->isConstant()) {
            target.setBasicBlock(program()->createBasicBlock(addressValue->constantValue().value()));
        } else {
            auto entries = getJumpTableEntries(target.address(), dataflow);
            if (!entries.empty()) {
                auto table = std::make_unique<ir::JumpTable>();
                foreach (ByteAddr targetAddress, entries) {
                    table->push_back(ir::JumpTableEntry(targetAddress, program()->createBasicBlock(targetAddress)));
                }
                target.setTable(std::move(table));
            }
        }
    }
}

std::vector<ByteAddr> IRGenerator::getJumpTableEntries(const ir::Term *target, const ir::dflow::Dataflow &dataflow) {
    std::vector<ByteAddr> result;

    auto arrayAccess = ir::misc::recognizeArrayAccess(target, dataflow);
    if (!arrayAccess) {
        return result;
    }

    /* Safety net. */
    const std::size_t maxTableEntries = 65536;
    const ByteSize entrySize = target->size() / CHAR_BIT;

    ByteAddr address = arrayAccess.base();
    while (boost::optional<ByteAddr> entry = module()->image()->readPointer(address, entrySize)) {
        if (!instructions()->get(*entry)) {
            break;
        }
        result.push_back(*entry);
        address += arrayAccess.stride();

        if (result.size() > maxTableEntries) {
            ncWarning("Jump table at address %1 seems to have at least %2 entries. Giving up reading it.", address, result.size());
            break;
        }
    }

    return result;
}

void IRGenerator::addJumpToDirectSuccessor(ir::BasicBlock *basicBlock) {
    assert(basicBlock != NULL);

    /*
     * If there is no jump or return at the end of the basic block,
     * add a jump to the direct successor.
     */
    if (!basicBlock->getTerminator()) {
        if (basicBlock->successorAddress() && basicBlock->successorAddress() != basicBlock->address()) {
            if (ir::BasicBlock *directSuccessor = program()->getBasicBlockStartingAt(*basicBlock->successorAddress())) {
                basicBlock->addStatement(std::make_unique<ir::Jump>(ir::JumpTarget(directSuccessor)));
            }
        }
    }
}

} // namespace irgen
} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
