/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <vector>

namespace nc {
namespace core {

namespace arch {
    class Architecture;
}

namespace ir {

class Function;
class Jump;
class Statement;
class Term;

namespace calls {
    class CallsData;
}

namespace cflow {
    class Graph;
}

namespace dflow {
    class Dataflow;
}

namespace usage {

class Usage;

/**
 * Class computes the set of terms that will be used during code generation.
 */
class UsageAnalyzer {
    Usage &usage_; ///< Usage information.
    const Function *function_; ///< Function to be analyzed.
    const dflow::Dataflow *dataflow_; ///< Dataflow information.
    const arch::Architecture *architecture_; ///< Architecture.
    const cflow::Graph *regionGraph_; ///< Reduced control-flow graph.
    calls::CallsData *callsData_; ///< Calls data.
    std::vector<const Jump *> uselessJumps_; ///< Useless jumps.

public:
    /**
     * Constructor.
     *
     * \param[out] usage Usage information.
     * \param[in] function Valid pointer to a function to be analyzed.
     * \param[in] dataflow Dataflow information.
     * \param[in] regionGraph Reduced control-flow graph.
     * \param[in] architecture Valid pointer to the architecture.
     * \param[in] regionGraph Reduced control-flow graph.
     * \param[in] callsData Valid pointer to the calls data.
     */
    UsageAnalyzer(Usage &usage, const Function *function,
        const dflow::Dataflow *dataflow, const arch::Architecture *architecture, 
        const cflow::Graph *regionGraph, calls::CallsData *callsData);

    /**
     * Virtual destructor.
     */
    virtual ~UsageAnalyzer() {}

    /**
     * \return Usage information.
     */
    Usage &usage() { return usage_; }

    /**
     * \return Usage information.
     */
    const Usage &usage() const { return usage_; }

    /**
     * \return Valid pointer to the function being analyzed.
     */
    const Function *function() const { return function_; }

    /**
     * \return Valid pointer to the dataflow information.
     */
    const dflow::Dataflow *dataflow() const { return dataflow_; }

    /**
     * \return Valid pointer to the architecture.
     */
    const arch::Architecture *architecture() const { return architecture_; }

    /**
     * \return Pointer to the reduced control-flow graph. Can be NULL.
     */
    const cflow::Graph *regionGraph() const { return regionGraph_; }

    /**
     * \return Pointer to the call graph. Can be NULL.
     */
    calls::CallsData *callsData() const { return callsData_; }

    /**
     * Computes the set of used terms.
     */
    void analyze();

protected:
    /**
     * Computes usage of statement's terms based on the statement's kind.
     *
     * \param[in] statement Statement.
     */
    virtual void computeUsage(const Statement *statement);

    /**
     * Computes usage of a term based on the term's kind.
     *
     * \param[in] term Term to consider.
     */
    virtual void computeUsage(const Term *term);

    /**
     * Marks as used all the terms, used by given term in order to generate code.
     *
     * \param[in] term Used term.
     */
    virtual void propagateUsage(const Term *term);

    /**
     * If given term is not used, marks it as used and propagates usage further.
     *
     * \param[in] term Term.
     */
    void makeUsed(const Term *term);
};

} // namespace usage
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
