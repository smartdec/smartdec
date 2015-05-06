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

#include <QTextCursor>

#include <boost/optional.hpp>

#include <nc/common/Types.h>

#include "TextView.h"

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
        class FunctionIdentifier;
        class LabelIdentifier;
        class TreeNode;
        class VariableIdentifier;
    }
}

namespace gui {

class CxxDocument;
class CppSyntaxHighlighter;

/**
 * Dock widget for showing C++ code.
 */
class CxxView: public TextView {
    Q_OBJECT

    /** Pointer to the C++ document being viewed. */
    CxxDocument *document_;

    /** LikeC tree nodes currently selected in text. */
    std::vector<const core::likec::TreeNode *> selectedNodes_;

    /** IR statements currently selected in text. */
    std::vector<const core::ir::Statement *> selectedStatements_;

    /** IR terms currently selected in text. */
    std::vector<const core::ir::Term *> selectedTerms_;

    /** Instructions currently selected in text. */
    std::vector<const core::arch::Instruction *> selectedInstructions_;

    /** Syntax highlighter for C++ code. */
    CppSyntaxHighlighter *highlighter_;

    public:

    /**
     * Constructor.
     *
     * \param[in] parent Pointer to the parent widget. Can be NULL.
     */
    CxxView(QWidget *parent = 0);

    /**
     * \return Pointer to the C++ document being viewed. Can be NULL.
     */
    CxxDocument *document() const { return document_; }

    /**
     * \return LikeC tree nodes currently selected in text.
     */
    const std::vector<const core::likec::TreeNode *> &selectedNodes() const { return selectedNodes_; }

    /**
     * \return IR statements currently selected in text.
     */
    const std::vector<const core::ir::Statement *> &selectedStatements() const { return selectedStatements_; }

    /**
     * \return IR terms currently selected in text.
     */
    const std::vector<const core::ir::Term *> &selectedTerms() const { return selectedTerms_; }

    /**
     * \return Instructions currently selected in text.
     */
    const std::vector<const core::arch::Instruction *> &selectedInstructions() const { return selectedInstructions_; }

    /**
     * \return Integer under cursor, if any, or boost::none otherwise.
     */
    boost::optional<ConstantValue> getSelectedInteger() const;

    /**
     * \return Pointer to the function identifier under cursor. Can be NULL.
     */
    const core::likec::FunctionIdentifier *getSelectedFunctionIdentifier() const;

    /**
     * \return Pointer to the variable identifier under cursor. Can be NULL.
     */
    const core::likec::VariableIdentifier *getSelectedVariableIdentifier() const;

    /**
     * \return Pointer to the label identifier under cursor. Can be NULL.
     */
    const core::likec::LabelIdentifier *getSelectedLabelIdentifier() const;

    public Q_SLOTS:

    /**
     * Sets the document being viewed.
     *
     * \param document Pointer to the new document. Can be NULL.
     */
    void setDocument(CxxDocument *document);

    /**
     * Highlights given LikeC tree nodes.
     *
     * \param nodes         Nodes to be highlighted.
     * \param ensureVisible Ensure that changes in highlighting are visible.
     */
    void highlightNodes(const std::vector<const core::likec::TreeNode *> &nodes, bool ensureVisible = true);

    /**
     * Highlights given instructions (i.e. C++ code produced from them).
     *
     * \param instructions  Instructions to be highlighted.
     * \param ensureVisible Ensure that changes in highlighting are visible.
     */
    void highlightInstructions(const std::vector<const core::arch::Instruction *> &instructions, bool ensureVisible = true);

    Q_SIGNALS:

    /**
     * Signal emitted when the set of currently selected LikeC tree nodes is changed.
     */
    void nodeSelectionChanged();

    /**
     * Signal emitted when the set of currently selected instructions is changed.
     */
    void instructionSelectionChanged();

    /**
     * Signal emitted when the set of currently selected IR statements is changed.
     */
    void statementSelectionChanged();

    /**
     * Signal emitted when the set of currently selected IR terms is changed.
     */
    void termSelectionChanged();

    private Q_SLOTS:

    /**
     * Updates information about current selections.
     */
    void updateSelection();

    /**
     * Highlights all references of the identifier under cursor.
     */
    void highlightReferences();

    /**
     * Goes to the declaration of the function under cursor.
     */
    void gotoFunctionDeclaration();

    /**
     * Goes to the declaration of the variable under cursor.
     */
    void gotoVariableDeclaration();

    /**
     * Goes to the label under cursor.
     */
    void gotoLabel();

    private:

    /**
     * Generates the tooltip text displaying the declaration of the function or the variable in a given position.
     *
     * \param position Position in the text.
     *
     * \return Generated tooltip text.
     */
    QString getDeclarationTooltip(int position);
    
    private Q_SLOTS:

    /**
     * Populates the context menu being created.
     *
     * \param menu Valid pointer to the menu being created.
     */
    void populateContextMenu(QMenu *menu);

    protected:
    
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
