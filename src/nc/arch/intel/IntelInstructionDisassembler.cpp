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

#include <boost/range/size.hpp>

#include <nc/common/CheckedCast.h>
#include <nc/common/Unreachable.h>

#define UD_NO_STDINT_DEFINE
#include <libudis86/udis86.h>

#include "IntelArchitecture.h"
#include "IntelInstruction.h"
#include "IntelMnemonics.h"
#include "IntelRegisters.h"

namespace nc {
namespace arch {
namespace intel {

class IntelInstructionDisassemblerPrivate {
    const IntelArchitecture *architecture_;
    ud_t ud_obj_;

    public:

    IntelInstructionDisassemblerPrivate(const IntelArchitecture *architecture):
        architecture_(architecture)
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

        const core::arch::Mnemonic *mnemonic = getMnemonic();
        if (!mnemonic) {
            return NULL;
        }

        Prefixes prefixes = getPrefixes();

        std::unique_ptr<IntelInstruction> result(new IntelInstruction(mnemonic, pc, instructionSize, prefixes));

        result->setOperandSize(ud_obj_.opr_mode);
        result->setAddressSize(ud_obj_.adr_mode);

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

        if (result->mnemonic()->number() == mnemonics::MOVSD && !result->operands().empty()) {
            /* This is actually MOVSD_SSE. */
            result->setMnemonic(IntelMnemonics::mnemonic(mnemonics::MOVSD_SSE));
        }

        return result;
    }

    Prefixes getPrefixes() {
        Prefixes result = 0;

        #define PREFIX(ud_name, nc_name) if (ud_obj_.pfx_##ud_name != UD_NONE) { result |= prefixes::nc_name; }
        PREFIX(lock, LOCK)
        PREFIX(rep, REP)
        PREFIX(repe, REPZ)
        PREFIX(repne, REPNZ)
        #undef PREFIX

        return result;
    }

