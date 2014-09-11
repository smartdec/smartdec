/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "CxxView.h"

#include <QInputDialog>
#include <QMenu>
#include <QPlainTextEdit>

#include <nc/common/Conversions.h>
#include <nc/core/likec/Expression.h>
#include <nc/core/likec/FunctionDefinition.h>
#include <nc/core/likec/LabelDeclaration.h>
#include <nc/core/likec/LabelStatement.h>
#include <nc/core/likec/VariableDeclaration.h>

#include "CppSyntaxHighlighter.h"
#include "CxxDocument.h"

namespace nc { namespace gui {

CxxView::CxxView(QWidget *parent):
    TextView(tr("C++"), parent),
    document_(NULL)
{
    highlighter_ = new CppSyntaxHighlighter(this);

    textEdit()->setTextInteractionFlags(Qt::TextEditorInteraction);

    connect(textEdit(), SIGNAL(cursorPositionChanged()), this, SLOT(updateSelection()));
    connect(textEdit(), SIGNAL(selectionChanged()), this, SLOT(updateSelection()));
    connect(textEdit(), SIGNAL(textChanged()), this, SLOT(updateSelection()));

    connect(textEdit(), SIGNAL(textChanged()), this, SLOT(highlightReferences()));
    connect(this, SIGNAL(nodeSelectionChanged()), this, SLOT(highlightReferences()));

    textEdit()->viewport()->installEventFilter(this);

    connect(this, SIGNAL(contextMenuCreated(QMenu *)), this, SLOT(populateContextMenu(QMenu *)));
}

void CxxView::setDocument(CxxDocument *document) {
    if (document == document_) {
        return;
    }

    /* QTextEdit crashes when extra selections get out of range. */
    textEdit()->setExtraSelections(QList<QTextEdit::ExtraSelection>());

    /* No signals until we are in a consistent state. */
    textEdit()->blockSignals(true);

    TextView::setDocument(document);
    highlighter_->setDocument(document);
    document_ = document;

    textEdit()->blockSignals(false);

    updateSelection();
}

void CxxView::updateSelection() {
    std::vector<const core::likec::TreeNode *> nodes;
    std::vector<const core::ir::Statement *> statements;
    std::vector<const core::ir::Term *> terms;
    std::vector<const core::arch::Instruction *> instructions;

    if (document()) {
        QTextCursor cursor = textEdit()->textCursor();
        if (cursor.hasSelection()) {
            Range<int> range(cursor.selectionStart(), cursor.selectionEnd());
            nodes = document()->getNodesIn(range);
        } else {
            if (auto node = document()->getLeafAt(cursor.position())) {
                nodes.push_back(node);
            }
        }

        foreach (const core::likec::TreeNode *node, nodes) {
            const core::ir::Statement *statement;
            const core::ir::Term *term;
            const core::arch::Instruction *instruction;

            document()->getOrigin(node, statement, term, instruction);

            if (statement) {
                statements.push_back(statement);
            }
            if (term) {
                terms.push_back(term);
            }
            if (instruction) {
                instructions.push_back(instruction);
            }
        }
    }

    if (selectedNodes_ != nodes) {
        selectedNodes_.swap(nodes);
        Q_EMIT nodeSelectionChanged();
    }
    if (selectedStatements_ != statements) {
        selectedStatements_.swap(statements);
        Q_EMIT statementSelectionChanged();
    }
    if (selectedTerms_ != terms) {
        selectedTerms_.swap(terms);
        Q_EMIT termSelectionChanged();
    }
    if (selectedInstructions_ != instructions) {
        selectedInstructions_.swap(instructions);
        Q_EMIT instructionSelectionChanged();
    }
}

boost::optional<ConstantValue> CxxView::getIntegerUnderCursor() const {
    auto cursor = textEdit()->textCursor();
    cursor.movePosition(QTextCursor::StartOfWord);
    cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);

