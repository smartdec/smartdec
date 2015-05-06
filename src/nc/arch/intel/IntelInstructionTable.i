/**
 * \file
 *
 * Instruction table for Intel architecture.
 *
 * This header is intended for multiple inclusion.
 */


/**
 * \def INSTR(lowercase, uppercase, description)
 * 
 * \param lowercase                    Name of the instruction as a lowercase 
 *                                     C++ identifier.
 * \param uppercase                    Name of the instruction as an UPPERCASE
 *                                     C++ identifier.
 * \param description                  Instruction description.
 */

/* Base x86 instructions. */
INSTR(aaa,         AAA,         "ASCII Adjust after Addition") // NOT IMPLEMENTED
INSTR(aad,         AAD,         "ASCII Adjust AX before Division") // NOT IMPLEMENTED
INSTR(aam,         AAM,         "ASCII Adjust AX after Multiply") // NOT IMPLEMENTED
INSTR(aas,         AAS,         "ASCII Adjust AL after Subtraction") // NOT IMPLEMENTED
INSTR(adc,         ADC,         "Add with Carry")
INSTR(add,         ADD,         "Add")
INSTR(and,         AND,         "Logical AND")
INSTR(arpl,        ARPL,        "Adjust RPL Field of Selector") // NOT IMPLEMENTED
INSTR(bound,       BOUND,       "Check Array Index Against Bounds")
INSTR(bsf,         BSF,         "Bit Scan Forward") // NOT IMPLEMENTED
INSTR(bsr,         BSR,         "Bit Scan Reverse") // NOT IMPLEMENTED
INSTR(bt,          BT,          "Bit Test")
INSTR(btc,         BTC,         "Bit Test and Complement") // NOT IMPLEMENTED
INSTR(btr,         BTR,         "Bit Test and Reset") // NOT IMPLEMENTED
INSTR(bts,         BTS,         "Bit Test and Set") // NOT IMPLEMENTED
INSTR(call,        CALL,        "Call Procedure")
INSTR(cbw,         CBW,         "AL -> AX (with sign)")
INSTR(cwde,        CWDE,        "AX -> EAX (with sign)")
INSTR(cdqe,        CDQE,        "EAX -> RAX (with sign)")
INSTR(clc,         CLC,         "Clear Carry Flag") // NOT IMPLEMENTED
INSTR(cld,         CLD,         "Clear Direction Flag")
INSTR(cli,         CLI,         "Clear Interrupt Flag") // NOT IMPLEMENTED
INSTR(clts,        CLTS,        "Clear Task-Switched Flag in CR0") // NOT IMPLEMENTED
INSTR(cmc,         CMC,         "Complement Carry Flag") // NOT IMPLEMENTED
INSTR(cmp,         CMP,         "Compare Two Operands")
INSTR(cmpsb,       CMPSB,       "Compare byte at ES:(E)SI or RSI with byte at ES:(E)DI or RDI and set flags")
INSTR(cmpsw,       CMPSW,       "Compare word at ES:(E)SI or RSI with word at ES:(E)DI or RDI and set flags")
INSTR(cmpsd,       CMPSD,       "Compare doubleword at ES:(E)SI or RSI with doubleword at ES:(E)DI or RDI and set flags")
INSTR(cmpsq,       CMPSQ,       "Compare quadword at ES:(E)SI or RSI with quadword at ES:(E)DI or RDI and set flags")
INSTR(cwd,         CWD,         "AX -> DX:AX (with sign)") // NOT IMPLEMENTED
INSTR(cdq,         CDQ,         "EAX -> EDX:EAX (with sign)") // NOT IMPLEMENTED
INSTR(cqo,         CQO,         "RAX -> RDX:RAX (with sign)") // NOT IMPLEMENTED
INSTR(daa,         DAA,         "Decimal Adjust AL after Addition") // NOT IMPLEMENTED
INSTR(das,         DAS,         "Decimal Adjust AL after Subtraction") // NOT IMPLEMENTED
INSTR(dec,         DEC,         "Decrement by 1")
INSTR(div,         DIV,         "Unsigned Divide")
INSTR(enterw,      ENTERW,      "Make Stack Frame for Procedure Parameters") // NOT IMPLEMENTED
INSTR(enter,       ENTER,       "Make Stack Frame for Procedure Parameters") // NOT IMPLEMENTED
INSTR(enterd,      ENTERD,      "Make Stack Frame for Procedure Parameters") // NOT IMPLEMENTED
INSTR(enterq,      ENTERQ,      "Make Stack Frame for Procedure Parameters") // NOT IMPLEMENTED
INSTR(hlt,         HLT,         "Halt") // NOT IMPLEMENTED
INSTR(idiv,        IDIV,        "Signed Divide")
INSTR(imul,        IMUL,        "Signed Multiply")
INSTR(in,          IN,          "Input from Port") // NOT IMPLEMENTED
INSTR(inc,         INC,         "Increment by 1")
INSTR(ins,         INS,         "Input Byte(s) from Port to String") // NOT IMPLEMENTED
INSTR(int,         INT,         "Call to Interrupt Procedure") // NOT IMPLEMENTED
INSTR(into,        INTO,        "Call to Interrupt Procedure if Overflow Flag = 1") // NOT IMPLEMENTED
INSTR(int3,        INT3,        "Trap to Debugger")
INSTR(iretw,       IRETW,       "Interrupt Return") // NOT IMPLEMENTED
INSTR(iret,        IRET,        "Interrupt Return") // NOT IMPLEMENTED
INSTR(iretd,       IRETD,       "Interrupt Return (use32)") // NOT IMPLEMENTED
INSTR(iretq,       IRETQ,       "Interrupt Return (use64)") // NOT IMPLEMENTED
INSTR(ja,          JA,          "Jump if Above (CF=0 & ZF=0)")
INSTR(jae,         JAE,         "Jump if Above or Equal (CF=0)")
INSTR(jb,          JB,          "Jump if Below (CF=1)")
INSTR(jbe,         JBE,         "Jump if Below or Equal (CF=1 | ZF=1)")
INSTR(jc,          JC,          "Jump if Carry (CF=1)")
INSTR(jcxz,        JCXZ,        "Jump if CX is 0")
INSTR(jecxz,       JECXZ,       "Jump if ECX is 0")
INSTR(jrcxz,       JRCXZ,       "Jump if RCX is 0")
INSTR(je,          JE,          "Jump if Equal (ZF=1)")
INSTR(jg,          JG,          "Jump if Greater (ZF=0 & SF=OF)")
INSTR(jge,         JGE,         "Jump if Greater or Equal (SF=OF)")
INSTR(jl,          JL,          "Jump if Less (SF!=OF)")
INSTR(jle,         JLE,         "Jump if Less or Equal (ZF=1 | SF!=OF)")
INSTR(jna,         JNA,         "Jump if Not Above (CF=1 | ZF=1)")
INSTR(jnae,        JNAE,        "Jump if Not Above or Equal (CF=1)")
INSTR(jnb,         JNB,         "Jump if Not Below (CF=0)")
INSTR(jnbe,        JNBE,        "Jump if Not Below or Equal (CF=0 & ZF=0)")
INSTR(jnc,         JNC,         "Jump if Not Carry (CF=0)")
INSTR(jne,         JNE,         "Jump if Not Equal (ZF=0)")
INSTR(jng,         JNG,         "Jump if Not Greater (ZF=1 | SF!=OF)")
INSTR(jnge,        JNGE,        "Jump if Not Greater or Equal (ZF=1)")
INSTR(jnl,         JNL,         "Jump if Not Less (SF=OF)")
INSTR(jnle,        JNLE,        "Jump if Not Less or Equal (ZF=0 & SF=OF)")
INSTR(jno,         JNO,         "Jump if Not Overflow (OF=0)")
INSTR(jnp,         JNP,         "Jump if Not Parity (PF=0)")
INSTR(jns,         JNS,         "Jump if Not Sign (SF=0)")
INSTR(jnz,         JNZ,         "Jump if Not Zero (ZF=0)")
INSTR(jo,          JO,          "Jump if Overflow (OF=1)")
INSTR(jp,          JP,          "Jump if Parity (PF=1)")
INSTR(jpe,         JPE,         "Jump if Parity Even (PF=1)")
INSTR(jpo,         JPO,         "Jump if Parity Odd  (PF=0)")
INSTR(js,          JS,          "Jump if Sign (SF=1)")
INSTR(jz,          JZ,          "Jump if Zero (ZF=1)")
INSTR(jmp,         JMP,         "Jump")
INSTR(jmpfi,       JMPFI,       "Indirect Far Jump") // NOT IMPLEMENTED
INSTR(jmpni,       JMPNI,       "Indirect Near Jump") // NOT IMPLEMENTED
INSTR(jmpshort,    JMPSHORT,    "Jump Short (not used)") // NOT IMPLEMENTED
INSTR(lahf,        LAHF,        "Load Flags into AH Register") // NOT IMPLEMENTED
INSTR(lar,         LAR,         "Load Access Right Byte") // NOT IMPLEMENTED
INSTR(lea,         LEA,         "Load Effective Address")
INSTR(leavew,      LEAVEW,      "High Level Procedure Exit")
INSTR(leave,       LEAVE,       "High Level Procedure Exit")
INSTR(leaved,      LEAVED,      "High Level Procedure Exit")
INSTR(leaveq,      LEAVEQ,      "High Level Procedure Exit")
INSTR(lgdt,        LGDT,        "Load Global Descriptor Table Register") // NOT IMPLEMENTED
INSTR(lidt,        LIDT,        "Load Interrupt Descriptor Table Register") // NOT IMPLEMENTED
INSTR(lgs,         LGS,         "Load Full Pointer to GS:xx") // NOT IMPLEMENTED
INSTR(lss,         LSS,         "Load Full Pointer to SS:xx") // NOT IMPLEMENTED
INSTR(lds,         LDS,         "Load Full Pointer to DS:xx") // NOT IMPLEMENTED
INSTR(les,         LES,         "Load Full Pointer to ES:xx") // NOT IMPLEMENTED
INSTR(lfs,         LFS,         "Load Full Pointer to FS:xx") // NOT IMPLEMENTED
INSTR(lldt,        LLDT,        "Load Local Descriptor Table Register") // NOT IMPLEMENTED
INSTR(lmsw,        LMSW,        "Load Machine Status Word") // NOT IMPLEMENTED
INSTR(lods,        LODS,        "Load String") // NOT IMPLEMENTED
INSTR(loop,        LOOP,        "Loop while CX != 0")
INSTR(loope,       LOOPE,       "Loop while CX != 0 and ZF=1")
INSTR(loopne,      LOOPNE,      "Loop while CX != 0 and ZF=0")
INSTR(lsl,         LSL,         "Load Segment Limit") // NOT IMPLEMENTED
INSTR(ltr,         LTR,         "Load Task Register") // NOT IMPLEMENTED
INSTR(mov,         MOV,         "Move Data")
INSTR(movsp,       MOVSP,       "Move to/from Special Registers") // NOT IMPLEMENTED
INSTR(movsb,       MOVSB,       "Move Byte(s) from String to String")
INSTR(movsw,       MOVSW,       "Move Word(s) from String to String")
INSTR(movsd,       MOVSD,       "Move Doubleword(s) from String to String")
INSTR(movsq,       MOVSQ,       "Move Quadword(s) from String to String")
INSTR(movsx,       MOVSX,       "Move with Sign-Extend")
INSTR(movzx,       MOVZX,       "Move with Zero-Extend")
INSTR(mul,         MUL,         "Unsigned Multiplication of AL or AX")
INSTR(neg,         NEG,         "Two's Complement Negation")
INSTR(nop,         NOP,         "No Operation")
INSTR(not,         NOT,         "One's Complement Negation")
INSTR(or,          OR,          "Logical Inclusive OR")
INSTR(out,         OUT,         "Output to Port") // NOT IMPLEMENTED
INSTR(outs,        OUTS,        "Output Byte(s) to Port") // NOT IMPLEMENTED
INSTR(pop,         POP,         "Pop a word from the Stack")
INSTR(popaw,       POPAW,       "Pop all General Registers") // NOT IMPLEMENTED
INSTR(popa,        POPA,        "Pop all General Registers") // NOT IMPLEMENTED
INSTR(popad,       POPAD,       "Pop all General Registers (use32)") // NOT IMPLEMENTED
INSTR(popaq,       POPAQ,       "Pop all General Registers (use64)") // NOT IMPLEMENTED
INSTR(popfw,       POPFW,       "Pop Stack into Flags Register") // NOT IMPLEMENTED
INSTR(popf,        POPF,        "Pop Stack into Flags Register")
INSTR(popfd,       POPFD,       "Pop Stack into Eflags Register")
INSTR(popfq,       POPFQ,       "Pop Stack into Rflags Register")
INSTR(push,        PUSH,        "Push Operand onto the Stack")
INSTR(pushaw,      PUSHAW,      "Push all General Registers") // NOT IMPLEMENTED
INSTR(pusha,       PUSHA,       "Push all General Registers") // NOT IMPLEMENTED
INSTR(pushad,      PUSHAD,      "Push all General Registers (use32)") // NOT IMPLEMENTED
INSTR(pushaq,      PUSHAQ,      "Push all General Registers (use64)") // NOT IMPLEMENTED
INSTR(pushfw,      PUSHFW,      "Push Flags Register onto the Stack") // NOT IMPLEMENTED
INSTR(pushfd,      PUSHFD,      "Push Flags Register onto the Stack (use32)") // NOT IMPLEMENTED
INSTR(pushfq,      PUSHFQ,      "Push Flags Register onto the Stack (use64)") // NOT IMPLEMENTED
INSTR(rcl,         RCL,         "Rotate Through Carry Left") // NOT IMPLEMENTED
INSTR(rcr,         RCR,         "Rotate Through Carry Right") // NOT IMPLEMENTED
INSTR(rol,         ROL,         "Rotate Left") // NOT IMPLEMENTED
INSTR(ror,         ROR,         "Rotate Right") // NOT IMPLEMENTED
INSTR(ret,         RET,         "Return Near from Procedure")
INSTR(retf,        RETF,        "Return Far from Procedure") // NOT IMPLEMENTED
INSTR(sahf,        SAHF,        "Store AH into Flags Register") // NOT IMPLEMENTED
INSTR(sal,         SAL,         "Shift Arithmetic Left")
INSTR(sar,         SAR,         "Shift Arithmetic Right")
INSTR(shl,         SHL,         "Shift Logical Left")
INSTR(shr,         SHR,         "Shift Logical Right")
INSTR(sbb,         SBB,         "Integer Subtraction with Borrow")
INSTR(scasb,       SCASB,       "Compare AL with byte at ES:(E)DI or RDI, then set status flags")
INSTR(scasw,       SCASW,       "Compare AX with word at ES:(E)DI or RDI, then set status flags")
INSTR(scasd,       SCASD,       "Compare EAX with doubleword at ES:(E)DI or RDI, then set status flags")
INSTR(scasq,       SCASQ,       "Compare RAX with quadword at ES:(E)DI or RDI, then set status flags")
INSTR(seta,        SETA,        "Set Byte if Above (CF=0 & ZF=0)")
INSTR(setae,       SETAE,       "Set Byte if Above or Equal (CF=0)")
INSTR(setb,        SETB,        "Set Byte if Below (CF=1)")
INSTR(setbe,       SETBE,       "Set Byte if Below or Equal (CF=1 | ZF=1)")
INSTR(setc,        SETC,        "Set Byte if Carry (CF=1)")
INSTR(sete,        SETE,        "Set Byte if Equal (ZF=1)")
INSTR(setg,        SETG,        "Set Byte if Greater (ZF=0 & SF=OF)")
INSTR(setge,       SETGE,       "Set Byte if Greater or Equal (SF=OF)")
INSTR(setl,        SETL,        "Set Byte if Less (SF!=OF)")
INSTR(setle,       SETLE,       "Set Byte if Less or Equal (ZF=1 | SF!=OF)")
INSTR(setna,       SETNA,       "Set Byte if Not Above (CF=1 | ZF=1)")
INSTR(setnae,      SETNAE,      "Set Byte if Not Above or Equal (CF=1)")
INSTR(setnb,       SETNB,       "Set Byte if Not Below (CF=0)")
INSTR(setnbe,      SETNBE,      "Set Byte if Not Below or Equal (CF=0 & ZF=0)")
INSTR(setnc,       SETNC,       "Set Byte if Not Carry (CF=0)")
INSTR(setne,       SETNE,       "Set Byte if Not Equal (ZF=0)")
INSTR(setng,       SETNG,       "Set Byte if Not Greater (ZF=1 | SF!=OF)")
INSTR(setnge,      SETNGE,      "Set Byte if Not Greater or Equal (ZF=1)")
INSTR(setnl,       SETNL,       "Set Byte if Not Less (SF=OF)")
INSTR(setnle,      SETNLE,      "Set Byte if Not Less or Equal (ZF=0 & SF=OF)")
INSTR(setno,       SETNO,       "Set Byte if Not Overflow (OF=0)")
INSTR(setnp,       SETNP,       "Set Byte if Not Parity (PF=0)")
INSTR(setns,       SETNS,       "Set Byte if Not Sign (SF=0)")
INSTR(setnz,       SETNZ,       "Set Byte if Not Zero (ZF=0)")
INSTR(seto,        SETO,        "Set Byte if Overflow (OF=1)")
INSTR(setp,        SETP,        "Set Byte if Parity (PF=1)")
INSTR(setpe,       SETPE,       "Set Byte if Parity Even (PF=1)")
INSTR(setpo,       SETPO,       "Set Byte if Parity Odd  (PF=0)")
INSTR(sets,        SETS,        "Set Byte if Sign (SF=1)")
INSTR(setz,        SETZ,        "Set Byte if Zero (ZF=1)")
INSTR(sgdt,        SGDT,        "Store Global Descriptor Table Register") // NOT IMPLEMENTED
INSTR(sidt,        SIDT,        "Store Interrupt Descriptor Table Register") // NOT IMPLEMENTED
INSTR(shld,        SHLD,        "Double Precision Shift Left") // NOT IMPLEMENTED
INSTR(shrd,        SHRD,        "Double Precision Shift Right") // NOT IMPLEMENTED
INSTR(sldt,        SLDT,        "Store Local Descriptor Table Register") // NOT IMPLEMENTED
INSTR(smsw,        SMSW,        "Store Machine Status Word") // NOT IMPLEMENTED
INSTR(stc,         STC,         "Set Carry Flag") // NOT IMPLEMENTED
INSTR(std,         STD,         "Set Direction Flag")
INSTR(sti,         STI,         "Set Interrupt Flag") // NOT IMPLEMENTED
INSTR(stosb,       STOSB,       "Store AL to ES:(E)DI or RDI")
INSTR(stosw,       STOSW,       "Store AX to ES:(E)DI or RDI")
INSTR(stosd,       STOSD,       "Store EAX to ES:(E)DI or RDI")
INSTR(stosq,       STOSQ,       "Store RAX to ES:(E)DI or RDI")
INSTR(str,         STR,         "Store Task Register") // NOT IMPLEMENTED
INSTR(sub,         SUB,         "Integer Subtraction")
INSTR(test,        TEST,        "Logical Compare")
INSTR(verr,        VERR,        "Verify a Segment for Reading") // NOT IMPLEMENTED
INSTR(verw,        VERW,        "Verify a Segment for Writing") // NOT IMPLEMENTED
INSTR(wait,        WAIT,        "Wait until BUSY# Pin is Inactive (HIGH)") // NOT IMPLEMENTED
INSTR(xchg,        XCHG,        "Exchange Register/Memory with Register")
INSTR(xlat,        XLAT,        "Table Lookup Translation") // NOT IMPLEMENTED
INSTR(xor,         XOR,         "Logical Exclusive OR")

