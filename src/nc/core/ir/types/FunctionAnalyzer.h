/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <vector>

namespace nc {
namespace core {
namespace ir {

class BinaryOperator;
class Function;
class Term;
class UnaryOperator;

namespace calling {
    class Hooks;
}

namespace dflow {
    class Dataflow;
}

namespace liveness {
    class Liveness;
}

namespace types {

class Types;

/**
 * This class performs reconstruction of types on function level.
 */
class FunctionAnalyzer {
    Types &types_; ///< Information about terms' types.
    const dflow::Dataflow &dataflow_; ///< Dataflow information for the function.
    const liveness::Liveness &liveness_; ///< Liveness information for the function.

public:
    /**
     * Constructor.
     *
     * \param[out] types Information about types of terms.
     * \param[in] function Valid pointer to the function being analyzed.
     * \param[in] dataflow Dataflow information for this function.
     * \param[in] liveness Liveness information for this function.
     */
    FunctionAnalyzer(Types &types, const Function *function, const dflow::Dataflow &dataflow,
        const liveness::Liveness &liveness);

    /**
     * Recomputes types of terms in the function being analyzed.
     *
     * \return True if types changed, false otherwise.
     */
    bool analyze();

private:
    /**
     * Recomputes type of the given term.
     *
     * \param term Valid pointer to a term.
     */
    void analyze(const Term *term);

    /**
     * Recomputes type of the given term.
     *
     * \param unary Valid pointer to a unary operator term.
     */
    void analyze(const UnaryOperator *unary);

    /**
     * Recomputes type of the given term.
     *
     * \param binary Valid pointer to a binary operator term.
     */
    void analyze(const BinaryOperator *binary);
};

}}}} // namespace nc::core::ir::types

/* vim:set et sts=4 sw=4: */
