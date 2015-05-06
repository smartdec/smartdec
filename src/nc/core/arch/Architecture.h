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

#include <utility> /* For std::pair. */
#include <vector>

#include <boost/unordered_map.hpp>

#include <nc/common/SizedValue.h>
#include <nc/common/Types.h>
#include <nc/core/ir/MemoryLocation.h>

namespace nc {
namespace core {

class UniversalAnalyzer;

namespace arch {

namespace disasm {
    class InstructionDisassembler;
}

namespace irgen {
    class InstructionAnalyzer;
}

class ConstantOperand;
class Mnemonics;
class Register;
class RegisterOperand;
class Registers;

/**
 * Architecture.
 */
class Architecture {
public:
    /**
     * Default constructor.
     */
    Architecture();

    /**
     * Virtual destructor.
     */
    virtual ~Architecture();

    /**
     * \returns                        Architecture's bitness.
     */
    SmallBitSize bitness() const { assert(mBitness); return mBitness; }

    /**
     * \return                         Maximum length of an instruction.
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
    const UniversalAnalyzer *universalAnalyzer() const { return mUniversalAnalyzer; }

    /**
     * \returns Valid pointer to the mnemonic container for this architecture.
     */
    const Mnemonics *mnemonics() const { return mMnemonics; }

    /**
     * \returns Valid pointer to the register container for this architecture.
     */
    const Registers *registers() const { return mRegisters; }

    /**
     * \param number                   Register number.
     * \returns                        Operand for the given register number,
     *                                 or NULL if no such register number exists.
     */
    RegisterOperand *registerOperand(int number) const;

    /**
     * \param regizter                 Register.
     * \returns                        Operand for the given register.
     */
    RegisterOperand *registerOperand(const Register *regizter) const;

    /**
     * \param value                    Constant value.
     * \returns                        Operand for the given constant value.
     */
    ConstantOperand *constantOperand(const SizedValue &value) const;

    /**
     * \return                         Pointer to instruction pointer register. Can be NULL.
     */
    const Register *instructionPointer() const { return mInstructionPointer; }

    /**
     * \param memoryLocation Memory location.
     *
     * \return True, if the memory location belongs to global memory accessible by all functions.
     */
    virtual bool isGlobalMemory(const ir::MemoryLocation &memoryLocation) const;

protected:
    /**
     * \param bitness                  Architecture bitness.
     */
    void initBitness(SmallBitSize bitness);

    /**
     * \param size                     Architecture's maximum instruction size.
     */
    void initMaxInstructionSize(SmallBitSize size);

    /**
     * \param disassembler             Valid pointer to the disassembler of a single instruction.
     */
    void initInstructionDisassembler(disasm::InstructionDisassembler *disassembler);

    /**
     * \param instructionAnalyzer Valid pointer to the instruction analyzer for this architecture.
     */
    void initInstructionAnalyzer(irgen::InstructionAnalyzer *instructionAnalyzer);

    /**
     * \param universalAnalyzer Valid pointer to the universal analyzer for this architecture.
     */
    void initUniversalAnalyzer(const UniversalAnalyzer *universalAnalyzer);

    /**
     * \param mnemonics Valid pointer to the mnemonics container for this architecture.
     */
    void initMnemonics(Mnemonics *mnemonics);

    /**
     * \param registers Valid pointer to the registers container for this architecture.
     */
    void initRegisters(Registers *registers);

    /**
     * Sets the operand being the instruction pointer.
     *
     * \param reg Instruction pointer register operand.
     */
    void initInstructionPointer(const Register *reg);

private:
    /**
     * Creates cached register operand for the given register.
     * 
     * \param regizter                 Register to register.
     */
    void addRegisterOperand(const Register *regizter);

    /** Architecture's bitness. */
    SmallBitSize mBitness;

    /** Maximum length of an instruction on this architecture. */
    SmallBitSize mMaxInstructionSize;

    /** Disassembler for parsing a single instruction. */
    disasm::InstructionDisassembler *mInstructionDisassembler;

    /** Instruction analyzer for this architecture. */
    irgen::InstructionAnalyzer *mInstructionAnalyzer;

    /** Universal analyzer for this architecture. */
    const UniversalAnalyzer *mUniversalAnalyzer;

    /** Instruction dictionary for this architecture. */
    Mnemonics *mMnemonics;

    /** Register container for this architecture. */
    Registers *mRegisters;

    /** Instruction pointer register. */
    const Register *mInstructionPointer;

    /** Cached register operands. */
    std::vector<RegisterOperand *> mRegisterOperandByNumber;

    /** Cached constant operands. */
    mutable boost::unordered_map<std::pair<ConstantValue, SmallBitSize>, ConstantOperand *> mConstantOperands;
};

} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
