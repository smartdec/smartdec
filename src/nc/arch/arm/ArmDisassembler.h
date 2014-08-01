/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/arch/Disassembler.h>

namespace nc {
namespace arch {
namespace arm {

class ArmArchitecture;

class ArmDisassembler: public core::arch::Disassembler {
public:
    ArmDisassembler(const ArmArchitecture *architecture);

    std::shared_ptr<core::arch::Instruction> disassembleSingleInstruction(ByteAddr pc, const void *buffer, ByteSize size) override;
};

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
