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

#include "IntelOperands.h"

#include <QTextStream>

#include <nc/common/Types.h>
#include <nc/core/arch/Operands.h>
#include <nc/core/ir/Terms.h>

#include "IntelArchitecture.h"
#include "IntelRegisters.h"

namespace nc {
namespace arch {
namespace intel {

#define REG(lowercase, uppercase, domain, offset, size, comment)                \
core::arch::RegisterOperand *IntelOperands::lowercase() const {                 \
    return architecture_->registerOperand(IntelRegisters::uppercase);           \
}
#include "IntelRegisterTable.i"
#undef REG

FpuStackOperand::FpuStackOperand(int index):
    core::arch::CachedOperand(IntelOperands::FPU_STACK, IntelRegisters::fpu_r0()->memoryLocation().size<SmallBitSize>()),
    index_(index)
{}

void FpuStackOperand::print(QTextStream &out) const {
    out << "st(" << index_ << ")";
}

} // namespace intel
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
