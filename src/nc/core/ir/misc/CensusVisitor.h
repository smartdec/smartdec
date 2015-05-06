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

#include <nc/common/Visitor.h>

namespace nc {
namespace core {
namespace ir {

class BasicBlock;
class Function;
class Statement;
class Term;

namespace calls {
    class CallsData;
}

namespace misc {

/**
 * An ultimate visitor of all statements and terms.
 */
class CensusVisitor:
    public Visitor<const BasicBlock>, public Visitor<const Function>,
    public Visitor<const Statement>, public Visitor<const Term>
{
    calls::CallsData *callsData_; ///< Calls data.

    std::vector<const Statement *> statements_; ///< Statements visited.
    std::vector<const Term *> terms_; ///< Terms visited.

    const Function *currentFunction_; ///< Current function.

    public:

    /**
     * Constructor.
     *
     * \param callsData Pointer to the calls data. Can be NULL.
     */
    CensusVisitor(calls::CallsData *callsData): callsData_(callsData), currentFunction_(NULL) {}

    /**
     * \return Pointer to the calls data being used. Can be NULL.
     */
    calls::CallsData *callsData() const { return callsData_; }

    /**
     * \return Statements visited.
     */
    const std::vector<const Statement *> &statements() const { return statements_; }

    /**
     * \return Terms visited.
     */
    const std::vector<const Term *> &terms() const { return terms_; }

    /**
     * Clears the lists of statements and terms visited.
     */
    void clear();

    virtual void operator()(const Function *function);
    virtual void operator()(const BasicBlock *basicBlock);
    virtual void operator()(const Statement *statement);
    virtual void operator()(const Term *term);
};

} // namespace misc
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
