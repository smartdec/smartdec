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

#include "IntelInstructionDisassembler.h"

#include <algorithm>
#include <cassert>

#include <nc/common/CheckedCast.h>
#include <nc/common/Unreachable.h>

#define UD_NO_STDINT_DEFINE
#include <libudis86/udis86.h>

#include "IntelArchitecture.h"
#include "IntelInstruction.h"
#include "IntelOperandCache.h"
#include "IntelRegisters.h"

namespace nc {
namespace arch {
namespace intel {

class IntelInstructionDisassemblerPrivate {
    const IntelArchitecture *architecture_;
    IntelOperandCache operandCache_;
    ud_t ud_obj_;

    public:

    IntelInstructionDisassemblerPrivate(const IntelArchitecture *architecture):
        architecture_(architecture), operandCache_(architecture)
    {
        assert(architecture != NULL);

        ud_init(&ud_obj_);
        ud_set_mode(&ud_obj_, architecture->bitness());
    }

    std::unique_ptr<IntelInstruction> disassemble(ByteAddr pc, const void *buffer, ByteSize size) {
        ud_set_pc(&ud_obj_, pc);
        ud_set_input_buffer(&ud_obj_, static_cast<unsigned char *>(const_cast<void *>(buffer)), checked_cast<std::size_t>(size));

        unsigned instructionSize = ud_disassemble(&ud_obj_);
        if (!instructionSize) {
            return NULL;
        }

        std::unique_ptr<IntelInstruction> result(new IntelInstruction(architecture_->bitness(), pc, instructionSize, buffer));

        SmallBitSize lastOperandSize = 0;
        foreach (auto &ud_operand, ud_obj_.operand) {
            if (core::arch::Operand *operand = getOperand(ud_operand, lastOperandSize)) {
                result->addOperand(operand);

                /*
                 * Let's use our size, because libudis86 can report zero sizes
                 * for non-existing registers like sil.
                 */
                lastOperandSize = operand->size();
            } else {
                break;
            }
        }

        return result;
    }

    core::arch::Operand *getOperand(const ud_operand &operand, SmallBitSize lastOperandSize) {
        switch (operand.type) {
            case UD_NONE:
                return NULL;
            case UD_OP_MEM:
                return getDereference(operand);
            case UD_OP_PTR:
                return operandCache_.getConstantOperand(SizedValue(operand.size, operand.lval.ptr.seg * 16 + operand.lval.ptr.off));
            case UD_OP_IMM:
                /* Signed number, sign-extended to match the size of the other operand. */
                return operandCache_.getConstantOperand(SizedValue(std::max(SmallBitSize(operand.size), lastOperandSize), getSignedValue(operand, operand.size)));
            case UD_OP_JIMM:
                return operandCache_.getConstantOperand(SizedValue(architecture_->bitness(), ud_obj_.pc + getSignedValue(operand, operand.size)));
            case UD_OP_CONST:
                /* This is some small constant value, like in "sar eax, 1". Its size is always zero. */
                assert(operand.size == 0);
                return operandCache_.getConstantOperand(SizedValue(8, operand.lval.ubyte));
            case UD_OP_REG:
                return getRegisterOperand(operand.base);
            default:
                unreachable();
        }
    }

    SignedConstantValue getUnsignedValue(const ud_operand &operand, SmallBitSize size) {
        switch (size) {
            case 0:  return 0;
            case 8:  return operand.lval.ubyte;
            case 16: return operand.lval.uword;
            case 32: return operand.lval.udword;
            case 64: return operand.lval.uqword;
            default: unreachable();
        }
    }

    SignedConstantValue getSignedValue(const ud_operand &operand, SmallBitSize size) {
        switch (size) {
            case 0:  return 0;
            case 8:  return operand.lval.sbyte;
            case 16: return operand.lval.sword;
            case 32: return operand.lval.sdword;
            case 64: return operand.lval.sqword;
            default: unreachable();
        }
    }

