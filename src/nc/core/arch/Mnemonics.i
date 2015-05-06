/* No include guards. */

/**
 * \file
 *
 * This file is to be used to generate mnemonics enumeration from instruction 
 * table. For example usage, see <tt>IntelMnemonics.h</tt>.
 */

#ifndef INSTRUCTION_TABLE
#    error Define INSTRUCTION_TABLE before including this file
#endif

#ifdef INSTR
#    error INSTR is expected not to be defined
#endif

	/* Mnemonic numbers. */
	enum {
#define INSTR(lowercase, uppercase, comment) \
    uppercase,
#include INSTRUCTION_TABLE
#undef INSTR
	};

#undef INSTRUCTION_TABLE

