/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "IntelOperandCache.h"

#include "IntelOperands.h"

namespace nc {
namespace arch {
namespace intel {

IntelOperandCache::IntelOperandCache(const core::arch::Architecture *architecture):
    OperandCache(architecture)
{}

IntelOperandCache::~IntelOperandCache() {}

FpuOperand *IntelOperandCache::getFpuOperand(int index) {
    auto &result = fpuOperands_[index];
    if (!result) {
        result.reset(new FpuOperand(index));
    }
    return result.get();
}

} // namespace intel
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
