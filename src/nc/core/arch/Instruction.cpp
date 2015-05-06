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

#include "Instruction.h"

#include <QTextStream>

#include <nc/common/Visitor.h>
#include <nc/common/Foreach.h>

#include "Operand.h"

namespace nc {
namespace core {
namespace arch {

Instruction::~Instruction() {
    clearOperands();
}

void Instruction::clearOperands() {
    foreach(Operand *operand, operands_) {
        operand->dispose();
    }
    operands_.clear();
}

void Instruction::removeOperand(std::size_t index) {
    operands_[index]->dispose();
    operands_.erase(operands_.begin() + index);
}

void Instruction::replaceOperand(std::size_t index, Operand *operand) {
    operands_[index]->dispose();
    operands_[index] = operand;
}

} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
