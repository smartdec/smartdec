/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <nc/core/arch/Operands.h>

namespace nc {
namespace arch {
namespace intel {

class IntelArchitecture;

/**
 * Container class for intel operands.
 */
class IntelOperands {
public:
    IntelOperands(IntelArchitecture *architecture): architecture_(architecture) {}

    enum {
        FPU_STACK = core::arch::Operand::USER
    };

    /* Operand accessors. */
#define REG(lowercase, uppercase, domain, offset, size, comment)                \
    core::arch::RegisterOperand *lowercase() const;
#include "IntelRegisterTable.i"
#undef REG

private:
    IntelArchitecture *architecture_;
};


/**
 * x87 FPU stack access, like st(0).
 */
class FpuStackOperand: public core::arch::CachedOperand {
protected:
    friend class IntelArchitecture; /* Calls constructor. */

    /**
     * Class constructor.
     *
     * It is not supposed to be called from outside IntelArchitecture.
     *
     * \param[in] index                Index of data register relative to FPU stack top.
     */
    FpuStackOperand(int index);

public:
    /**
     * \return                         Index of accessed element.
     */
    int index() const { return index_; }

    virtual void print(QTextStream &out) const override;

private:
    int index_; ///< Index of accessed element.
};

} // namespace intel
} // namespace arch
} // namespace nc

NC_REGISTER_OPERAND_CLASS(nc::arch::intel::FpuStackOperand, nc::arch::intel::IntelOperands::FPU_STACK)
