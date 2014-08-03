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
}

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
