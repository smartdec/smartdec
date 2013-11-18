/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory> /* std::unique_ptr */

#include <boost/unordered_map.hpp>

#include <nc/common/Visitor.h>

#include <nc/core/ir/MemoryLocation.h>

namespace nc {
namespace core {
namespace ir {

class Function;
class MemoryLocation;
class Statement;
class Term;

namespace dflow {
    class ExecutionContext;
}

namespace cconv {

class CallingConvention;
class Signature;

/**
 * Hook being executed before function's entry is executed.
 */
class EntryHook {
    /** Term for initializing stack pointer. */
    std::unique_ptr<Term> stackPointer_;

    /** Statements executed when a function is entered. */
    std::vector<std::unique_ptr<const Statement>> entryStatements_;

    /** Mapping of argument memory locations to corresponding terms. */
    boost::unordered_map<MemoryLocation, std::unique_ptr<Term>> arguments_;

public:
    /**
     * Class constructor.
     *
     * \param[in] convention Valid pointer to the calling convention.
     * \param[in] signature Pointer to the function's signature. Can be NULL.
     */
    EntryHook(const CallingConvention *convention, const Signature *signature);

    /**
     * Destructor.
     */
    ~EntryHook();

    /**
     * \return Statements that are executed behind the scence on function entry. Can be NULL.
     *
     * These statements will be actually used for generation of function->entry() basic block's code.
     */
    const std::vector<const Statement *> &entryStatements() const {
        return reinterpret_cast<const std::vector<const Statement *> &>(entryStatements_);
    }

    /**
     * This method is called just before function's entry
     * basic block gets executed.
     *
     * \param context Execution context.
     */
    void execute(dflow::ExecutionContext &context);

    /**
     * \param memoryLocation Memory location.
     *
     * \return Pointer to the term representing the argument at given memory
     *         location. Will be NULL, if signature does not include such an
     *         argument.
     */
    const Term *getArgumentTerm(const MemoryLocation &memoryLocation);

    /**
     * Calls visitor for child statements.
     *
     * \param[in] visitor Visitor.
     */
    void visitChildStatements(Visitor<const Statement> &visitor) const;

    /**
     * Calls visitor for child terms.
     *
     * \param[in] visitor Visitor.
     */
    void visitChildTerms(Visitor<const Term> &visitor) const;
};

} // namespace cconv
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
