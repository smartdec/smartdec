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

namespace arch {
    class Architecture;
    class Instructions;
}

namespace ir {
    class Function;

    namespace cconv {
        class CalleeId;
    }
}

class Module;
class Context;

/**
 * Class capable of performing various kinds of analysis in the right order.
 *
 * If an architecture requires to do some analysis differently,
 * you can derive from this class, reimplement certain virtual functions,
 * and register it by calling Architecture::setMasterAnalyzer().
 * 
 * Methods of this class can be executed concurrently.
 * (Though, only on different context currently.)
 * Therefore, they all are const.
 */
class MasterAnalyzer {
    public:

    /**
     * Virtual destructor.
     */
    virtual ~MasterAnalyzer();

    /**
     * Builds an intermediate representation of a program from a set of instructions.
     *
     * \param context Context.
     */
    virtual void createProgram(Context &context) const;

    /**
     * Isolates functions in the program.
     *
     * \param context Context.
     */
    virtual void createFunctions(Context &context) const;

    /**
     * Picks and sets the name for a function.
     *
     * \param context Context.
     * \param function Valid pointer to the function.
     */
    virtual void pickFunctionName(Context &context, ir::Function *function) const;

    /**
     * Creates the calls data will the calling convention detector using detectCallingConvention method of this class.
     *
     * \param context Context.
     */
    virtual void createHooks(Context &context) const;

    /**
     * Detects and sets the calling convention of a function.
     *
     * \param context Context.
     * \param descriptor Descriptor of the function.
     */
    virtual void detectCallingConvention(Context &context, const ir::cconv::CalleeId &descriptor) const;

    /**
     * Constructs term to function mapping.
     *
     * \param context Context.
     */
    virtual void computeTermToFunctionMapping(Context &context) const;

    /**
     * Analyzes the dataflow of a function.
     *
     * \param context Context.
     * \param function Valid pointer to the function.
     */
    virtual void analyzeDataflow(Context &context, const ir::Function *function) const;

    /**
     * Reconstructs signatures of functions.
     *
     * \param context Context.
     */
    virtual void reconstructSignatures(Context &context) const;

    /**
     * Analyzes the usage of function's terms.
     *
     * \param context Context.
     * \param function Valid pointer to the function.
     */
    virtual void computeUsage(Context &context, const ir::Function *function) const;

    /**
     * Computes types of function's terms.
     *
     * \param context Context.
     * \param function Valid pointer to the function.
     */
    virtual void reconstructTypes(Context &context, const ir::Function *function) const;

    /**
     * Reconstructs variables of the function.
     *
     * \param context Context.
     * \param function Valid pointer to the function.
     */
    virtual void reconstructVariables(Context &context, const ir::Function *function) const;

    /**
     * Does structural analysis of the function.
     *
     * \param context Context.
     * \param function Valid pointer to the function.
     */
    virtual void doStructuralAnalysis(Context &context, const ir::Function *function) const;

    /**
     * Generates LikeC tree for the context.
     *
     * \param context Context.
     */
    virtual void generateTree(Context &context) const;

#ifdef NC_TREE_CHECKS
    /**
     * Checks generated LikeC tree and makes the process crash if something is wrong.
     *
     * \param context Context.
     */
    virtual void checkTree(Context &context) const;
#endif
};

} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
