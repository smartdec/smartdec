/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/arch/Disassembler.h>

#include <capstone/capstone.h>

namespace nc {
namespace arch {
namespace arm {

class ArmArchitecture;

class ArmDisassembler: public core::arch::Disassembler {
    csh handle_;

public:
    ArmDisassembler(const ArmArchitecture *architecture);

    virtual ~ArmDisassembler();

    std::shared_ptr<core::arch::Instruction> disassembleSingleInstruction(ByteAddr pc, const void *buffer, ByteSize size) override;
};

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
