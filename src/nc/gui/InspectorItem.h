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

#include <memory>
#include <vector>

#include <QString>

namespace nc {

namespace core {
    namespace arch {
        class Instruction;
    }

    namespace ir {
        class Statement;
        class Term;
    }

    namespace likec {
        class TreeNode;
        class Type;
    }
}

namespace gui {

/**
 * An item in the TreeInspector's tree model.
 */
class InspectorItem {
    QString text_; ///< Item's text.
    const core::likec::TreeNode *node_; ///< Associated LikeC tree node.
    const core::ir::Term *term_; ///< Associated IR term.
    const core::ir::Statement *statement_; ///< Associated IR statement.
    const core::arch::Instruction *instruction_; ///< Associated instruction.
    const core::likec::Type *type_; ///< LikeC type.
    bool expanded_; ///< True iff children of the node have already been computed.

    InspectorItem *parent_; ///< Parent item.
    int row_; ///< Index of this item in parent's children list.

    std::vector<std::unique_ptr<InspectorItem>> children_; ///< Children of the item.

    public:

    /**
     * Constructor.
     *
     * \param[in] text Item's text.
     */
    InspectorItem(const QString &text):
        text_(text),
        node_(NULL),
        term_(NULL),
        statement_(NULL),
        instruction_(NULL),
        type_(NULL),
        expanded_(false),
        parent_(NULL),
        row_(-1)
    {}

    /**
     * \return Item's text.
     */
    const QString &text() const { return text_; }

    /**
     * Sets item's text.
     *
     * \param[in] text New text.
     */
    void setText(const QString &text) { text_ = text; }

    /**
     * Adds a comment to the item.
     * Effectively, adds the comment to item's text.
     *
     * \param[in] comment Comment.
     */
    void addComment(const QString &comment);

    /**
     * \return Pointer to the associated LikeC node. Can be NULL.
     */
    const core::likec::TreeNode *node() const { return node_; }

    /**
     * Sets associated LikeC node.
     *
     * \param[in] node Associated LikeC node. Can be NULL.
     */
    void setNode(const core::likec::TreeNode *node) { node_ = node; }

    /**
     * \return Pointer to the associated IR term. Can be NULL.
     */
    const core::ir::Term *term() const { return term_; }

    /**
     * Sets associated IR term.
     *
     * \param[in] term Associated IR term. Can be NULL.
     */
    void setTerm(const core::ir::Term *term) { term_ = term; }

    /**
     * \return Pointer to the associated IR statement. Can be NULL.
     */
    const core::ir::Statement *statement() const { return statement_; }

    /**
     * Sets associated IR statement.
     *
     * \param[in] statement Associated IR term. Can be NULL.
     */
    void setStatement(const core::ir::Statement *statement) { statement_ = statement; }

    /**
     * \return Pointer to the associated instruction. Can be NULL.
     */
    const core::arch::Instruction *instruction() const { return instruction_; }

    /**
     * Sets associated instruction.
     *
     * \param[in] instruction Associated instruction. Can be NULL.
     */
    void setInstruction(const core::arch::Instruction *instruction) { instruction_ = instruction; }

    /**
     * Sets associated instruction.
     *
     * \param[in] type Associated LikeC type. Can be NULL.
     */
    void setType(const core::likec::Type *type) { type_ = type; }

    /**
     * \return Pointer to the LikeC type. Can be NULL.
     */
    const core::likec::Type *type() const { return type_; }

    /**
     * \return True iff children of the node have already been computed.
     */
    bool expanded() const { return expanded_; }

    /**
     * Sets the flag whether item's children have been computed.
     *
     * \param[in] value New flag value.
     */
    void setExpanded(bool value) { expanded_ = value; }

    /**
     * \return Pointer to the item's parent. Can be NULL.
     */
    InspectorItem *parent() const { return parent_; }

    /**
     * \return Row in which this item is placed, i.e. the index of this item in parent's children list.
     */
    int row() const { return row_; }

    /**
     * \return Column in which this item is placed, i.e. always zero.
     */
    int column() const { return 0; }

    /**
     * Adds a child item.
     *
     * \param item Valid pointer to the child being added.
     */
    InspectorItem *addChild(InspectorItem *item);

    /**
     * Adds a child with given text.
     *
     * \param[in] text          Text.
     */
    InspectorItem *addChild(const QString &text);

    /**
     * Adds a child with given text and associated with given LikeC node.
     *
     * \param[in] text          Text.
     * \param[in] node          Pointer to a LikeC node.
     */
    InspectorItem *addChild(const QString &text, const core::likec::TreeNode *node);

    /**
     * Adds a child with given text and associated with given IR term.
     *
     * \param[in] text          Text.
     * \param[in] term          Pointer to an IR term.
     */
    InspectorItem *addChild(const QString &text, const core::ir::Term *term);

    /**
     * Adds a child with given text and associated with given IR statement.
     *
     * \param[in] text          Text.
     * \param[in] statement     Pointer to an IR statement.
     */
    InspectorItem *addChild(const QString &text, const core::ir::Statement *statement);

    /**
     * Adds a child with given text and associated with given instruction.
     *
     * \param[in] text          Text.
     * \param[in] instruction   Pointer to an instruction.
     */
    InspectorItem *addChild(const QString &text, const core::arch::Instruction *instruction);

    /**
     * Adds a child with given text and associated with given LikeC type.
     *
     * \param[in] text          Text.
     * \param[in] type          Pointer to a LikeC type.
     */
    InspectorItem *addChild(const QString &text, const core::likec::Type *type);

    /**
     * \return Children of the item.
     */
    const std::vector<std::unique_ptr<InspectorItem>> &children() const { return children_; }
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