    core::arch::Operand *getRegisterOperand(enum ud_type type) {
        int number;

        switch (type) {

        #define REG(ud_name, nc_name) case UD_R_##ud_name: number = IntelRegisters::nc_name; break;
        #define REG_ST(n) case UD_R_ST##n: return operandCache_.getFpuOperand(n);

        REG(AL, AL)
        REG(CL, CL)
        REG(DL, DL)
        REG(BL, BL)
        REG(AH, AH)
        REG(CH, CH)
        REG(DH, DH)
        REG(BH, BH)
        REG(SPL, SPL)
        REG(BPL, BPL)
        REG(SIL, SIL)
        REG(DIL, DIL)
        REG(R8B, R8B)
        REG(R9B, R9B)
        REG(R10B, R10B)
        REG(R11B, R11B)
        REG(R12B, R12B)
        REG(R13B, R13B)
        REG(R14B, R14B)
        REG(R15B, R15B)
        REG(AX, AX)
        REG(CX, CX)
        REG(DX, DX)
        REG(BX, BX)
        REG(SP, SP)
        REG(BP, BP)
        REG(SI, SI)
        REG(DI, DI)
        REG(R8W, R8W)
        REG(R9W, R9W)
        REG(R10W, R10W)
        REG(R11W, R11W)
        REG(R12W, R12W)
        REG(R13W, R13W)
        REG(R14W, R14W)
        REG(R15W, R15W)
        REG(EAX, EAX)
        REG(ECX, ECX)
        REG(EDX, EDX)
        REG(EBX, EBX)
        REG(ESP, ESP)
        REG(EBP, EBP)
        REG(ESI, ESI)
        REG(EDI, EDI)
        REG(R8D, R8D)
        REG(R9D, R9D)
        REG(R10D, R10D)
        REG(R11D, R11D)
        REG(R12D, R12D)
        REG(R13D, R13D)
        REG(R14D, R14D)
        REG(R15D, R15D)
        REG(RAX, RAX)
        REG(RCX, RCX)
        REG(RDX, RDX)
        REG(RBX, RBX)
        REG(RSP, RSP)
        REG(RBP, RBP)
        REG(RSI, RSI)
        REG(RDI, RDI)
        REG(R8, R8)
        REG(R9, R9)
        REG(R10, R10)
        REG(R11, R11)
        REG(R12, R12)
        REG(R13, R13)
        REG(R14, R14)
        REG(R15, R15)
        REG(ES, ES)
        REG(CS, CS)
        REG(SS, SS)
        REG(DS, DS)
        REG(FS, FS)
        REG(GS, GS)
        REG(CR0, CR0)
        REG(CR1, CR1)
        REG(CR2, CR2)
        REG(CR3, CR3)
        REG(CR4, CR4)
        REG(CR5, CR5)
        REG(CR6, CR6)
        REG(CR7, CR7)
        REG(CR8, CR8)
        REG(CR9, CR9)
        REG(CR10, CR10)
        REG(CR11, CR11)
        REG(CR12, CR12)
        REG(CR13, CR13)
        REG(CR14, CR14)
        REG(CR15, CR15)
        REG(DR0, DR0)
        REG(DR1, DR1)
        REG(DR2, DR2)
        REG(DR3, DR3)
        REG(DR4, DR4)
        REG(DR5, DR5)
        REG(DR6, DR6)
        REG(DR7, DR7)
        REG(DR8, DR8)
        REG(DR9, DR9)
        REG(DR10, DR10)
        REG(DR11, DR11)
        REG(DR12, DR12)
        REG(DR13, DR13)
        REG(DR14, DR14)
        REG(DR15, DR15)
        REG(MM0, MM0)
        REG(MM1, MM1)
        REG(MM2, MM2)
        REG(MM3, MM3)
        REG(MM4, MM4)
        REG(MM5, MM5)
        REG(MM6, MM6)
        REG(MM7, MM7)
        REG_ST(0)
        REG_ST(1)
        REG_ST(2)
        REG_ST(3)
        REG_ST(4)
        REG_ST(5)
        REG_ST(6)
        REG_ST(7)
        REG(XMM0, XMM0)
        REG(XMM1, XMM1)
        REG(XMM2, XMM2)
        REG(XMM3, XMM3)
        REG(XMM4, XMM4)
        REG(XMM5, XMM5)
        REG(XMM6, XMM6)
        REG(XMM7, XMM7)
        REG(XMM8, XMM8)
        REG(XMM9, XMM9)
        REG(XMM10, XMM10)
        REG(XMM11, XMM11)
        REG(XMM12, XMM12)
        REG(XMM13, XMM13)
        REG(XMM14, XMM14)
        REG(XMM15, XMM15)
        REG(RIP, RIP)

        #undef REG
        #undef REG_ST

        default:
            return NULL;
        }

        return operandCache_.getRegisterOperand(number);
    }

    core::arch::DereferenceOperand *getDereference(const ud_operand &operand) {
        assert(operand.type == UD_OP_MEM);

        core::arch::Operand *address = getRegisterOperand(operand.base);

        if (operand.scale != 0) {
            if (core::arch::Operand *index = getRegisterOperand(operand.index)) {
                if (operand.scale != 1) {
                    index = new core::arch::MultiplicationOperand(
                        index,
                        operandCache_.getConstantOperand(SizedValue(ud_obj_.adr_mode, operand.scale)),
                        ud_obj_.adr_mode);
                }
                if (address) {
                    address = new core::arch::AdditionOperand(address, index, ud_obj_.adr_mode);
                } else {
                    address = index;
                }
            }
        }

        auto offsetValue = SizedValue(operand.offset, getUnsignedValue(operand, operand.offset));

        if (offsetValue.value() || !address) {
            core::arch::ConstantOperand *offset = operandCache_.getConstantOperand(offsetValue);

            if (address) {
                address = new core::arch::AdditionOperand(address, offset, ud_obj_.adr_mode);
            } else {
                address = offset;
            }
        }

        return new core::arch::DereferenceOperand(address, operand.size);
    }
};

IntelInstructionDisassembler::IntelInstructionDisassembler(const IntelArchitecture *architecture):
    impl_(new IntelInstructionDisassemblerPrivate(architecture))
{}

IntelInstructionDisassembler::~IntelInstructionDisassembler() {}

std::unique_ptr<core::arch::Instruction> IntelInstructionDisassembler::disassemble(ByteAddr pc, const void *buffer, ByteSize size) const {
    return impl_->disassemble(pc, buffer, size);
}

} // namespace intel
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