    const core::arch::Mnemonic *getMnemonic() {
        int number;

        // TODO: add missing instructions to IntelInstructionTable.i
        switch(ud_obj_.mnemonic) {

        #define MNEMONIC(ud_name, nc_name) case UD_I##ud_name: number = mnemonics::nc_name; break;
        #define PREFIX(ud_name, nc_name) /* Ignore. */

        //MNEMONIC(invalid, INVALID)
        //MNEMONIC(3dnow, 3DNOW)
        //MNEMONIC(none, NONE)
        //MNEMONIC(db, DB)
        MNEMONIC(pause, PAUSE)
        MNEMONIC(aaa, AAA)
        MNEMONIC(aad, AAD)
        MNEMONIC(aam, AAM)
        MNEMONIC(aas, AAS)
        MNEMONIC(adc, ADC)
        MNEMONIC(add, ADD)
        MNEMONIC(addpd, ADDPD)
        MNEMONIC(addps, ADDPS)
        MNEMONIC(addsd, ADDSD)
        MNEMONIC(addss, ADDSS)
        MNEMONIC(and, AND)
        MNEMONIC(andpd, ANDPD)
        MNEMONIC(andps, ANDPS)
        MNEMONIC(andnpd, ANDNPD)
        MNEMONIC(andnps, ANDNPS)
        MNEMONIC(arpl, ARPL)
        MNEMONIC(movsxd, MOVSXD)
        MNEMONIC(bound, BOUND)
        MNEMONIC(bsf, BSF)
        MNEMONIC(bsr, BSR)
        MNEMONIC(bswap, BSWAP)
        MNEMONIC(bt, BT)
        MNEMONIC(btc, BTC)
        MNEMONIC(btr, BTR)
        MNEMONIC(bts, BTS)
        MNEMONIC(call, CALL)
        MNEMONIC(cbw, CBW)
        MNEMONIC(cwde, CWDE)
        MNEMONIC(cdqe, CDQE)
        MNEMONIC(clc, CLC)
        MNEMONIC(cld, CLD)
        MNEMONIC(clflush, CLFLUSH)
        //MNEMONIC(clgi, CLGI)
        MNEMONIC(cli, CLI)
        MNEMONIC(clts, CLTS)
        MNEMONIC(cmc, CMC)
        MNEMONIC(cmovo, CMOVO)
        MNEMONIC(cmovno, CMOVNO)
        MNEMONIC(cmovb, CMOVB)
        MNEMONIC(cmovae, CMOVAE)
        MNEMONIC(cmovz, CMOVZ)
        MNEMONIC(cmovnz, CMOVNZ)
        MNEMONIC(cmovbe, CMOVBE)
        MNEMONIC(cmova, CMOVA)
        MNEMONIC(cmovs, CMOVS)
        MNEMONIC(cmovns, CMOVNS)
        MNEMONIC(cmovp, CMOVP)
        MNEMONIC(cmovnp, CMOVNP)
        MNEMONIC(cmovl, CMOVL)
        MNEMONIC(cmovge, CMOVGE)
        MNEMONIC(cmovle, CMOVLE)
        MNEMONIC(cmovg, CMOVG)
        MNEMONIC(cmp, CMP)
        MNEMONIC(cmppd, CMPPD)
        MNEMONIC(cmpps, CMPPS)
        MNEMONIC(cmpsb, CMPSB)
        MNEMONIC(cmpsw, CMPSW)
        MNEMONIC(cmpsd, CMPSD)
        MNEMONIC(cmpsq, CMPSQ)
        MNEMONIC(cmpss, CMPSS)
        MNEMONIC(cmpxchg, CMPXCHG)
        MNEMONIC(cmpxchg8b, CMPXCHG8B)
        MNEMONIC(comisd, COMISD)
        MNEMONIC(comiss, COMISS)
        MNEMONIC(cpuid, CPUID)
        MNEMONIC(cvtdq2pd, CVTDQ2PD)
        MNEMONIC(cvtdq2ps, CVTDQ2PS)
        MNEMONIC(cvtpd2dq, CVTPD2DQ)
        MNEMONIC(cvtpd2pi, CVTPD2PI)
        MNEMONIC(cvtpd2ps, CVTPD2PS)
        MNEMONIC(cvtpi2ps, CVTPI2PS)
        MNEMONIC(cvtpi2pd, CVTPI2PD)
        MNEMONIC(cvtps2dq, CVTPS2DQ)
        MNEMONIC(cvtps2pi, CVTPS2PI)
        MNEMONIC(cvtps2pd, CVTPS2PD)
        MNEMONIC(cvtsd2si, CVTSD2SI)
        MNEMONIC(cvtsd2ss, CVTSD2SS)
        MNEMONIC(cvtsi2ss, CVTSI2SS)
        MNEMONIC(cvtss2si, CVTSS2SI)
        MNEMONIC(cvtss2sd, CVTSS2SD)
        MNEMONIC(cvttpd2pi, CVTTPD2PI)
        MNEMONIC(cvttpd2dq, CVTTPD2DQ)
        MNEMONIC(cvttps2dq, CVTTPS2DQ)
        MNEMONIC(cvttps2pi, CVTTPS2PI)
        MNEMONIC(cvttsd2si, CVTTSD2SI)
        MNEMONIC(cvtsi2sd, CVTSI2SD)
        MNEMONIC(cvttss2si, CVTTSS2SI)
        MNEMONIC(cwd, CWD)
        MNEMONIC(cdq, CDQ)
        MNEMONIC(cqo, CQO)
        MNEMONIC(daa, DAA)
        MNEMONIC(das, DAS)
        MNEMONIC(dec, DEC)
        MNEMONIC(div, DIV)
        MNEMONIC(divpd, DIVPD)
        MNEMONIC(divps, DIVPS)
        MNEMONIC(divsd, DIVSD)
        MNEMONIC(divss, DIVSS)
        MNEMONIC(emms, EMMS)
        MNEMONIC(enter, ENTER)
        MNEMONIC(f2xm1, F2XM1)
        MNEMONIC(fabs, FABS)
        MNEMONIC(fadd, FADD)
        MNEMONIC(faddp, FADDP)
        MNEMONIC(fbld, FBLD)
        MNEMONIC(fbstp, FBSTP)
        MNEMONIC(fchs, FCHS)
        MNEMONIC(fclex, FCLEX)
        MNEMONIC(fcmovb, FCMOVB)
        MNEMONIC(fcmove, FCMOVE)
        MNEMONIC(fcmovbe, FCMOVBE)
        MNEMONIC(fcmovu, FCMOVU)
        MNEMONIC(fcmovnb, FCMOVNB)
        MNEMONIC(fcmovne, FCMOVNE)
        MNEMONIC(fcmovnbe, FCMOVNBE)
        MNEMONIC(fcmovnu, FCMOVNU)
        MNEMONIC(fucomi, FUCOMI)
        MNEMONIC(fcom, FCOM)
        MNEMONIC(fcom2, FCOM2)
        MNEMONIC(fcomp3, FCOMP3)
        MNEMONIC(fcomi, FCOMI)
        MNEMONIC(fucomip, FUCOMIP)
        MNEMONIC(fcomip, FCOMIP)
        MNEMONIC(fcomp, FCOMP)
        MNEMONIC(fcomp5, FCOMP5)
        MNEMONIC(fcompp, FCOMPP)
        MNEMONIC(fcos, FCOS)
        MNEMONIC(fdecstp, FDECSTP)
        MNEMONIC(fdiv, FDIV)
        MNEMONIC(fdivp, FDIVP)
        MNEMONIC(fdivr, FDIVR)
        MNEMONIC(fdivrp, FDIVRP)
        MNEMONIC(femms, FEMMS)
        MNEMONIC(ffree, FFREE)
        MNEMONIC(ffreep, FFREEP)
        MNEMONIC(ficom, FICOM)
        MNEMONIC(ficomp, FICOMP)
        MNEMONIC(fild, FILD)
        //MNEMONIC(fncstp, FNCSTP)
        MNEMONIC(fninit, FNINIT)
        MNEMONIC(fiadd, FIADD)
        MNEMONIC(fidivr, FIDIVR)
        MNEMONIC(fidiv, FIDIV)
        MNEMONIC(fisub, FISUB)
        MNEMONIC(fisubr, FISUBR)
        MNEMONIC(fist, FIST)
        MNEMONIC(fistp, FISTP)
        MNEMONIC(fisttp, FISTTP)
        MNEMONIC(fld, FLD)
        MNEMONIC(fld1, FLD1)
        MNEMONIC(fldl2t, FLDL2T)
        MNEMONIC(fldl2e, FLDL2E)
        //MNEMONIC(fldlpi, FLDLPI)
        MNEMONIC(fldlg2, FLDLG2)
        MNEMONIC(fldln2, FLDLN2)
        MNEMONIC(fldz, FLDZ)
        MNEMONIC(fldcw, FLDCW)
        MNEMONIC(fldenv, FLDENV)
        MNEMONIC(fmul, FMUL)
        MNEMONIC(fmulp, FMULP)
        MNEMONIC(fimul, FIMUL)
        MNEMONIC(fnop, FNOP)
        MNEMONIC(fpatan, FPATAN)
        MNEMONIC(fprem, FPREM)
        MNEMONIC(fprem1, FPREM1)
        MNEMONIC(fptan, FPTAN)
        MNEMONIC(frndint, FRNDINT)
        MNEMONIC(frstor, FRSTOR)
        MNEMONIC(fnsave, FNSAVE)
        MNEMONIC(fscale, FSCALE)
        MNEMONIC(fsin, FSIN)
        MNEMONIC(fsincos, FSINCOS)
        MNEMONIC(fsqrt, FSQRT)
        MNEMONIC(fstp, FSTP)
        MNEMONIC(fstp1, FSTP1)
        MNEMONIC(fstp8, FSTP8)
        MNEMONIC(fstp9, FSTP9)
        MNEMONIC(fst, FST)
        MNEMONIC(fnstcw, FNSTCW)
        MNEMONIC(fnstenv, FNSTENV)
        MNEMONIC(fnstsw, FNSTSW)
        MNEMONIC(fsub, FSUB)
        MNEMONIC(fsubp, FSUBP)
        MNEMONIC(fsubr, FSUBR)
        MNEMONIC(fsubrp, FSUBRP)
        MNEMONIC(ftst, FTST)
        MNEMONIC(fucom, FUCOM)
        MNEMONIC(fucomp, FUCOMP)
        MNEMONIC(fucompp, FUCOMPP)
        MNEMONIC(fxam, FXAM)
        MNEMONIC(fxch, FXCH)
        MNEMONIC(fxch4, FXCH4)
        MNEMONIC(fxch7, FXCH7)
        MNEMONIC(fxrstor, FXRSTOR)
        MNEMONIC(fxsave, FXSAVE)
        MNEMONIC(fpxtract, FXTRACT)
        MNEMONIC(fyl2x, FYL2X)
        MNEMONIC(fyl2xp1, FYL2XP1)
        MNEMONIC(hlt, HLT)
        MNEMONIC(idiv, IDIV)
        MNEMONIC(in, IN)
        MNEMONIC(imul, IMUL)
        MNEMONIC(inc, INC)
        MNEMONIC(insb, INS)
        MNEMONIC(insw, INS)
        MNEMONIC(insd, INS)
        //MNEMONIC(int1, INT1)
        MNEMONIC(int3, INT3)
        MNEMONIC(int, INT)
        MNEMONIC(into, INTO)
        MNEMONIC(invd, INVD)
        //MNEMONIC(invept, INVEPT)
        MNEMONIC(invlpg, INVLPG)
        //MNEMONIC(invlpga, INVLPGA)
        //MNEMONIC(invvpid, INVVPID)
        MNEMONIC(iretw, IRETW)
        MNEMONIC(iretd, IRETD)
        MNEMONIC(iretq, IRETQ)
        MNEMONIC(jo, JO)
        MNEMONIC(jno, JNO)
        MNEMONIC(jb, JB)
        MNEMONIC(jae, JAE)
        MNEMONIC(jz, JZ)
        MNEMONIC(jnz, JNZ)
        MNEMONIC(jbe, JBE)
        MNEMONIC(ja, JA)
        MNEMONIC(js, JS)
        MNEMONIC(jns, JNS)
        MNEMONIC(jp, JP)
        MNEMONIC(jnp, JNP)
        MNEMONIC(jl, JL)
        MNEMONIC(jge, JGE)
        MNEMONIC(jle, JLE)
        MNEMONIC(jg, JG)
        MNEMONIC(jcxz, JCXZ)
        MNEMONIC(jecxz, JECXZ)
        MNEMONIC(jrcxz, JRCXZ)
        MNEMONIC(jmp, JMP)
        MNEMONIC(lahf, LAHF)
        MNEMONIC(lar, LAR)
        MNEMONIC(lddqu, LDDQU)
        MNEMONIC(ldmxcsr, LDMXCSR)
        MNEMONIC(lds, LDS)
        MNEMONIC(lea, LEA)
        MNEMONIC(les, LES)
        MNEMONIC(lfs, LFS)
        MNEMONIC(lgs, LGS)
        MNEMONIC(lidt, LIDT)
        MNEMONIC(lss, LSS)
        MNEMONIC(leave, LEAVE)
        MNEMONIC(lfence, LFENCE)
        MNEMONIC(lgdt, LGDT)
        MNEMONIC(lldt, LLDT)
        MNEMONIC(lmsw, LMSW)
        PREFIX(lock, LOCK)
        MNEMONIC(lodsb, LODS)
        MNEMONIC(lodsw, LODS)
        MNEMONIC(lodsd, LODS)
        MNEMONIC(lodsq, LODS)
        MNEMONIC(loopnz, LOOPNE)
        MNEMONIC(loope, LOOPE)
        MNEMONIC(loop, LOOP)
        MNEMONIC(lsl, LSL)
        MNEMONIC(ltr, LTR)
        MNEMONIC(maskmovq, MASKMOVQ)
        MNEMONIC(maxpd, MAXPD)
        MNEMONIC(maxps, MAXPS)
        MNEMONIC(maxsd, MAXSD)
        MNEMONIC(maxss, MAXSS)
        MNEMONIC(mfence, MFENCE)
        MNEMONIC(minpd, MINPD)
        MNEMONIC(minps, MINPS)
        MNEMONIC(minsd, MINSD)
        MNEMONIC(minss, MINSS)
        MNEMONIC(monitor, MONITOR)
        //MNEMONIC(montmul, MONTMUL)
        MNEMONIC(mov, MOV)
        MNEMONIC(movapd, MOVAPD)
        MNEMONIC(movaps, MOVAPS)
        MNEMONIC(movd, MOVD)
        MNEMONIC(movhpd, MOVHPD)
        MNEMONIC(movhps, MOVHPS)
        MNEMONIC(movlhps, MOVLHPS)
        MNEMONIC(movlpd, MOVLPD)
        MNEMONIC(movlps, MOVLPS)
        MNEMONIC(movhlps, MOVHLPS)
        MNEMONIC(movmskpd, MOVMSKPD)
        MNEMONIC(movmskps, MOVMSKPS)
        MNEMONIC(movntdq, MOVNTDQ)
        MNEMONIC(movnti, MOVNTI)
        MNEMONIC(movntpd, MOVNTPD)
        MNEMONIC(movntps, MOVNTPS)
        MNEMONIC(movntq, MOVNTQ)
        MNEMONIC(movq, MOVQ)
        MNEMONIC(movsb, MOVSB)
        MNEMONIC(movsw, MOVSW)
        MNEMONIC(movsd, MOVSD) /* Can be actually MOVSD_SSE. Distinguished later. */
        MNEMONIC(movsq, MOVSQ)
        MNEMONIC(movss, MOVSS)
        MNEMONIC(movsx, MOVSX)
        MNEMONIC(movupd, MOVUPD)
        MNEMONIC(movups, MOVUPS)
        MNEMONIC(movzx, MOVZX)
        MNEMONIC(mul, MUL)
        MNEMONIC(mulpd, MULPD)
        MNEMONIC(mulps, MULPS)
        MNEMONIC(mulsd, MULSD)
        MNEMONIC(mulss, MULSS)
        MNEMONIC(mwait, MWAIT)
        MNEMONIC(neg, NEG)
        MNEMONIC(nop, NOP)
        MNEMONIC(not, NOT)
        MNEMONIC(or, OR)
        MNEMONIC(orpd, ORPD)
        MNEMONIC(orps, ORPS)
        MNEMONIC(out, OUT)
        MNEMONIC(outsb, OUTS)
        MNEMONIC(outsw, OUTS)
        MNEMONIC(outsd, OUTS)
        MNEMONIC(outsq, OUTS)
        MNEMONIC(packsswb, PACKSSWB)
        MNEMONIC(packssdw, PACKSSDW)
        MNEMONIC(packuswb, PACKUSWB)
        MNEMONIC(paddb, PADDB)
        MNEMONIC(paddw, PADDW)
        MNEMONIC(paddd, PADDD)
        MNEMONIC(paddsb, PADDSB)
        MNEMONIC(paddsw, PADDSW)
        MNEMONIC(paddusb, PADDUSB)
        MNEMONIC(paddusw, PADDUSW)
        MNEMONIC(pand, PAND)
        MNEMONIC(pandn, PANDN)
        MNEMONIC(pavgb, PAVGB)
        MNEMONIC(pavgw, PAVGW)
        MNEMONIC(pcmpeqb, PCMPEQB)
        MNEMONIC(pcmpeqw, PCMPEQW)
        MNEMONIC(pcmpeqd, PCMPEQD)
        MNEMONIC(pcmpgtb, PCMPGTB)
        MNEMONIC(pcmpgtw, PCMPGTW)
        MNEMONIC(pcmpgtd, PCMPGTD)
        //MNEMONIC(pextrb, PEXTRB)
        //MNEMONIC(pextrd, PEXTRD)
        //MNEMONIC(pextrq, PEXTRQ)
        MNEMONIC(pextrw, PEXTRW)
        MNEMONIC(pinsrw, PINSRW)
        MNEMONIC(pmaddwd, PMADDWD)
        MNEMONIC(pmaxsw, PMAXSW)
        MNEMONIC(pmaxub, PMAXUB)
        MNEMONIC(pminsw, PMINSW)
        MNEMONIC(pminub, PMINUB)
        MNEMONIC(pmovmskb, PMOVMSKB)
        MNEMONIC(pmulhuw, PMULHUW)
        MNEMONIC(pmulhw, PMULHW)
        MNEMONIC(pmullw, PMULLW)
        MNEMONIC(pop, POP)
        MNEMONIC(popa, POPA)
        MNEMONIC(popad, POPAD)
        MNEMONIC(popfw, POPFW)
        MNEMONIC(popfd, POPFD)
        MNEMONIC(popfq, POPFQ)
        MNEMONIC(por, POR)
        MNEMONIC(prefetch, PREFETCH)
        MNEMONIC(prefetchnta, PREFETCHNTA)
        MNEMONIC(prefetcht0, PREFETCHT0)
        MNEMONIC(prefetcht1, PREFETCHT1)
        MNEMONIC(prefetcht2, PREFETCHT2)
        MNEMONIC(psadbw, PSADBW)
        MNEMONIC(pshufw, PSHUFW)
        MNEMONIC(psllw, PSLLW)
        MNEMONIC(pslld, PSLLD)
        MNEMONIC(psllq, PSLLQ)
        MNEMONIC(psraw, PSRAW)
        MNEMONIC(psrad, PSRAD)
        MNEMONIC(psrlw, PSRLW)
        MNEMONIC(psrld, PSRLD)
        MNEMONIC(psrlq, PSRLQ)
        MNEMONIC(psubb, PSUBB)
        MNEMONIC(psubw, PSUBW)
        MNEMONIC(psubd, PSUBD)
        MNEMONIC(psubsb, PSUBSB)
        MNEMONIC(psubsw, PSUBSW)
        MNEMONIC(psubusb, PSUBUSB)
        MNEMONIC(psubusw, PSUBUSW)
        MNEMONIC(punpckhbw, PUNPCKHBW)
        MNEMONIC(punpckhwd, PUNPCKHWD)
        MNEMONIC(punpckhdq, PUNPCKHDQ)
        MNEMONIC(punpcklbw, PUNPCKLBW)
        MNEMONIC(punpcklwd, PUNPCKLWD)
        MNEMONIC(punpckldq, PUNPCKLDQ)
        MNEMONIC(pi2fw, PI2FW)
        MNEMONIC(pi2fd, PI2FD)
        MNEMONIC(pf2iw, PF2IW)
        MNEMONIC(pf2id, PF2ID)
        MNEMONIC(pfnacc, PFNACC)
        MNEMONIC(pfpnacc, PFPNACC)
        MNEMONIC(pfcmpge, PFCMPGE)
        MNEMONIC(pfmin, PFMIN)
        MNEMONIC(pfrcp, PFRCP)
        MNEMONIC(pfrsqrt, PFRSQRT)
        MNEMONIC(pfsub, PFSUB)
        MNEMONIC(pfadd, PFADD)
        MNEMONIC(pfcmpgt, PFCMPGT)
        MNEMONIC(pfmax, PFMAX)
        MNEMONIC(pfrcpit1, PFRCPIT1)
        MNEMONIC(pfrsqit1, PFRSQIT1)
        MNEMONIC(pfsubr, PFSUBR)
        MNEMONIC(pfacc, PFACC)
        MNEMONIC(pfcmpeq, PFCMPEQ)
        MNEMONIC(pfmul, PFMUL)
        MNEMONIC(pfrcpit2, PFRCPIT2)
        MNEMONIC(pmulhrw, PMULHRW)
        MNEMONIC(pswapd, PSWAPD)
        MNEMONIC(pavgusb, PAVGUSB)
        MNEMONIC(push, PUSH)
        MNEMONIC(pusha, PUSHA)
        MNEMONIC(pushad, PUSHAD)
        MNEMONIC(pushfw, PUSHFW)
        MNEMONIC(pushfd, PUSHFD)
        MNEMONIC(pushfq, PUSHFQ)
        MNEMONIC(pxor, PXOR)
        MNEMONIC(rcl, RCL)
        MNEMONIC(rcr, RCR)
        MNEMONIC(rol, ROL)
        MNEMONIC(ror, ROR)
        MNEMONIC(rcpps, RCPPS)
        MNEMONIC(rcpss, RCPSS)
        MNEMONIC(rdmsr, RDMSR)
        MNEMONIC(rdpmc, RDPMC)
        MNEMONIC(rdtsc, RDTSC)
        MNEMONIC(rdtscp, RDTSCP)
        PREFIX(repne, REPNZ)
        PREFIX(rep, REP)
        MNEMONIC(ret, RET)
        MNEMONIC(retf, RETF)
        MNEMONIC(rsm, RSM)
        MNEMONIC(rsqrtps, RSQRTPS)
        MNEMONIC(rsqrtss, RSQRTSS)
        MNEMONIC(sahf, SAHF)
        //MNEMONIC(salc, SALC)
        MNEMONIC(sar, SAR)
        MNEMONIC(shl, SHL)
        MNEMONIC(shr, SHR)
        MNEMONIC(sbb, SBB)
        MNEMONIC(scasb, SCASB)
        MNEMONIC(scasw, SCASW)
        MNEMONIC(scasd, SCASD)
        MNEMONIC(scasq, SCASQ)
        MNEMONIC(seto, SETO)
        MNEMONIC(setno, SETNO)
        MNEMONIC(setb, SETB)
        MNEMONIC(setnb, SETNB)
        MNEMONIC(setz, SETZ)
        MNEMONIC(setnz, SETNZ)
        MNEMONIC(setbe, SETBE)
        MNEMONIC(seta, SETA)
        MNEMONIC(sets, SETS)
        MNEMONIC(setns, SETNS)
        MNEMONIC(setp, SETP)
        MNEMONIC(setnp, SETNP)
        MNEMONIC(setl, SETL)
        MNEMONIC(setge, SETGE)
        MNEMONIC(setle, SETLE)
        MNEMONIC(setg, SETG)
        MNEMONIC(sfence, SFENCE)
        MNEMONIC(sgdt, SGDT)
        MNEMONIC(shld, SHLD)
        MNEMONIC(shrd, SHRD)
        MNEMONIC(shufpd, SHUFPD)
        MNEMONIC(shufps, SHUFPS)
        MNEMONIC(sidt, SIDT)
        MNEMONIC(sldt, SLDT)
        MNEMONIC(smsw, SMSW)
        MNEMONIC(sqrtps, SQRTPS)
        MNEMONIC(sqrtpd, SQRTPD)
        MNEMONIC(sqrtsd, SQRTSD)
        MNEMONIC(sqrtss, SQRTSS)
        MNEMONIC(stc, STC)
        MNEMONIC(std, STD)
        //MNEMONIC(stgi, STGI)
        MNEMONIC(sti, STI)
        //MNEMONIC(skinit, SKINIT)
        MNEMONIC(stmxcsr, STMXCSR)
        MNEMONIC(stosb, STOSB)
        MNEMONIC(stosw, STOSW)
        MNEMONIC(stosd, STOSD)
        MNEMONIC(stosq, STOSQ)
        MNEMONIC(str, STR)
        MNEMONIC(sub, SUB)
        MNEMONIC(subpd, SUBPD)
        MNEMONIC(subps, SUBPS)
        MNEMONIC(subsd, SUBSD)
        MNEMONIC(subss, SUBSS)
        MNEMONIC(swapgs, SWAPGS)
        MNEMONIC(syscall, SYSCALL)
        MNEMONIC(sysenter, SYSENTER)
        MNEMONIC(sysexit, SYSEXIT)
        MNEMONIC(sysret, SYSRET)
        MNEMONIC(test, TEST)
        MNEMONIC(ucomisd, UCOMISD)
        MNEMONIC(ucomiss, UCOMISS)
        //MNEMONIC(ud2, UD2)
        MNEMONIC(unpckhpd, UNPCKHPD)
        MNEMONIC(unpckhps, UNPCKHPS)
        MNEMONIC(unpcklps, UNPCKLPS)
        MNEMONIC(unpcklpd, UNPCKLPD)
        MNEMONIC(verr, VERR)
        MNEMONIC(verw, VERW)
        MNEMONIC(vmcall, VMCALL)
        MNEMONIC(vmclear, VMCLEAR)
        MNEMONIC(vmxon, VMXON)
        MNEMONIC(vmptrld, VMPTRLD)
        MNEMONIC(vmptrst, VMPTRST)
        MNEMONIC(vmlaunch, VMLAUNCH)
        MNEMONIC(vmresume, VMRESUME)
        MNEMONIC(vmxoff, VMXOFF)
        MNEMONIC(vmread, VMREAD)
        MNEMONIC(vmwrite, VMWRITE)
        //MNEMONIC(vmrun, VMRUN)
        //MNEMONIC(vmmcall, VMMCALL)
        //MNEMONIC(vmload, VMLOAD)
        //MNEMONIC(vmsave, VMSAVE)
        MNEMONIC(wait, WAIT)
        MNEMONIC(wbinvd, WBINVD)
        MNEMONIC(wrmsr, WRMSR)
        MNEMONIC(xadd, XADD)
        MNEMONIC(xchg, XCHG)
        MNEMONIC(xlatb, XLAT)
        MNEMONIC(xor, XOR)
        MNEMONIC(xorpd, XORPD)
        MNEMONIC(xorps, XORPS)
        //MNEMONIC(xcryptecb, XCRYPTECB)
        //MNEMONIC(xcryptcbc, XCRYPTCBC)
        //MNEMONIC(xcryptctr, XCRYPTCTR)
        //MNEMONIC(xcryptcfb, XCRYPTCFB)
        //MNEMONIC(xcryptofb, XCRYPTOFB)
        //MNEMONIC(xsha1, XSHA1)
        //MNEMONIC(xsha256, XSHA256)
        //MNEMONIC(xstore, XSTORE)
        MNEMONIC(movdqa, MOVDQA)
        MNEMONIC(movdq2q, MOVDQ2Q)
        MNEMONIC(movdqu, MOVDQU)
        MNEMONIC(movq2dq, MOVQ2DQ)
        MNEMONIC(paddq, PADDQ)
        MNEMONIC(psubq, PSUBQ)
        MNEMONIC(pmuludq, PMULUDQ)
        MNEMONIC(pshufhw, PSHUFHW)
        MNEMONIC(pshuflw, PSHUFLW)
        MNEMONIC(pshufd, PSHUFD)
        MNEMONIC(pslldq, PSLLDQ)
        MNEMONIC(psrldq, PSRLDQ)
        MNEMONIC(punpckhqdq, PUNPCKHQDQ)
        MNEMONIC(punpcklqdq, PUNPCKLQDQ)
        MNEMONIC(addsubpd, ADDSUBPD)
        MNEMONIC(addsubps, ADDSUBPS)
        MNEMONIC(haddpd, HADDPD)
        MNEMONIC(haddps, HADDPS)
        MNEMONIC(hsubpd, HSUBPD)
        MNEMONIC(hsubps, HSUBPS)
        MNEMONIC(movddup, MOVDDUP)
        MNEMONIC(movshdup, MOVSHDUP)
        MNEMONIC(movsldup, MOVSLDUP)
        MNEMONIC(pabsb, PABSB)
        MNEMONIC(pabsw, PABSW)
        MNEMONIC(pabsd, PABSD)
        MNEMONIC(psignb, PSIGNB)
        MNEMONIC(phaddw, PHADDW)
        MNEMONIC(phaddd, PHADDD)
        MNEMONIC(phaddsw, PHADDSW)
        MNEMONIC(pmaddubsw, PMADDUBSW)
        MNEMONIC(phsubw, PHSUBW)
        MNEMONIC(phsubd, PHSUBD)
        MNEMONIC(phsubsw, PHSUBSW)
        MNEMONIC(psignd, PSIGND)
        MNEMONIC(psignw, PSIGNW)
        MNEMONIC(pmulhrsw, PMULHRSW)
        MNEMONIC(palignr, PALIGNR)
        //MNEMONIC(pblendvb, PBLENDVB)
        //MNEMONIC(pmuldq, PMULDQ)
        //MNEMONIC(pminsb, PMINSB)
        //MNEMONIC(pminsd, PMINSD)
        //MNEMONIC(pminuw, PMINUW)
        //MNEMONIC(pminud, PMINUD)
        //MNEMONIC(pmaxsb, PMAXSB)
        //MNEMONIC(pmaxsd, PMAXSD)
        //MNEMONIC(pmaxud, PMAXUD)
        //MNEMONIC(pmulld, PMULLD)
        //MNEMONIC(phminposuw, PHMINPOSUW)
        //MNEMONIC(roundps, ROUNDPS)
        //MNEMONIC(roundpd, ROUNDPD)
        //MNEMONIC(roundss, ROUNDSS)
        //MNEMONIC(roundsd, ROUNDSD)
        //MNEMONIC(blendpd, BLENDPD)
        //MNEMONIC(pblendw, PBLENDW)
        //MNEMONIC(blendps, BLENDPS)
        //MNEMONIC(blendvpd, BLENDVPD)
        //MNEMONIC(blendvps, BLENDVPS)
        //MNEMONIC(dpps, DPPS)
        //MNEMONIC(dppd, DPPD)
        //MNEMONIC(mpsadbw, MPSADBW)
        //MNEMONIC(extractps, EXTRACTPS)

        #undef MNEMONIC
        #undef PREFIX

        default:
            return NULL;
        }
        return IntelMnemonics::mnemonic(number);
    }

