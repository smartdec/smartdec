/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory>
#include <vector>

#include <boost/unordered_map.hpp>

#include <nc/common/SizedValue.h>

namespace nc {
namespace core {
namespace arch {

class Architecture;
class ConstantOperand;
class Register;
class RegisterOperand;

/**
 * Owner and factory of various kinds of operands.
 */
class OperandCache {
public:
    /**
     * Constructor.
     *
     * \param architecture Valid pointer to an architecture.
     */
    OperandCache(const Architecture *architecture);

    /**
     * Destructor.
     */
    ~OperandCache();

    /**
     * \param number Register number.
     *
     * \returns Valid pointer to the operand for the given register
     *          number, or NULL if no such register number exists.
     */
    RegisterOperand *getRegisterOperand(int number) const;

    /**
     * \param reg Valid pointer to a register.
     *
     * \returns Valid pointer to the operand for the given register
     *          number, or NULL if no such register exists.
     */
    RegisterOperand *getRegisterOperand(const Register *reg) const;

    /**
     * \param value Value of a constant.
     *
     * \returns Valid pointer to the operand for this constant.
     */
    ConstantOperand *getConstantOperand(const SizedValue &value);

private:
    /** Cached register operands. */
    std::vector<std::unique_ptr<RegisterOperand>> registerOperands_;

    /** Cached constant operands. */
    boost::unordered_map<std::pair<ConstantValue, SmallBitSize>, std::unique_ptr<ConstantOperand>> constantOperands_;
};

} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
