/**
 * \file
 *
 * Register table for Intel architecture.
 *
 * This header is intended for multiple inclusion.
 */


/**
 * \def REG(lowercase, uppercase, domain, offset, size, comment)
 *
 * 
 * \param lowercase                    Name of the register as a lowercase 
 *                                     C++ identifier.
 * \param uppercase                    Name of the register as an UPPERCASE
 *                                     C++ identifier.
 * \param domain                       Number of register's domain.
 * \param offset                       Bit offset of the register inside
 *                                     the domain.
 * \param size                         Bit size of the register.
 * \param comment                      Register description.
 */
REG(rax,              RAX,              0,   0, 64, "")
REG(eax,              EAX,              0,   0, 32, "")
REG(ax,               AX,               0,   0, 16, "")
REG(al,               AL,               0,   0,  8, "")
REG(ah,               AH,               0,   8,  8, "")

REG(rbx,              RBX,              1,   0, 64, "")
REG(ebx,              EBX,              1,   0, 32, "")
REG(bx,               BX,               1,   0, 16, "")
REG(bl,               BL,               1,   0,  8, "")
REG(bh,               BH,               1,   8,  8, "")

REG(rcx,              RCX,              2,   0, 64, "")
REG(ecx,              ECX,              2,   0, 32, "")
REG(cx,               CX,               2,   0, 16, "")
REG(cl,               CL,               2,   0,  8, "")
REG(ch,               CH,               2,   8,  8, "")

REG(rdx,              RDX,              3,   0, 64, "")
REG(edx,              EDX,              3,   0, 32, "")
REG(dx,               DX,               3,   0, 16, "")
REG(dl,               DL,               3,   0,  8, "")
REG(dh,               DH,               3,   8,  8, "")

REG(rsp,              RSP,              4,   0, 64, "")
REG(esp,              ESP,              4,   0, 32, "")
REG(sp,               SP,               4,   0, 16, "")
REG(spl,              SPL,              4,   0,  8, "")

REG(rbp,              RBP,              5,   0, 64, "")
REG(ebp,              EBP,              5,   0, 32, "")
REG(bp,               BP,               5,   0, 16, "")
REG(bpl,              BPL,              5,   0,  8, "")

REG(rsi,              RSI,              6,   0, 64, "")
REG(esi,              ESI,              6,   0, 32, "")
REG(si,               SI,               6,   0, 16, "")
REG(sil,              SIL,              6,   0,  8, "")

REG(rdi,              RDI,              7,   0, 64, "")
REG(edi,              EDI,              7,   0, 32, "")
REG(di,               DI,               7,   0, 16, "")
REG(dil,              DIL,              7,   0,  8, "")

REG(r8,               R8,               8,   0, 64, "")
REG(r8d,              R8D,              8,   0, 32, "")
REG(r8w,              R8W,              8,   0, 16, "")
REG(r8b,              R8B,              8,   0,  8, "")

REG(r9,               R9,               9,   0, 64, "")
REG(r9d,              R9D,              9,   0, 32, "")
REG(r9w,              R9W,              9,   0, 16, "")
REG(r9b,              R9B,              9,   0,  8, "")

REG(r10,              R10,              10,  0, 64, "")
REG(r10d,             R10D,             10,  0, 32, "")
REG(r10w,             R10W,             10,  0, 16, "")
REG(r10b,             R10B,             10,  0,  8, "")

REG(r11,              R11,              11,  0, 64, "")
REG(r11d,             R11D,             11,  0, 32, "")
REG(r11w,             R11W,             11,  0, 16, "")
REG(r11b,             R11B,             11,  0,  8, "")

REG(r12,              R12,              12,  0, 64, "")
REG(r12d,             R12D,             12,  0, 32, "")
REG(r12w,             R12W,             12,  0, 16, "")
REG(r12b,             R12B,             12,  0,  8, "")

REG(r13,              R13,              13,  0, 64, "")
REG(r13d,             R13D,             13,  0, 32, "")
REG(r13w,             R13W,             13,  0, 16, "")
REG(r13b,             R13B,             13,  0,  8, "")

REG(r14,              R14,              14,  0, 64, "")
REG(r14d,             R14D,             14,  0, 32, "")
REG(r14w,             R14W,             14,  0, 16, "")
REG(r14b,             R14B,             14,  0,  8, "")

REG(r15,              R15,              15,  0, 64, "")
REG(r15d,             R15D,             15,  0, 32, "")
REG(r15w,             R15W,             15,  0, 16, "")
REG(r15b,             R15B,             15,  0,  8, "")

