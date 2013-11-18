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

#include <memory> /* std::unique_ptr */

#include <nc/common/Visitor.h>

#include <boost/unordered_map.hpp>

namespace nc {
namespace core {
namespace ir {

class Statement;
class Term;

namespace dflow {
    class ExecutionContext;
}

namespace calling {

class Convention;
class Signature;

/**
 * Hook being executed after a return is executed.
 */
class ReturnHook {
    /** Mapping of terms where return values may be kept to their clones. */
    boost::unordered_map<const Term *, std::unique_ptr<Term>> returnValues_;

public:
    /**
     * Class constructor.
     *
     * \param[in] convention Valid pointer to the calling convention.
     * \param[in] signature Pointer to the function's signature. Can be NULL.
     */
    ReturnHook(const Convention *convention, const Signature *signature);

    /**
     * Destructor.
     */
    ~ReturnHook();

    /**
     * A method being called when specified return statement is executed.
     * 
     * \param context Execution context.
     */
    void execute(dflow::ExecutionContext &context);

    /**
     * \param term Valid pointer to a term.
     *
     * \return Pointer to the term representing the argument identified by
     *         the given term. Will be NULL, if signature does not include
     *         such an argument.
     */
    const Term *getReturnValueTerm(const Term *term);

    /**
     * Calls visitor for child statements.
     *
     * \param[in] visitor Visitor.
     */
    void visitChildStatements(Visitor<const Statement> &visitor) const;

    /**
     * Calls visitor for child terms.
     *
     * \param[in] visitor Visitor.
     */
    void visitChildTerms(Visitor<const Term> &visitor) const;
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */
