/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "ArmDisassembler.h"

#include "ArmArchitecture.h"
#include "ArmInstruction.h"

namespace nc {
namespace arch {
namespace arm {

ArmDisassembler::ArmDisassembler(const ArmArchitecture *architecture):
    core::arch::Disassembler(architecture)
{
    if (cs_open(CS_ARCH_ARM, CS_MODE_ARM, &handle_) != CS_ERR_OK) {
        assert(!"Could not initialize Capstone's ARM disassembler.");
    }
}

ArmDisassembler::~ArmDisassembler() {
    if (cs_close(&handle_) != CS_ERR_OK) {
        assert(!"Could not deinitialize Capstone's ARM disassembler.");
    }
}

std::shared_ptr<core::arch::Instruction> ArmDisassembler::disassembleSingleInstruction(ByteAddr pc, const void *buffer, ByteSize size) {
    cs_insn *insn;
    auto count = cs_disasm_ex(handle_, reinterpret_cast<const uint8_t *>(buffer), size, pc, 1, &insn);

    std::shared_ptr<core::arch::Instruction> result;

    if (count) {
        assert(count == 1);
        result = std::make_shared<ArmInstruction>(
            CS_MODE_ARM,
            insn->address,
            insn->size,
            buffer);
    }

    cs_free(insn, count);

    return result;
}

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
