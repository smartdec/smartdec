/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

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

#include <cassert>

namespace nc {

class CancellationToken;

namespace core {

namespace arch {
    class Architecture;
}

namespace ir {

class BasicBlock;
class Function;
class Statement;
class Term;
class UnaryOperator;
class BinaryOperator;

namespace calls {
    class CallsData;
}

namespace dflow {

class Dataflow;
class ExecutionContext;

/**
 * Dataflow analyzer.
 */
class DataflowAnalyzer {
    Dataflow &dataflow_; ///< Dataflow information.
    const arch::Architecture *architecture_; ///< Valid pointer to architecture description.
    const Function *function_; ///< Analyzed function.
    calls::CallsData *callsData_; ///< Calls data.

    public:

    /**
     * Constructor.
     *
     * \param dataflow An object where to store results of analyses.
     * \param architecture Valid pointer to architecture description.
     * \param function Pointer to the analyzed function. Can be NULL.
     * \param callsData Pointer to the calls data. Can be NULL.
     */
    DataflowAnalyzer(Dataflow &dataflow,
        const arch::Architecture *architecture,
        const Function *function = NULL,
        calls::CallsData *callsData = NULL
    ):
        dataflow_(dataflow), architecture_(architecture), function_(function), callsData_(callsData)
    {
        assert(architecture != NULL);
    }

    /**
     * Virtual destructor.
     */
    virtual ~DataflowAnalyzer() {}

    /**
     * An object where the results of analyses are stored.
     */
    Dataflow &dataflow() const { return dataflow_; }

    /**
     * \return Function being analyzed.
     */

    /**
     * \return Valid pointer to architecture description.
     */
    const arch::Architecture *architecture() const { return architecture_; }

    /**
     * \return Pointer to the analyzed function. Can be NULL.
     */
    const Function *function() const { return function_; }

    /**
     * \return Pointer to the calls data. Can be NULL.
     */
    calls::CallsData *callsData() const { return callsData_; }

    /**
     * Performs joint reaching definitions and constant propagation/folding
     * analysis on the function given to the constructor.
     *
     * \param[in] canceled  Cancellation token.
     */
    void analyze(const CancellationToken &canceled);

    /**
     * Executes a statement.
     *
     * \param[in] statement Valid pointer to a statement.
     * \param     context   Execution context.
     */
    virtual void execute(const Statement *statement, ExecutionContext &context);

    /**
     * Executes a term.
     *
     * \param[in] term      Valid pointer to a term.
     * \param     context   Execution context.
     */
    virtual void execute(const Term *term, ExecutionContext &context);

    protected:

    /**
     * Computes term's value by merging values of reaching definitions.
     *
     * \param[in] term      Valid pointer to a term.
     */
    virtual void mergeReachingValues(const Term *term);

    /**
     * Executes a unary operator.
     *
     * \param[in] unary     Valid pointer to a UnaryOperator instance.
     * \param     context   Execution context.
     */
    virtual void executeUnaryOperator(const UnaryOperator *unary, ExecutionContext &context);

    /**
     * Executes a binary operator.
     *
     * \param[in] binary    Valid pointer to a BinaryOperator instance.
     * \param     context   Execution context.
     */
    virtual void executeBinaryOperator(const BinaryOperator *binary, ExecutionContext &context);
};

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
