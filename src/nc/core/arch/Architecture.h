/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <memory>
#include <vector>

#include <QString>

#include <nc/common/ByteOrder.h>
#include <nc/common/Types.h>

namespace nc {
namespace core {

class MasterAnalyzer;

namespace ir {
    class MemoryLocation;

    namespace calling {
        class CallingConvention;
    }
}

namespace arch {

namespace disasm {
    class InstructionDisassembler;
}

namespace irgen {
    class InstructionAnalyzer;
}

class Mnemonics;
class Register;
class Registers;

/**
 * Immutable class describing an architecture.
 */
class Architecture {
public:
    /**
     * Constructor.
     */
    Architecture();

    /**
     * Virtual destructor.
     */
    virtual ~Architecture();

    /**
     * \return Name of the architecture.
     */
    const QString &name() const { return mName; }

    /**
     * \returns Architecture's bitness (data pointer size).
     */
    SmallBitSize bitness() const { assert(mBitness); return mBitness; }

    /**
     * \return Architecture's byte order.
     */
    ByteOrder byteOrder() const { assert(mByteOrder != ByteOrder::Unknown); return mByteOrder; }

    /**
     * \return Maximal length of an instruction.
     */
    SmallByteSize maxInstructionSize() const { assert(mMaxInstructionSize); return mMaxInstructionSize; }

    /**
     * \returns Valid pointer to the disassembler for a single instruction.
     */
    const disasm::InstructionDisassembler *instructionDisassembler() const { return mInstructionDisassembler; }

    /**
     * \returns Valid pointer to the instruction analyzer for this architecture.
     */
    const irgen::InstructionAnalyzer *instructionAnalyzer() const { return mInstructionAnalyzer; }

    /**
     * \returns Valid pointer to the universal analyzer for this architecture.
     */
    const MasterAnalyzer *masterAnalyzer() const { return mMasterAnalyzer; }

    /**
     * \returns Valid pointer to the mnemonic container for this architecture.
     */
    const Mnemonics *mnemonics() const { return mMnemonics; }

    /**
     * \returns Valid pointer to the register container for this architecture.
     */
    const Registers *registers() const { return mRegisters; }

    /**
     * \return Pointer to instruction pointer register. Can be NULL.
     */
    const Register *instructionPointer() const { return mInstructionPointer; }

    /**
     * \param memoryLocation Memory location.
     *
     * \return True, if the memory location belongs to global memory accessible by all functions.
     */
    virtual bool isGlobalMemory(const ir::MemoryLocation &memoryLocation) const;

    /**
     * \return List of available calling conventions.
     */
    const std::vector<const ir::calling::CallingConvention *> &callingConventions() const {
        return reinterpret_cast<const std::vector<const ir::calling::CallingConvention *> &>(callingConventions_);
    }

    /**
     * \return Pointer to the calling convention with the given name. Can be NULL.
     */
    const ir::calling::CallingConvention *getCallingConvention(const QString &name) const;

protected:
    /**
     * Sets the name of the architecture.
     * The name must be sent only once.
     *
     * \param name Non-empty new name of the architecture.
     */
    void setName(QString name);

    /**
     * Sets the architecture's bitness.
     *
     * \param bitness Architecture's bitness.
     */
    void setBitness(SmallBitSize bitness);

    /**
     * Sets the architecture's byte order.
     *
     * \param byteOrder Byte order.
     */
    void setByteOrder(ByteOrder byteOrder);

    /**
     * \param size Architecture's maximum instruction size.
     */
    void setMaxInstructionSize(SmallBitSize size);

    /**
     * \param disassembler Valid pointer to the disassembler of a single instruction.
     */
    void setInstructionDisassembler(disasm::InstructionDisassembler *disassembler);

    /**
     * \param instructionAnalyzer Valid pointer to the instruction analyzer for this architecture.
     */
    void setInstructionAnalyzer(irgen::InstructionAnalyzer *instructionAnalyzer);

    /**
     * \param masterAnalyzer Valid pointer to the universal analyzer for this architecture.
     */
    void setMasterAnalyzer(const MasterAnalyzer *masterAnalyzer);

    /**
     * \param mnemonics Valid pointer to the mnemonics container for this architecture.
     */
    void setMnemonics(Mnemonics *mnemonics);

    /**
     * \param registers Valid pointer to the registers container for this architecture.
     */
    void setRegisters(Registers *registers);

    /**
     * Sets the operand being the instruction pointer.
     *
     * \param reg Instruction pointer register operand.
     */
    void setInstructionPointer(const Register *reg);

    /**
     * Adds a calling convention.
     * There must be no calling convention with the same name in the repository.
     *
     * \param convention Valid pointer to the calling convention.
     */
    void addCallingConvention(std::unique_ptr<ir::calling::CallingConvention> convention);

private:
    /** Name of the architecture. */
    QString mName;

    /** Architecture's bitness. */
    SmallBitSize mBitness;

    /** Architecture's byte order. */
    ByteOrder mByteOrder;

    /** Maximum length of an instruction on this architecture. */
    SmallBitSize mMaxInstructionSize;

    /** Disassembler for parsing a single instruction. */
    disasm::InstructionDisassembler *mInstructionDisassembler;

    /** Instruction analyzer for this architecture. */
    irgen::InstructionAnalyzer *mInstructionAnalyzer;

    /** Master analyzer for this architecture. */
    const MasterAnalyzer *mMasterAnalyzer;

    /** Instruction dictionary for this architecture. */
    Mnemonics *mMnemonics;

    /** Register container for this architecture. */
    Registers *mRegisters;

    /** Instruction pointer register. */
    const Register *mInstructionPointer;

    /** Calling conventions. */
    std::vector<std::unique_ptr<ir::calling::CallingConvention>> callingConventions_;
};

} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
