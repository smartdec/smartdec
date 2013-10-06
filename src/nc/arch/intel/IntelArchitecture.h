/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

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

#include <nc/core/arch/Architecture.h>

namespace nc {

namespace core {
    namespace ir {
        namespace calls {
            class CallingConvention;
        }
    }
}

namespace arch {
namespace intel {

class FpuOperand;
class IntelInstructionAnalyzer;
class IntelInstructionDisassembler;
class IntelOperands;
class IntelUniversalAnalyzer;
class IntelRegisters;

class IntelArchitecture: public core::arch::Architecture {
public:
    /**
     * Processor mode.
     */
    enum Mode {
        REAL_MODE,      ///< 16-bit real mode.
        PROTECTED_MODE, ///< 32-bit protected mode.
        LONG_MODE,      ///< 64-bit long mode.
    };

    /**
     * Constructor.
     *
     * \param mode Processor mode.
     */
    IntelArchitecture(Mode mode);

    /**
     * Destructor.
     */
    virtual ~IntelArchitecture();

    const IntelOperands *operands() const {
        return mOperands.get();
    }

    /**
     * \return Valid pointer to the stack pointer register.
     */
    const core::arch::Register *stackPointer() const { return mStackPointer; }

    /**
     * \return Valid pointer to the stack frame base pointer register.
     */
    const core::arch::Register *basePointer() const { return mBasePointer; }

protected:
    friend class IntelRegisters;

private:
    std::unique_ptr<IntelOperands> mOperands;
    std::unique_ptr<IntelInstructionDisassembler> mInstructionDisassembler;
    std::unique_ptr<IntelInstructionAnalyzer> mInstructionAnalyzer;
    std::unique_ptr<IntelUniversalAnalyzer> mUniversalAnalyzer;

    /** Stack pointer register. */
    const core::arch::Register *mStackPointer;

    /** Stack frame base pointer register. */
    const core::arch::Register *mBasePointer;
};

} // namespace intel
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
