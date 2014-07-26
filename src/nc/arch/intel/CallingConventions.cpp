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

#include "CallingConventions.h"

#include <nc/common/make_unique.h>

#include <nc/core/arch/irgen/InstructionAnalyzer.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>

#include "IntelArchitecture.h"
#include "IntelRegisters.h"
#include "IntelInstructionAnalyzer.h"

namespace nc {
namespace arch {
namespace intel {

AMD64CallingConvention::AMD64CallingConvention(const IntelArchitecture *architecture):
    Convention(QLatin1String("amd64"))
{
    setStackPointer(architecture->stackPointer()->memoryLocation());

    setFirstArgumentOffset(64);
    setArgumentAlignment(64);

    /* Used for integer and pointer arguments. */
    std::vector<core::ir::MemoryLocation> scalarArgs;
    scalarArgs.push_back(IntelRegisters::rdi()->memoryLocation());
    scalarArgs.push_back(IntelRegisters::rsi()->memoryLocation());
    scalarArgs.push_back(IntelRegisters::rdx()->memoryLocation());
    scalarArgs.push_back(IntelRegisters::rcx()->memoryLocation());
    scalarArgs.push_back(IntelRegisters::r8()->memoryLocation());
    scalarArgs.push_back(IntelRegisters::r9()->memoryLocation());
    addArgumentGroup(std::move(scalarArgs));

    /* Used for floating-point arguments. */
    std::vector<core::ir::MemoryLocation> fpArgs;
    fpArgs.push_back(IntelRegisters::xmm0()->memoryLocation());
    fpArgs.push_back(IntelRegisters::xmm1()->memoryLocation());
    fpArgs.push_back(IntelRegisters::xmm2()->memoryLocation());
    fpArgs.push_back(IntelRegisters::xmm3()->memoryLocation());
    fpArgs.push_back(IntelRegisters::xmm4()->memoryLocation());
    fpArgs.push_back(IntelRegisters::xmm5()->memoryLocation());
    fpArgs.push_back(IntelRegisters::xmm6()->memoryLocation());
    fpArgs.push_back(IntelRegisters::xmm7()->memoryLocation());
    addArgumentGroup(std::move(fpArgs));

    addReturnValueTerm(IntelInstructionAnalyzer::createTerm(IntelRegisters::rax()));
    addReturnValueTerm(IntelInstructionAnalyzer::createTerm(IntelRegisters::eax()));
    addReturnValueTerm(IntelInstructionAnalyzer::createTerm(IntelRegisters::ax()));
    addReturnValueTerm(IntelInstructionAnalyzer::createTerm(IntelRegisters::al()));
    addReturnValueTerm(IntelInstructionAnalyzer::createTerm(IntelRegisters::xmm0()));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        IntelInstructionAnalyzer::createTerm(IntelRegisters::df()),
        std::make_unique<core::ir::Constant>(SizedValue(IntelRegisters::df()->size(), 0))
    ));
}

Microsoft64CallingConvention::Microsoft64CallingConvention(const IntelArchitecture *architecture):
    Convention(QLatin1String("microsoft64"))
{
    setStackPointer(architecture->stackPointer()->memoryLocation());

    setFirstArgumentOffset(64);
    setArgumentAlignment(64);

    /* Used for integer and pointer arguments. */
    std::vector<core::ir::MemoryLocation> scalarArgs;
    scalarArgs.push_back(IntelRegisters::rcx()->memoryLocation());
    scalarArgs.push_back(IntelRegisters::rdx()->memoryLocation());
    scalarArgs.push_back(IntelRegisters::r8()->memoryLocation());
    scalarArgs.push_back(IntelRegisters::r9()->memoryLocation());
    addArgumentGroup(std::move(scalarArgs));

    /* Used for floating-point arguments. */
    std::vector<core::ir::MemoryLocation> fpArgs;
    fpArgs.push_back(IntelRegisters::xmm0()->memoryLocation());
    fpArgs.push_back(IntelRegisters::xmm1()->memoryLocation());
    fpArgs.push_back(IntelRegisters::xmm2()->memoryLocation());
    fpArgs.push_back(IntelRegisters::xmm3()->memoryLocation());
    addArgumentGroup(std::move(fpArgs));

    addReturnValueTerm(IntelInstructionAnalyzer::createTerm(IntelRegisters::rax()));
    addReturnValueTerm(IntelInstructionAnalyzer::createTerm(IntelRegisters::eax()));
    addReturnValueTerm(IntelInstructionAnalyzer::createTerm(IntelRegisters::ax()));
    addReturnValueTerm(IntelInstructionAnalyzer::createTerm(IntelRegisters::al()));
    addReturnValueTerm(IntelInstructionAnalyzer::createTerm(IntelRegisters::xmm0()));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        IntelInstructionAnalyzer::createTerm(IntelRegisters::df()),
        std::make_unique<core::ir::Constant>(SizedValue(IntelRegisters::df()->size(), 0))
    ));
}

