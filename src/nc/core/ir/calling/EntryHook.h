/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <boost/unordered_map.hpp>

#include <nc/common/ilist.h>
#include <nc/common/Range.h>

namespace nc {
namespace core {
namespace ir {

class Function;
class Statement;
class Term;

namespace calling {

class Convention;
class FunctionSignature;

/**
 * Hook installed at function's entry.
 */
class EntryHook {
    /** Statements inserted during installation. */
    nc::ilist<Statement> statements_;

    /** Mapping from argument terms to their clones. */
    boost::unordered_map<const Term *, const Term *> argumentTerms_;

    /** Number of inserted statements. */
    std::size_t insertedStatementsCount_;

public:
    /**
     * Class constructor.
     *
     * \param[in] convention    Valid pointer to the calling convention.
     * \param[in] signature     Pointer to the function's signature. Can be NULL.
     */
    EntryHook(const Convention *convention, const FunctionSignature *signature);

    /**
     * Destructor.
     */
    ~EntryHook();

    /**
     * Instruments a function.
     *
     * \param[in] function Valid pointer to the function.
     */
    void instrument(Function *function);

    /**
     * Deinstruments the previously instrumented function.
     *
     * \param[in] function Valid pointer to the function.
     */
    void deinstrument(Function *function);

    /**
     * \param term Valid pointer to the term representing an argument
     *             in the signature.
     *
     * \return Pointer to the term representing this argument in the instrumentation.
     *         Will be NULL, if the signature does not include such an argument.
     */
    const Term *getArgumentTerm(const Term *term) const {
        assert(term != NULL);
        return nc::find(argumentTerms_, term);
    }

    /**
     * \return Mapping from argument terms to their clones.
     */
    const boost::unordered_map<const Term *, const Term *> &argumentTerms() const { return argumentTerms_; }
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
