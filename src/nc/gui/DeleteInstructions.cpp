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

#include "DeleteInstructions.h"

#include <nc/common/Foreach.h>

#include <nc/core/arch/Instructions.h>

#include "Project.h"

namespace nc {
namespace gui {

DeleteInstructions::DeleteInstructions(Project *project, const std::vector<const core::arch::Instruction *> &instructions):
    project_(project)
{
    assert(project);

    instructions_.reserve(instructions.size());

    foreach (const core::arch::Instruction *instruction, instructions) {
        auto instr = project->instructions()->get(instruction->addr());
        if (instr.get() == instruction) {
            instructions_.push_back(instr);
        }
    }
}

void DeleteInstructions::work() {
    project_->logToken() << tr("Deleting %1 instruction(s)...", NULL, static_cast<int>(instructions_.size())).arg(instructions_.size());

    auto newInstructions = std::make_shared<core::arch::Instructions>(*project_->instructions());
    foreach (const auto &instruction, instructions_) {
        newInstructions->remove(instruction.get());
    }

    project_->setInstructions(newInstructions);

    project_->logToken() << tr("Deletion completed.", NULL, static_cast<int>(instructions_.size()));
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
