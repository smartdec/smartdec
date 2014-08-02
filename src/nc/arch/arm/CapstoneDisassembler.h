/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <cassert>
#include <memory>

#include <capstone/capstone.h>

#include <nc/common/Exception.h>
#include <nc/common/Types.h>

namespace nc {
namespace arch {
namespace arm {

/**
 * Deleter for Capstone instructions.
 */
class CapstoneDeleter {
    std::size_t count_;

public:
    explicit
    CapstoneDeleter(std::size_t count): count_(count) {}

    void operator()(cs_insn *ptr) const {
        cs_free(ptr, count_);
    }
};

/**
 * Owning pointer to a capstone instruction.
 */
typedef std::unique_ptr<cs_insn, CapstoneDeleter> CapstoneInstructionPtr;

/**
 * This is a thin RAII wrapper over Capstone disassembler.
 */
class CapstoneDisassembler {
    csh handle_;

public:
    /**
     * Constructor.
     *
     * \param arch Architecture.
     * \param mode Mode.
     */
    CapstoneDisassembler(cs_arch arch, int mode) {
        auto result = cs_open(arch, static_cast<cs_mode>(mode), &handle_);
        if (result != CS_ERR_OK) {
            throw nc::Exception(cs_strerror(result));
        }
    }

    /**
     * Destructor.
     */
    ~CapstoneDisassembler() {
        if (cs_close(&handle_) != CS_ERR_OK) {
            assert(!"Could not deinitialize Capstone's ARM disassembler.");
        }
    }

    /**
     * Disassembles given number of instruction.
     *
     * \param[in] pc Virtual address of the first instruction.
     * \param[in] buffer Valid pointer to the buffer containing the instructions.
     * \param[in] size Buffer size.
     * \param[in] count How many instructions to disassemble.
     *
     * \return Pointer to the instruction disassembled from the buffer if disassembling succeeded, NULL otherwise.
     */
    CapstoneInstructionPtr disassemble(ByteAddr pc, const void *buffer, ByteSize size, std::size_t count = 1) {
        cs_insn *insn;
        count = cs_disasm_ex(handle_, reinterpret_cast<const uint8_t *>(buffer), size, pc, count, &insn);
        return CapstoneInstructionPtr(insn, CapstoneDeleter(count));
    }
};

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
