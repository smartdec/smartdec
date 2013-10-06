/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory>

#include <boost/unordered_map.hpp>

#include <nc/core/arch/OperandCache.h>

namespace nc {
namespace arch {
namespace intel {

class FpuStackOperand;

/**
 * Owner and factory of various kinds of operands.
 */
class IntelOperandCache: public core::arch::OperandCache {
public:
    /**
     * Constructor.
     *
     * \param architecture Valid pointer to an architecture.
     */
    IntelOperandCache(const core::arch::Architecture *architecture);

    /**
     * Destructor.
     */
    ~IntelOperandCache();

    /**
     * \param index FPU stack index.
     *
     * \returns Valid pointer to the operand for the given FPU stack index.
     */
    FpuStackOperand *getFpuStackOperand(int index);

private:
    /** Cached FPU stack operands. */
    boost::unordered_map<int, std::unique_ptr<FpuStackOperand>> fpuStackOperands_;
};

} // namespace intel
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