/* 486 instructions. */
INSTR(cmpxchg,     CMPXCHG,     "Compare and Exchange") // NOT IMPLEMENTED
INSTR(bswap,       BSWAP,       "Swap bits in EAX") // NOT IMPLEMENTED
INSTR(xadd,        XADD,        "t:=dest; dest:=src+dest; src:=t") // NOT IMPLEMENTED
INSTR(invd,        INVD,        "Invalidate Data Cache") // NOT IMPLEMENTED
INSTR(wbinvd,      WBINVD,      "Invalidate Data Cache (write changes)") // NOT IMPLEMENTED
INSTR(invlpg,      INVLPG,      "Invalidate TLB entry") // NOT IMPLEMENTED

/* Pentium instructions. */
INSTR(rdmsr,       RDMSR,       "Read Machine Status Register") // NOT IMPLEMENTED
INSTR(wrmsr,       WRMSR,       "Write Machine Status Register") // NOT IMPLEMENTED
INSTR(cpuid,       CPUID,       "Get CPU ID")
INSTR(cmpxchg8b,   CMPXCHG8B,   "Compare and Exchange Eight Bytes") // NOT IMPLEMENTED
INSTR(rdtsc,       RDTSC,       "Read Time Stamp Counter") // NOT IMPLEMENTED
INSTR(rdtscp,      RDTSCP,      "Read Time-Stamp Counter and Processor ID") // NOT IMPLEMENTED
INSTR(rsm,         RSM,         "Resume from System Management Mode") // NOT IMPLEMENTED

