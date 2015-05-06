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

#include <memory> /* std::shared_ptr */
#include <vector>

#include <boost/unordered_map.hpp>

#include <QTextDocument>

#include <nc/common/Range.h>

#include "RangeTracker.h"

namespace nc {

namespace core {
    class Context;

    namespace arch {
        class Instruction;
    }

    namespace ir {
        class Statement;
        class Term;
    }

    namespace likec {
        class Declaration;
        class TreeNode;
        class LabelDeclaration;
        class LabelStatement;
    }
}

namespace gui {

/**
 * Text document containing C++ listing.
 */
class CxxDocument: public QTextDocument {
    Q_OBJECT

    public:

    /**
     * Constructor.
     *
     * \param parent    Pointer to the parent object. Can be NULL.
     */
    CxxDocument(QObject *parent = NULL);

    /**
     * Sets the associated context instance.
     *
     * \param context Pointer to the context. Can be NULL.
     */
    void setContext(const std::shared_ptr<const core::Context> &context = std::shared_ptr<const core::Context>());

    /**
     * \return Pointer to the associated context instance. Can be NULL.
     */
    const std::shared_ptr<const core::Context> &context() const { return context_; }

    /**
     * \return Tracker of tree nodes' positions in the text.
     */
    const RangeTracker<const core::likec::TreeNode> &tracker() const { return tracker_; }

    /**
     * \param instruction Valid pointer to an instruction.
     *
     * \return Text ranges of code generated from the instruction.
     */
    const std::vector<TextRange> &getRanges(const core::arch::Instruction *instruction) {
        return nc::find(instruction2ranges_, instruction);
    }

    /**
     * \param declaration Valid pointer to a declaration tree node.
     *
     * \return All the tree nodes using this declaration.
     */
    const std::vector<const core::likec::TreeNode *> getUses(const core::likec::Declaration *declaration) {
        return nc::find(declaration2uses_, declaration);
    }

    /**
     * \param declaration Valid pointer to a label declaration node.
     *
     * \return Pointer to the matching label statement. Can be NULL.
     */
    const core::likec::LabelStatement *getLabelStatement(const core::likec::LabelDeclaration *declaration) {
        return nc::find(label2statement_, declaration);
    }

    /**
     * For a node, computes statement, term, and instruction, from which
     * this node has originated.
     *
     * \param[in]  node         Valid pointer to a tree node.
     * \param[out] statement    Original statement.
     * \param[out] term         Original term.
     * \param[out] instruction  Original instruction.
     */
    void getOrigin(const core::likec::TreeNode *node, const core::ir::Statement *&statement,
                   const core::ir::Term *&term, const core::arch::Instruction *&instruction);

    public Q_SLOTS:

    /**
     * Regenerates the listing.
     */
    void updateContents();

    private:

    /** Associated Context instance. */
    std::shared_ptr<const core::Context> context_;

    /** Tracker of nodes' positions in the text. */
    RangeTracker<const core::likec::TreeNode> tracker_;

    /** Mapping from an instruction to text ranges of code generated from this instruction. */
    boost::unordered_map<const core::arch::Instruction *, std::vector<TextRange>> instruction2ranges_;

    /** Mapping from a declaration to all the tree nodes using this declaration. */
    boost::unordered_map<const core::likec::Declaration *, std::vector<const core::likec::TreeNode *>> declaration2uses_;

    /** Mapping from a label declaration to matching LabelStatement. */
    boost::unordered_map<const core::likec::LabelDeclaration *, const core::likec::LabelStatement *> label2statement_;
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
