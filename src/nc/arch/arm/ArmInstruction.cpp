/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "ArmInstruction.h"

namespace nc {
namespace arch {
namespace arm {

void ArmInstruction::print(QTextStream &out) const {
    out << text_;
}

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
