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
namespace core {
namespace ir {

class Function;

namespace calls {
    class CallsData;
}

namespace dflow {
    class Dataflow;
}

namespace vars {

class Variables;

/**
 * Class performing reconstruction of variables.
 */
class VariableAnalyzer {
    Variables &variables_; ///< Mapping of terms to variables.
    const dflow::Dataflow &dataflow_; ///< Dataflow information.
    calls::CallsData *callsData_; ///< Calls data.

    public:

    /**
     * Class constructor.
     *
     * \param variables Information about variables.
     * \param dataflow Dataflow information.
     * \param callsData Valid pointer to the calls data.
     */
    VariableAnalyzer(Variables &variables, const dflow::Dataflow &dataflow, calls::CallsData *callsData):
        variables_(variables), dataflow_(dataflow), callsData_(callsData)
    {}

    /**
     * Virtual destructor.
     */
    virtual ~VariableAnalyzer() {}

    /**
     * \return Mapping of terms to variables.
     */
    Variables &variables() { return variables_; }

    /**
     * \return Mapping of terms to variables.
     */
    const Variables &variables() const { return variables_; }

    /**
     * \return Dataflow information.
     */
    const dflow::Dataflow &dataflow() const { return dataflow_; }

    /**
     * \return Pointer to the calls data. Can be NULL.
     */
    calls::CallsData *callsData() const { return callsData_; }

    /**
     * Computes mapping of terms to variables for the given function.
     *
     * \param[in] function Function to analyze.
     */
    virtual void analyze(const Function *function);
};

} // namespace vars
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
