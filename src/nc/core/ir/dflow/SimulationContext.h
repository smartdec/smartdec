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

#include "ReachingDefinitions.h"

namespace nc {
namespace core {
namespace ir {
namespace dflow {

class DataflowAnalyzer;

/**
 * Simulation context.
 * It is passed to simulate() methods of Statement and Term classes.
 * Context represents the state of simulated program.
 */
class SimulationContext {
    DataflowAnalyzer &analyzer_; ///< DataflowAnalyzer performing the simulation.
    ReachingDefinitions definitions_; ///< Reaching definitions.
    const Function *function_; ///< Function being simulated.
    bool fixpointReached_; ///< Whether stationary point in reaching definitions is reached.

    public:

    /**
     * Class constructor.
     *
     * \param analyzer                  DataflowAnalyzer performing the simulation.
     * \param function                  Pointer to the function being simulated. Can be NULL.
     * \param fixpointReached           Flag which is true if reaching definitions didn't change
     *                                  during last iteration of function's simulation.
     */
    SimulationContext(DataflowAnalyzer &analyzer, const Function *function = NULL, bool fixpointReached = false):
        analyzer_(analyzer),
        function_(function),
        fixpointReached_(fixpointReached)
    {}

    /**
     * \return DataflowAnalyzer performing the simulation.
     */
    DataflowAnalyzer &analyzer() { return analyzer_; }

    /**
     * \return DataflowAnalyzer performing the simulation.
     */
    const DataflowAnalyzer &analyzer() const { return analyzer_; }

    /**
     * \return Pointer to the function being simulated. Can be NULL.
     */
    const Function *function() const { return function_; }

    /**
     * \return Whether stationary point in reaching definitions is reached.
     */
    bool fixpointReached() const { return fixpointReached_; }

    /**
     * \return Reaching definitions.
     */
    ReachingDefinitions &definitions() { return definitions_; }

    /**
     * \return Reaching definitions.
     */
    const ReachingDefinitions &definitions() const { return definitions_; }
};

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
