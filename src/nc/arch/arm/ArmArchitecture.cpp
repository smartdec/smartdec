/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "ArmArchitecture.h"

#include <nc/common/make_unique.h>

#include "ArmDisassembler.h"
#include "ArmInstructionAnalyzer.h"

namespace nc {
namespace arch {
namespace arm {

ArmArchitecture::ArmArchitecture(ByteOrder byteOrder) {
    if (byteOrder == ByteOrder::LittleEndian) {
        setName(QLatin1String("arm-le"));
    } else {
        setName(QLatin1String("arm-be"));
    }
    setByteOrder(byteOrder);
    setMaxInstructionSize(4);
}

ArmArchitecture::~ArmArchitecture() {}

std::unique_ptr<core::arch::Disassembler> ArmArchitecture::createDisassembler() const {
    return std::make_unique<ArmDisassembler>(this);
}

std::unique_ptr<core::irgen::InstructionAnalyzer> ArmArchitecture::createInstructionAnalyzer() const {
    return std::make_unique<ArmInstructionAnalyzer>();
}

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