/* Pentium Pro instructions. */
INSTR(cmova,       CMOVA,       "Move if Above (CF=0 & ZF=0)")
INSTR(cmovae,      CMOVAE,      "Move if Above or Equal (CF=0)")
INSTR(cmovb,       CMOVB,       "Move if Below (CF=1)")
INSTR(cmovbe,      CMOVBE,      "Move if Below or Equal (CF=1 | ZF=1)")
INSTR(cmove,       CMOVE,       "Move if Equal (ZF=1)")
INSTR(cmovg,       CMOVG,       "Move if Greater (ZF=0 & SF=OF)")
INSTR(cmovge,      CMOVGE,      "Move if Greater or Equal (SF=OF)")
INSTR(cmovl,       CMOVL,       "Move if Less (SF!=OF)")
INSTR(cmovle,      CMOVLE,      "Move if Less or Equal (ZF=1 | SF!=OF)")
INSTR(cmovnb,      CMOVNB,      "Move if Not Below (CF=0)")
INSTR(cmovne,      CMOVNE,      "Move if Not Equal (ZF=0)")
INSTR(cmovno,      CMOVNO,      "Move if Not Overflow (OF=0)")
INSTR(cmovnp,      CMOVNP,      "Move if Not Parity (PF=0)")
INSTR(cmovns,      CMOVNS,      "Move if Not Sign (SF=0)")
INSTR(cmovnz,      CMOVNZ,      "Move if Not Zero (ZF=0)")
INSTR(cmovo,       CMOVO,       "Move if Overflow (OF=1)")
INSTR(cmovp,       CMOVP,       "Move if Parity (PF=1)")
INSTR(cmovs,       CMOVS,       "Move if Sign (SF=1)")
INSTR(cmovz,       CMOVZ,       "Move if Zero (ZF=1)")
INSTR(fcmovb,      FCMOVB,      "Floating Move if Below") // NOT IMPLEMENTED
INSTR(fcmovbe,     FCMOVBE,     "Floating Move if Below or Equal") // NOT IMPLEMENTED
INSTR(fcmove,      FCMOVE,      "Floating Move if Equal") // NOT IMPLEMENTED
INSTR(fcmovnb,     FCMOVNB,     "Floating Move if Not Below") // NOT IMPLEMENTED
INSTR(fcmovnbe,    FCMOVNBE,    "Floating Move if Not Below or Equal") // NOT IMPLEMENTED
INSTR(fcmovne,     FCMOVNE,     "Floating Move if Not Equal") // NOT IMPLEMENTED
INSTR(fcmovnu,     FCMOVNU,     "Floating Move if Not Unordered") // NOT IMPLEMENTED
INSTR(fcmovu,      FCMOVU,      "Floating Move if Unordered") // NOT IMPLEMENTED
INSTR(fcomi,       FCOMI,       "FP Compare, result in EFLAGS") // NOT IMPLEMENTED
INSTR(fcomip,      FCOMIP,      "FP Compare, result in EFLAGS, pop stack") // NOT IMPLEMENTED
INSTR(fucomi,      FUCOMI,      "FP Unordered Compare, result in EFLAGS") // NOT IMPLEMENTED
INSTR(fucomip,     FUCOMIP,     "FP Unordered Compare, result in EFLAGS, pop stack") // NOT IMPLEMENTED
INSTR(rdpmc,       RDPMC,       "Read Performance Monitor Counter") // NOT IMPLEMENTED

