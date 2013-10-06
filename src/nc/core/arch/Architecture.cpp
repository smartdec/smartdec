/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

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

#include <nc/common/Foreach.h>

#include <nc/core/ir/MemoryLocation.h>
#include <nc/core/ir/calls/CallingConvention.h>

namespace nc {
namespace core {
namespace arch {

Architecture::Architecture():
    mBitness(0),
    mByteOrder(ByteOrder::Unknown),
    mMaxInstructionSize(0),
    mInstructionDisassembler(NULL),
    mInstructionAnalyzer(NULL),
    mUniversalAnalyzer(NULL),
    mMnemonics(NULL),
    mRegisters(NULL),
    mInstructionPointer(NULL)
{}

Architecture::~Architecture() {}

void Architecture::setName(QString name) {
    assert(mName.isEmpty() && "Name must be non-empty.");
    assert(!name.isEmpty() && "Name cannot be reset.");

    mName = std::move(name);
}

void Architecture::setBitness(SmallBitSize bitness) {
    assert(bitness > 0 && "Bitness must be a positive integer.");
    assert(mBitness == 0 && "Bitness cannot be reset.");

    mBitness = bitness;
}

void Architecture::setByteOrder(ByteOrder byteOrder) {
    assert(mByteOrder == ByteOrder::Unknown && "Byte order is already set.");
    assert(byteOrder != ByteOrder::Unknown && "Byte order cannot be set to unknown.");

    mByteOrder = byteOrder;
}

void Architecture::setMaxInstructionSize(SmallBitSize size) {
    assert(size > 0 && "Maximal instruction size must be a positive integer.");
    assert(mMaxInstructionSize == 0 && "Maximal instruction size cannot be reset.");

    mMaxInstructionSize = size;
}

void Architecture::setInstructionDisassembler(disasm::InstructionDisassembler *disassembler) {
    assert(disassembler != NULL);
    assert(mInstructionDisassembler == NULL && "Instruction disassembler is already set.");

    mInstructionDisassembler = disassembler;
}

void Architecture::setInstructionPointer(const Register *reg) {
    assert(reg != NULL);
    assert(mInstructionPointer == NULL && "Instruction pointer is already set.");

    mInstructionPointer = reg;
}

void Architecture::setInstructionAnalyzer(irgen::InstructionAnalyzer *instructionAnalyzer) {
    assert(instructionAnalyzer != NULL);
    assert(mInstructionAnalyzer == NULL && "Instruction analyzer is already set.");

    mInstructionAnalyzer = instructionAnalyzer;
}

void Architecture::setUniversalAnalyzer(const UniversalAnalyzer *universalAnalyzer) {
    assert(universalAnalyzer != NULL);
    assert(mUniversalAnalyzer == NULL && "Universal analyzer is already set.");

    mUniversalAnalyzer = universalAnalyzer;
}

void Architecture::setMnemonics(Mnemonics *mnemonics) {
    assert(mnemonics != NULL);
    assert(mMnemonics == NULL && "Instruction dictionary is already set.");

    mMnemonics = mnemonics;
}

void Architecture::setRegisters(Registers *registers) {
    assert(registers != NULL);
    assert(mMnemonics == NULL && "Register container is already set.");

    mRegisters = registers;
}

bool Architecture::isGlobalMemory(const ir::MemoryLocation &memoryLocation) const {
    return memoryLocation.domain() == ir::MemoryDomain::MEMORY;
}

void Architecture::addCallingConvention(std::unique_ptr<ir::calls::CallingConvention> convention) {
    assert(convention != NULL);
    assert(getCallingConvention(convention->name()) == NULL &&
           "No two calling conventions with the same name allowed.");

    callingConventions_.push_back(std::move(convention));
}

const ir::calls::CallingConvention *Architecture::getCallingConvention(const QString &name) const {
    foreach (auto convention, callingConventions()) {
        if (convention->name() == name) {
            return convention;
        }
    }
    return NULL;
}

} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
