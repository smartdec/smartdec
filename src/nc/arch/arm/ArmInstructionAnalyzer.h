/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/irgen/InstructionAnalyzer.h>

namespace nc {
namespace arch {
namespace arm {

class ArmInstructionAnalyzer: public core::irgen::InstructionAnalyzer {
protected:
    virtual void doCreateStatements(const core::arch::Instruction *instruction, core::ir::Program *program) override;
};

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
