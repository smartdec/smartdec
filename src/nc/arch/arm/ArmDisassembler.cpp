/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "ArmDisassembler.h"

#include "ArmArchitecture.h"

namespace nc {
namespace arch {
namespace arm {

ArmDisassembler::ArmDisassembler(const ArmArchitecture *architecture):
    core::arch::Disassembler(architecture)
{}

std::shared_ptr<core::arch::Instruction> ArmDisassembler::disassembleSingleInstruction(ByteAddr pc, const void *buffer, ByteSize size) {
    return NULL; // TODO
}

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