REG(cs,               CS,               16,  0, 16, "")
REG(ds,               DS,               17,  0, 16, "")
REG(es,               ES,               18,  0, 16, "")
REG(fs,               FS,               19,  0, 16, "")
REG(gs,               GS,               20,  0, 16, "")
REG(ss,               SS,               21,  0, 16, "")

REG(rip,              RIP,              22,  0, 64, "")
REG(eip,              EIP,              22,  0, 32, "")
REG(ip,               IP,               22,  0, 16, "")

REG(rflags,           RFLAGS,           23,  0, 64, "")
REG(eflags,           EFLAGS,           23,  0, 32, "")
REG(flags,            FLAGS,            23,  0, 16, "")
REG(cf,               CF,               23,  0,  1, "Carry flag")
REG(pf,               PF,               23,  2,  1, "Parity flag")
REG(af,               AF,               23,  4,  1, "Adjust flag")
REG(zf,               ZF,               23,  6,  1, "Zero flag")
REG(sf,               SF,               23,  7,  1, "Sign flag")
REG(tf,               TF,               23,  8,  1, "Trap flag")
REG(if_,              IF,               23,  9,  1, "Interrupt enable flag")
REG(df,               DF,               23, 10,  1, "Direction flag")
REG(of,               OF,               23, 11,  1, "Overflow flag")
REG(iopl,             IOPL,             23, 12,  2, "I/O privilege level (286+ only)")
REG(nt,               NT,               23, 14,  1, "Nested task flag (286+ only)")
REG(rf,               RF,               23, 16,  1, "Resume flag (386+ only)")
REG(vm,               VM,               23, 17,  1, "Virtual 8086 mode flag (386+ only)")
REG(ac,               AC,               23, 18,  1, "Alignment check(486SX+ only)")
REG(vif,              VIF,              23, 19,  1, "Virtual interrupt flag (Pentium+)")
REG(vip,              VIP,              23, 20,  1, "Virtual interrupt pending (Pentium+)")
REG(id,               ID,               23, 21,  1, "Identification (Pentium+)")

REG(pseudo_flags,     PSEUDO_FLAGS,     24,  0, 128, "")
REG(less,             LESS,             24,  0,  1, "")
REG(less_or_equal,    LESS_OR_EQUAL,    24,  1,  1, "")
REG(greater,          GREATER,          24,  2,  1, "")
REG(greater_or_equal, GREATER_OR_EQUAL, 24,  3,  1, "")
REG(below,            BELOW,            24,  4,  1, "")
REG(below_or_equal,   BELOW_OR_EQUAL,   24,  5,  1, "")
REG(above,            ABOVE,            24,  6,  1, "")
REG(above_or_equal,   ABOVE_OR_EQUAL,   24,  7,  1, "")

REG(fpu_status_word,  FPU_STATUS_WORD,  25,  0, 16, "x87 FPU Status Word")
REG(fpu_ie,           FPU_IE,           25,  0,  1, "Invalid Operation")
REG(fpu_de,           FPU_DE,           25,  1,  1, "Denormalized Operand")
REG(fpu_ze,           FPU_ZE,           25,  2,  1, "Zero Divide")
REG(fpu_oe,           FPU_OE,           25,  3,  1, "Overflow")
REG(fpu_ue,           FPU_UE,           25,  4,  1, "Underflow")
REG(fpu_pe,           FPU_PE,           25,  5,  1, "Precision")
REG(fpu_sf,           FPU_SF,           25,  6,  1, "Stack Fault")
REG(fpu_es,           FPU_ES,           25,  7,  1, "Error Summary Status")
REG(fpu_c0,           FPU_C0,           25,  8,  1, "Condition Code bit 0")
REG(fpu_c1,           FPU_C1,           25,  9,  1, "Condition Code bit 1")
REG(fpu_c2,           FPU_C2,           25, 10,  1, "Condition Code bit 2")
REG(fpu_top,          FPU_TOP,          25, 11,  3, "Top of Stack Pointer")
REG(fpu_c3,           FPU_C3,           25, 14,  1, "Condition Code bit 3")
REG(fpu_b,            FPU_B,            25, 15,  1, "FPU Busy")

