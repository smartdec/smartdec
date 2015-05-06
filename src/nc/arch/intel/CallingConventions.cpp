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

#include <nc/core/arch/Operand.h>
#include <nc/core/arch/Operands.h>
#include <nc/core/arch/irgen/InstructionAnalyzer.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>

#include "IntelArchitecture.h"
#include "IntelOperands.h"

namespace nc {
namespace arch {
namespace intel {

AMD64CallingConvention::AMD64CallingConvention(const IntelArchitecture *architecture): core::ir::calls::GenericCallingConvention() {
    const IntelOperands *operands = architecture->operands();

    setStackPointer(operands->rsp()->memoryLocation());

    setFirstArgumentOffset(64);
    setArgumentAlignment(64);

    addArgumentGroup(
        core::ir::calls::ArgumentGroup("Integer Arguments")
        << (core::ir::calls::Argument() << operands->rdi() << operands->edi() << operands->di())
        << (core::ir::calls::Argument() << operands->rsi() << operands->esi() << operands->si())
        << (core::ir::calls::Argument() << operands->rdx() << operands->edx() << operands->dx() << operands->dl())
        << (core::ir::calls::Argument() << operands->rcx() << operands->ecx() << operands->cx() << operands->cl())
        << (core::ir::calls::Argument() << operands->r8() << operands->r8d() << operands->r8w() << operands->r8b())
        << (core::ir::calls::Argument() << operands->r9() << operands->r9d() << operands->r9w() << operands->r9b())
    );

    addArgumentGroup(
        core::ir::calls::ArgumentGroup("Floating-point Arguments")
        << (core::ir::calls::Argument() << operands->xmm0())
        << (core::ir::calls::Argument() << operands->xmm1())
        << (core::ir::calls::Argument() << operands->xmm2())
        << (core::ir::calls::Argument() << operands->xmm3())
        << (core::ir::calls::Argument() << operands->xmm4())
        << (core::ir::calls::Argument() << operands->xmm5())
        << (core::ir::calls::Argument() << operands->xmm6())
        << (core::ir::calls::Argument() << operands->xmm7())
    );

    const core::arch::irgen::InstructionAnalyzer *analyzer = architecture->instructionAnalyzer();
    addReturnValue(analyzer->createTerm(operands->rax()));
    addReturnValue(analyzer->createTerm(operands->eax()));
    addReturnValue(analyzer->createTerm(operands->ax()));
    addReturnValue(analyzer->createTerm(operands->al()));
    addReturnValue(analyzer->createTerm(operands->xmm0()));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        analyzer->createTerm(architecture->operands()->df()),
        std::make_unique<core::ir::Constant>(SizedValue(0, architecture->operands()->df()->size()))
    ));
}

Microsoft64CallingConvention::Microsoft64CallingConvention(const IntelArchitecture *architecture): core::ir::calls::GenericCallingConvention() {
    const IntelOperands *operands = architecture->operands();

    setStackPointer(operands->rsp()->memoryLocation());

    setFirstArgumentOffset(64);
    setArgumentAlignment(64);

    addArgumentGroup(
        core::ir::calls::ArgumentGroup("Integer Arguments")
        << (core::ir::calls::Argument() << operands->rcx() << operands->ecx() << operands->cx() << operands->cl())
        << (core::ir::calls::Argument() << operands->rdx() << operands->edx() << operands->dx() << operands->dl())
        << (core::ir::calls::Argument() << operands->r8() << operands->r8d() << operands->r8w() << operands->r8b())
        << (core::ir::calls::Argument() << operands->r9() << operands->r9d() << operands->r9w() << operands->r9b())
    );

    addArgumentGroup(
        core::ir::calls::ArgumentGroup("Floating-point Arguments")
        << (core::ir::calls::Argument() << operands->xmm0())
        << (core::ir::calls::Argument() << operands->xmm1())
        << (core::ir::calls::Argument() << operands->xmm2())
        << (core::ir::calls::Argument() << operands->xmm3())
    );

    const core::arch::irgen::InstructionAnalyzer *analyzer = architecture->instructionAnalyzer();
    addReturnValue(analyzer->createTerm(operands->rax()));
    addReturnValue(analyzer->createTerm(operands->eax()));
    addReturnValue(analyzer->createTerm(operands->ax()));
    addReturnValue(analyzer->createTerm(operands->al()));
    addReturnValue(analyzer->createTerm(operands->xmm0()));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        analyzer->createTerm(architecture->operands()->df()),
        std::make_unique<core::ir::Constant>(SizedValue(0, architecture->operands()->df()->size()))
    ));
}

Cdecl32CallingConvention::Cdecl32CallingConvention(const IntelArchitecture *architecture): core::ir::calls::GenericCallingConvention() {
    const IntelOperands *operands = architecture->operands();

    setStackPointer(operands->esp()->memoryLocation());

    setFirstArgumentOffset(32);
    setArgumentAlignment(32);

    const core::arch::irgen::InstructionAnalyzer *analyzer = architecture->instructionAnalyzer();
    addReturnValue(analyzer->createTerm(operands->eax()));
    addReturnValue(analyzer->createTerm(operands->ax()));
    addReturnValue(analyzer->createTerm(operands->al()));

    addReturnValue(analyzer->createTerm(architecture->fpuStackOperand(0)));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        analyzer->createTerm(architecture->operands()->df()),
        std::make_unique<core::ir::Constant>(SizedValue(0, architecture->operands()->df()->size()))
    ));
}

Cdecl16CallingConvention::Cdecl16CallingConvention(const IntelArchitecture *architecture): core::ir::calls::GenericCallingConvention() {
    const IntelOperands *operands = architecture->operands();

    setStackPointer(operands->sp()->memoryLocation());

    setFirstArgumentOffset(16);
    setArgumentAlignment(16);

    const core::arch::irgen::InstructionAnalyzer *analyzer = architecture->instructionAnalyzer();
    addReturnValue(analyzer->createTerm(operands->ax()));
    addReturnValue(analyzer->createTerm(operands->al()));

    addReturnValue(analyzer->createTerm(architecture->fpuStackOperand(0)));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        analyzer->createTerm(architecture->operands()->df()),
        std::make_unique<core::ir::Constant>(SizedValue(0, architecture->operands()->df()->size()))
    ));
}

StdcallCallingConvention::StdcallCallingConvention(const IntelArchitecture *architecture): core::ir::calls::GenericCallingConvention() {
    const IntelOperands *operands = architecture->operands();

    setStackPointer(operands->esp()->memoryLocation());

    setFirstArgumentOffset(32);
    setArgumentAlignment(32);
    setCalleeCleanup(true);

    const core::arch::irgen::InstructionAnalyzer *analyzer = architecture->instructionAnalyzer();
    addReturnValue(analyzer->createTerm(operands->eax()));
    addReturnValue(analyzer->createTerm(operands->ax()));
    addReturnValue(analyzer->createTerm(operands->al()));

    addReturnValue(analyzer->createTerm(architecture->fpuStackOperand(0)));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        analyzer->createTerm(architecture->operands()->df()),
        std::make_unique<core::ir::Constant>(SizedValue(0, architecture->operands()->df()->size()))
    ));
}

} // namespace intel
} // namespace arch
} // namespace nc

/* vim:set et ts=4 sw=4: */
