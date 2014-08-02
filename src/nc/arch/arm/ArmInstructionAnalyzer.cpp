/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "ArmInstructionAnalyzer.h"

#include <nc/common/CheckedCast.h>
#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/core/ir/Program.h>
#include <nc/core/irgen/Expressions.h>
#include <nc/core/irgen/InvalidInstructionException.h>

#include "ArmArchitecture.h"
#include "ArmInstruction.h"
#include "CapstoneDisassembler.h"

namespace nc {
namespace arch {
namespace arm {

namespace {

class ArmExpressionFactory: public core::irgen::expressions::ExpressionFactory<ArmExpressionFactory> {
public:
    ArmExpressionFactory(const core::arch::Architecture *architecture):
        core::irgen::expressions::ExpressionFactory<ArmExpressionFactory>(architecture)
    {}
};

typedef core::irgen::expressions::ExpressionFactoryCallback<ArmExpressionFactory> ArmExpressionFactoryCallback;

} // anonymous namespace

class ArmInstructionAnalyzerImpl {
    const ArmArchitecture *architecture_;
    std::vector<std::pair<int, CapstoneDisassembler>> disassemblers_;

public:
    ArmInstructionAnalyzerImpl(const ArmArchitecture *architecture):
        architecture_(architecture)
    {
        assert(architecture_ != NULL);
    }

    void createStatements(const ArmInstruction *instruction, core::ir::Program *program) {
        assert(instruction != NULL);
        assert(program != NULL);

        auto instr = disassemble(instruction);
        assert(instr != NULL);

        ArmExpressionFactory factory(architecture_);
        ArmExpressionFactoryCallback _(factory, program->getBasicBlockForInstruction(instruction), instruction);

        _(std::make_unique<core::ir::InlineAssembly>());

        using namespace core::irgen::expressions;
    }

private:
    CapstoneDisassembler &getDisassembler(int mode) {
        foreach (auto &modeAndDisassembler, disassemblers_) {
            if (modeAndDisassembler.first == mode) {
                return modeAndDisassembler.second;
            }
        }
        disassemblers_.push_back(std::make_pair(mode, CapstoneDisassembler(CS_ARCH_ARM, mode)));
        return disassemblers_.back().second;
    }

    CapstoneInstructionPtr disassemble(const ArmInstruction *instruction) {
        return getDisassembler(instruction->mode()).disassemble(instruction->addr(), instruction->bytes(), instruction->size());
    }
};

ArmInstructionAnalyzer::ArmInstructionAnalyzer(const ArmArchitecture *architecture):
    impl_(std::make_unique<ArmInstructionAnalyzerImpl>(architecture))
{
}

ArmInstructionAnalyzer::~ArmInstructionAnalyzer() {
}

void ArmInstructionAnalyzer::doCreateStatements(const core::arch::Instruction *instruction, core::ir::Program *program) {
    impl_->createStatements(checked_cast<const ArmInstruction *>(instruction), program);
}

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
