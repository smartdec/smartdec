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

namespace nc {
namespace core {
namespace ir {

class Function;

namespace cconv {
    class Hooks;
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
    cconv::Hooks *hooks_; ///< Calls data.

    public:

    /**
     * Class constructor.
     *
     * \param variables Information about variables.
     * \param dataflow Dataflow information.
     * \param hooks Valid pointer to the calls data.
     */
    VariableAnalyzer(Variables &variables, const dflow::Dataflow &dataflow, cconv::Hooks *hooks):
        variables_(variables), dataflow_(dataflow), hooks_(hooks)
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
    cconv::Hooks *hooks() const { return hooks_; }

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
