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

#include "IntelInstructionAnalyzer.h"

#include <nc/common/CheckedCast.h>
#include <nc/common/Unreachable.h>

#include <nc/core/arch/Operand.h>
#include <nc/core/arch/Operands.h>
#include <nc/core/arch/irgen/Expressions.h>
#include <nc/core/arch/irgen/InvalidInstructionException.h>
#include <nc/core/ir/Program.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>

#include "IntelMnemonics.h"
#include "IntelArchitecture.h"
#include "IntelInstruction.h"
#include "IntelRegisters.h"
#include "IntelOperands.h"

namespace nc {
namespace arch {
namespace intel {

namespace {

// -------------------------------------------------------------------------- //
// Expressions
// -------------------------------------------------------------------------- //
// what's the array '_'?
//      Not an array, a transformer for expression trees that roughly mimics 
//      boost::phoenix/boost::lambda convention of using brackets for
//      curly braces when writing "C++-in-C++".
//      
//      Need to think of what to document here. 
class IntelExpressionFactory: public core::arch::irgen::expressions::ExpressionFactory<IntelExpressionFactory> {
public:
    IntelExpressionFactory(core::arch::Architecture *architecture, const core::arch::Instruction *instruction): 
        core::arch::irgen::expressions::ExpressionFactory<IntelExpressionFactory>(architecture, instruction)
    {}
};

typedef core::arch::irgen::expressions::ExpressionFactoryCallback<IntelExpressionFactory> IntelExpressionFactoryCallback;

#define NC_DEFINE_REGISTER_EXPRESSION(lowercase, uppercase)                     \
core::arch::irgen::expressions::RegisterExpression lowercase() {                             \
    return core::arch::irgen::expressions::RegisterExpression(IntelRegisters::uppercase);    \
}                                                                               \

NC_DEFINE_REGISTER_EXPRESSION(cf, CF)
NC_DEFINE_REGISTER_EXPRESSION(pf, PF)
NC_DEFINE_REGISTER_EXPRESSION(zf, ZF)
NC_DEFINE_REGISTER_EXPRESSION(sf, SF)
NC_DEFINE_REGISTER_EXPRESSION(of, OF)
NC_DEFINE_REGISTER_EXPRESSION(af, AF)
NC_DEFINE_REGISTER_EXPRESSION(df, DF)
NC_DEFINE_REGISTER_EXPRESSION(flags, FLAGS)
NC_DEFINE_REGISTER_EXPRESSION(eflags, EFLAGS)
NC_DEFINE_REGISTER_EXPRESSION(rflags, RFLAGS)

NC_DEFINE_REGISTER_EXPRESSION(pseudo_flags, PSEUDO_FLAGS)
NC_DEFINE_REGISTER_EXPRESSION(less, LESS)
NC_DEFINE_REGISTER_EXPRESSION(less_or_equal, LESS_OR_EQUAL)
NC_DEFINE_REGISTER_EXPRESSION(greater, GREATER)
NC_DEFINE_REGISTER_EXPRESSION(greater_or_equal, GREATER_OR_EQUAL)
NC_DEFINE_REGISTER_EXPRESSION(below, BELOW)
NC_DEFINE_REGISTER_EXPRESSION(below_or_equal, BELOW_OR_EQUAL)
NC_DEFINE_REGISTER_EXPRESSION(above, ABOVE)
NC_DEFINE_REGISTER_EXPRESSION(above_or_equal, ABOVE_OR_EQUAL)

NC_DEFINE_REGISTER_EXPRESSION(cx, CX)
NC_DEFINE_REGISTER_EXPRESSION(ecx, ECX)
NC_DEFINE_REGISTER_EXPRESSION(rcx, RCX)

#undef NC_DEFINE_REGISTER_EXPRESSION

core::arch::irgen::expressions::MemoryLocationExpression
resizedRegister(const core::arch::Register *reg, SmallBitSize size) {
    return core::ir::MemoryLocation(reg->memoryLocation().domain(), 0, size);
}

core::arch::irgen::expressions::MemoryLocationExpression
temporary(SmallBitSize size) {
    return resizedRegister(IntelRegisters::tmp64(), size);
}

} // anonymous namespace


// -------------------------------------------------------------------------- //
// IntelInstructionAnalyzer
// -------------------------------------------------------------------------- //
void IntelInstructionAnalyzer::doCreateStatements(const core::arch::Instruction *instruction, core::ir::Program *program) const {
    assert(instruction != NULL);

    using namespace mnemonics;
    using namespace intel;

    const IntelInstruction *instr = checked_cast<const IntelInstruction *>(instruction);

    /* Sanity checks */
    switch (instr->mnemonic()->number()) {
        case CLD:
        case CBW:
        case CWDE:
        case CDQE:
        case CMPSB:
        case CMPSW:
        case CMPSD:
        case CMPSQ:
        case CPUID:
        case INT3:
        case LEAVEW:
        case LEAVE:
        case LEAVED:
        case LEAVEQ:
        case MOVSB:
        case MOVSW:
        case MOVSD:
        case MOVSQ:
        case POPFW:
        case POPFD:
        case POPFQ:
        case PUSHFW:
        case PUSHFD:
        case PUSHFQ:
        case SCASB:
        case SCASW:
        case SCASD:
        case SCASQ:
        case STD:
        case STOSB:
        case STOSW:
        case STOSD:
        case STOSQ:
            if (instr->operands().size() != 0) {
                throw core::arch::irgen::InvalidInstructionException("instruction takes no arguments");
            }
            break;
        case RET:
            if(instr->operands().size() > 1) {
                throw core::arch::irgen::InvalidInstructionException("instruction takes zero or one argument");
            }
            break;
        case CALL:
        case DEC:
        case DIV:
        case INC:
        case INT:
        case JA:
        case JAE:
        case JB:
        case JBE:
        case JC:
        case JCXZ:
        case JECXZ:
        case JRCXZ:
        case JE:
        case JG:
        case JGE:
        case JL:
        case JLE:
        case JNA:
        case JNAE:
        case JNB:
        case JNBE:
        case JNC:
        case JNE:
        case JNG:
        case JNGE:
        case JNL:
        case JNLE:
        case JNO:
        case JNP:
        case JNS:
        case JNZ:
        case JO:
        case JP:
        case JPE:
        case JPO:
        case JS:
        case JZ:
        case JMP:
        case JMPFI:
        case JMPNI:
        case JMPSHORT:
        case LOOP:
        case LOOPE:
        case LOOPNE:
        case MUL:
        case NEG:
        case NOT:
        case POP:
        case PUSH:
        case SETA:
        case SETAE:
        case SETB:
        case SETBE:
        case SETC:
        case SETE:
        case SETG:
        case SETGE:
        case SETL:
        case SETLE:
        case SETNA:
        case SETNAE:
        case SETNB:
        case SETNBE:
        case SETNC:
        case SETNE:
        case SETNG:
        case SETNGE:
        case SETNL:
        case SETNLE:
        case SETNO:
        case SETNP:
        case SETNS:
        case SETNZ:
        case SETO:
        case SETP:
        case SETPE:
        case SETPO:
        case SETS:
        case SETZ:
            if (instr->operands().size() != 1) {
                throw core::arch::irgen::InvalidInstructionException("instruction takes exactly one operand");
            }
            break;
        case ADC:
        case ADD:
        case AND:
        case BOUND:
        case BT:
        case CMOVA:
        case CMOVAE:
        case CMOVB:
        case CMOVBE:
        case CMOVE:
        case CMOVG:
        case CMOVGE:
        case CMOVL:
        case CMOVLE:
        case CMOVNB:
        case CMOVNE:
        case CMOVNO:
        case CMOVNP:
        case CMOVNS:
        case CMOVNZ:
        case CMOVO:
        case CMOVP:
        case CMOVS:
        case CMOVZ:
        case CMP:
        case CMPXCHG:
        case LEA:
        case MOV:
        case MOVSX:
        case MOVZX:
        case SBB:
        case SUB:
        case TEST:
        case OR:
        case XCHG:
        case XOR:
            if (instr->operands().size() != 2) {
                throw core::arch::irgen::InvalidInstructionException("instruction takes exactly two operands");
            }
            break;
        case IDIV: /* Sometimes disassemblers put eax as the first operand. */
        case SAL:
        case SAR:
        case SHL:
        case SHR:
            if (instr->operands().size() != 1 && instr->operands().size() != 2) {
                throw core::arch::irgen::InvalidInstructionException("instruction takes one or two operands");
            }
            break;
        case IMUL:
            if (instr->operands().size() < 1 || instr->operands().size() > 3) {
                throw core::arch::irgen::InvalidInstructionException("instruction takes one, two, or three operands");
            }
            break;
        case NOP:
            /* No checks. */
            break;
    }

    core::ir::BasicBlock *cachedDirectSuccessor = NULL;
    auto directSuccessor = [&]() -> core::ir::BasicBlock * {
        if (!cachedDirectSuccessor) {
            cachedDirectSuccessor = program->createBasicBlock(instr->endAddr());
        }
        return cachedDirectSuccessor;
    };

    const IntelOperands *operands = mArchitecture->operands();

    IntelExpressionFactory factory(mArchitecture, instr);
    IntelExpressionFactoryCallback _(factory, program->getBasicBlockForInstruction(instr));

    /* Using '_' as variable name for expression factory callback to type less. 
     * Don't get surprised. And remember that expression trees are fed to
     * the callback via operator[]. */

    using namespace core::arch::irgen::expressions;

    /* Describing semantics */
    switch (instr->mnemonic()->number()) {
        case ADC: {
            _[
                operand(0) = operand(0) + operand(1) + cf(),
                cf() = intrinsic(),
                pf() = intrinsic(),
                zf() = operand(0) == constant(0),
                sf() = signed_(operand(0)) < constant(0),
                of() = intrinsic(),
                af() = intrinsic(),
                kill(pseudo_flags())
            ];
            break;
        }
        case ADD: {
            _[
                operand(0) = operand(0) + operand(1),
                cf() = intrinsic(),
                pf() = intrinsic(),
                zf() = operand(0) == constant(0),
                sf() = signed_(operand(0)) < constant(0),
                of() = intrinsic(),
                af() = intrinsic(),
                kill(pseudo_flags())
            ];
            break;
        }
        case AND: {
            if (instr->operand(0) == instr->operand(1)) {
                _[operand(0) = operand(0)];
            } else {
                _[operand(0) = operand(0) & operand(1)];
            }

            _[
                cf() = constant(0),
                pf() = intrinsic(),
                zf() = operand(0) == constant(0),
                sf() = intrinsic(),
                of() = constant(0),
                af() = undefined(),
                kill(pseudo_flags())
            ];
            break;
        }
        case BOUND: {
            /* Deprecated, used mostly for debugging, it's better to generate no IR code at all. */
            break;
        }
        case BT: {
            _[
                cf() = resize(unsigned_(operand(0)) >> operand(1)),
                of() = undefined(),
                sf() = undefined(),
                zf() = undefined(),
                af() = undefined(),
                pf() = undefined(),
                kill(pseudo_flags())
            ];
            break;
        }
        case CALL: {
            core::arch::RegisterOperand *sp = mArchitecture->registerOperand(mArchitecture->stackPointer());
            core::arch::RegisterOperand *ip = mArchitecture->registerOperand(mArchitecture->instructionPointer());

            _[
                operand(sp) = operand(sp) - constant(ip->size() / CHAR_BIT),
                *operand(sp) = operand(ip),
                call(operand(0))
            ];
            break;
        }
        case CBW: {
            _[
                operand(operands->ax()) = sign_extend(operand(operands->al()))
            ];
            break;
        }
        case CWDE: {
            _[
                operand(operands->eax()) = sign_extend(operand(operands->ax()))
            ];
            break;
        }
        case CDQE: {
            _[
                operand(operands->rax()) = sign_extend(operand(operands->eax()))
            ];
            break;
        }
        case CLD: {
            _[
                df() = constant(0)
            ];
            break;
        }
        case CMP: {
            _[
                cf() = intrinsic(),
                pf() = intrinsic(),
                zf() = operand(0) == operand(1),
                sf() = signed_(operand(0)) < operand(1),
                of() = intrinsic(),
                af() = intrinsic(),

                less()             = signed_(operand(0)) < operand(1),
                less_or_equal()    = signed_(operand(0)) <= operand(1),
                greater()          = signed_(operand(0)) > operand(1),
                greater_or_equal() = signed_(operand(0)) >= operand(1),
                below()            = unsigned_(operand(0)) < operand(1),
                below_or_equal()   = unsigned_(operand(0)) <= operand(1),
                above()            = unsigned_(operand(0)) > operand(1),
                above_or_equal()   = unsigned_(operand(0)) >= operand(1)
            ];
            break;
        }
        case CMPSB: case CMPSW: case CMPSD: case CMPSQ:
        case MOVSB: case MOVSW: case MOVSD: case MOVSQ:
        case SCASB: case SCASW: case SCASD: case SCASQ:
        case STOSB: case STOSW: case STOSD: case STOSQ: {
            SmallBitSize accessSize;

            switch (instr->mnemonic()->number()) {
                case CMPSB: case MOVSB: case SCASB: case STOSB:
                    accessSize = 8;
                    break;
                case CMPSW: case MOVSW: case SCASW: case STOSW:
                    accessSize = 16;
                    break;
                case CMPSD: case MOVSD: case SCASD: case STOSD:
                    accessSize = 32;
                    break;
                case CMPSQ: case MOVSQ: case SCASQ: case STOSQ:
                    accessSize = 64;
                    break;
                default:
                    unreachable();
            }

            auto di = resizedRegister(IntelRegisters::di(), instr->addressSize());
            auto si = resizedRegister(IntelRegisters::si(), instr->addressSize());
            auto cx = resizedRegister(IntelRegisters::cx(), instr->operandSize());

            auto increment = temporary(instr->addressSize());
            _[
                increment = constant(accessSize / CHAR_BIT) - constant(2 * accessSize / CHAR_BIT, si.memoryLocation().size()) * df()
            ];

            IntelExpressionFactoryCallback condition(factory, program->createBasicBlock());
            IntelExpressionFactoryCallback body(factory, program->createBasicBlock());

            _[jump(condition.basicBlock())];

            if (instr->prefixes() & (prefixes::REP | prefixes::REPZ | prefixes::REPNZ)) {
                condition[jump(cx, body.basicBlock(), directSuccessor())];

                body[cx = cx - constant(1)];
            } else {
                condition[jump(body.basicBlock())];
            }

            bool repPrefixIsValid = false;
            switch (instr->mnemonic()->number()) {
                case CMPSB: case CMPSW: case CMPSD: case CMPSQ: {
                    auto left = dereference(si, accessSize);
                    auto right = dereference(di, accessSize);

                    body[
                        cf() = intrinsic(),
                        pf() = intrinsic(),
                        zf() = left == right,
                        sf() = signed_(left) < right,
                        of() = intrinsic(),
                        af() = intrinsic(),

                        less()             = signed_(left) < right,
                        less_or_equal()    = signed_(left) <= right,
                        greater()          = signed_(left) > right,
                        greater_or_equal() = signed_(left) >= right,
                        below()            = unsigned_(left) < right,
                        below_or_equal()   = unsigned_(left) <= right,
                        above()            = unsigned_(left) > right,
                        above_or_equal()   = unsigned_(left) >= right
                    ];
                }
                case MOVSB: case MOVSW: case MOVSD: case MOVSQ: {
                    repPrefixIsValid = true;
                    body[dereference(di, accessSize) = *si];
                    break;
                }
                case SCASB: case SCASW: case SCASD: case SCASQ: {
                    auto left = dereference(di, accessSize);
                    auto right = resizedRegister(IntelRegisters::ax(), accessSize);

                    body[
                        cf() = intrinsic(),
                        pf() = intrinsic(),
                        zf() = left == right,
                        sf() = signed_(left) < right,
                        of() = intrinsic(),
                        af() = intrinsic(),

                        less()             = signed_(left) < right,
                        less_or_equal()    = signed_(left) <= right,
                        greater()          = signed_(left) > right,
                        greater_or_equal() = signed_(left) >= right,
                        below()            = unsigned_(left) < right,
                        below_or_equal()   = unsigned_(left) <= right,
                        above()            = unsigned_(left) > right,
                        above_or_equal()   = unsigned_(left) >= right
                    ];
                    break;
                }
                case STOSB: case STOSW: case STOSD: case STOSQ: {
                    repPrefixIsValid = true;
                    body[dereference(di, accessSize) = resizedRegister(IntelRegisters::ax(), accessSize)];
                    break;
                }
                default:
                    unreachable();
            }

            body[
                di = di + increment,
                si = si + increment
            ];

            /* libudis86 sets REP prefix together with REPZ/REPNZ. */
            if ((instr->prefixes() & prefixes::REP) && repPrefixIsValid) {
                body[jump(condition.basicBlock())];
            } else if (instr->prefixes() & prefixes::REPZ) {
                body[jump(zf(), condition.basicBlock(), directSuccessor())];
            } else if (instr->prefixes() & prefixes::REPNZ) {
                body[jump(!zf(), condition.basicBlock(), directSuccessor())];
            } else {
                body[jump(directSuccessor())];
            }
            break;
        }
        case CMPXCHG: {
            IntelExpressionFactoryCallback then(factory, program->createBasicBlock());

            _[
                cf() = intrinsic(),
                pf() = intrinsic(),
                zf() = operand(0) == operand(1),
                sf() = intrinsic(),
                of() = intrinsic(),
                af() = intrinsic(),
                kill(pseudo_flags()),

                jump(zf(), then.basicBlock(), directSuccessor())
            ];

            auto tmp = temporary(instr->operand(0)->size());
            then[
                tmp = operand(0),
                operand(0) = operand(1),
                operand(1) = tmp,
                jump(directSuccessor())
            ];

            break;
        }
        case CPUID: {
            _[
                operand(operands->eax())= intrinsic(),
                operand(operands->ebx())= intrinsic(),
                operand(operands->ecx())= intrinsic(),
                operand(operands->edx())= intrinsic()
            ];
            break;
        }
        case DEC: {
            _[
                operand(0) = operand(0) - constant(1),
                pf() = intrinsic(),
                zf() = operand(0) == constant(0),
                sf() = signed_(operand(0)) < constant(0),
                of() = intrinsic(),
                af() = intrinsic(),
                kill(pseudo_flags())
            ];
            break;
        }
        case DIV: {
            auto ax = resizedRegister(IntelRegisters::ax(), std::max(instr->operand(0)->size(), 16));
            auto dx = resizedRegister(IntelRegisters::dx(), std::max(instr->operand(0)->size(), 16));

            _[
                dx = unsigned_(ax) % operand(0),
                ax = unsigned_(ax) / operand(0),
                cf() = undefined(),
                of() = undefined(),
                sf() = undefined(),
                zf() = undefined(),
                af() = undefined(),
                pf() = undefined()
            ];
            break;
        }
        case IDIV: {
            /* `idiv ebx' or `idiv eax,ebx' */

            auto ax = resizedRegister(IntelRegisters::ax(), std::max(instr->operand(0)->size(), 16));
            auto dx = resizedRegister(IntelRegisters::dx(), std::max(instr->operand(0)->size(), 16));

            _[
                dx = signed_(ax) % operand(instr->operands().back()),
                ax = signed_(ax) / operand(instr->operands().back()),
                cf() = undefined(),
                of() = undefined(),
                sf() = undefined(),
                zf() = undefined(),
                af() = undefined(),
                pf() = undefined()
            ];
            break;
        }
        case INT: {
            if (const core::arch::ConstantOperand *constant = instr->operand(0)->asConstant()) {
                if (constant->value().value() == 3) {
                    /* int 3 is debug break, remove it. */
                    break;
                }
            }
            _(std::make_unique<core::ir::InlineAssembly>());
            break;
        }
        case INT3: {
            /* int 3 is a debug break, remove it. */
            break;
        }
        case IMUL: case MUL: {
            /* All compilers always use IMUL, not MUL. Even for unsigned operands.
             * See http://stackoverflow.com/questions/4039378/x86-mul-instruction-from-vs-2008-2010
             *
             * So, there is no such thing as signed or unsigned multiplication.
             */
            if (instr->operands().size() == 1) {
                /* result2:result1 = arg0 * op0 */
                core::arch::RegisterOperand *arg0;
                core::arch::RegisterOperand *result1;
                core::arch::RegisterOperand *result2;

                switch (instr->operand(0)->size()) {
                    case 8:
                        arg0 = operands->al();
                        result1 = operands->ax();
			result2 = 0;
                        break;
                    case 16:
                        arg0 = operands->ax();
                        result1 = operands->ax();
                        result2 = operands->dx();
                        break;
                    case 32:
                        arg0 = operands->eax();
                        result1 = operands->eax();
                        result2 = operands->edx();
                        break;
                    case 64:
                        arg0 = operands->rax();
                        result1 = operands->rax();
                        result2 = operands->rdx();
                        break;
                    default:
                        throw core::arch::irgen::InvalidInstructionException("strange argument size");
                }

                if (result1->size() == arg0->size()) {
                    _[operand(result1) = operand(arg0) * operand(0)];
                } else {
                    _[operand(result1) = sign_extend(operand(arg0)) * sign_extend(operand(0))];
                }
                if (result2) {
                    _[operand(result2) = intrinsic()];
                }
            } else if (instr->operands().size() == 2) {
                _[operand(0) = operand(0) * operand(1)];
            } else if (instr->operands().size() == 3) {
                _[operand(0) = operand(1) * operand(2)];
            }

            _[
                cf() = intrinsic(),
                of() = intrinsic(),
                sf() = undefined(),
                zf() = undefined(),
                af() = undefined(),
                pf() = undefined()
            ];
            break;
        }
        case INC: {
            _[
                operand(0) = operand(0) + constant(1),
                pf() = intrinsic(),
                zf() = operand(0) == constant(0),
                sf() = signed_(operand(0)) < constant(0),
                of() = intrinsic(),
                af() = intrinsic(),
                kill(pseudo_flags())
            ];
            break;
        }
        case JA: case JNBE: {
            _[jump(choice(above(), !cf() && !zf()), operand(0), directSuccessor())];
            break;
        }
        case JAE: case JNB: {
            _[jump(choice(above_or_equal(), !cf()), operand(0), directSuccessor())];
            break;
        }
        case JB: case JNAE: {
            _[jump(choice(below(), cf()), operand(0), directSuccessor())];
            break;
        }
        case JBE: case JNA: {
            _[jump(choice(below_or_equal(), cf() || zf()), operand(0), directSuccessor())];
            break;
        }
        case JC: {
            _[jump(cf(), operand(0), directSuccessor())];
            break;
        }
        case JCXZ: {
            _[jump(!cx(), operand(0), directSuccessor())];
            break;
        }
        case JECXZ: {
            _[jump(!ecx(), operand(0), directSuccessor())];
            break;
        }
        case JRCXZ: {
            _[jump(!rcx(), operand(0), directSuccessor())];
            break;
        }
        case JE: case JZ: {
            _[jump(zf(), operand(0), directSuccessor())];
            break;
        }
        case JG: case JNLE: {
            _[jump(choice(greater(), !zf() || sf() == of()), operand(0), directSuccessor())];
            break;
        }
        case JGE: case JNL: {
            _[jump(choice(greater_or_equal(), sf() == of()), operand(0), directSuccessor())];
            break;
        }
        case JL: case JNGE: {
            _[jump(choice(less(), !(sf() == of())), operand(0), directSuccessor())];
            break;
        }
        case JLE: case JNG: {
            _[jump(choice(less_or_equal(), zf() || !(sf() == of())), operand(0), directSuccessor())];
            break;
        }
        case JNC: {
            _[jump(!cf(), operand(0), directSuccessor())];
            break;
        }
        case JNE: case JNZ: {
            _[jump(!zf(), operand(0), directSuccessor())];
            break;
        }
        case JNO: {
            _[jump(!of(), operand(0), directSuccessor())];
            break;
        }
        case JNP: case JPO: {
            _[jump(!pf(), operand(0), directSuccessor())];
            break;
        }
        case JNS: {
            _[jump(!sf(), operand(0), directSuccessor())];
            break;
        }
        case JO: {
            _[jump(of(), operand(0), directSuccessor())];
            break;
        }
        case JP: case JPE: {
            _[jump(pf(), operand(0), directSuccessor())];
            break;
        }
        case JS: {
            _[jump(sf(), operand(0), directSuccessor())];
            break;
        }
        case JMP: {
            _[jump(operand(0))];
            break;
        }
        case CMOVA: case CMOVAE: case CMOVB: case CMOVBE: case CMOVE:
        case CMOVG: case CMOVGE: case CMOVL: case CMOVLE: case CMOVNB:
        case CMOVNE: case CMOVNO: case CMOVNP: case CMOVNS: case CMOVNZ:
        case CMOVO: case CMOVP: case CMOVS: case CMOVZ: {
            IntelExpressionFactoryCallback then(factory, program->createBasicBlock());

            switch (instruction->mnemonic()->number()) {
                case CMOVA:
                    _[jump(choice(above(), !cf() && !zf()), then.basicBlock(), directSuccessor())]; break;
                case CMOVAE: case CMOVNB:
                    _[jump(choice(above_or_equal(), !cf()), then.basicBlock(), directSuccessor())]; break;
                case CMOVB:
                    _[jump(choice(below(), cf()), then.basicBlock(), directSuccessor())]; break;
                case CMOVBE:
                    _[jump(choice(below_or_equal(), cf() || zf()), then.basicBlock(), directSuccessor())]; break;
                case CMOVE: case CMOVZ:
                    _[jump(zf(), then.basicBlock(), directSuccessor())]; break;
                case CMOVG:
                    _[jump(choice(greater(), !zf() || sf() == of()), then.basicBlock(), directSuccessor())]; break;
                case CMOVGE:
                    _[jump(choice(greater_or_equal(), sf() == of()), then.basicBlock(), directSuccessor())]; break;
                case CMOVL:
                    _[jump(choice(less(), !(sf() == of())), then.basicBlock(), directSuccessor())]; break;
                case CMOVLE:
                    _[jump(choice(less_or_equal(), zf() || !(sf() == of())), then.basicBlock(), directSuccessor())]; break;
                case CMOVNE: case CMOVNZ:
                    _[jump(!zf(), then.basicBlock(), directSuccessor())]; break;
                case CMOVNO:
                    _[jump(!of(), then.basicBlock(), directSuccessor())]; break;
                case CMOVNP:
                    _[jump(!pf(), then.basicBlock(), directSuccessor())]; break;
                case CMOVNS:
                    _[jump(!sf(), then.basicBlock(), directSuccessor())]; break;
                case CMOVO:
                    _[jump(of(), then.basicBlock(), directSuccessor())]; break;
                case CMOVP:
                    _[jump(pf(), then.basicBlock(), directSuccessor())]; break;
                case CMOVS:
                    _[jump(sf(), then.basicBlock(), directSuccessor())]; break;
                default: unreachable();
            }

            then[
                operand(0) = operand(1),
                jump(directSuccessor())
            ];

            break;
        }
        case LEA: {
            if (const core::arch::DereferenceOperand *dereference = instr->operand(1)->asDereference()) {
                if (instr->operand(0)->size() == dereference->operand()->size()) {
                    _[operand(0) = operand(dereference->operand())];
                } else {
                    _[operand(0) = resize(operand(dereference->operand()))];
                }
            } else {
                throw core::arch::irgen::InvalidInstructionException("lea's second argument must be a memory operand");
            }
            break;
        }
        case LEAVEW: case LEAVE: case LEAVED: case LEAVEQ: {
            core::arch::RegisterOperand *sp = mArchitecture->registerOperand(mArchitecture->stackPointer());
            core::arch::RegisterOperand *bp = mArchitecture->registerOperand(mArchitecture->basePointer());

            _[
                operand(sp) = operand(bp),
                operand(bp) = *operand(sp),
                operand(sp) = operand(sp) + constant(bp->size() / CHAR_BIT)
            ];
            break;
        }
        case LOOP:
        case LOOPE:
        case LOOPNE: {
            auto cx = resizedRegister(IntelRegisters::cx(), instr->addressSize());

            _[cx = cx - constant(1)];

            switch (instr->mnemonic()->number()) {
                case LOOP:
                    _[jump(cx, operand(0), directSuccessor())];
                    break;
                case LOOPE:
                    _[jump(cx && zf(), operand(0), directSuccessor())];
                    break;
                case LOOPNE:
                    _[jump(cx && !zf(), operand(0), directSuccessor())];
                    break;
                default:
                    unreachable();
            }
            break;
        }
        case MOV: {
            if (instr->operand(0)->size() == instr->operand(1)->size()) {
                _[operand(0) = operand(1)];
            } else if (instr->operand(0)->size() > instr->operand(1)->size()) {
                /* For example, move of a small constant to a big register. */
                _[operand(0) = zero_extend(operand(1))];
            } else {
                /* Happens in assignments to segment registers. Known bug of udis86. */
                _[operand(0) = resize(operand(1))];
            }
            break;
        }
        case MOVSX: case MOVSXD: {
            _[operand(0) = sign_extend(operand(1))];
            break;
        }
        case MOVZX: {
            _[operand(0) = zero_extend(operand(1))];
            break;
        }
        case NEG: {
            _[
                cf() = !(operand(0) == constant(0)),
                operand(0) = -operand(0),
                pf() = intrinsic(),
                zf() = operand(0) == constant(0),
                sf() = intrinsic(),
                of() = intrinsic(),
                af() = intrinsic(),
                kill(pseudo_flags())
            ];
            break;
        }
        case NOP: {
            break;
        }
        case NOT: {
            _[operand(0) = ~operand(0)];
            break;
        }
        case OR: {
            if (instr->operand(0) == instr->operand(1)) {
                _[operand(0) = operand(0)];
            } else {
                _[operand(0) = operand(0) | operand(1)];
            }

            _[
                cf() = constant(0),
                pf() = intrinsic(),
                zf() = operand(0) == constant(0),
                sf() = intrinsic(),
                of() = constant(0),
                af() = undefined(),
                kill(pseudo_flags())
            ];
            break;
        }
        case POP: {
            core::arch::RegisterOperand *sp = mArchitecture->registerOperand(mArchitecture->stackPointer());
            _[
                operand(0) = *operand(sp),
                operand(sp) = operand(sp) + constant(instr->operand(0)->size() / CHAR_BIT)
            ];
            break;
        }
        case PUSH: {
            core::arch::RegisterOperand *sp = mArchitecture->registerOperand(mArchitecture->stackPointer());
            _[
                operand(sp) = operand(sp) - constant(instr->operand(0)->size() / CHAR_BIT),
                *operand(sp) = operand(0)
            ];
            break;
        }
        case POPFW: {
            core::arch::RegisterOperand *sp = mArchitecture->registerOperand(mArchitecture->stackPointer());
            _[
                *operand(sp) = flags(),
                operand(sp) = operand(sp) + constant(2)
            ];
            break;
        }
        case POPFD: {
            core::arch::RegisterOperand *sp = mArchitecture->registerOperand(mArchitecture->stackPointer());
            _[
                eflags() = *operand(sp),
                operand(sp) = operand(sp) + constant(4)
            ];
            break;
        }
        case POPFQ: {
            core::arch::RegisterOperand *sp = mArchitecture->registerOperand(mArchitecture->stackPointer());
            _[
                rflags() = *operand(sp),
                operand(sp) = operand(sp) + constant(8)
            ];
            break;
        }
        case PUSHFW: {
            core::arch::RegisterOperand *sp = mArchitecture->registerOperand(mArchitecture->stackPointer());
            _[
                operand(sp) = operand(sp) - constant(2),
                *operand(sp) = flags()
            ];
            break;
        }
        case PUSHFD: {
            core::arch::RegisterOperand *sp = mArchitecture->registerOperand(mArchitecture->stackPointer());
            _[
                operand(sp) = operand(sp) - constant(4),
                *operand(sp) = eflags() & constant(0x00fcffff)
            ];
            break;
        }
        case PUSHFQ: {
            core::arch::RegisterOperand *sp = mArchitecture->registerOperand(mArchitecture->stackPointer());
            _[
                operand(sp) = operand(sp) - constant(8),
                *operand(sp) = rflags() & constant(0x00fcffff)
            ];
            break;
        }
        case RET: {
            core::arch::RegisterOperand *sp = mArchitecture->registerOperand(mArchitecture->stackPointer());
            core::arch::RegisterOperand *ip = mArchitecture->registerOperand(mArchitecture->instructionPointer());

            _[
                operand(ip) = *operand(sp),
                operand(sp) = operand(sp) + constant(ip->size() / CHAR_BIT)
            ];

            if (instr->operands().size() == 1) {
                _[operand(sp) = operand(sp) + operand(0)];
            }

            _[return_()];
            break;
        }
        case SAL: case SHL: {
            if (instr->operands().size() == 1) {
                _[operand(0) = operand(0) << constant(1)];
            } else {
                _[operand(0) = operand(0) << operand(1)];
            }

            _[
                cf() = intrinsic(),
                sf() = undefined(),
                zf() = operand(0) == constant(0),
                pf() = intrinsic()
            ];
            break;
        }
        case SAR: {
            if (instr->operands().size() == 1) {
                _[operand(0) = signed_(operand(0)) >> constant(1)];
            } else {
                _[operand(0) = signed_(operand(0)) >> operand(1)];
            }

            _[
                cf() = intrinsic(),
                sf() = undefined(),
                zf() = operand(0) == constant(0),
                pf() = intrinsic()
            ];
            break;
        }
        case SBB: {
            _[
                less()             = signed_(operand(0))   <  operand(1) + cf(),
                less_or_equal()    = signed_(operand(0))   <= operand(1) + cf(),
                greater()          = signed_(operand(0))   >  operand(1) + cf(),
                greater_or_equal() = signed_(operand(0))   >= operand(1) + cf(),
                below()            = unsigned_(operand(0)) <  operand(1) + cf(),
                below_or_equal()   = unsigned_(operand(0)) <= operand(1) + cf(),
                above()            = unsigned_(operand(0)) >  operand(1) + cf(),
                above_or_equal()   = unsigned_(operand(0)) >= operand(1) + cf(),

                operand(0) = operand(0) - (operand(1) + cf()),

                cf() = intrinsic(),
                pf() = intrinsic(),
                zf() = operand(0) == constant(0) + cf(),
                sf() = signed_(operand(0)) < constant(0) + cf(),
                of() = intrinsic(),
                af() = intrinsic()
            ];
            break;
        }
        case SETA: case SETNBE: {
            _[operand(0) = zero_extend(choice(above(), !cf() && !zf()))];
            break;
        }
        case SETAE: case SETNB: {
            _[operand(0) = zero_extend(choice(above_or_equal(), !cf()))];
            break;
        }
        case SETB: case SETNAE: {
            _[operand(0) = zero_extend(choice(below(), cf()))];
            break;
        }
        case SETBE: case SETNA: {
            _[operand(0) = zero_extend(choice(below_or_equal(), cf() || zf()))];
            break;
        }
        case SETC: {
            _[operand(0) = zero_extend(cf())];
            break;
        }
        case SETE: case SETZ: {
            _[operand(0) = zero_extend(zf())];
            break;
        }
        case SETG: case SETNLE: {
            _[operand(0) = zero_extend(choice(greater(), !zf() || sf() == of()))];
            break;
        }
        case SETGE: case SETNL: {
            _[operand(0) = zero_extend(choice(greater_or_equal(), sf() == of()))];
            break;
        }
        case SETL: case SETNGE: {
            _[operand(0) = zero_extend(choice(less(), !(sf() == of())))];
            break;
        }
        case SETLE: case SETNG: {
            _[operand(0) = zero_extend(choice(less_or_equal(), zf() || !(sf() == of())))];
            break;
        }
        case SETNC: {
            _[operand(0) = zero_extend(!cf())];
            break;
        }
        case SETNE: case SETNZ: {
            _[operand(0) = zero_extend(!zf())];
            break;
        }
        case SETNO: {
            _[operand(0) = zero_extend(!of())];
            break;
        }
        case SETNP: case SETPO: {
            _[operand(0) = zero_extend(!pf())];
            break;
        }
        case SETNS: {
            _[operand(0) = zero_extend(!sf())];
            break;
        }
        case SETO: {
            _[operand(0) = zero_extend(of())];
            break;
        }
        case SETP: case SETPE: {
            _[operand(0) = zero_extend(pf())];
            break;
        }
        case SETS: {
            _[operand(0) = zero_extend(sf())];
            break;
        }
        case SHR: {
            if(instr->operands().size() == 1) {
                _[operand(0) = unsigned_(operand(0)) >> constant(1)];
            } else {
                _[operand(0) = unsigned_(operand(0)) >> operand(1)];
            }

            _[
                cf() = intrinsic(),
                sf() = undefined(),
                zf() = operand(0) == constant(0),
                pf() = intrinsic()
            ];
            break;
        }
        case STD: {
            _[
                df() = constant(1)
            ];
            break;
        }
        case SUB: {
            _[
                less()             = signed_(operand(0)) < operand(1),
                less_or_equal()    = signed_(operand(0)) <= operand(1),
                greater()          = signed_(operand(0)) > operand(1),
                greater_or_equal() = signed_(operand(0)) >= operand(1),
                below()            = unsigned_(operand(0)) < operand(1),
                below_or_equal()   = unsigned_(operand(0)) <= operand(1),
                above()            = unsigned_(operand(0)) > operand(1),
                above_or_equal()   = unsigned_(operand(0)) >= operand(1),

                operand(0) = operand(0) - operand(1),

                cf() = intrinsic(),
                pf() = intrinsic(),
                zf() = operand(0) == constant(0),
                sf() = signed_(operand(0)) < constant(0),
                of() = intrinsic(),
                af() = intrinsic()
            ];
            break;
        }
        case TEST: {
            _[
                cf() = constant(0),
                pf() = intrinsic()
            ];

            if (instr->operand(0) == instr->operand(1)) {
                _[zf() = operand(0) == constant(0)];
            } else {
                _[zf() = (operand(0) & operand(1)) == constant(0)];
            }

            _[
                sf() = intrinsic(),
                of() = constant(0),
                af() = undefined(),
                kill(pseudo_flags())
            ];
            break;
        }
        case XCHG: {
            auto tmp = temporary(instr->operand(0)->size());

            _[
                tmp = operand(0),
                operand(0) = operand(1),
                operand(1) = tmp
            ];
            break;
        }
        case XOR: {
            if (instr->operand(0) == instr->operand(1)) {
                _[operand(0) = constant(0)];
            } else {
                _[operand(0) = operand(0) ^ operand(1)];
            }

            _[
                cf() = constant(0),
                pf() = intrinsic(),
                zf() = operand(0) == constant(0),
                sf() = intrinsic(),
                of() = constant(0),
                af() = undefined(),
                kill(pseudo_flags())
            ];
            break;
        }
        default: {
            _(std::make_unique<core::ir::InlineAssembly>());
            break;
        }
    }
}

std::unique_ptr<core::ir::Term> IntelInstructionAnalyzer::doCreateTerm(const core::arch::Operand *operand) const {
    const IntelOperands *operands = mArchitecture->operands();
    
    switch (operand->kind()) {
    case IntelOperands::FPU_STACK: {
        const FpuStackOperand *fpuStack = operand->as<FpuStackOperand>();

        return std::make_unique<core::ir::Dereference>(
            std::make_unique<core::ir::BinaryOperator>(
                core::ir::BinaryOperator::MUL,
                std::make_unique<core::ir::BinaryOperator>(
                    core::ir::BinaryOperator::ADD,
                    createTerm(operands->fpu_top()),
                    std::make_unique<core::ir::Constant>(SizedValue(fpuStack->index(), 32))
                ),
                std::make_unique<core::ir::Constant>(SizedValue(operands->fpu_r0()->memoryLocation().size(), 32))
            ),
            operands->fpu_r0()->memoryLocation().domain(),
            operands->fpu_r0()->size()
        );
    }
    default: 
        return core::arch::irgen::InstructionAnalyzer::doCreateTerm(operand);
    }
}

} // namespace intel
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
