/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "CallingConventions.h"

#include "ArmArchitecture.h"
#include "ArmRegisters.h"

namespace nc {
namespace arch {
namespace arm {

DefaultCallingConvention::DefaultCallingConvention():
    core::ir::calling::Convention(QLatin1String("Default"))
{
    setStackPointer(ArmRegisters::sp()->memoryLocation());

    setFirstArgumentOffset(0);
    setArgumentAlignment(32);

    std::vector<core::ir::MemoryLocation> args;
    args.push_back(ArmRegisters::r0()->memoryLocation());
    args.push_back(ArmRegisters::r1()->memoryLocation());
    args.push_back(ArmRegisters::r2()->memoryLocation());
    args.push_back(ArmRegisters::r3()->memoryLocation());
    addArgumentGroup(std::move(args));

    addReturnValueLocation(ArmRegisters::r0()->memoryLocation());
}

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
