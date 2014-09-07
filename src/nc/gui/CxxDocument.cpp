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

#include "CxxDocument.h"

#include <algorithm>

#include <QPlainTextDocumentLayout>
#include <QTextStream>

#include <nc/core/Context.h>

#include <nc/core/ir/Statement.h>
#include <nc/core/ir/Term.h>

#include <nc/core/likec/Expression.h>
#include <nc/core/likec/FunctionDeclaration.h>
#include <nc/core/likec/FunctionIdentifier.h>
#include <nc/core/likec/LabelDeclaration.h>
#include <nc/core/likec/LabelIdentifier.h>
#include <nc/core/likec/LabelStatement.h>
#include <nc/core/likec/Statement.h>
#include <nc/core/likec/Tree.h>
#include <nc/core/likec/VariableDeclaration.h>
#include <nc/core/likec/VariableIdentifier.h>

#include "RangeTreeBuilder.h"

namespace nc { namespace gui {

namespace {

QString printTree(const core::likec::Tree &tree, RangeTree &rangeTree) {
    class Callback: public PrintCallback<const core::likec::TreeNode *> {
        RangeTreeBuilder builder_;
        const QString &out_;

    public:
        Callback(RangeTree &tree, const QString &out) : builder_(tree), out_(out) {}

        void onStartPrinting(const core::likec::TreeNode *node) override {
            builder_.onStart((void *)(node), out_.size());
        }
        void onEndPrinting(const core::likec::TreeNode *node) override {
            builder_.onEnd((void *)(node), out_.size());
        }
    };

    QString result;
    QTextStream stream(&result);
    Callback callback(rangeTree, result);

    tree.print(stream, &callback);

    return result;
}

} // anonymous namespace

CxxDocument::CxxDocument(QObject *parent, std::shared_ptr<const core::Context> context):
    QTextDocument(parent), context_(std::move(context))
{
    setDocumentLayout(new QPlainTextDocumentLayout(this));

    if (context_ && context_->tree()) {
        setPlainText(printTree(*context_->tree(), rangeTree_));
        if (rangeTree_.root()) {
            computeReverseMappings(rangeTree_.root());
        }
    }

    connect(this, SIGNAL(contentsChange(int, int, int)), this, SLOT(onContentsChange(int, int, int)));
}

void CxxDocument::computeReverseMappings(const RangeNode *rangeNode) {
    auto node = (const core::likec::TreeNode *)rangeNode->data();

    node2rangeNode_[node] = rangeNode;

    const core::ir::Statement *statement;
    const core::ir::Term *term;
    const core::arch::Instruction *instruction;

    getOrigin(node, statement, term, instruction);

    if (instruction) {
        instruction2rangeNodes_[instruction].push_back(rangeNode);
    }

    if (auto *expression = node->as<core::likec::Expression>()) {
        if (auto *identifier = expression->as<core::likec::FunctionIdentifier>()) {
            declaration2uses_[identifier->declaration()].push_back(identifier);
        } else if (auto *identifier = expression->as<core::likec::LabelIdentifier>()) {
            declaration2uses_[identifier->declaration()].push_back(identifier);
        } else if (auto *identifier = expression->as<core::likec::VariableIdentifier>()) {
            declaration2uses_[identifier->declaration()].push_back(identifier);
        }
    } else if (auto *statement = node->as<core::likec::Statement>()) {
        if (auto *labelStatement = statement->as<core::likec::LabelStatement>()) {
            declaration2uses_[labelStatement->label()].push_back(labelStatement);
            label2statement_[labelStatement->label()] = labelStatement;
        }
    }

    foreach (const auto &child, rangeNode->children()) {
        computeReverseMappings(&child);
    }
}

const core::likec::TreeNode *CxxDocument::getLeafAt(int position) const {
    if (auto rangeNode = rangeTree_.getLeafAt(position)) {
        return (const core::likec::TreeNode *)rangeNode->data();
    }
    return NULL;
}

std::vector<const core::likec::TreeNode *> CxxDocument::getNodesIn(const Range<int> &range) const {
    auto rangeNodes = rangeTree_.getNodesIn(range);

    std::vector<const core::likec::TreeNode *> result;
    result.reserve(result.size());

    foreach (auto rangeNode, rangeNodes) {
        result.push_back((const core::likec::TreeNode *)rangeNode->data());
    }

    return result;
}

Range<int> CxxDocument::getRange(const core::likec::TreeNode *node) const {
    assert(node != NULL);
    if (auto rangeNode = nc::find(node2rangeNode_, node)) {
        return rangeTree_.getRange(rangeNode);
    }
    return Range<int>();
}

void CxxDocument::getRanges(const core::arch::Instruction *instruction, std::vector<Range<int>> &result) const {
    assert(instruction != NULL);

    const auto &rangeNodes = nc::find(instruction2rangeNodes_, instruction);

    result.reserve(result.size() + rangeNodes.size());

    foreach (auto rangeNode, rangeNodes) {
        if (auto range = rangeTree_.getRange(rangeNode)) {
            result.push_back(range);
        }
    }
}

void CxxDocument::getOrigin(const core::likec::TreeNode *node, const core::ir::Statement *&statement,
                            const core::ir::Term *&term, const core::arch::Instruction *&instruction)
{
    statement = NULL;
    term = NULL;
    instruction = NULL;

    if (const core::likec::Statement *stmt = node->as<core::likec::Statement>()) {
        statement = stmt->statement();
    } else if (const core::likec::Expression *expr = node->as<core::likec::Expression>()) {
        term = expr->term();
        if (term) {
            statement = term->statement();
        }
    }

    if (statement) {
        instruction = statement->instruction();
    }
}

void CxxDocument::onContentsChange(int position, int charsRemoved, int charsAdded) {
    if (charsRemoved > 0) {
        rangeTree_.handleRemoval(position, charsRemoved);
    }
    if (charsAdded > 0) {
        rangeTree_.handleInsertion(position, charsAdded);
    }
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
