/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "ArmInstructionAnalyzer.h"

#include <QCoreApplication>

#include <boost/range/size.hpp>

#include <nc/common/CheckedCast.h>
#include <nc/common/Foreach.h>
#include <nc/common/Unreachable.h>
#include <nc/common/make_unique.h>

#include <nc/core/ir/Program.h>
#include <nc/core/irgen/Expressions.h>
#include <nc/core/irgen/InvalidInstructionException.h>

#include "ArmArchitecture.h"
#include "ArmInstruction.h"
#include "ArmRegisters.h"
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
    Q_DECLARE_TR_FUNCTIONS(ArmInstructionAnalyzerImpl)

    const ArmArchitecture *architecture_;
    std::vector<std::pair<int, CapstoneDisassembler>> disassemblers_;
    const cs_arm *detail_;
    SmallBitSize sizeHint_;

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

        detail_ = &instr->detail->arm;
        sizeHint_ = 32;

        switch (instr->id) {
        case ARM_INS_LDR: {
            /* FALLTHROUGH */
        }
        default: {
            _(std::make_unique<core::ir::InlineAssembly>());
        }
        } /* switch */

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

    core::irgen::expressions::TermExpression operand(std::size_t index) const {
        return operand(index, sizeHint_);
    }

    core::irgen::expressions::TermExpression operand(std::size_t index, SmallBitSize sizeHint) const {
        return core::irgen::expressions::TermExpression(createTermForOperand(index, sizeHint));
    }

    std::unique_ptr<core::ir::Term> createTermForOperand(std::size_t index, SmallBitSize sizeHint) const {
        assert(index < boost::size(detail_->operands));

        const auto &operand = detail_->operands[index];
        switch (operand.type) {
            case ARM_OP_REG: {
                auto reg = getRegister(operand.reg);
                if (sizeHint <= reg->size()) {
                    throw core::irgen::InvalidInstructionException(tr("Size hint (%1) exceeds register size (%2).").arg(sizeHint).arg(reg->size()));
                }
                return std::make_unique<core::ir::MemoryLocationAccess>(reg->memoryLocation().resized(sizeHint));
            }
            case ARM_OP_CIMM:
                throw core::irgen::InvalidInstructionException(tr("Don't know how to deal with CIMM operands."));
            case ARM_OP_PIMM:
                throw core::irgen::InvalidInstructionException(tr("Don't know how to deal with PIMM operands."));
            case ARM_OP_IMM:
                // TODO: what about shifts?
                return std::make_unique<core::ir::Constant>(SizedValue(sizeHint, operand.imm));
            case ARM_OP_FP:
                throw core::irgen::InvalidInstructionException(tr("Don't know how to deal with FP operands."));
            case ARM_OP_MEM: {
                const auto &mem = operand.mem;

                auto address = createRegisterAccess(mem.base);

                if (mem.index != ARM_REG_INVALID) {
                    assert(mem.scale == 1 || mem.scale != -1);

                    address = std::make_unique<core::ir::BinaryOperator>(
                        mem.scale == 1 ? core::ir::BinaryOperator::ADD : core::ir::BinaryOperator::SUB,
                        std::move(address),
                        createRegisterAccess(mem.index),
                        address->size()
                    );
                }

                if (mem.disp != 0) {
                    address = std::make_unique<core::ir::BinaryOperator>(
                        core::ir::BinaryOperator::ADD,
                        std::move(address),
                        std::make_unique<core::ir::Constant>(SizedValue(address->size(), mem.disp)),
                        address->size()
                    );
                }

                return std::make_unique<core::ir::Dereference>(std::move(address), core::ir::MemoryDomain::MEMORY, sizeHint);
            }
            default:
                unreachable();
        }
    }

    std::unique_ptr<core::ir::Term> createRegisterAccess(int reg) const {
        return ArmInstructionAnalyzer::createTerm(getRegister(reg));
    }

    const core::arch::Register *getRegister(int reg) const {
        switch (reg) {
        #define REG(lowercase, uppercase, domain, offset, size, comment) \
            case ARM_REG_##uppercase: return ArmRegisters::lowercase();
        #include "ArmRegisterTable.i"
        #undef REG

        default:
            throw core::irgen::InvalidInstructionException(tr("Invalid register number: %1").arg(reg));
        }
    }
};

ArmInstructionAnalyzer::ArmInstructionAnalyzer(const ArmArchitecture *architecture):
    impl_(std::make_unique<ArmInstructionAnalyzerImpl>(architecture))
{}

ArmInstructionAnalyzer::~ArmInstructionAnalyzer() {
}

void ArmInstructionAnalyzer::doCreateStatements(const core::arch::Instruction *instruction, core::ir::Program *program) {
    impl_->createStatements(checked_cast<const ArmInstruction *>(instruction), program);
}

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