Cdecl32CallingConvention::Cdecl32CallingConvention(const IntelArchitecture *architecture):
    Convention(QLatin1String("cdecl32"))
{
    setStackPointer(architecture->stackPointer()->memoryLocation());

    setFirstArgumentOffset(32);
    setArgumentAlignment(32);

    addReturnValueTerm(IntelInstructionAnalyzer::createTerm(IntelRegisters::eax()));
    addReturnValueTerm(IntelInstructionAnalyzer::createTerm(IntelRegisters::ax()));
    addReturnValueTerm(IntelInstructionAnalyzer::createTerm(IntelRegisters::al()));
    addReturnValueTerm(IntelInstructionAnalyzer::createFpuTerm(0));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        IntelInstructionAnalyzer::createTerm(IntelRegisters::df()),
        std::make_unique<core::ir::Constant>(SizedValue(IntelRegisters::df()->size(), 0))
    ));
}

Cdecl16CallingConvention::Cdecl16CallingConvention(const IntelArchitecture *architecture):
    Convention(QLatin1String("cdecl16"))
{
    setStackPointer(architecture->stackPointer()->memoryLocation());

    setFirstArgumentOffset(16);
    setArgumentAlignment(16);

    addReturnValueTerm(IntelInstructionAnalyzer::createTerm(IntelRegisters::ax()));
    addReturnValueTerm(IntelInstructionAnalyzer::createTerm(IntelRegisters::al()));
    addReturnValueTerm(IntelInstructionAnalyzer::createFpuTerm(0));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        IntelInstructionAnalyzer::createTerm(IntelRegisters::df()),
        std::make_unique<core::ir::Constant>(SizedValue(IntelRegisters::df()->size(), 0))
    ));
}

Stdcall32CallingConvention::Stdcall32CallingConvention(const IntelArchitecture *architecture):
    Convention(QLatin1String("stdcall32"))
{
    setStackPointer(architecture->stackPointer()->memoryLocation());

    setFirstArgumentOffset(32);
    setArgumentAlignment(32);
    setCalleeCleanup(true);

    addReturnValueTerm(IntelInstructionAnalyzer::createTerm(IntelRegisters::eax()));
    addReturnValueTerm(IntelInstructionAnalyzer::createTerm(IntelRegisters::ax()));
    addReturnValueTerm(IntelInstructionAnalyzer::createTerm(IntelRegisters::al()));
    addReturnValueTerm(IntelInstructionAnalyzer::createFpuTerm(0));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        IntelInstructionAnalyzer::createTerm(IntelRegisters::df()),
        std::make_unique<core::ir::Constant>(SizedValue(IntelRegisters::df()->size(), 0))
    ));
}

} // namespace intel
} // namespace arch
} // namespace nc

/* vim:set et ts=4 sw=4: */
