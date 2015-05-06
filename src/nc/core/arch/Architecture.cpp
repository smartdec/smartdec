//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "Architecture.h"

#include <boost/range/adaptor/map.hpp>

#include <nc/common/BitTwiddling.h>
#include <nc/common/Foreach.h>

#include <nc/core/arch/irgen/InstructionAnalyzer.h>

#include "Operands.h"
#include "Registers.h"

namespace nc {
namespace core {
namespace arch {

Architecture::Architecture():
    mBitness(0),
    mMaxInstructionSize(0),
    mInstructionDisassembler(NULL),
    mInstructionAnalyzer(NULL),
    mUniversalAnalyzer(NULL),
    mMnemonics(NULL),
    mRegisters(NULL),
    mInstructionPointer(NULL)
{}

void Architecture::initBitness(SmallBitSize bitness) {
    assert(bitness > 0 && "Bitness must be a positive integer.");
    assert(mBitness == 0 && "Bitness cannot be reset.");

    mBitness = bitness;
}

void Architecture::initMaxInstructionSize(SmallBitSize size) {
    assert(size > 0 && "Maximum instruction size must be a positive integer.");
    assert(mMaxInstructionSize == 0 && "Maximum instruction cannot be reset.");

    mMaxInstructionSize = size;
}

void Architecture::initInstructionDisassembler(disasm::InstructionDisassembler *disassembler) {
    assert(disassembler != NULL);
    assert(mInstructionDisassembler == NULL && "Instruction disassembler cannot be reset.");

    mInstructionDisassembler = disassembler;
}

void Architecture::initInstructionPointer(const Register *reg) {
    assert(reg != NULL);
    assert(mInstructionPointer == NULL && "Instruction pointer cannot be reset.");

    mInstructionPointer = reg;
}

void Architecture::initInstructionAnalyzer(irgen::InstructionAnalyzer *instructionAnalyzer) {
    assert(instructionAnalyzer != NULL);
    assert(mInstructionAnalyzer == NULL && "Instruction analyzer cannot be reset.");

    mInstructionAnalyzer = instructionAnalyzer;
}

void Architecture::initUniversalAnalyzer(const UniversalAnalyzer *universalAnalyzer) {
    assert(universalAnalyzer != NULL);
    assert(mUniversalAnalyzer == NULL && "Universal analyzer is already set.");

    mUniversalAnalyzer = universalAnalyzer;
}

void Architecture::initMnemonics(Mnemonics *mnemonics) {
    assert(mnemonics != NULL);
    assert(mMnemonics == NULL && "Instruction dictionary already initialized.");

    mMnemonics = mnemonics;
}

void Architecture::initRegisters(Registers *registers) {
    assert(registers != NULL);
    assert(mMnemonics == NULL && "Register container already initialized.");

    mRegisters = registers;
    foreach(const Register *regizter, registers->registers()) {
        addRegisterOperand(regizter);
    }
}

Architecture::~Architecture() {
    foreach(Operand *operand, mConstantOperands | boost::adaptors::map_values) {
        delete operand;
    }

    foreach(Operand *operand, mRegisterOperandByNumber) {
        delete operand;
    }
}

RegisterOperand *Architecture::registerOperand(int number) const {
    if (number < 0 || static_cast<std::size_t>(number) >= mRegisterOperandByNumber.size())
        return NULL;
        
    return mRegisterOperandByNumber[number];
}

RegisterOperand *Architecture::registerOperand(const Register *regizter) const {
    return registerOperand(regizter->number());
}

void Architecture::addRegisterOperand(const Register *regizter) {
    assert(regizter != NULL);

    int number = regizter->number();

    assert(number >= 0);
    assert(registerOperand(regizter) == NULL); /* No re-registration. */

    if(static_cast<std::size_t>(number) >= mRegisterOperandByNumber.size())
        mRegisterOperandByNumber.resize((number + 1) * 2, NULL);
    
    mRegisterOperandByNumber[number] = new RegisterOperand(regizter);
}

ConstantOperand *Architecture::constantOperand(const SizedValue &value) const {
    auto &result = mConstantOperands[std::make_pair(value.value(), value.size())];
    if (!result) {
        result = new ConstantOperand(value);
    }
    return result;
}

bool Architecture::isGlobalMemory(const ir::MemoryLocation &memoryLocation) const {
    return memoryLocation.domain() == ir::MemoryDomain::MEMORY;
}

} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