REG(fpu_r0,           FPU_R0,           26, 0 * 80, 80, "")
REG(fpu_r1,           FPU_R1,           26, 1 * 80, 80, "")
REG(fpu_r2,           FPU_R2,           26, 2 * 80, 80, "")
REG(fpu_r3,           FPU_R3,           26, 3 * 80, 80, "")
REG(fpu_r4,           FPU_R4,           26, 4 * 80, 80, "")
REG(fpu_r5,           FPU_R5,           26, 5 * 80, 80, "")
REG(fpu_r6,           FPU_R6,           26, 6 * 80, 80, "")
REG(fpu_r7,           FPU_R7,           26, 7 * 80, 80, "")

/* Lower bits of MMX registers are shared with the 80-bit-wide FPU stack. */
REG(mm0,              MM0,              26, 0 * 80, 64, "")
REG(mm1,              MM1,              26, 1 * 80, 64, "")
REG(mm2,              MM2,              26, 2 * 80, 64, "")
REG(mm3,              MM3,              26, 3 * 80, 64, "")
REG(mm4,              MM4,              26, 4 * 80, 64, "")
REG(mm5,              MM5,              26, 5 * 80, 64, "")
REG(mm6,              MM6,              26, 6 * 80, 64, "")
REG(mm7,              MM7,              26, 7 * 80, 64, "")

REG(xmm0,             XMM0,             27, 0, 128, "")
REG(xmm1,             XMM1,             28, 0, 128, "")
REG(xmm2,             XMM2,             29, 0, 128, "")
REG(xmm3,             XMM3,             30, 0, 128, "")
REG(xmm4,             XMM4,             31, 0, 128, "")
REG(xmm5,             XMM5,             32, 0, 128, "")
REG(xmm6,             XMM6,             33, 0, 128, "")
REG(xmm7,             XMM7,             34, 0, 128, "")
REG(xmm8,             XMM8,             35, 0, 128, "")
REG(xmm9,             XMM9,             36, 0, 128, "")
REG(xmm10,            XMM10,            37, 0, 128, "")
REG(xmm11,            XMM11,            38, 0, 128, "")
REG(xmm12,            XMM12,            39, 0, 128, "")
REG(xmm13,            XMM13,            40, 0, 128, "")
REG(xmm14,            XMM14,            41, 0, 128, "")
REG(xmm15,            XMM15,            42, 0, 128, "")

REG(cr0,              CR0,              43, 0, 32, "")
REG(cr1,              CR1,              44, 0, 32, "")
REG(cr2,              CR2,              45, 0, 32, "")
REG(cr3,              CR3,              46, 0, 32, "")
REG(cr4,              CR4,              47, 0, 32, "")
REG(cr5,              CR5,              48, 0, 32, "")
REG(cr6,              CR6,              49, 0, 32, "")
REG(cr7,              CR7,              50, 0, 32, "")
REG(cr8,              CR8,              51, 0, 32, "")
REG(cr9,              CR9,              52, 0, 32, "")
REG(cr10,             CR10,             53, 0, 32, "")
REG(cr11,             CR11,             54, 0, 32, "")
REG(cr12,             CR12,             55, 0, 32, "")
REG(cr13,             CR13,             56, 0, 32, "")
REG(cr14,             CR14,             57, 0, 32, "")
REG(cr15,             CR15,             58, 0, 32, "")

REG(dr0,              DR0,              59, 0, 32, "")
REG(dr1,              DR1,              60, 0, 32, "")
REG(dr2,              DR2,              61, 0, 32, "")
REG(dr3,              DR3,              62, 0, 32, "")
REG(dr4,              DR4,              63, 0, 32, "")
REG(dr5,              DR5,              64, 0, 32, "")
REG(dr6,              DR6,              65, 0, 32, "")
REG(dr7,              DR7,              66, 0, 32, "")
REG(dr8,              DR8,              67, 0, 32, "")
REG(dr9,              DR9,              68, 0, 32, "")
REG(dr10,             DR10,             69, 0, 32, "")
REG(dr11,             DR11,             70, 0, 32, "")
REG(dr12,             DR12,             71, 0, 32, "")
REG(dr13,             DR13,             72, 0, 32, "")
REG(dr14,             DR14,             73, 0, 32, "")
REG(dr15,             DR15,             74, 0, 32, "")

REG(tmp8,             TMP8,             75, 0, 8,  "Temporary 8-bit register")
REG(tmp16,            TMP16,            75, 0, 16, "Temporary 16-bit register")
REG(tmp32,            TMP32,            75, 0, 32, "Temporary 32-bit register")
REG(tmp64,            TMP64,            75, 0, 64, "Temporary 64-bit register")

/* vim:set et sts=4 sw=4: */