    core::arch::Operand *getOperand(const ud_operand &operand, SmallBitSize lastOperandSize) {
        switch (operand.type) {
            case UD_NONE:
                return NULL;
            case UD_OP_MEM:
                return getDereference(operand);
            case UD_OP_PTR:
                return architecture_->constantOperand(SizedValue(operand.lval.ptr.seg * 16 + operand.lval.ptr.off, operand.size));
            case UD_OP_IMM:
                /* Signed number, sign-extended to match the size of the other operand. */
                return architecture_->constantOperand(SizedValue(getSignedValue(operand, operand.size), std::max(SmallBitSize(operand.size), lastOperandSize)));
            case UD_OP_JIMM:
                return architecture_->constantOperand(SizedValue(ud_obj_.pc + getSignedValue(operand, operand.size), architecture_->bitness()));
            case UD_OP_CONST:
                /* This is some small constant value, like in "sar eax, 1". Its size is always zero. */
                assert(operand.size == 0);
                return architecture_->constantOperand(SizedValue(operand.lval.ubyte, 8));
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
        #define REG_ST(n) case UD_R_ST##n: return architecture_->fpuStackOperand(n);

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

        return architecture_->registerOperand(number);
    }

    core::arch::DereferenceOperand *getDereference(const ud_operand &operand) {
        assert(operand.type == UD_OP_MEM);

        core::arch::Operand *address = getRegisterOperand(operand.base);

        if (operand.scale != 0) {
            if (core::arch::Operand *index = getRegisterOperand(operand.index)) {
                if (operand.scale != 1) {
                    index = new core::arch::MultiplicationOperand(
                        index,
                        architecture_->constantOperand(SizedValue(operand.scale, ud_obj_.adr_mode)),
                        ud_obj_.adr_mode);
                }
                if (address) {
                    address = new core::arch::AdditionOperand(address, index, ud_obj_.adr_mode);
                } else {
                    address = index;
                }
            }
        }

        auto offsetValue = SizedValue(getUnsignedValue(operand, operand.offset), operand.offset);

        if (offsetValue.value() || !address) {
            core::arch::ConstantOperand *offset = architecture_->constantOperand(offsetValue);

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
