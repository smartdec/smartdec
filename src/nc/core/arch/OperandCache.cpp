/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "OperandCache.h"

#include <nc/common/make_unique.h>

#include "Architecture.h"
#include "Operands.h"
#include "Registers.h"

namespace nc {
namespace core {
namespace arch {

OperandCache::OperandCache(const Architecture *architecture) {
    assert(architecture != NULL);

    registerOperands_.reserve(architecture->registers()->registers().size());

    foreach (auto reg, architecture->registers()->registers()) {
        assert(getRegisterOperand(reg) == NULL);

        if (reg->number() >= registerOperands_.size()) {
            registerOperands_.resize(reg->number() + 1);
        }

        registerOperands_[reg->number()].reset(new RegisterOperand(reg));
    }
}

OperandCache::~OperandCache() {}

RegisterOperand *OperandCache::getRegisterOperand(int number) const {
    if (0 <= number && static_cast<std::size_t>(number) < registerOperands_.size()) {
        return registerOperands_[number].get();
    } else {
        return NULL;
    }
}

RegisterOperand *OperandCache::getRegisterOperand(const Register *reg) const {
    assert(reg != NULL);

    return getRegisterOperand(reg->number());
}

ConstantOperand *OperandCache::getConstantOperand(const SizedValue &value) {
    auto &result = constantOperands_[std::make_pair(value.value(), value.size())];

    if (!result) {
        result.reset(new ConstantOperand(value));
    }

    return result.get();
}

} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
