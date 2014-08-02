/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "ArmInstruction.h"

namespace nc {
namespace arch {
namespace arm {

void ArmInstruction::print(QTextStream &out) const {
    csh handle;
    if (cs_open(CS_ARCH_ARM, mode_, &handle) != CS_ERR_OK) {
        assert(!"Could not initialize Capstone's ARM disassembler.");
    }

    cs_insn *insn;
    auto count = cs_disasm_ex(handle, &bytes_[0], size(), addr(), 1, &insn);
    if (count != 1) {
        assert(!"Failed to disassemble the already disassembled instruction again.");
    }

    out << insn->mnemonic << " " << insn->op_str;

    cs_free(insn, count);
    if (cs_close(&handle) != CS_ERR_OK) {
        assert(!"Could not deinitialize Capstone's ARM disassembler.");
    }
}

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
