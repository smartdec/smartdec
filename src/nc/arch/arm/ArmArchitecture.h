/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/arch/Architecture.h>

namespace nc {
namespace arch {
namespace arm {

class ArmArchitecture: public nc::core::arch::Architecture {
    std::unique_ptr<core::MasterAnalyzer> masterAnalyzer_;

public:
    explicit
    ArmArchitecture(ByteOrder byteOrder);

    virtual ~ArmArchitecture();

    std::unique_ptr<core::arch::Disassembler> createDisassembler() const override;

    std::unique_ptr<core::irgen::InstructionAnalyzer> createInstructionAnalyzer() const override;
};

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
