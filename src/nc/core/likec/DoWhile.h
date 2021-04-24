/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

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

#include "Statement.h"

#include <memory> /* unique_ptr */

namespace nc {
namespace core {
namespace likec {

class Expression;

/**
 * Do-While loop.
 */
class DoWhile: public Statement {
    std::unique_ptr<Statement> body_; ///< Loop body.
    std::unique_ptr<Expression> condition_; ///< Loop condition.

public:
    /**
     * Class constructor.
     *
     * \param[in] condition Valid pointer to the loop condition.
     * \param[in] body Valid pointer to the loop body.
     */
    DoWhile(std::unique_ptr<Statement> body, std::unique_ptr<Expression> condition):
        Statement(DO_WHILE), body_(std::move(body)), condition_(std::move(condition))
    {}

    /**
     * \return Loop body.
     */
    std::unique_ptr<Statement> &body() { return body_; }

    /**
     * \return Loop body.
     */
    const Statement *body() const { return body_.get(); }

    /**
     * \return Loop condition.
     */
    std::unique_ptr<Expression> &condition() { return condition_; }

    /**
     * \return Loop condition.
     */
    const Expression *condition() const { return condition_.get(); }

protected:
    void doCallOnChildren(const std::function<void(TreeNode *)> &fun) override;
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::Statement, nc::core::likec::DoWhile, nc::core::likec::Statement::DO_WHILE)

/* vim:set et sts=4 sw=4: */
