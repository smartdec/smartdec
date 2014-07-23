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

#include "IntelInstructionAnalyzer.h"

#include <libudis86/udis86.h>

#include <nc/common/CheckedCast.h>
#include <nc/common/Unreachable.h>

#include <nc/core/arch/Operand.h>
#include <nc/core/arch/Operands.h>
#include <nc/core/arch/irgen/Expressions.h>
#include <nc/core/arch/irgen/InvalidInstructionException.h>
#include <nc/core/ir/Program.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>

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


void IntelInstructionAnalyzer::doCreateStatements(const core::arch::Instruction *instruction, core::ir::Program *program) const {
    assert(instruction != NULL);

    const IntelInstruction *instr = checked_cast<const IntelInstruction *>(instruction);

    ud_t ud_obj;
    ud_init(&ud_obj);
    ud_set_mode(&ud_obj, architecture_->bitness());
    ud_set_pc(&ud_obj, instr->addr());
    ud_set_input_buffer(&ud_obj, const_cast<uint8_t *>(instr->bytes()), checked_cast<std::size_t>(instr->size()));
    ud_disassemble(&ud_obj);

    assert(ud_obj.mnemonic != UD_Iinvalid);

    core::ir::BasicBlock *cachedDirectSuccessor = NULL;
    auto directSuccessor = [&]() -> core::ir::BasicBlock * {
        if (!cachedDirectSuccessor) {
            cachedDirectSuccessor = program->createBasicBlock(instr->endAddr());
        }
        return cachedDirectSuccessor;
    };

    IntelExpressionFactory factory(architecture_, instr);
    IntelExpressionFactoryCallback _(factory, program->getBasicBlockForInstruction(instr));

    /* Sanity checks */
    switch (ud_obj.mnemonic) {
        case UD_Icld:
        case UD_Icbw:
        case UD_Icwde:
        case UD_Icdqe:
        case UD_Icmpsb:
        case UD_Icmpsw:
        case UD_Icmpsd:
        case UD_Icmpsq:
        case UD_Icpuid:
        case UD_Iint3:
        case UD_Ileave:
        case UD_Imovsb:
        case UD_Imovsw:
        case UD_Imovsd:
        case UD_Imovsq:
        case UD_Ipopfw:
        case UD_Ipopfd:
        case UD_Ipopfq:
        case UD_Ipushfw:
        case UD_Ipushfd:
        case UD_Ipushfq:
        case UD_Iscasb:
        case UD_Iscasw:
        case UD_Iscasd:
        case UD_Iscasq:
        case UD_Istd:
        case UD_Istosb:
        case UD_Istosw:
        case UD_Istosd:
        case UD_Istosq:
            if (instr->operands().size() != 0) {
                throw core::arch::irgen::InvalidInstructionException("instruction takes no arguments");
            }
            break;
        case UD_Iret:
            if(instr->operands().size() > 1) {
                throw core::arch::irgen::InvalidInstructionException("instruction takes zero or one argument");
            }
            break;
        case UD_Icall:
        case UD_Idec:
        case UD_Idiv:
        case UD_Iidiv:
        case UD_Iinc:
        case UD_Iint:
        case UD_Ija:
        case UD_Ijae:
        case UD_Ijb:
        case UD_Ijbe:
        case UD_Ijcxz:
        case UD_Ijecxz:
        case UD_Ijg:
        case UD_Ijge:
        case UD_Ijl:
        case UD_Ijle:
        case UD_Ijmp:
        case UD_Ijno:
        case UD_Ijnp:
        case UD_Ijns:
        case UD_Ijnz:
        case UD_Ijo:
        case UD_Ijp:
        case UD_Ijrcxz:
        case UD_Ijs:
        case UD_Ijz:
        case UD_Iloop:
        case UD_Iloope:
        case UD_Iloopnz:
        case UD_Imul:
        case UD_Ineg:
        case UD_Inot:
        case UD_Ipop:
        case UD_Ipush:
        case UD_Iseta:
        case UD_Isetb:
        case UD_Isetbe:
        case UD_Isetg:
        case UD_Isetge:
        case UD_Isetl:
        case UD_Isetle:
        case UD_Isetnb:
        case UD_Isetno:
        case UD_Isetnp:
        case UD_Isetns:
        case UD_Isetnz:
        case UD_Iseto:
        case UD_Isetp:
        case UD_Isets:
        case UD_Isetz:
            if (instr->operands().size() != 1) {
                throw core::arch::irgen::InvalidInstructionException("instruction takes exactly one operand");
            }
            break;
        case UD_Iadc:
        case UD_Iadd:
        case UD_Iand:
        case UD_Ibound:
        case UD_Ibt:
        case UD_Icmova:
        case UD_Icmovae:
        case UD_Icmovb:
        case UD_Icmovbe:
        case UD_Icmovg:
        case UD_Icmovge:
        case UD_Icmovl:
        case UD_Icmovle:
        case UD_Icmovno:
        case UD_Icmovnp:
        case UD_Icmovns:
        case UD_Icmovnz:
        case UD_Icmovo:
        case UD_Icmovp:
        case UD_Icmovs:
        case UD_Icmovz:
        case UD_Icmp:
        case UD_Icmpxchg:
        case UD_Ilea:
        case UD_Imov:
        case UD_Imovsx:
        case UD_Imovzx:
        case UD_Ior:
        case UD_Isbb:
        case UD_Isub:
        case UD_Itest:
        case UD_Ixchg:
        case UD_Ixor:
            if (instr->operands().size() != 2) {
                throw core::arch::irgen::InvalidInstructionException("instruction takes exactly two operands");
            }
            break;
        case UD_Isar:
        case UD_Ishl:
        case UD_Ishr:
            if (instr->operands().size() != 1 && instr->operands().size() != 2) {
                throw core::arch::irgen::InvalidInstructionException("instruction takes one or two operands");
            }
            break;
        case UD_Iimul:
            if (instr->operands().size() < 1 || instr->operands().size() > 3) {
                throw core::arch::irgen::InvalidInstructionException("instruction takes one, two, or three operands");
            }
            break;
        case UD_Inop:
            /* No checks. */
            break;
        default: {
            /* Unsupported instruction. */
            _(std::make_unique<core::ir::InlineAssembly>());
            return;
        }
    }

    using namespace core::arch::irgen::expressions;

    /* Describing semantics */
    switch (ud_obj.mnemonic) {
        case UD_Iadc: {
            _[
                operand(0) = operand(0) + operand(1) + zero_extend(cf()),
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
        case UD_Iadd: {
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
        case UD_Iand: {
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
        case UD_Ibound: {
            /* Deprecated, used mostly for debugging, it's better to generate no IR code at all. */
            break;
        }
        case UD_Ibt: {
            _[
                cf() = truncate(unsigned_(operand(0)) >> operand(1)),
                of() = undefined(),
                sf() = undefined(),
                zf() = undefined(),
                af() = undefined(),
                pf() = undefined(),
                kill(pseudo_flags())
            ];
            break;
        }
        case UD_Icall: {
            auto sp = architecture_->stackPointer();
            auto ip = architecture_->instructionPointer();

            _[
                regizter(sp) = regizter(sp) - constant(ip->size() / CHAR_BIT),
                *regizter(sp) = regizter(ip),
                call(operand(0)),
                regizter(sp) = regizter(sp) + constant(ip->size() / CHAR_BIT)
            ];
            break;
        }
        case UD_Icbw: {
            _[
                regizter(IntelRegisters::ax()) = sign_extend(regizter(IntelRegisters::al()))
            ];
            break;
        }
        case UD_Icwde: {
            _[
                regizter(IntelRegisters::eax()) = sign_extend(regizter(IntelRegisters::ax()))
            ];
            break;
        }
        case UD_Icdqe: {
            _[
                regizter(IntelRegisters::rax()) = sign_extend(regizter(IntelRegisters::eax()))
            ];
            break;
        }
        case UD_Icld: {
            _[
                df() = constant(0)
            ];
            break;
        }
        case UD_Icmp: {
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
        case UD_Icmpsb: case UD_Icmpsw: case UD_Icmpsd: case UD_Icmpsq:
        case UD_Imovsb: case UD_Imovsw: case UD_Imovsd: case UD_Imovsq:
        case UD_Iscasb: case UD_Iscasw: case UD_Iscasd: case UD_Iscasq:
        case UD_Istosb: case UD_Istosw: case UD_Istosd: case UD_Istosq: {
            SmallBitSize accessSize;

            switch (ud_obj.mnemonic) {
                case UD_Icmpsb: case UD_Imovsb: case UD_Iscasb: case UD_Istosb:
                    accessSize = 8;
                    break;
                case UD_Icmpsw: case UD_Imovsw: case UD_Iscasw: case UD_Istosw:
                    accessSize = 16;
                    break;
                case UD_Icmpsd: case UD_Imovsd: case UD_Iscasd: case UD_Istosd:
                    accessSize = 32;
                    break;
                case UD_Icmpsq: case UD_Imovsq: case UD_Iscasq: case UD_Istosq:
                    accessSize = 64;
                    break;
                default:
                    unreachable();
            }

            auto di = resizedRegister(IntelRegisters::di(), ud_obj.adr_mode);
            auto si = resizedRegister(IntelRegisters::si(), ud_obj.adr_mode);
            auto cx = resizedRegister(IntelRegisters::cx(), ud_obj.opr_mode);

            auto increment = temporary(ud_obj.adr_mode);
            _[
                increment = constant(accessSize / CHAR_BIT) - constant(2 * accessSize / CHAR_BIT, si.memoryLocation().size()) * zero_extend(df())
            ];

            IntelExpressionFactoryCallback condition(factory, program->createBasicBlock());
            IntelExpressionFactoryCallback body(factory, program->createBasicBlock());

            _[jump(condition.basicBlock())];

            if (ud_obj.pfx_rep != UD_NONE || ud_obj.pfx_repe != UD_NONE || ud_obj.pfx_repne != UD_NONE) {
                condition[jump(cx, body.basicBlock(), directSuccessor())];

                body[cx = cx - constant(1)];
            } else {
                condition[jump(body.basicBlock())];
            }

            bool repPrefixIsValid = false;
            switch (ud_obj.mnemonic) {
                case UD_Icmpsb: case UD_Icmpsw: case UD_Icmpsd: case UD_Icmpsq: {
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
                case UD_Imovsb: case UD_Imovsw: case UD_Imovsd: case UD_Imovsq: {
                    repPrefixIsValid = true;
                    body[dereference(di, accessSize) = *si];
                    break;
                }
                case UD_Iscasb: case UD_Iscasw: case UD_Iscasd: case UD_Iscasq: {
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
                case UD_Istosb: case UD_Istosw: case UD_Istosd: case UD_Istosq: {
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
            if (ud_obj.pfx_rep != UD_NONE && repPrefixIsValid) {
                body[jump(condition.basicBlock())];
            } else if (ud_obj.pfx_repe != UD_NONE) {
                body[jump(zf(), condition.basicBlock(), directSuccessor())];
            } else if (ud_obj.pfx_repne != UD_NONE) {
                body[jump(~zf(), condition.basicBlock(), directSuccessor())];
            } else {
                body[jump(directSuccessor())];
            }
            break;
        }
        case UD_Icmpxchg: {
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
        case UD_Icpuid: {
            _[
                regizter(IntelRegisters::eax()) = intrinsic(),
                regizter(IntelRegisters::ebx()) = intrinsic(),
                regizter(IntelRegisters::ecx()) = intrinsic(),
                regizter(IntelRegisters::edx()) = intrinsic()
            ];
            break;
        }
        case UD_Idec: {
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
        case UD_Idiv: {
            auto size = std::max(instr->operand(0)->size(), 16);
            auto ax = resizedRegister(IntelRegisters::ax(), size);
            auto dx = resizedRegister(IntelRegisters::dx(), size);

            if (instr->operand(0)->size() >= 16) {
                _[
                    dx = unsigned_(ax) % operand(0),
                    ax = unsigned_(ax) / operand(0)
                ];
            } else {
                _[
                    dx = unsigned_(ax) % sign_extend(operand(0)),
                    ax = unsigned_(ax) / sign_extend(operand(0))
                ];
            }
            break;
        }
        case UD_Iidiv: {
#if 0
            /* A correct implementation, however, generating complicated code. */

            auto size = std::max(instr->operand(0)->size(), 16);
            auto ax = resizedRegister(IntelRegisters::ax(), size);
            auto dx = resizedRegister(IntelRegisters::dx(), size);
            auto tmp = temporary(size * 2);

            _[
                tmp = (zero_extend(dx, size * 2) << constant(size)) | zero_extend(ax, size * 2),
                dx = truncate(signed_(tmp) % sign_extend(operand(0))),
                ax = truncate(signed_(tmp) / sign_extend(operand(0)))
            ];
#endif

            auto size = std::max(instr->operand(0)->size(), 16);
            auto ax = resizedRegister(IntelRegisters::ax(), size);
            auto dx = resizedRegister(IntelRegisters::dx(), size);

            if (instr->operand(0)->size() >= 16) {
                _[
                    dx = signed_(ax) % operand(0),
                    ax = signed_(ax) / operand(0)
                ];
            } else {
                _[
                    dx = signed_(ax) % sign_extend(operand(0)),
                    ax = signed_(ax) / sign_extend(operand(0))
                ];
            }

            _[
                cf() = undefined(),
                of() = undefined(),
                sf() = undefined(),
                zf() = undefined(),
                af() = undefined(),
                pf() = undefined()
            ];
            break;
        }
        case UD_Iint: {
            if (const core::arch::ConstantOperand *constant = instr->operand(0)->asConstant()) {
                if (constant->value().value() == 3) {
                    /* int 3 is debug break, remove it. */
                    break;
                }
            }
            _(std::make_unique<core::ir::InlineAssembly>());
            break;
        }
        case UD_Iint3: {
            /* int 3 is a debug break, remove it. */
            break;
        }
        case UD_Iimul: case UD_Imul: {
            /* All compilers always use IMUL, not MUL. Even for unsigned operands.
             * See http://stackoverflow.com/questions/4039378/x86-mul-instruction-from-vs-2008-2010
             *
             * So, there is no such thing as signed or unsigned multiplication.
             */
            if (instr->operands().size() == 1) {
                /* result2:result1 = arg0 * op0 */
                const core::arch::Register *arg0;
                const core::arch::Register *result1;
                const core::arch::Register *result2;

                switch (instr->operand(0)->size()) {
                    case 8:
                        arg0 = IntelRegisters::al();
                        result1 = IntelRegisters::ax();
			result2 = 0;
                        break;
                    case 16:
                        arg0 = IntelRegisters::ax();
                        result1 = IntelRegisters::ax();
                        result2 = IntelRegisters::dx();
                        break;
                    case 32:
                        arg0 = IntelRegisters::eax();
                        result1 = IntelRegisters::eax();
                        result2 = IntelRegisters::edx();
                        break;
                    case 64:
                        arg0 = IntelRegisters::rax();
                        result1 = IntelRegisters::rax();
                        result2 = IntelRegisters::rdx();
                        break;
                    default:
                        throw core::arch::irgen::InvalidInstructionException("strange argument size");
                }

                if (result1->size() == arg0->size()) {
                    _[regizter(result1) = regizter(arg0) * operand(0)];
                } else {
                    _[regizter(result1) = sign_extend(regizter(arg0)) * sign_extend(operand(0))];
                }
                if (result2) {
                    _[regizter(result2) = intrinsic()];
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
        case UD_Iinc: {
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
        case UD_Ija: {
            _[jump(choice(above(), ~cf() & ~zf()), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijae: {
            _[jump(choice(above_or_equal(), ~cf()), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijb: {
            _[jump(choice(below(), cf()), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijbe: {
            _[jump(choice(below_or_equal(), cf() | zf()), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijcxz: {
            _[jump(~(cx() == constant(0)), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijecxz: {
            _[jump(~(ecx() == constant(0)), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijrcxz: {
            _[jump(~(rcx() == constant(0)), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijz: {
            _[jump(zf(), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijg: {
            _[jump(choice(greater(), ~zf() | (sf() == of())), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijge: {
            _[jump(choice(greater_or_equal(), sf() == of()), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijl: {
            _[jump(choice(less(), ~(sf() == of())), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijle: {
            _[jump(choice(less_or_equal(), zf() | ~(sf() == of())), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijnz: {
            _[jump(~zf(), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijno: {
            _[jump(~of(), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijnp: {
            _[jump(~pf(), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijns: {
            _[jump(~sf(), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijo: {
            _[jump(of(), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijp: {
            _[jump(pf(), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijs: {
            _[jump(sf(), operand(0), directSuccessor())];
            break;
        }
        case UD_Ijmp: {
            _[jump(operand(0))];
            break;
        }
        case UD_Icmova: case UD_Icmovae: case UD_Icmovb: case UD_Icmovbe: 
        case UD_Icmovz: case UD_Icmovg: case UD_Icmovge: case UD_Icmovl:
        case UD_Icmovle: case UD_Icmovnz: case UD_Icmovno: case UD_Icmovnp:
        case UD_Icmovns: case UD_Icmovo: case UD_Icmovp: case UD_Icmovs: {
            IntelExpressionFactoryCallback then(factory, program->createBasicBlock());

            switch (ud_obj.mnemonic) {
                case UD_Icmova:
                    _[jump(choice(above(), ~cf() & ~zf()), then.basicBlock(), directSuccessor())]; break;
                case UD_Icmovae:
                    _[jump(choice(above_or_equal(), ~cf()), then.basicBlock(), directSuccessor())]; break;
                case UD_Icmovb:
                    _[jump(choice(below(), cf()), then.basicBlock(), directSuccessor())]; break;
                case UD_Icmovbe:
                    _[jump(choice(below_or_equal(), cf() | zf()), then.basicBlock(), directSuccessor())]; break;
                case UD_Icmovz:
                    _[jump(zf(), then.basicBlock(), directSuccessor())]; break;
                case UD_Icmovg:
                    _[jump(choice(greater(), ~zf() | (sf() == of())), then.basicBlock(), directSuccessor())]; break;
                case UD_Icmovge:
                    _[jump(choice(greater_or_equal(), sf() == of()), then.basicBlock(), directSuccessor())]; break;
                case UD_Icmovl:
                    _[jump(choice(less(), ~(sf() == of())), then.basicBlock(), directSuccessor())]; break;
                case UD_Icmovle:
                    _[jump(choice(less_or_equal(), zf() | ~(sf() == of())), then.basicBlock(), directSuccessor())]; break;
                case UD_Icmovnz:
                    _[jump(~zf(), then.basicBlock(), directSuccessor())]; break;
                case UD_Icmovno:
                    _[jump(~of(), then.basicBlock(), directSuccessor())]; break;
                case UD_Icmovnp:
                    _[jump(~pf(), then.basicBlock(), directSuccessor())]; break;
                case UD_Icmovns:
                    _[jump(~sf(), then.basicBlock(), directSuccessor())]; break;
                case UD_Icmovo:
                    _[jump(of(), then.basicBlock(), directSuccessor())]; break;
                case UD_Icmovp:
                    _[jump(pf(), then.basicBlock(), directSuccessor())]; break;
                case UD_Icmovs:
                    _[jump(sf(), then.basicBlock(), directSuccessor())]; break;
                default: unreachable();
            }

            then[
                operand(0) = operand(1),
                jump(directSuccessor())
            ];

            break;
        }
        case UD_Ilea: {
            if (const core::arch::DereferenceOperand *dereference = instr->operand(1)->asDereference()) {
                if (instr->operand(0)->size() == dereference->operand()->size()) {
                    _[operand(0) = operand(dereference->operand())];
                } else if (instr->operand(0)->size() < dereference->operand()->size()) {
                    _[operand(0) = truncate(operand(dereference->operand()))];
                } else if (instr->operand(0)->size() > dereference->operand()->size()) {
                    _[operand(0) = zero_extend(operand(dereference->operand()))];
                }
            } else {
                throw core::arch::irgen::InvalidInstructionException("lea's second argument must be a memory operand");
            }
            break;
        }
        case UD_Ileave: {
            auto sp = architecture_->stackPointer();
            auto bp = architecture_->basePointer();

            _[
                regizter(sp) = regizter(bp),
                regizter(bp) = *regizter(sp),
                regizter(sp) = regizter(sp) + constant(bp->size() / CHAR_BIT)
            ];
            break;
        }
        case UD_Iloop:
        case UD_Iloope:
        case UD_Iloopnz: {
            auto cx = resizedRegister(IntelRegisters::cx(), ud_obj.adr_mode);

            _[cx = cx - constant(1)];

            switch (ud_obj.mnemonic) {
                case UD_Iloop:
                    _[jump(~(cx == constant(0)), operand(0), directSuccessor())];
                    break;
                case UD_Iloope:
                    _[jump(~(cx == constant(0)) & zf(), operand(0), directSuccessor())];
                    break;
                case UD_Iloopnz:
                    _[jump(~(cx == constant(0)) & ~zf(), operand(0), directSuccessor())];
                    break;
                default:
                    unreachable();
            }
            break;
        }
        case UD_Imov: {
            if (instr->operand(0)->size() == instr->operand(1)->size()) {
                _[operand(0) = operand(1)];
            } else if (instr->operand(0)->size() > instr->operand(1)->size()) {
                /* For example, move of a small constant to a big register. */
                _[operand(0) = zero_extend(operand(1))];
            } else {
                /* Happens in assignments to segment registers. Known bug of udis86. */
                _[operand(0) = truncate(operand(1))];
            }
            break;
        }
        case UD_Imovsx: {
            if (instr->operand(0)->size() == instr->operand(1)->size()) {
                _[operand(0) = operand(1)];
            } else {
                _[operand(0) = sign_extend(operand(1))];
            }
            break;
        }
        case UD_Imovzx: {
            _[operand(0) = zero_extend(operand(1))];
            break;
        }
        case UD_Ineg: {
            _[
                cf() = ~(operand(0) == constant(0)),
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
        case UD_Inop: {
            break;
        }
        case UD_Inot: {
            _[operand(0) = ~operand(0)];
            break;
        }
        case UD_Ior: {
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
        case UD_Ipop: {
            auto sp = architecture_->stackPointer();
            _[
                operand(0) = *regizter(sp),
                regizter(sp) = regizter(sp) + constant(instr->operand(0)->size() / CHAR_BIT)
            ];
            break;
        }
        case UD_Ipush: {
            auto sp = architecture_->stackPointer();
            _[
                regizter(sp) = regizter(sp) - constant(instr->operand(0)->size() / CHAR_BIT),
                *regizter(sp) = operand(0)
            ];
            break;
        }
        case UD_Ipopfw: {
            auto sp = architecture_->stackPointer();
            _[
                *regizter(sp) = flags(),
                regizter(sp) = regizter(sp) + constant(2)
            ];
            break;
        }
        case UD_Ipopfd: {
            auto sp = architecture_->stackPointer();
            _[
                eflags() = *regizter(sp),
                regizter(sp) = regizter(sp) + constant(4)
            ];
            break;
        }
        case UD_Ipopfq: {
            auto sp = architecture_->stackPointer();
            _[
                rflags() = *regizter(sp),
                regizter(sp) = regizter(sp) + constant(8)
            ];
            break;
        }
        case UD_Ipushfw: {
            auto sp = architecture_->stackPointer();
            _[
                regizter(sp) = regizter(sp) - constant(2),
                *regizter(sp) = flags()
            ];
            break;
        }
        case UD_Ipushfd: {
            auto sp = architecture_->stackPointer();
            _[
                regizter(sp) = regizter(sp) - constant(4),
                *regizter(sp) = eflags() & constant(0x00fcffff)
            ];
            break;
        }
        case UD_Ipushfq: {
            auto sp = architecture_->stackPointer();
            _[
                regizter(sp) = regizter(sp) - constant(8),
                *regizter(sp) = rflags() & constant(0x00fcffff)
            ];
            break;
        }
        case UD_Iret: {
            auto sp = architecture_->stackPointer();
            auto ip = architecture_->instructionPointer();

            _[
                regizter(ip) = *regizter(sp),
                regizter(sp) = regizter(sp) + constant(ip->size() / CHAR_BIT)
            ];

            if (instr->operands().size() == 1) {
                _[regizter(sp) = regizter(sp) + zero_extend(operand(0))];
            }

            _[return_()];
            break;
        }
        case UD_Ishl: {
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
        case UD_Isar: {
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
        case UD_Isbb: {
            _[
                less()             = signed_(operand(0))   <  operand(1) + zero_extend(cf()),
                less_or_equal()    = signed_(operand(0))   <= operand(1) + zero_extend(cf()),
                greater()          = signed_(operand(0))   >  operand(1) + zero_extend(cf()),
                greater_or_equal() = signed_(operand(0))   >= operand(1) + zero_extend(cf()),
                below()            = unsigned_(operand(0)) <  operand(1) + zero_extend(cf()),
                below_or_equal()   = unsigned_(operand(0)) <= operand(1) + zero_extend(cf()),
                above()            = unsigned_(operand(0)) >  operand(1) + zero_extend(cf()),
                above_or_equal()   = unsigned_(operand(0)) >= operand(1) + zero_extend(cf()),

                operand(0) = operand(0) - (operand(1) + zero_extend(cf())),

                cf() = intrinsic(),
                pf() = intrinsic(),
                zf() = operand(0) == constant(0) + zero_extend(cf()),
                sf() = signed_(operand(0)) < constant(0) + zero_extend(cf()),
                of() = intrinsic(),
                af() = intrinsic()
            ];
            break;
        }
        case UD_Iseta: {
            _[operand(0) = zero_extend(choice(above(), ~cf() & ~zf()))];
            break;
        }
        case UD_Isetnb: {
            _[operand(0) = zero_extend(choice(above_or_equal(), ~cf()))];
            break;
        }
        case UD_Isetb: {
            _[operand(0) = zero_extend(choice(below(), cf()))];
            break;
        }
        case UD_Isetbe: {
            _[operand(0) = zero_extend(choice(below_or_equal(), cf() | zf()))];
            break;
        }
        case UD_Isetz: {
            _[operand(0) = zero_extend(zf())];
            break;
        }
        case UD_Isetg: {
            _[operand(0) = zero_extend(choice(greater(), ~zf() | (sf() == of())))];
            break;
        }
        case UD_Isetge: {
            _[operand(0) = zero_extend(choice(greater_or_equal(), sf() == of()))];
            break;
        }
        case UD_Isetl: {
            _[operand(0) = zero_extend(choice(less(), ~(sf() == of())))];
            break;
        }
        case UD_Isetle: {
            _[operand(0) = zero_extend(choice(less_or_equal(), zf() | ~(sf() == of())))];
            break;
        }
        case UD_Isetnz: {
            _[operand(0) = zero_extend(~zf())];
            break;
        }
        case UD_Isetno: {
            _[operand(0) = zero_extend(~of())];
            break;
        }
        case UD_Isetnp: {
            _[operand(0) = zero_extend(~pf())];
            break;
        }
        case UD_Isetns: {
            _[operand(0) = zero_extend(~sf())];
            break;
        }
        case UD_Iseto: {
            _[operand(0) = zero_extend(of())];
            break;
        }
        case UD_Isetp: {
            _[operand(0) = zero_extend(pf())];
            break;
        }
        case UD_Isets: {
            _[operand(0) = zero_extend(sf())];
            break;
        }
        case UD_Ishr: {
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
        case UD_Istd: {
            _[
                df() = constant(1)
            ];
            break;
        }
        case UD_Isub: {
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
        case UD_Itest: {
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
        case UD_Ixchg: {
            auto tmp = temporary(instr->operand(0)->size());

            _[
                tmp = operand(0),
                operand(0) = operand(1),
                operand(1) = tmp
            ];
            break;
        }
        case UD_Ixor: {
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
            unreachable();
        }
    }
}

std::unique_ptr<core::ir::Term> IntelInstructionAnalyzer::createFpuTerm(int index) const {
    const SmallBitSize addressSize = 16;

    return std::make_unique<core::ir::Dereference>(
        std::make_unique<core::ir::BinaryOperator>(
            core::ir::BinaryOperator::MUL,
            std::make_unique<core::ir::UnaryOperator>(
                core::ir::UnaryOperator::ZERO_EXTEND,
                std::make_unique<core::ir::BinaryOperator>(
                    core::ir::BinaryOperator::ADD,
                    createTerm(IntelRegisters::fpu_top()),
                    std::make_unique<core::ir::Constant>(SizedValue(IntelRegisters::fpu_top()->size(), index)),
                    IntelRegisters::fpu_top()->size()
                ),
                addressSize
            ),
            std::make_unique<core::ir::Constant>(SizedValue(addressSize, IntelRegisters::fpu_r0()->size())),
            addressSize
        ),
        IntelRegisters::fpu_r0()->memoryLocation().domain(),
        addressSize
    );
}

std::unique_ptr<core::ir::Term> IntelInstructionAnalyzer::doCreateTerm(const core::arch::Operand *operand) const {
    switch (operand->kind()) {
    case core::arch::Operand::REGISTER: {
        auto reg = operand->as<core::arch::RegisterOperand>()->regizter();
        if (reg == architecture_->instructionPointer()) {
            return std::make_unique<core::ir::Intrinsic>(core::ir::Intrinsic::NEXT_INSTRUCTION_ADDRESS, reg->size());
        } else {
            return core::arch::irgen::InstructionAnalyzer::doCreateTerm(operand);
        }
    }
    case IntelOperands::FPU_STACK: {
        return createFpuTerm(operand->as<FpuOperand>()->index());
    }
    default: 
        return core::arch::irgen::InstructionAnalyzer::doCreateTerm(operand);
    }
}

} // namespace intel
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