    ConstantValue value;
    if (stringToInt(cursor.selectedText(), &value)) {
        return value;
    }
    return boost::none;
}

const core::likec::TreeNode *CxxView::getNodeUnderCursor() const {
    return document()->getLeafAt(textEdit()->textCursor().position());
}

const core::likec::Declaration *CxxView::getDeclarationOfIdentifierUnderCursor() const {
    if (auto node = getNodeUnderCursor()) {
        if (auto declaration = node->as<core::likec::Declaration>()) {
            return declaration;
        } else {
            return document()->getDeclaration(node);
        }
    }
    return NULL;
}

void CxxView::gotoDeclaration() {
    if (auto declaration = getDeclarationOfIdentifierUnderCursor()) {
        if (auto range = document()->getRange(declaration)) {
            moveCursor(range.start());
        }
    }
}

void CxxView::gotoLabel() {
    if (auto declaration = getDeclarationOfIdentifierUnderCursor()) {
        if (auto labelDeclaration = declaration->as<core::likec::LabelDeclaration>()) {
            if (auto statement = document()->getLabelStatement(labelDeclaration)) {
                if (auto range = document()->getRange(statement)) {
                    moveCursor(range.start());
                }
            }
        }
    }
}

void CxxView::highlightReferences() {
    if (!document()) {
        return;
    }

    std::vector<const core::likec::TreeNode *> nodes;

    if (auto declaration = getDeclarationOfIdentifierUnderCursor()) {
        const auto &uses = document()->getUses(declaration);
        nodes.insert(nodes.end(), uses.begin(), uses.end());
        if (declaration->is<core::likec::VariableDeclaration>()) {
            nodes.push_back(declaration);
        }
    }


    highlightNodes(nodes, false);
}

void CxxView::highlightNodes(const std::vector<const core::likec::TreeNode *> &nodes, bool ensureVisible) {
    if (!document()) {
        return;
    }

    std::vector<Range<int>> ranges;
    ranges.reserve(nodes.size());

    foreach (auto node, nodes) {
        ranges.push_back(document()->getRange(node));
    }

    highlight(std::move(ranges), ensureVisible);
}

void CxxView::highlightInstructions(const std::vector<const core::arch::Instruction *> &instructions, bool ensureVisible) {
    if (!document()) {
        return;
    }

    std::vector<Range<int>> ranges;

    foreach (const core::arch::Instruction *instruction, instructions) {
        document()->getRanges(instruction, ranges);
    }

    highlight(std::move(ranges), ensureVisible);
}

QString CxxView::getDeclarationTooltip(int position) const {
    const auto maxLineCount = 5;

    if (auto node = document()->getLeafAt(position)) {
        if (auto declaration = document()->getDeclaration(node)) {
            if (auto range = document()->getRange(declaration)) {
                auto text = document()->getText(range);
                int lineCount = 0;
                for (int i = 0; i < text.size(); ++i) {
                    if (text[i] == QChar::ParagraphSeparator) {
                        text[i] = '\n';
                        if (++lineCount == maxLineCount) {
                            text.truncate(i + 1);
                            text += tr("...");
                            break;
                        }
                    }
                }
                return text;
            }
        }
    }

    return QString();
}

void CxxView::rename() {
    if (auto declaration = getDeclarationOfIdentifierUnderCursor()) {
        QString oldName;
        if (auto functionDeclaration = declaration->as<core::likec::FunctionDeclaration>()) {
            oldName = document()->getText(document()->getRange(functionDeclaration->functionIdentifier()));
        } else if (auto functionDefinition = declaration->as<core::likec::FunctionDefinition>()) {
            oldName = document()->getText(document()->getRange(functionDefinition->functionIdentifier()));
        } else if (auto variableDeclaration = declaration->as<core::likec::VariableDeclaration>()) {
            oldName = document()->getText(document()->getRange(variableDeclaration->variableIdentifier()));
        } else {
            return;
        }

        auto newName = QInputDialog::getText(this, tr("Rename"), tr("New name:"), QLineEdit::Normal, oldName);
        if (!newName.isEmpty()) {
            document()->rename(declaration, newName);
        }
    }
}

void CxxView::populateContextMenu(QMenu *menu) {
    menu->addSeparator();

    if (auto declaration = getDeclarationOfIdentifierUnderCursor()) {
        if (declaration->is<core::likec::FunctionDeclaration>() || declaration->is<core::likec::FunctionDefinition>() ||
            declaration->is<core::likec::VariableDeclaration>()) {
            menu->addAction(tr("Go to Declaration"), this, SLOT(gotoDeclaration()));
        } else if (declaration->is<core::likec::LabelDeclaration>()) {
            menu->addAction(tr("Go to Label"), this, SLOT(gotoLabel()));
        }
        menu->addAction(tr("Rename..."), this, SLOT(rename()));
    }
}

bool CxxView::eventFilter(QObject *watched, QEvent *event) {
    if (watched == textEdit()->viewport()) {
        if (event->type() == QEvent::ToolTip) {
            QHelpEvent *ev = static_cast<QHelpEvent*>(event);

            textEdit()->setToolTip(getDeclarationTooltip(textEdit()->cursorForPosition(ev->pos()).position()));
        }
    }

    return TextView::eventFilter(watched, event);
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
