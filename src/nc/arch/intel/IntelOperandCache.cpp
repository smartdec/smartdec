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

FpuStackOperand *IntelOperandCache::getFpuStackOperand(int index) {
    auto &result = fpuStackOperands_[index];
    if (!result) {
        result.reset(new FpuStackOperand(index));
    }
    return result.get();
}

} // namespace intel
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
