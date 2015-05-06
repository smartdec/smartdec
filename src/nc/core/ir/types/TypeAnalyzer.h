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

namespace nc {

class CancellationToken;

namespace core {
namespace ir {

class Assignment;
class BinaryOperator;
class Constant;
class Dereference;
class Function;
class Statement;
class Term;
class UnaryOperator;

namespace calls {
    class CallsData;
}

namespace dflow {
    class Dataflow;
}

namespace usage {
    class Usage;
}

namespace types {

class Types;

/**
 * Class computing the traits of terms' types.
 */
class TypeAnalyzer {
    Types &types_; ///< Information about terms' types.
    const dflow::Dataflow &dataflow_; ///< Dataflow information.
    const usage::Usage &usage_; ///< Set of terms producing actual high-level code.
    calls::CallsData *callsData_; ///< Calls data.

    public:

    /**
     * Class constructor.
     *
     * \param types Information about terms' types.
     * \param dataflow Dataflow information.
     * \param usage Set of terms producing actual high-level code.
     * \param callsData Pointer to the calls data. Can be NULL.
     */
    TypeAnalyzer(Types &types, const dflow::Dataflow &dataflow, const usage::Usage &usage, calls::CallsData *callsData):
        types_(types), dataflow_(dataflow), usage_(usage), callsData_(callsData)
    {}

    /**
     * Virtual destructor.
     */
    virtual ~TypeAnalyzer() {}

    /**
     * \return Information about terms' types.
     */
    Types &types() { return types_; }

    /**
     * \return Information about terms' types.
     */
    const Types &types() const { return types_; }

    /**
     * \return Dataflow information.
     */
    const dflow::Dataflow& dataflow() const { return dataflow_; }

    /**
     * \return Usage information.
     */
    const usage::Usage& usage() const { return usage_; }

    /**
     * \return Pointer to the calls data. Can be NULL.
     */
    calls::CallsData *callsData() const { return callsData_; }

    /**
     * Computes type traits for all the terms in given function.
     *
     * \param[in] function Valid pointer to a function.
     * \param[in] canceled Cancellation token.
     */
    void analyze(const Function *function, const CancellationToken &canceled);

    protected:

    /**
     * Updates the information about type traits of given term and (possibly) its children.
     *
     * \param[in] term Term to analyze.
     */
    virtual void analyze(const Term *term);

    /**
     * Updates the information about type traits of terms being the arguments of given statement.
     *
     * \param[in] statement Statement to analyze.
     */
    virtual void analyze(const Statement *statement);

    private:

    void analyze(const Constant *constant);
    void analyze(const Dereference *dereference);
    void analyze(const UnaryOperator *unary);
    void analyze(const BinaryOperator *binary);
};

}}}} // namespace nc::core::ir::types

/* vim:set et sts=4 sw=4: */
