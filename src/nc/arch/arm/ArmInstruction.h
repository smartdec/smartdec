/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/arch/Instruction.h>

namespace nc {
namespace arch {
namespace arm {

class ArmInstruction: public core::arch::Instruction {
    QString text_;

public:
    ArmInstruction(ByteAddr addr, SmallByteSize size, QString text):
        Instruction(addr, size), text_(text)
    {}

    void print(QTextStream &out) const override;
};

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
