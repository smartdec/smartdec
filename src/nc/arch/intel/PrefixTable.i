/**
 * \file
 *
 * Table of instruction prefixes for Intel architecture.
 *
 * This header is intended for multiple inclusion.
 */

/**
 * \def PREFIX(lowercase, uppercase, bitmask, comment)
 *
 * \param lowercase Name of the instruction as a lowercase C++ identifier.
 * \param uppercase Name of the instruction as an uppercase C++ identifier.
 * \param bitmask Value of the bit mask of this prefix.
 * \param comment Instruction description.
 */

PREFIX(lock,    LOCK,   0x1,    "Execute instruction atomically")
PREFIX(rep,     REP,    0x2,    "Repeat string operation until RCX or (E)CX = 0")
PREFIX(repz,    REPZ,   0x4,    "Repeat string operation until RCX or (E)CX = 0, or ZF = 0")
PREFIX(repnz,   REPNZ,  0x8,    "Repeat string operation until RCX or (E)CX = 0, or ZF = 1")

/* vim:set et sts=4 sw=4: */