/* FPP instructuions. */
INSTR(fld,         FLD,         "Load Real") // NOT IMPLEMENTED
INSTR(fst,         FST,         "Store Real") // NOT IMPLEMENTED
INSTR(fstp,        FSTP,        "Store Real and Pop") // NOT IMPLEMENTED
INSTR(fxch,        FXCH,        "Exchange Registers") // NOT IMPLEMENTED
INSTR(fild,        FILD,        "Load Integer") // NOT IMPLEMENTED
INSTR(fist,        FIST,        "Store Integer") // NOT IMPLEMENTED
INSTR(fistp,       FISTP,       "Store Integer and Pop") // NOT IMPLEMENTED
INSTR(fbld,        FBLD,        "Load BCD") // NOT IMPLEMENTED
INSTR(fbstp,       FBSTP,       "Store BCD and Pop") // NOT IMPLEMENTED
INSTR(fadd,        FADD,        "Add Real") // NOT IMPLEMENTED
INSTR(faddp,       FADDP,       "Add Real and Pop") // NOT IMPLEMENTED
INSTR(fiadd,       FIADD,       "Add Integer") // NOT IMPLEMENTED
INSTR(fsub,        FSUB,        "Subtract Real") // NOT IMPLEMENTED
INSTR(fsubp,       FSUBP,       "Subtract Real and Pop") // NOT IMPLEMENTED
INSTR(fisub,       FISUB,       "Subtract Integer") // NOT IMPLEMENTED
INSTR(fsubr,       FSUBR,       "Subtract Real Reversed") // NOT IMPLEMENTED
INSTR(fsubrp,      FSUBRP,      "Subtract Real Reversed and Pop") // NOT IMPLEMENTED
INSTR(fisubr,      FISUBR,      "Subtract Integer Reversed") // NOT IMPLEMENTED
INSTR(fmul,        FMUL,        "Multiply Real") // NOT IMPLEMENTED
INSTR(fmulp,       FMULP,       "Multiply Real and Pop") // NOT IMPLEMENTED
INSTR(fimul,       FIMUL,       "Multiply Integer") // NOT IMPLEMENTED
INSTR(fdiv,        FDIV,        "Divide Real") // NOT IMPLEMENTED
INSTR(fdivp,       FDIVP,       "Divide Real and Pop") // NOT IMPLEMENTED
INSTR(fidiv,       FIDIV,       "Divide Integer") // NOT IMPLEMENTED
INSTR(fdivr,       FDIVR,       "Divide Real Reversed") // NOT IMPLEMENTED
INSTR(fdivrp,      FDIVRP,      "Divide Real Reversed and Pop") // NOT IMPLEMENTED
INSTR(fidivr,      FIDIVR,      "Divide Integer Reversed") // NOT IMPLEMENTED
INSTR(fsqrt,       FSQRT,       "Square Root") // NOT IMPLEMENTED
INSTR(fscale,      FSCALE,      "Scale:  st(0) <- st(0) * 2^st(1)") // NOT IMPLEMENTED
INSTR(fprem,       FPREM,       "Partial Remainder") // NOT IMPLEMENTED
INSTR(frndint,     FRNDINT,     "Round to Integer") // NOT IMPLEMENTED
INSTR(fxtract,     FXTRACT,     "Extract exponent and significand") // NOT IMPLEMENTED
INSTR(fabs,        FABS,        "Absolute value") // NOT IMPLEMENTED
INSTR(fchs,        FCHS,        "Change Sign") // NOT IMPLEMENTED
INSTR(fcom,        FCOM,        "Compare Real") // NOT IMPLEMENTED
INSTR(fcomp,       FCOMP,       "Compare Real and Pop") // NOT IMPLEMENTED
INSTR(fcompp,      FCOMPP,      "Compare Real and Pop Twice") // NOT IMPLEMENTED
INSTR(ficom,       FICOM,       "Compare Integer") // NOT IMPLEMENTED
INSTR(ficomp,      FICOMP,      "Compare Integer and Pop") // NOT IMPLEMENTED
INSTR(ftst,        FTST,        "Test") // NOT IMPLEMENTED
INSTR(fxam,        FXAM,        "Examine") // NOT IMPLEMENTED
INSTR(fptan,       FPTAN,       "Partial tangent") // NOT IMPLEMENTED
INSTR(fpatan,      FPATAN,      "Partial arctangent") // NOT IMPLEMENTED
INSTR(f2xm1,       F2XM1,       "2^x - 1") // NOT IMPLEMENTED
INSTR(fyl2x,       FYL2X,       "Y * lg2(X)") // NOT IMPLEMENTED
INSTR(fyl2xp1,     FYL2XP1,     "Y * lg2(X+1)") // NOT IMPLEMENTED
INSTR(fldz,        FLDZ,        "Load +0.0") // NOT IMPLEMENTED
INSTR(fld1,        FLD1,        "Load +1.0") // NOT IMPLEMENTED
INSTR(fldpi,       FLDPI,       "Load PI=3.14...") // NOT IMPLEMENTED
INSTR(fldl2t,      FLDL2T,      "Load lg2(10)") // NOT IMPLEMENTED
INSTR(fldl2e,      FLDL2E,      "Load lg2(e)") // NOT IMPLEMENTED
INSTR(fldlg2,      FLDLG2,      "Load lg10(2)") // NOT IMPLEMENTED
INSTR(fldln2,      FLDLN2,      "Load ln(2)") // NOT IMPLEMENTED
INSTR(finit,       FINIT,       "Initialize Processor") // NOT IMPLEMENTED
INSTR(fninit,      FNINIT,      "Initialize Processor (no wait)") // NOT IMPLEMENTED
INSTR(fsetpm,      FSETPM,      "Set Protected Mode") // NOT IMPLEMENTED
INSTR(fldcw,       FLDCW,       "Load Control Word") // NOT IMPLEMENTED
INSTR(fstcw,       FSTCW,       "Store Control Word") // NOT IMPLEMENTED
INSTR(fnstcw,      FNSTCW,      "Store Control Word (no wait)") // NOT IMPLEMENTED
INSTR(fstsw,       FSTSW,       "Store Status Word") // NOT IMPLEMENTED
INSTR(fnstsw,      FNSTSW,      "Store Status Word (no wait)") // NOT IMPLEMENTED
INSTR(fclex,       FCLEX,       "Clear Exceptions") // NOT IMPLEMENTED
INSTR(fnclex,      FNCLEX,      "Clear Exceptions (no wait)") // NOT IMPLEMENTED
INSTR(fstenv,      FSTENV,      "Store Environment") // NOT IMPLEMENTED
INSTR(fnstenv,     FNSTENV,     "Store Environment (no wait)") // NOT IMPLEMENTED
INSTR(fldenv,      FLDENV,      "Load Environment") // NOT IMPLEMENTED
INSTR(fsave,       FSAVE,       "Save State") // NOT IMPLEMENTED
INSTR(fnsave,      FNSAVE,      "Save State (no wait)") // NOT IMPLEMENTED
INSTR(frstor,      FRSTOR,      "Restore State") // NOT IMPLEMENTED
INSTR(fincstp,     FINCSTP,     "Increment Stack Pointer") // NOT IMPLEMENTED
INSTR(fdecstp,     FDECSTP,     "Decrement Stack Pointer") // NOT IMPLEMENTED
INSTR(ffree,       FFREE,       "Free Register") // NOT IMPLEMENTED
INSTR(fnop,        FNOP,        "No Operation") // NOT IMPLEMENTED
INSTR(feni,        FENI,        "(8087 only)") // NOT IMPLEMENTED
INSTR(fneni,       FNENI,       "(no wait) (8087 only)") // NOT IMPLEMENTED
INSTR(fdisi,       FDISI,       "(8087 only)") // NOT IMPLEMENTED
INSTR(fndisi,      FNDISI,      "(no wait) (8087 only)") // NOT IMPLEMENTED

/* 80387 instructions. */
INSTR(fprem1,      FPREM1,      "Partial Remainder ( < half )") // NOT IMPLEMENTED
INSTR(fsincos,     FSINCOS,     "t<-cos(st); st<-sin(st); push t") // NOT IMPLEMENTED
INSTR(fsin,        FSIN,        "Sine") // NOT IMPLEMENTED
INSTR(fcos,        FCOS,        "Cosine") // NOT IMPLEMENTED
INSTR(fucom,       FUCOM,       "Compare Unordered Real") // NOT IMPLEMENTED
INSTR(fucomp,      FUCOMP,      "Compare Unordered Real and Pop") // NOT IMPLEMENTED
INSTR(fucompp,     FUCOMPP,     "Compare Unordered Real and Pop Twice") // NOT IMPLEMENTED

/* Undocumented instructions. */
INSTR(setalc,      SETALC,      "Set AL to Carry Flag") // NOT IMPLEMENTED
INSTR(svdc,        SVDC,        "Save Register and Descriptor") // NOT IMPLEMENTED
INSTR(rsdc,        RSDC,        "Restore Register and Descriptor") // NOT IMPLEMENTED
INSTR(svldt,       SVLDT,       "Save LDTR and Descriptor") // NOT IMPLEMENTED
INSTR(rsldt,       RSLDT,       "Restore LDTR and Descriptor") // NOT IMPLEMENTED
INSTR(svts,        SVTS,        "Save TR and Descriptor") // NOT IMPLEMENTED
INSTR(rsts,        RSTS,        "Restore TR and Descriptor") // NOT IMPLEMENTED
INSTR(icebp,       ICEBP,       "ICE Break Point") // NOT IMPLEMENTED
INSTR(loadall,     LOADALL,     "Load the entire CPU state from ES:EDI") // NOT IMPLEMENTED /* Note that this is a 386 LOADALL (opcode 0F07). There also exists a 286 LOADALL (opcode 0F05). */

