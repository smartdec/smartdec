/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "CallingConventions.h"

#include <nc/common/make_unique.h>

#include <nc/core/arch/irgen/InstructionAnalyzer.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>

#include "X86Architecture.h"
#include "X86Registers.h"
#include "X86InstructionAnalyzer.h"

namespace nc {
namespace arch {
namespace x86 {

AMD64CallingConvention::AMD64CallingConvention(const X86Architecture *architecture):
    Convention(QLatin1String("amd64"))
{
    setStackPointer(architecture->stackPointer()->memoryLocation());

    setFirstArgumentOffset(64);
    setArgumentAlignment(64);

    /* Used for integer and pointer arguments. */
    std::vector<core::ir::MemoryLocation> scalarArgs;
    scalarArgs.push_back(X86Registers::rdi()->memoryLocation());
    scalarArgs.push_back(X86Registers::rsi()->memoryLocation());
    scalarArgs.push_back(X86Registers::rdx()->memoryLocation());
    scalarArgs.push_back(X86Registers::rcx()->memoryLocation());
    scalarArgs.push_back(X86Registers::r8()->memoryLocation());
    scalarArgs.push_back(X86Registers::r9()->memoryLocation());
    addArgumentGroup(std::move(scalarArgs));

    /* Used for floating-point arguments. */
    std::vector<core::ir::MemoryLocation> fpArgs;
    fpArgs.push_back(X86Registers::xmm0()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm1()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm2()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm3()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm4()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm5()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm6()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm7()->memoryLocation());
    addArgumentGroup(std::move(fpArgs));

    addReturnValueTerm(X86InstructionAnalyzer::createTerm(X86Registers::rax()));
    addReturnValueTerm(X86InstructionAnalyzer::createTerm(X86Registers::eax()));
    addReturnValueTerm(X86InstructionAnalyzer::createTerm(X86Registers::ax()));
    addReturnValueTerm(X86InstructionAnalyzer::createTerm(X86Registers::al()));
    addReturnValueTerm(X86InstructionAnalyzer::createTerm(X86Registers::xmm0()));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        X86InstructionAnalyzer::createTerm(X86Registers::df()),
        std::make_unique<core::ir::Constant>(SizedValue(X86Registers::df()->size(), 0))
    ));
}

Microsoft64CallingConvention::Microsoft64CallingConvention(const X86Architecture *architecture):
    Convention(QLatin1String("microsoft64"))
{
    setStackPointer(architecture->stackPointer()->memoryLocation());

    setFirstArgumentOffset(64);
    setArgumentAlignment(64);

    /* Used for integer and pointer arguments. */
    std::vector<core::ir::MemoryLocation> scalarArgs;
    scalarArgs.push_back(X86Registers::rcx()->memoryLocation());
    scalarArgs.push_back(X86Registers::rdx()->memoryLocation());
    scalarArgs.push_back(X86Registers::r8()->memoryLocation());
    scalarArgs.push_back(X86Registers::r9()->memoryLocation());
    addArgumentGroup(std::move(scalarArgs));

    /* Used for floating-point arguments. */
    std::vector<core::ir::MemoryLocation> fpArgs;
    fpArgs.push_back(X86Registers::xmm0()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm1()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm2()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm3()->memoryLocation());
    addArgumentGroup(std::move(fpArgs));

    addReturnValueTerm(X86InstructionAnalyzer::createTerm(X86Registers::rax()));
    addReturnValueTerm(X86InstructionAnalyzer::createTerm(X86Registers::eax()));
    addReturnValueTerm(X86InstructionAnalyzer::createTerm(X86Registers::ax()));
    addReturnValueTerm(X86InstructionAnalyzer::createTerm(X86Registers::al()));
    addReturnValueTerm(X86InstructionAnalyzer::createTerm(X86Registers::xmm0()));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        X86InstructionAnalyzer::createTerm(X86Registers::df()),
        std::make_unique<core::ir::Constant>(SizedValue(X86Registers::df()->size(), 0))
    ));
}

Cdecl32CallingConvention::Cdecl32CallingConvention(const X86Architecture *architecture):
    Convention(QLatin1String("cdecl32"))
{
    setStackPointer(architecture->stackPointer()->memoryLocation());

    setFirstArgumentOffset(32);
    setArgumentAlignment(32);

    addReturnValueTerm(X86InstructionAnalyzer::createTerm(X86Registers::eax()));
    addReturnValueTerm(X86InstructionAnalyzer::createTerm(X86Registers::ax()));
    addReturnValueTerm(X86InstructionAnalyzer::createTerm(X86Registers::al()));
    addReturnValueTerm(X86InstructionAnalyzer::createFpuTerm(0));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        X86InstructionAnalyzer::createTerm(X86Registers::df()),
        std::make_unique<core::ir::Constant>(SizedValue(X86Registers::df()->size(), 0))
    ));
}

Cdecl16CallingConvention::Cdecl16CallingConvention(const X86Architecture *architecture):
    Convention(QLatin1String("cdecl16"))
{
    setStackPointer(architecture->stackPointer()->memoryLocation());

    setFirstArgumentOffset(16);
    setArgumentAlignment(16);

    addReturnValueTerm(X86InstructionAnalyzer::createTerm(X86Registers::ax()));
    addReturnValueTerm(X86InstructionAnalyzer::createTerm(X86Registers::al()));
    addReturnValueTerm(X86InstructionAnalyzer::createFpuTerm(0));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        X86InstructionAnalyzer::createTerm(X86Registers::df()),
        std::make_unique<core::ir::Constant>(SizedValue(X86Registers::df()->size(), 0))
    ));
}

Stdcall32CallingConvention::Stdcall32CallingConvention(const X86Architecture *architecture):
    Convention(QLatin1String("stdcall32"))
{
    setStackPointer(architecture->stackPointer()->memoryLocation());

    setFirstArgumentOffset(32);
    setArgumentAlignment(32);
    setCalleeCleanup(true);

    addReturnValueTerm(X86InstructionAnalyzer::createTerm(X86Registers::eax()));
    addReturnValueTerm(X86InstructionAnalyzer::createTerm(X86Registers::ax()));
    addReturnValueTerm(X86InstructionAnalyzer::createTerm(X86Registers::al()));
    addReturnValueTerm(X86InstructionAnalyzer::createFpuTerm(0));

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        X86InstructionAnalyzer::createTerm(X86Registers::df()),
        std::make_unique<core::ir::Constant>(SizedValue(X86Registers::df()->size(), 0))
    ));
}

} // namespace x86
} // namespace arch
} // namespace nc

/* vim:set et ts=4 sw=4: */
