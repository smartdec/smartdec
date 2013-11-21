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
    setStackPointer(IntelRegisters::rsp()->memoryLocation());

    setFirstArgumentOffset(64);
    setArgumentAlignment(64);

    addArgumentGroup(
        core::ir::calling::ArgumentGroup(QLatin1String("Integer Arguments"))
        << IntelRegisters::rdi()
        << IntelRegisters::rsi()
        << IntelRegisters::rdx()
        << IntelRegisters::rcx()
        << IntelRegisters::r8()
        << IntelRegisters::r9()
    );

    addArgumentGroup(
        core::ir::calling::ArgumentGroup(QLatin1String("Floating-point Arguments"))
        << IntelRegisters::xmm0()
        << IntelRegisters::xmm1()
        << IntelRegisters::xmm2()
        << IntelRegisters::xmm3()
        << IntelRegisters::xmm4()
        << IntelRegisters::xmm5()
        << IntelRegisters::xmm6()
        << IntelRegisters::xmm7()
    );

    auto analyzer = checked_cast<const IntelInstructionAnalyzer *>(architecture->instructionAnalyzer());
    addReturnValue(analyzer->createTerm(IntelRegisters::rax()));
    addReturnValue(analyzer->createTerm(IntelRegisters::eax()));
    addReturnValue(analyzer->createTerm(IntelRegisters::ax()));
    addReturnValue(analyzer->createTerm(IntelRegisters::al()));
    addReturnValue(analyzer->createTerm(IntelRegisters::xmm0()));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        analyzer->createTerm(IntelRegisters::df()),
        std::make_unique<core::ir::Constant>(SizedValue(IntelRegisters::df()->size(), 0))
    ));
}

Microsoft64CallingConvention::Microsoft64CallingConvention(const IntelArchitecture *architecture):
    Convention(QLatin1String("microsoft64"))
{
    setStackPointer(IntelRegisters::rsp()->memoryLocation());

    setFirstArgumentOffset(64);
    setArgumentAlignment(64);

    addArgumentGroup(
        core::ir::calling::ArgumentGroup("Integer Arguments")
        << IntelRegisters::rcx()
        << IntelRegisters::rdx()
        << IntelRegisters::r8()
        << IntelRegisters::r9()
    );

    addArgumentGroup(
        core::ir::calling::ArgumentGroup("Floating-point Arguments")
        << IntelRegisters::xmm0()
        << IntelRegisters::xmm1()
        << IntelRegisters::xmm2()
        << IntelRegisters::xmm3()
    );

    auto analyzer = checked_cast<const IntelInstructionAnalyzer *>(architecture->instructionAnalyzer());
    addReturnValue(analyzer->createTerm(IntelRegisters::rax()));
    addReturnValue(analyzer->createTerm(IntelRegisters::eax()));
    addReturnValue(analyzer->createTerm(IntelRegisters::ax()));
    addReturnValue(analyzer->createTerm(IntelRegisters::al()));
    addReturnValue(analyzer->createTerm(IntelRegisters::xmm0()));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        analyzer->createTerm(IntelRegisters::df()),
        std::make_unique<core::ir::Constant>(SizedValue(IntelRegisters::df()->size(), 0))
    ));
}

Cdecl32CallingConvention::Cdecl32CallingConvention(const IntelArchitecture *architecture):
    Convention(QLatin1String("cdecl32"))
{
    setStackPointer(IntelRegisters::esp()->memoryLocation());

    setFirstArgumentOffset(32);
    setArgumentAlignment(32);

    auto analyzer = checked_cast<const IntelInstructionAnalyzer *>(architecture->instructionAnalyzer());
    addReturnValue(analyzer->createTerm(IntelRegisters::eax()));
    addReturnValue(analyzer->createTerm(IntelRegisters::ax()));
    addReturnValue(analyzer->createTerm(IntelRegisters::al()));
    addReturnValue(analyzer->createFpuTerm(0));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        analyzer->createTerm(IntelRegisters::df()),
        std::make_unique<core::ir::Constant>(SizedValue(IntelRegisters::df()->size(), 0))
    ));
}

Cdecl16CallingConvention::Cdecl16CallingConvention(const IntelArchitecture *architecture):
    Convention(QLatin1String("cdecl16"))
{
    setStackPointer(IntelRegisters::sp()->memoryLocation());

    setFirstArgumentOffset(16);
    setArgumentAlignment(16);

    auto analyzer = checked_cast<const IntelInstructionAnalyzer *>(architecture->instructionAnalyzer());
    addReturnValue(analyzer->createTerm(IntelRegisters::ax()));
    addReturnValue(analyzer->createTerm(IntelRegisters::al()));
    addReturnValue(analyzer->createFpuTerm(0));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        analyzer->createTerm(IntelRegisters::df()),
        std::make_unique<core::ir::Constant>(SizedValue(IntelRegisters::df()->size(), 0))
    ));
}

Stdcall32CallingConvention::Stdcall32CallingConvention(const IntelArchitecture *architecture):
    Convention(QLatin1String("stdcall32"))
{
    setStackPointer(IntelRegisters::esp()->memoryLocation());

    setFirstArgumentOffset(32);
    setArgumentAlignment(32);
    setCalleeCleanup(true);

    auto analyzer = checked_cast<const IntelInstructionAnalyzer *>(architecture->instructionAnalyzer());
    addReturnValue(analyzer->createTerm(IntelRegisters::eax()));
    addReturnValue(analyzer->createTerm(IntelRegisters::ax()));
    addReturnValue(analyzer->createTerm(IntelRegisters::al()));
    addReturnValue(analyzer->createFpuTerm(0));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        analyzer->createTerm(IntelRegisters::df()),
        std::make_unique<core::ir::Constant>(SizedValue(IntelRegisters::df()->size(), 0))
    ));
}

} // namespace intel
} // namespace arch
} // namespace nc

/* vim:set et ts=4 sw=4: */
