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

#include <nc/common/Unused.h>
#include <nc/common/Visitor.h>

#include "TreeNode.h"

namespace nc {
namespace core {

namespace ir {
    class Statement;
}

namespace likec {

class Expression;

/**
 * Base class for statement node.
 */
class Statement: public TreeNode {
    NC_CLASS_WITH_KINDS(Statement, statementKind)

    const ir::Statement *statement_; ///< Formulae statement that this statement was created from.

public:
    enum {
        BLOCK,                          ///< Block.
        BREAK,                          ///< Break.
        CONTINUE,                       ///< Continue.
        DO_WHILE,                       ///< Do-While loop.
        EXPRESSION_STATEMENT,           ///< Expression statement.
        GOTO,                           ///< Goto.
        IF,                             ///< Conditional statement.
        LABEL_STATEMENT,                ///< Label.
        RETURN,                         ///< Return.
        WHILE,                          ///< While loop.
        INLINE_ASSEMBLY,                ///< Inline assembly.
        COMMENT,                        ///< Comment statement.
        SWITCH,                         ///< Switch.
        CASE_LABEL,                     ///< Case label.
        DEFAULT_LABEL,                  ///< Default case label.
        USER_STATEMENT = 1000           ///< Base for user-defined statements.
    };

    /**
     * Class constructor.
     *
     * \param[in] tree Owning tree.
     * \param[in] statementKind Statement kind.
     */
    Statement(Tree &tree, int statementKind):
        TreeNode(tree, STATEMENT), statementKind_(statementKind), statement_(NULL)
    {}

    /**
     * \return Formulae statement that this statement was created from.
     */
    const ir::Statement *statement() const { return statement_; }

    /**
     * \param[in] statement Formulae statement that this statement was created from.
     */
    void setStatement(const ir::Statement *statement) {
        assert(statement != NULL);
        assert(statement_ == NULL); /* Must be used for initialization only. */

        statement_ = statement;
    }

    virtual Statement *rewrite() override { return this; }

protected:
    /**
     * Prints nested statement doing appropriate indenting in case of blocks and usual statements.
     *
     * \param[in] statement Statement to print.
     * \param[in] context Print context.
     */
    static void printNestedStatement(const Statement *statement, PrintContext &context);
};

} // namespace likec
} // namespace core
} // namespace nc

NC_REGISTER_CLASS_KIND(nc::core::likec::TreeNode, nc::core::likec::Statement, nc::core::likec::TreeNode::STATEMENT)

/* vim:set et sts=4 sw=4: */
