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
class SimulationContext;

/**
 * Analyzer of function's dataflow.
 */
class DataflowAnalyzer {
    Dataflow &dataflow_; ///< Results of analyses.
    const arch::Architecture *architecture_; ///< Valid pointer to architecture description.
    calls::CallsData *callsData_; ///< Calls data.

    public:

    /**
     * Constructor.
     *
     * \param dataflow An object where to store results of analyses.
     * \param architecture Valid pointer to architecture description.
     * \param callsData Pointer to the calls data. Can be NULL.
     */
    DataflowAnalyzer(Dataflow &dataflow, const arch::Architecture *architecture, calls::CallsData *callsData = NULL):
        dataflow_(dataflow), architecture_(architecture), callsData_(callsData)
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
    Dataflow &dataflow() { return dataflow_; }

    /**
     * An object where the results of analyses are stored.
     */
    const Dataflow &dataflow() const { return dataflow_; }

    /**
     * \return Valid pointer to architecture description.
     */
    const arch::Architecture *architecture() const { return architecture_; }

    /**
     * \return Pointer to the calls data. Can be NULL.
     */
    calls::CallsData *callsData() const { return callsData_; }

    /**
     * Performs joint dataflow and constant propagation/folding analysis on a function.
     *
     * \param[in] function Function to analyze.
     * \param[in] canceled Cancellation token.
     */
    void analyze(const Function *function, const CancellationToken &canceled);

    /**
     * Simulates execution of a statement.
     *
     * \param[in] statement             Valid pointer to a statement.
     * \param     context               Simulation context.
     */
    virtual void simulate(const Statement *statement, SimulationContext &context);

    /**
     * Simulates computing of a term.
     *
     * \param[in] term                  Valid pointer to a term.
     * \param     context               Simulation context.
     */
    virtual void simulate(const Term *term, SimulationContext &context);

    protected:

    /**
     * Simulates computing of a unary operator.
     *
     * \param[in] unary                 Valid pointer to a UnaryOperator instance.
     * \param     context               Simulation context.
     */
    virtual void simulateUnaryOperator(const UnaryOperator *unary, SimulationContext &context);

    /**
     * Simulates computing of a binary operator.
     *
     * \param[in] binary                Valid pointer to a BinaryOperator instance.
     * \param     context               Simulation context.
     */
    virtual void simulateBinaryOperator(const BinaryOperator *binary, SimulationContext &context);
};

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
