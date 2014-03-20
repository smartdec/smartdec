/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory>
#include <vector>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <nc/core/ir/MemoryLocation.h>

namespace nc {
namespace core {
namespace ir {

class MemoryLocation;
class Statement;
class Term;

namespace dflow {
    class ExecutionContext;
}

namespace calling {

class Convention;
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
    boost::unordered_map<const Term *, std::unique_ptr<Term>> arguments_;

    /** Set of the values in arguments_. */
    boost::unordered_set<const Term *> argumentsSet_;

public:
    /**
     * Class constructor.
     *
     * \param[in] convention Valid pointer to the calling convention.
     * \param[in] signature Pointer to the function's signature. Can be NULL.
     */
    EntryHook(const Convention *convention, const Signature *signature);

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
     * \param term Valid pointer to a term representing an argument
     *             in a signature.
     *
     * \return Pointer to the term representing this argument in the hook.
     *         Will be NULL, if signature does not include such an argument.
     */
    const Term *getArgumentTerm(const Term *term) const;

    /**
     * \param term Valid pointer to a term.
     *
     * \return True if the term is an argument term belonging to this hook,
     *         false otherwise.
     */
    bool isArgumentTerm(const Term *term) const;
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