/* MMX instructions. */
INSTR(emms,        EMMS,        "Empty MMX state") // NOT IMPLEMENTED
INSTR(movd,        MOVD,        "Move 32 bits") // NOT IMPLEMENTED
INSTR(movq,        MOVQ,        "Move 64 bits") // NOT IMPLEMENTED
INSTR(packsswb,    PACKSSWB,    "Pack with Signed Saturation (Word->Byte)") // NOT IMPLEMENTED
INSTR(packssdw,    PACKSSDW,    "Pack with Signed Saturation (Dword->Word)") // NOT IMPLEMENTED
INSTR(packuswb,    PACKUSWB,    "Pack with Unsigned Saturation (Word->Byte)") // NOT IMPLEMENTED
INSTR(paddb,       PADDB,       "Packed Add Byte") // NOT IMPLEMENTED
INSTR(paddw,       PADDW,       "Packed Add Word") // NOT IMPLEMENTED
INSTR(paddd,       PADDD,       "Packed Add Dword") // NOT IMPLEMENTED
INSTR(paddsb,      PADDSB,      "Packed Add with Saturation (Byte)") // NOT IMPLEMENTED
INSTR(paddsw,      PADDSW,      "Packed Add with Saturation (Word)") // NOT IMPLEMENTED
INSTR(paddusb,     PADDUSB,     "Packed Add Unsigned with Saturation (Byte)") // NOT IMPLEMENTED
INSTR(paddusw,     PADDUSW,     "Packed Add Unsigned with Saturation (Word)") // NOT IMPLEMENTED
INSTR(pand,        PAND,        "Bitwise Logical And") // NOT IMPLEMENTED
INSTR(pandn,       PANDN,       "Bitwise Logical And Not") // NOT IMPLEMENTED
INSTR(pcmpeqb,     PCMPEQB,     "Packed Compare for Equal (Byte)") // NOT IMPLEMENTED
INSTR(pcmpeqw,     PCMPEQW,     "Packed Compare for Equal (Word)") // NOT IMPLEMENTED
INSTR(pcmpeqd,     PCMPEQD,     "Packed Compare for Equal (Dword)") // NOT IMPLEMENTED
INSTR(pcmpgtb,     PCMPGTB,     "Packed Compare for Greater Than (Byte)") // NOT IMPLEMENTED
INSTR(pcmpgtw,     PCMPGTW,     "Packed Compare for Greater Than (Word)") // NOT IMPLEMENTED
INSTR(pcmpgtd,     PCMPGTD,     "Packed Compare for Greater Than (Dword)") // NOT IMPLEMENTED
INSTR(pmaddwd,     PMADDWD,     "Packed Multiply and Add") // NOT IMPLEMENTED
INSTR(pmulhw,      PMULHW,      "Packed Multiply High") // NOT IMPLEMENTED
INSTR(pmullw,      PMULLW,      "Packed Multiply Low") // NOT IMPLEMENTED
INSTR(por,         POR,         "Bitwise Logical Or") // NOT IMPLEMENTED
INSTR(psllw,       PSLLW,       "Packed Shift Left Logical (Word)") // NOT IMPLEMENTED
INSTR(pslld,       PSLLD,       "Packed Shift Left Logical (Dword)") // NOT IMPLEMENTED
INSTR(psllq,       PSLLQ,       "Packed Shift Left Logical (Qword)") // NOT IMPLEMENTED
INSTR(psraw,       PSRAW,       "Packed Shift Right Arithmetic (Word)") // NOT IMPLEMENTED
INSTR(psrad,       PSRAD,       "Packed Shift Right Arithmetic (Dword)") // NOT IMPLEMENTED
INSTR(psrlw,       PSRLW,       "Packed Shift Right Logical (Word)") // NOT IMPLEMENTED
INSTR(psrld,       PSRLD,       "Packed Shift Right Logical (Dword)") // NOT IMPLEMENTED
INSTR(psrlq,       PSRLQ,       "Packed Shift Right Logical (Qword)") // NOT IMPLEMENTED
INSTR(psubb,       PSUBB,       "Packed Subtract Byte") // NOT IMPLEMENTED
INSTR(psubw,       PSUBW,       "Packed Subtract Word") // NOT IMPLEMENTED
INSTR(psubd,       PSUBD,       "Packed Subtract Dword") // NOT IMPLEMENTED
INSTR(psubsb,      PSUBSB,      "Packed Subtract with Saturation (Byte)") // NOT IMPLEMENTED
INSTR(psubsw,      PSUBSW,      "Packed Subtract with Saturation (Word)") // NOT IMPLEMENTED
INSTR(psubusb,     PSUBUSB,     "Packed Subtract Unsigned with Saturation (Byte)") // NOT IMPLEMENTED
INSTR(psubusw,     PSUBUSW,     "Packed Subtract Unsigned with Saturation (Word)") // NOT IMPLEMENTED
INSTR(punpckhbw,   PUNPCKHBW,   "Unpack High Packed Data (Byte->Word)") // NOT IMPLEMENTED
INSTR(punpckhwd,   PUNPCKHWD,   "Unpack High Packed Data (Word->Dword)") // NOT IMPLEMENTED
INSTR(punpckhdq,   PUNPCKHDQ,   "Unpack High Packed Data (Dword->Qword)") // NOT IMPLEMENTED
INSTR(punpcklbw,   PUNPCKLBW,   "Unpack Low Packed Data (Byte->Word)") // NOT IMPLEMENTED
INSTR(punpcklwd,   PUNPCKLWD,   "Unpack Low Packed Data (Word->Dword)") // NOT IMPLEMENTED
INSTR(punpckldq,   PUNPCKLDQ,   "Unpack Low Packed Data (Dword->Qword)") // NOT IMPLEMENTED
INSTR(pxor,        PXOR,        "Bitwise Logical Exclusive Or") // NOT IMPLEMENTED

/* Undocumented Deschutes processor instructions. */
INSTR(fxsave,      FXSAVE,      "Fast save FP context") // NOT IMPLEMENTED
INSTR(fxrstor,     FXRSTOR,     "Fast restore FP context") // NOT IMPLEMENTED

/* Pentium II instructions. */
INSTR(sysenter,    SYSENTER,    "Fast Transition to System Call Entry Point") // NOT IMPLEMENTED
INSTR(sysexit,     SYSEXIT,     "Fast Transition from System Call Entry Point") // NOT IMPLEMENTED

/* 3DNow! instructions. */
INSTR(pavgusb,     PAVGUSB,     "Packed 8-bit Unsigned Integer Averaging") // NOT IMPLEMENTED
INSTR(pfadd,       PFADD,       "Packed Floating-Point Addition") // NOT IMPLEMENTED
INSTR(pfsub,       PFSUB,       "Packed Floating-Point Subtraction") // NOT IMPLEMENTED
INSTR(pfsubr,      PFSUBR,      "Packed Floating-Point Reverse Subtraction") // NOT IMPLEMENTED
INSTR(pfacc,       PFACC,       "Packed Floating-Point Accumulate") // NOT IMPLEMENTED
INSTR(pfcmpge,     PFCMPGE,     "Packed Floating-Point Comparison, Greater or Equal") // NOT IMPLEMENTED
INSTR(pfcmpgt,     PFCMPGT,     "Packed Floating-Point Comparison, Greater") // NOT IMPLEMENTED
INSTR(pfcmpeq,     PFCMPEQ,     "Packed Floating-Point Comparison, Equal") // NOT IMPLEMENTED
INSTR(pfmin,       PFMIN,       "Packed Floating-Point Minimum") // NOT IMPLEMENTED
INSTR(pfmax,       PFMAX,       "Packed Floating-Point Maximum") // NOT IMPLEMENTED
INSTR(pi2fd,       PI2FD,       "Packed 32-bit Integer to Floating-Point") // NOT IMPLEMENTED
INSTR(pf2id,       PF2ID,       "Packed Floating-Point to 32-bit Integer") // NOT IMPLEMENTED
INSTR(pfrcp,       PFRCP,       "Packed Floating-Point Reciprocal Approximation") // NOT IMPLEMENTED
INSTR(pfrsqrt,     PFRSQRT,     "Packed Floating-Point Reciprocal Square Root Approximation") // NOT IMPLEMENTED
INSTR(pfmul,       PFMUL,       "Packed Floating-Point Multiplication") // NOT IMPLEMENTED
INSTR(pfrcpit1,    PFRCPIT1,    "Packed Floating-Point Reciprocal First Iteration Step") // NOT IMPLEMENTED
INSTR(pfrsqit1,    PFRSQIT1,    "Packed Floating-Point Reciprocal Square Root First Iteration Step") // NOT IMPLEMENTED
INSTR(pfrcpit2,    PFRCPIT2,    "Packed Floating-Point Reciprocal Second Iteration Step") // NOT IMPLEMENTED
INSTR(pmulhrw,     PMULHRW,     "Packed Floating-Point 16-bit Integer Multiply with rounding") // NOT IMPLEMENTED
INSTR(femms,       FEMMS,       "Faster entry/exit of the MMX or floating-point state") // NOT IMPLEMENTED
INSTR(prefetch,    PREFETCH,    "Prefetch at least a 32-byte line into L1 data cache") // NOT IMPLEMENTED
INSTR(prefetchw,   PREFETCHW,   "Prefetch processor cache line into L1 data cache (mark as modified)") // NOT IMPLEMENTED

