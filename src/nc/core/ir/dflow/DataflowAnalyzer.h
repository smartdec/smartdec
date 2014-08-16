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

#include <QCoreApplication>

#include <nc/common/CancellationToken.h>
#include <nc/common/LogToken.h>

#include <cassert>

namespace nc {

namespace core {

namespace arch {
    class Architecture;
}

namespace ir {

class Function;
class MemoryLocation;
class Statement;
class Term;
class UnaryOperator;
class BinaryOperator;

namespace dflow {

class AbstractValue;
class Dataflow;
class ExecutionContext;
class ReachingDefinitions;

/**
 * Implements a dataflow analysis based on abstract interpretation loop.
 */
class DataflowAnalyzer {
    Q_DECLARE_TR_FUNCTIONS(DataflowAnalyzer)

    Dataflow &dataflow_; ///< Dataflow information.
    const arch::Architecture *architecture_; ///< Valid pointer to architecture description.
    const CancellationToken &canceled_;
    const LogToken &log_;

public:
    /**
     * Constructor.
     *
     * \param dataflow      An object where to store results of analyses.
     * \param architecture  Valid pointer to architecture description.
     * \param canceled      Cancellation token.
     * \param log           Log token.
     */
    DataflowAnalyzer(Dataflow &dataflow, const arch::Architecture *architecture,
        const CancellationToken &canceled, const LogToken &log):
        dataflow_(dataflow), architecture_(architecture), canceled_(canceled), log_(log)
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
     * \return Valid pointer to architecture description.
     */
    const arch::Architecture *architecture() const { return architecture_; }

    /**
     * Performs joint reaching definitions and constant propagation/folding
     * analysis on the function given to the constructor.
     *
     * \param[in] function  Valid pointer to the function.
     */
    void analyze(const Function *function);

    /**
     * Executes a statement.
     *
     * \param statement Valid pointer to a statement.
     * \param context   Execution context.
     */
    virtual void execute(const Statement *statement, ExecutionContext &context);

    /**
     * Executes a term.
     *
     * \param term      Valid pointer to a term.
     * \param context   Execution context.
     */
    virtual void execute(const Term *term, ExecutionContext &context);

protected:
    /**
     * Executes a unary operator.
     *
     * \param[in] unary     Valid pointer to a UnaryOperator instance.
     * \param     context   Execution context.
     */
    void executeUnaryOperator(const UnaryOperator *unary, ExecutionContext &context);

    /**
     * Executes a binary operator.
     *
     * \param[in] binary    Valid pointer to a BinaryOperator instance.
     * \param     context   Execution context.
     */
    void executeBinaryOperator(const BinaryOperator *binary, ExecutionContext &context);

    /**
     * Applies a unary operator to an abstract value.
     *
     * \param unary Valid pointer to the unary operand.
     * \param a Operand value.
     *
     * \return Resulting abstract value. Its size is equal to unary->size().
     */
    AbstractValue apply(const UnaryOperator *unary, const AbstractValue &a);

    /**
     * Applies a binary operator to an abstract value.
     *
     * \param binary Valid pointer to the binary operand.
     * \param a Left operand's value.
     * \param b Right operand's value.
     *
     * \return Resulting abstract value. Its size is equal to unary->size().
     */
    AbstractValue apply(const BinaryOperator *binary, const AbstractValue &a, const AbstractValue &b);

    /**
     * Remembers the new memory location of a term, updates reaching definitions
     * in the execution context accordingly.
     *
     * \param term              Valid pointer to a term.
     * \param newMemoryLocation Memory location of this term. Can be invalid.
     * \param context           Execution context.
     */
    void setMemoryLocation(const Term *term, const MemoryLocation &newMemoryLocation, ExecutionContext &context);

    /**
     * Computes term's value by merging values of reaching definitions.
     *
     * \param term          Valid pointer to a read term.
     * \param termLocation  Valid memory location of this term.
     * \param definitions   Reaching definitions of this memory location.
     */
    void mergeReachingValues(const Term *term, const MemoryLocation &termLocation, const ReachingDefinitions &definitions);
};

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
