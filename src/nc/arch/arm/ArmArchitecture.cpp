/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "ArmArchitecture.h"

#include <nc/common/make_unique.h>

#include <nc/core/MasterAnalyzer.h>

#include "ArmDisassembler.h"
#include "ArmInstruction.h"
#include "ArmInstructionAnalyzer.h"
#include "ArmRegisters.h"
#include "CallingConventions.h"

namespace nc {
namespace arch {
namespace arm {

ArmArchitecture::ArmArchitecture(ByteOrder byteOrder) {
    if (byteOrder == ByteOrder::LittleEndian) {
        setName(QLatin1String("arm-le"));
    } else {
        setName(QLatin1String("arm-be"));
    }
    setBitness(32);
    setByteOrder(byteOrder);
    setMaxInstructionSize(ArmInstruction::MAX_SIZE);

    setRegisters(ArmRegisters::instance());

    masterAnalyzer_ = std::make_unique<core::MasterAnalyzer>();
    setMasterAnalyzer(masterAnalyzer_.get());

    addCallingConvention(std::make_unique<DefaultCallingConvention>());
}

ArmArchitecture::~ArmArchitecture() {}

std::unique_ptr<core::arch::Disassembler> ArmArchitecture::createDisassembler() const {
    return std::make_unique<ArmDisassembler>(this);
}

std::unique_ptr<core::irgen::InstructionAnalyzer> ArmArchitecture::createInstructionAnalyzer() const {
    return std::make_unique<ArmInstructionAnalyzer>(this);
}

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