/* Pentium III instructions. */
INSTR(addps,       ADDPS,       "Packed Single-FP Add") // NOT IMPLEMENTED
INSTR(addss,       ADDSS,       "Scalar Single-FP Add") // NOT IMPLEMENTED
INSTR(andnps,      ANDNPS,      "Bitwise Logical And Not for Single-FP") // NOT IMPLEMENTED
INSTR(andps,       ANDPS,       "Bitwise Logical And for Single-FP") // NOT IMPLEMENTED
INSTR(cmpps,       CMPPS,       "Packed Single-FP Compare") // NOT IMPLEMENTED
INSTR(cmpss,       CMPSS,       "Scalar Single-FP Compare") // NOT IMPLEMENTED
INSTR(comiss,      COMISS,      "Scalar Ordered Single-FP Compare and Set EFLAGS") // NOT IMPLEMENTED
INSTR(cvtpi2ps,    CVTPI2PS,    "Packed signed INT32 to Packed Single-FP conversion") // NOT IMPLEMENTED
INSTR(cvtps2pi,    CVTPS2PI,    "Packed Single-FP to Packed INT32 conversion") // NOT IMPLEMENTED
INSTR(cvtsi2ss,    CVTSI2SS,    "Scalar signed INT32 to Single-FP conversion") // NOT IMPLEMENTED
INSTR(cvtss2si,    CVTSS2SI,    "Scalar Single-FP to signed INT32 conversion") // NOT IMPLEMENTED
INSTR(cvttps2pi,   CVTTPS2PI,   "Packed Single-FP to Packed INT32 conversion (truncate)") // NOT IMPLEMENTED
INSTR(cvttss2si,   CVTTSS2SI,   "Scalar Single-FP to signed INT32 conversion (truncate)") // NOT IMPLEMENTED
INSTR(divps,       DIVPS,       "Packed Single-FP Divide") // NOT IMPLEMENTED
INSTR(divss,       DIVSS,       "Scalar Single-FP Divide") // NOT IMPLEMENTED
INSTR(ldmxcsr,     LDMXCSR,     "Load Streaming SIMD Extensions Technology Control/Status Register") // NOT IMPLEMENTED
INSTR(maxps,       MAXPS,       "Packed Single-FP Maximum") // NOT IMPLEMENTED
INSTR(maxss,       MAXSS,       "Scalar Single-FP Maximum") // NOT IMPLEMENTED
INSTR(minps,       MINPS,       "Packed Single-FP Minimum") // NOT IMPLEMENTED
INSTR(minss,       MINSS,       "Scalar Single-FP Minimum") // NOT IMPLEMENTED
INSTR(movaps,      MOVAPS,      "Move Aligned Four Packed Single-FP") // NOT IMPLEMENTED
INSTR(movhlps,     MOVHLPS,     "Move High to Low Packed Single-FP") // NOT IMPLEMENTED
INSTR(movhps,      MOVHPS,      "Move High Packed Single-FP") // NOT IMPLEMENTED
INSTR(movlhps,     MOVLHPS,     "Move Low to High Packed Single-FP") // NOT IMPLEMENTED
INSTR(movlps,      MOVLPS,      "Move Low Packed Single-FP") // NOT IMPLEMENTED
INSTR(movmskps,    MOVMSKPS,    "Move Mask to Register") // NOT IMPLEMENTED
INSTR(movss,       MOVSS,       "Move Scalar Single-FP") // NOT IMPLEMENTED
INSTR(movups,      MOVUPS,      "Move Unaligned Four Packed Single-FP") // NOT IMPLEMENTED
INSTR(mulps,       MULPS,       "Packed Single-FP Multiply") // NOT IMPLEMENTED
INSTR(mulss,       MULSS,       "Scalar Single-FP Multiply") // NOT IMPLEMENTED
INSTR(orps,        ORPS,        "Bitwise Logical OR for Single-FP Data") // NOT IMPLEMENTED
INSTR(rcpps,       RCPPS,       "Packed Single-FP Reciprocal") // NOT IMPLEMENTED
INSTR(rcpss,       RCPSS,       "Scalar Single-FP Reciprocal") // NOT IMPLEMENTED
INSTR(rsqrtps,     RSQRTPS,     "Packed Single-FP Square Root Reciprocal") // NOT IMPLEMENTED
INSTR(rsqrtss,     RSQRTSS,     "Scalar Single-FP Square Root Reciprocal") // NOT IMPLEMENTED
INSTR(shufps,      SHUFPS,      "Shuffle Single-FP") // NOT IMPLEMENTED
INSTR(sqrtps,      SQRTPS,      "Packed Single-FP Square Root") // NOT IMPLEMENTED
INSTR(sqrtss,      SQRTSS,      "Scalar Single-FP Square Root") // NOT IMPLEMENTED
INSTR(stmxcsr,     STMXCSR,     "Store Streaming SIMD Extensions Technology Control/Status Register") // NOT IMPLEMENTED
INSTR(subps,       SUBPS,       "Packed Single-FP Subtract") // NOT IMPLEMENTED
INSTR(subss,       SUBSS,       "Scalar Single-FP Subtract") // NOT IMPLEMENTED
INSTR(ucomiss,     UCOMISS,     "Scalar Unordered Single-FP Compare and Set EFLAGS") // NOT IMPLEMENTED
INSTR(unpckhps,    UNPCKHPS,    "Unpack High Packed Single-FP Data") // NOT IMPLEMENTED
INSTR(unpcklps,    UNPCKLPS,    "Unpack Low Packed Single-FP Data") // NOT IMPLEMENTED
INSTR(xorps,       XORPS,       "Bitwise Logical XOR for Single-FP Data") // NOT IMPLEMENTED
INSTR(pavgb,       PAVGB,       "Packed Average (Byte)") // NOT IMPLEMENTED
INSTR(pavgw,       PAVGW,       "Packed Average (Word)") // NOT IMPLEMENTED
INSTR(pextrw,      PEXTRW,      "Extract Word") // NOT IMPLEMENTED
INSTR(pinsrw,      PINSRW,      "Insert Word") // NOT IMPLEMENTED
INSTR(pmaxsw,      PMAXSW,      "Packed Signed Integer Word Maximum") // NOT IMPLEMENTED
INSTR(pmaxub,      PMAXUB,      "Packed Unsigned Integer Byte Maximum") // NOT IMPLEMENTED
INSTR(pminsw,      PMINSW,      "Packed Signed Integer Word Minimum") // NOT IMPLEMENTED
INSTR(pminub,      PMINUB,      "Packed Unsigned Integer Byte Minimum") // NOT IMPLEMENTED
INSTR(pmovmskb,    PMOVMSKB,    "Move Byte Mask to Integer") // NOT IMPLEMENTED
INSTR(pmulhuw,     PMULHUW,     "Packed Multiply High Unsigned") // NOT IMPLEMENTED
INSTR(psadbw,      PSADBW,      "Packed Sum of Absolute Differences") // NOT IMPLEMENTED
INSTR(pshufw,      PSHUFW,      "Packed Shuffle Word") // NOT IMPLEMENTED
INSTR(maskmovq,    MASKMOVQ,    "Byte Mask write") // NOT IMPLEMENTED
INSTR(movntps,     MOVNTPS,     "Move Aligned Four Packed Single-FP Non Temporal") // NOT IMPLEMENTED
INSTR(movntq,      MOVNTQ,      "Move 64 Bits Non Temporal") // NOT IMPLEMENTED
INSTR(prefetcht0,  PREFETCHT0,  "Prefetch to all cache levels") // NOT IMPLEMENTED
INSTR(prefetcht1,  PREFETCHT1,  "Prefetch to all cache levels") // NOT IMPLEMENTED
INSTR(prefetcht2,  PREFETCHT2,  "Prefetch to L2 cache") // NOT IMPLEMENTED
INSTR(prefetchnta, PREFETCHNTA, "Prefetch to L1 cache") // NOT IMPLEMENTED
INSTR(sfence,      SFENCE,      "Store Fence") // NOT IMPLEMENTED

/* Pentium III Pseudo instructions. */
INSTR(cmpeqps,     CMPEQPS,     "Packed Single-FP Compare EQ") // NOT IMPLEMENTED
INSTR(cmpltps,     CMPLTPS,     "Packed Single-FP Compare LT") // NOT IMPLEMENTED
INSTR(cmpleps,     CMPLEPS,     "Packed Single-FP Compare LE") // NOT IMPLEMENTED
INSTR(cmpunordps,  CMPUNORDPS,  "Packed Single-FP Compare UNORD") // NOT IMPLEMENTED
INSTR(cmpneqps,    CMPNEQPS,    "Packed Single-FP Compare NOT EQ") // NOT IMPLEMENTED
INSTR(cmpnltps,    CMPNLTPS,    "Packed Single-FP Compare NOT LT") // NOT IMPLEMENTED
INSTR(cmpnleps,    CMPNLEPS,    "Packed Single-FP Compare NOT LE") // NOT IMPLEMENTED
INSTR(cmpordps,    CMPORDPS,    "Packed Single-FP Compare ORDERED") // NOT IMPLEMENTED
INSTR(cmpeqss,     CMPEQSS,     "Scalar Single-FP Compare EQ") // NOT IMPLEMENTED
INSTR(cmpltss,     CMPLTSS,     "Scalar Single-FP Compare LT") // NOT IMPLEMENTED
INSTR(cmpless,     CMPLESS,     "Scalar Single-FP Compare LE") // NOT IMPLEMENTED
INSTR(cmpunordss,  CMPUNORDSS,  "Scalar Single-FP Compare UNORD") // NOT IMPLEMENTED
INSTR(cmpneqss,    CMPNEQSS,    "Scalar Single-FP Compare NOT EQ") // NOT IMPLEMENTED
INSTR(cmpnltss,    CMPNLTSS,    "Scalar Single-FP Compare NOT LT") // NOT IMPLEMENTED
INSTR(cmpnless,    CMPNLESS,    "Scalar Single-FP Compare NOT LE") // NOT IMPLEMENTED
INSTR(cmpordss,    CMPORDSS,    "Scalar Single-FP Compare ORDERED") // NOT IMPLEMENTED

