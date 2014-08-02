/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "ArmInstruction.h"

#include "CapstoneDisassembler.h"

namespace nc {
namespace arch {
namespace arm {

void ArmInstruction::print(QTextStream &out) const {
    auto instr = CapstoneDisassembler(CS_ARCH_ARM, mode_).disassemble(addr(), &bytes_[0], size(), 1);
    assert(instr != NULL);

    out << instr->mnemonic << " " << instr->op_str;
}

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
