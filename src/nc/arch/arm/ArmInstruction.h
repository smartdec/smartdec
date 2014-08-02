/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <array>

#include <capstone/capstone.h>

#include <nc/core/arch/Instruction.h>

namespace nc {
namespace arch {
namespace arm {

class ArmInstruction: public core::arch::Instruction {
public:
    /** Max size of an instruction. */
    static const SmallByteSize MAX_SIZE = 4;

private:
    /** Encoding mode of this instruction. */
    int mode_;

    /** Binary representation of the instruction. */
    std::array<uint8_t, MAX_SIZE> bytes_;

public:
    /**
     * Class constructor.
     *
     * \param[in] mode Encoding mode of this instruction.
     * \param[in] addr Instruction address in bytes.
     * \param[in] size Instruction size in bytes.
     * \param[in] bytes Valid pointer to the bytes of the instruction.
     */
    ArmInstruction(int mode, ByteAddr addr, SmallByteSize size, const void *bytes):
        Instruction(addr, size), mode_(mode)
    {
        assert(size > 0);
        assert(size <= MAX_SIZE);
        memcpy(&bytes_, bytes, size);
    }

    void print(QTextStream &out) const override;
};

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