INSTR(cmpeqpd,     CMPEQPD,     "Packed Double-FP Compare EQ") // NOT IMPLEMENTED
INSTR(cmpltpd,     CMPLTPD,     "Packed Double-FP Compare LT") // NOT IMPLEMENTED
INSTR(cmplepd,     CMPLEPD,     "Packed Double-FP Compare LE") // NOT IMPLEMENTED
INSTR(cmpunordpd,  CMPUNORDPD,  "Packed Double-FP Compare UNORD") // NOT IMPLEMENTED
INSTR(cmpneqpd,    CMPNEQPD,    "Packed Double-FP Compare NOT EQ") // NOT IMPLEMENTED
INSTR(cmpnltpd,    CMPNLTPD,    "Packed Double-FP Compare NOT LT") // NOT IMPLEMENTED
INSTR(cmpnlepd,    CMPNLEPD,    "Packed Double-FP Compare NOT LE") // NOT IMPLEMENTED
INSTR(cmpordpd,    CMPORDPD,    "Packed Double-FP Compare ORDERED") // NOT IMPLEMENTED
INSTR(cmpeqsd,     CMPEQSD,     "Scalar Double-FP Compare EQ") // NOT IMPLEMENTED
INSTR(cmpltsd,     CMPLTSD,     "Scalar Double-FP Compare LT") // NOT IMPLEMENTED
INSTR(cmplesd,     CMPLESD,     "Scalar Double-FP Compare LE") // NOT IMPLEMENTED
INSTR(cmpunordsd,  CMPUNORDSD,  "Scalar Double-FP Compare UNORD") // NOT IMPLEMENTED
INSTR(cmpneqsd,    CMPNEQSD,    "Scalar Double-FP Compare NOT EQ") // NOT IMPLEMENTED
INSTR(cmpnltsd,    CMPNLTSD,    "Scalar Double-FP Compare NOT LT") // NOT IMPLEMENTED
INSTR(cmpnlesd,    CMPNLESD,    "Scalar Double-FP Compare NOT LE") // NOT IMPLEMENTED
INSTR(cmpordsd,    CMPORDSD,    "Scalar Double-FP Compare ORDERED") // NOT IMPLEMENTED

/* AMD K7 instructions. */
INSTR(pf2iw,       PF2IW,       "Packed Floating-Point to Integer with Sign Extend") // NOT IMPLEMENTED
INSTR(pfnacc,      PFNACC,      "Packed Floating-Point Negative Accumulate") // NOT IMPLEMENTED
INSTR(pfpnacc,     PFPNACC,     "Packed Floating-Point Mixed Positive-Negative Accumulate") // NOT IMPLEMENTED
INSTR(pi2fw,       PI2FW,       "Packed 16-bit Integer to Floating-Point") // NOT IMPLEMENTED
INSTR(pswapd,      PSWAPD,      "Packed Swap Double Word") // NOT IMPLEMENTED

/* Undocumented FP instructions. */
INSTR(fstp1,       FSTP1,       "Alias of Store Real and Pop") // NOT IMPLEMENTED
INSTR(fcom2,       FCOM2,       "Alias of Compare Real") // NOT IMPLEMENTED
INSTR(fcomp3,      FCOMP3,      "Alias of Compare Real and Pop") // NOT IMPLEMENTED
INSTR(fxch4,       FXCH4,       "Alias of Exchange Registers") // NOT IMPLEMENTED
INSTR(fcomp5,      FCOMP5,      "Alias of Compare Real and Pop") // NOT IMPLEMENTED
INSTR(ffreep,      FFREEP,      "Free Register and Pop") // NOT IMPLEMENTED
INSTR(fxch7,       FXCH7,       "Alias of Exchange Registers") // NOT IMPLEMENTED
INSTR(fstp8,       FSTP8,       "Alias of Store Real and Pop") // NOT IMPLEMENTED
INSTR(fstp9,       FSTP9,       "Alias of Store Real and Pop") // NOT IMPLEMENTED

/* Pentium 4 instructions. */
INSTR(addpd,       ADDPD,       "Add Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(addsd,       ADDSD,       "Add Scalar Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(andnpd,      ANDNPD,      "Bitwise Logical AND NOT of Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(andpd,       ANDPD,       "Bitwise Logical AND of Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(clflush,     CLFLUSH,     "Flush Cache Line") // NOT IMPLEMENTED
INSTR(cmppd,       CMPPD,       "Compare Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(cmpsd_sse,   CMPSD_SSE,   "Compare Scalar Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(comisd,      COMISD,      "Compare Scalar Ordered Double-Precision Floating-Point Values and Set EFLAGS") // NOT IMPLEMENTED
INSTR(cvtdq2pd,    CVTDQ2PD,    "Convert Packed Doubleword Integers to Packed Single-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(cvtdq2ps,    CVTDQ2PS,    "Convert Packed Doubleword Integers to Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(cvtpd2dq,    CVTPD2DQ,    "Convert Packed Double-Precision Floating-Point Values to Packed Doubleword Integers") // NOT IMPLEMENTED
INSTR(cvtpd2pi,    CVTPD2PI,    "Convert Packed Double-Precision Floating-Point Values to Packed Doubleword Integers") // NOT IMPLEMENTED
INSTR(cvtpd2ps,    CVTPD2PS,    "Convert Packed Double-Precision Floating-Point Values to Packed Single-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(cvtpi2pd,    CVTPI2PD,    "Convert Packed Doubleword Integers to Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(cvtps2dq,    CVTPS2DQ,    "Convert Packed Single-Precision Floating-Point Values to Packed Doubleword Integers") // NOT IMPLEMENTED
INSTR(cvtps2pd,    CVTPS2PD,    "Convert Packed Single-Precision Floating-Point Values to Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(cvtsd2si,    CVTSD2SI,    "Convert Scalar Double-Precision Floating-Point Value to Doubleword Integer") // NOT IMPLEMENTED
INSTR(cvtsd2ss,    CVTSD2SS,    "Convert Scalar Double-Precision Floating-Point Value to Scalar Single-Precision Floating-Point Value") // NOT IMPLEMENTED
INSTR(cvtsi2sd,    CVTSI2SD,    "Convert Doubleword Integer to Scalar Double-Precision Floating-Point Value") // NOT IMPLEMENTED
INSTR(cvtss2sd,    CVTSS2SD,    "Convert Scalar Single-Precision Floating-Point Value to Scalar Double-Precision Floating-Point Value") // NOT IMPLEMENTED
INSTR(cvttpd2dq,   CVTTPD2DQ,   "Convert With Truncation Packed Double-Precision Floating-Point Values to Packed Doubleword Integers") // NOT IMPLEMENTED
INSTR(cvttpd2pi,   CVTTPD2PI,   "Convert with Truncation Packed Double-Precision Floating-Point Values to Packed Doubleword Integers") // NOT IMPLEMENTED
INSTR(cvttps2dq,   CVTTPS2DQ,   "Convert With Truncation Packed Single-Precision Floating-Point Values to Packed Doubleword Integers") // NOT IMPLEMENTED
INSTR(cvttsd2si,   CVTTSD2SI,   "Convert with Truncation Scalar Double-Precision Floating-Point Value to Doubleword Integer") // NOT IMPLEMENTED
INSTR(divpd,       DIVPD,       "Divide Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(divsd,       DIVSD,       "Divide Scalar Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(lfence,      LFENCE,      "Load Fence") // NOT IMPLEMENTED
INSTR(maskmovdqu,  MASKMOVDQU,  "Store Selected Bytes of Double Quadword") // NOT IMPLEMENTED
INSTR(maxpd,       MAXPD,       "Return Maximum Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(maxsd,       MAXSD,       "Return Maximum Scalar Double-Precision Floating-Point Value") // NOT IMPLEMENTED
INSTR(mfence,      MFENCE,      "Memory Fence") // NOT IMPLEMENTED
INSTR(minpd,       MINPD,       "Return Minimum Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(minsd,       MINSD,       "Return Minimum Scalar Double-Precision Floating-Point Value") // NOT IMPLEMENTED
INSTR(movapd,      MOVAPD,      "Move Aligned Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(movdq2q,     MOVDQ2Q,     "Move Quadword from XMM to MMX Register") // NOT IMPLEMENTED
INSTR(movdqa,      MOVDQA,      "Move Aligned Double Quadword") // NOT IMPLEMENTED
INSTR(movdqu,      MOVDQU,      "Move Unaligned Double Quadword") // NOT IMPLEMENTED
INSTR(movhpd,      MOVHPD,      "Move High Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(movlpd,      MOVLPD,      "Move Low Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(movmskpd,    MOVMSKPD,    "Extract Packed Double-Precision Floating-Point Sign Mask") // NOT IMPLEMENTED
INSTR(movntdq,     MOVNTDQ,     "Store Double Quadword Using Non-Temporal Hint") // NOT IMPLEMENTED
INSTR(movnti,      MOVNTI,      "Store Doubleword Using Non-Temporal Hint") // NOT IMPLEMENTED
INSTR(movntpd,     MOVNTPD,     "Store Packed Double-Precision Floating-Point Values Using Non-Temporal Hint") // NOT IMPLEMENTED
INSTR(movq2dq,     MOVQ2DQ,     "Move Quadword from MMX to XMM Register") // NOT IMPLEMENTED
INSTR(movsd_sse,   MOVSD_SSE,   "Move Scalar Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(movupd,      MOVUPD,      "Move Unaligned Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(mulpd,       MULPD,       "Multiply Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(mulsd,       MULSD,       "Multiply Scalar Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(orpd,        ORPD,        "Bitwise Logical OR of Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(paddq,       PADDQ,       "Add Packed Quadword Integers") // NOT IMPLEMENTED
INSTR(pause,       PAUSE,       "Spin Loop Hint") // NOT IMPLEMENTED
INSTR(pmuludq,     PMULUDQ,     "Multiply Packed Unsigned Doubleword Integers") // NOT IMPLEMENTED
INSTR(pshufd,      PSHUFD,      "Shuffle Packed Doublewords") // NOT IMPLEMENTED
INSTR(pshufhw,     PSHUFHW,     "Shuffle Packed High Words") // NOT IMPLEMENTED
INSTR(pshuflw,     PSHUFLW,     "Shuffle Packed Low Words") // NOT IMPLEMENTED
INSTR(pslldq,      PSLLDQ,      "Shift Double Quadword Left Logical") // NOT IMPLEMENTED
INSTR(psrldq,      PSRLDQ,      "Shift Double Quadword Right Logical") // NOT IMPLEMENTED
INSTR(psubq,       PSUBQ,       "Subtract Packed Quadword Integers") // NOT IMPLEMENTED
INSTR(punpckhqdq,  PUNPCKHQDQ,  "Unpack High Data") // NOT IMPLEMENTED
INSTR(punpcklqdq,  PUNPCKLQDQ,  "Unpack Low Data") // NOT IMPLEMENTED
INSTR(shufpd,      SHUFPD,      "Shuffle Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(sqrtpd,      SQRTPD,      "Compute Square Roots of Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(sqrtsd,      SQRTSD,      "Compute Square Rootof Scalar Double-Precision Floating-Point Value") // NOT IMPLEMENTED
INSTR(subpd,       SUBPD,       "Subtract Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(subsd,       SUBSD,       "Subtract Scalar Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(ucomisd,     UCOMISD,     "Unordered Compare Scalar Ordered Double-Precision Floating-Point Values and Set EFLAGS") // NOT IMPLEMENTED
INSTR(unpckhpd,    UNPCKHPD,    "Unpack and Interleave High Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(unpcklpd,    UNPCKLPD,    "Unpack and Interleave Low Packed Double-Precision Floating-Point Values") // NOT IMPLEMENTED
INSTR(xorpd,       XORPD,       "Bitwise Logical OR of Double-Precision Floating-Point Values") // NOT IMPLEMENTED

/* AMD syscall/sysret instructions. */
INSTR(syscall,     SYSCALL,     "Low latency system call") // NOT IMPLEMENTED
INSTR(sysret,      SYSRET,      "Return from system call") // NOT IMPLEMENTED

/* AMD64 instructions. */
INSTR(swapgs,      SWAPGS,      "Exchange GS base with KernelGSBase MSR") // NOT IMPLEMENTED

/* New Pentium instructions (SSE3). */
INSTR(movddup,     MOVDDUP,     "Move One Double-FP and Duplicate") // NOT IMPLEMENTED
INSTR(movshdup,    MOVSHDUP,    "Move Packed Single-FP High and Duplicate") // NOT IMPLEMENTED
INSTR(movsldup,    MOVSLDUP,    "Move Packed Single-FP Low and Duplicate") // NOT IMPLEMENTED

/* Missing AMD64 instructions. */
INSTR(movsxd,      MOVSXD,      "Move with Sign-Extend Doubleword")
INSTR(cmpxchg16b,  CMPXCHG16B,  "Compare and Exchange 16 Bytes") // NOT IMPLEMENTED

/* SSE3 instructions. */
INSTR(addsubpd,    ADDSUBPD,    "Add /Sub packed DP FP numbers") // NOT IMPLEMENTED
INSTR(addsubps,    ADDSUBPS,    "Add /Sub packed SP FP numbers") // NOT IMPLEMENTED
INSTR(haddpd,      HADDPD,      "Add horizontally packed DP FP numbers") // NOT IMPLEMENTED
INSTR(haddps,      HADDPS,      "Add horizontally packed SP FP numbers") // NOT IMPLEMENTED
INSTR(hsubpd,      HSUBPD,      "Sub horizontally packed DP FP numbers") // NOT IMPLEMENTED
INSTR(hsubps,      HSUBPS,      "Sub horizontally packed SP FP numbers") // NOT IMPLEMENTED
INSTR(monitor,     MONITOR,     "Set up a linear address range to be monitored by hardware") // NOT IMPLEMENTED
INSTR(mwait,       MWAIT,       "Wait until write-back store performed within the range specified by the MONITOR instruction") // NOT IMPLEMENTED
INSTR(fisttp,      FISTTP,      "Store ST in intXX (chop) and pop") // NOT IMPLEMENTED
INSTR(lddqu,       LDDQU,       "Load unaligned integer 128-bit") // NOT IMPLEMENTED

/* SSSE3 instructions. */
INSTR(psignb,      PSIGNB,      "Packed SIGN Byte") // NOT IMPLEMENTED
INSTR(psignw,      PSIGNW,      "Packed SIGN Word") // NOT IMPLEMENTED
INSTR(psignd,      PSIGND,      "Packed SIGN Doubleword") // NOT IMPLEMENTED
INSTR(pshufb,      PSHUFB,      "Packed Shuffle Bytes") // NOT IMPLEMENTED
INSTR(pmulhrsw,    PMULHRSW,    "Packed Multiply High with Round and Scale") // NOT IMPLEMENTED
INSTR(pmaddubsw,   PMADDUBSW,   "Multiply and Add Packed Signed and Unsigned Bytes") // NOT IMPLEMENTED
INSTR(phsubsw,     PHSUBSW,     "Packed Horizontal Subtract and Saturate") // NOT IMPLEMENTED
INSTR(phaddsw,     PHADDSW,     "Packed Horizontal Add and Saturate") // NOT IMPLEMENTED
INSTR(phaddw,      PHADDW,      "Packed Horizontal Add Word") // NOT IMPLEMENTED
INSTR(phaddd,      PHADDD,      "Packed Horizontal Add Doubleword") // NOT IMPLEMENTED
INSTR(phsubw,      PHSUBW,      "Packed Horizontal Subtract Word") // NOT IMPLEMENTED
INSTR(phsubd,      PHSUBD,      "Packed Horizontal Subtract Doubleword") // NOT IMPLEMENTED
INSTR(palignr,     PALIGNR,     "Packed Align Right") // NOT IMPLEMENTED
INSTR(pabsb,       PABSB,       "Packed Absolute Value Byte") // NOT IMPLEMENTED
INSTR(pabsw,       PABSW,       "Packed Absolute Value Word") // NOT IMPLEMENTED
INSTR(pabsd,       PABSD,       "Packed Absolute Value Doubleword") // NOT IMPLEMENTED

/* VMX instructions. */
INSTR(vmcall,      VMCALL,      "Call to VM Monitor") // NOT IMPLEMENTED
INSTR(vmclear,     VMCLEAR,     "Clear Virtual Machine Control Structure") // NOT IMPLEMENTED
INSTR(vmlaunch,    VMLAUNCH,    "Launch Virtual Machine") // NOT IMPLEMENTED
INSTR(vmresume,    VMRESUME,    "Resume Virtual Machine") // NOT IMPLEMENTED
INSTR(vmptrld,     VMPTRLD,     "Load Pointer to Virtual Machine Control Structure") // NOT IMPLEMENTED
INSTR(vmptrst,     VMPTRST,     "Store Pointer to Virtual Machine Control Structure") // NOT IMPLEMENTED
INSTR(vmread,      VMREAD,      "Read Field from Virtual Machine Control Structure") // NOT IMPLEMENTED
INSTR(vmwrite,     VMWRITE,     "Write Field from Virtual Machine Control Structure") // NOT IMPLEMENTED
INSTR(vmxoff,      VMXOFF,      "Leave VMX Operation") // NOT IMPLEMENTED
INSTR(vmxon,       VMXON,       "Enter VMX Operation") // NOT IMPLEMENTED

/* vim:set et sts=4 sw=4: */
