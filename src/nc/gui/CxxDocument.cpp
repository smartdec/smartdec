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

#include <QBuffer>
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

#include "RangePrintCallback.h"

namespace nc { namespace gui {

CxxDocument::CxxDocument(QObject *parent):
    QTextDocument(parent)
{
    setDocumentLayout(new QPlainTextDocumentLayout(this));
}

void CxxDocument::setContext(const std::shared_ptr<const core::Context> &context) {
    if (context != context_) {
        context_ = context;
        updateContents();
    }
}

void CxxDocument::updateContents() {
    tracker_.clear();
    instruction2ranges_.clear();

    if (!context()) {
        clear();
        return;
    }

    class Callback: public RangePrintCallback<const core::likec::TreeNode> {
        CxxDocument *document_;

        public:

        Callback(QTextStream &stream, CxxDocument *document):
            RangePrintCallback(stream), document_(document)
        {}

        void onRange(const core::likec::TreeNode *node, const TextRange &range) override {
            document_->tracker_.addRange(node, range);

            const core::ir::Statement *statement;
            const core::ir::Term *term;
            const core::arch::Instruction *instruction;

            document_->getOrigin(node, statement, term, instruction);

            if (instruction) {
                document_->instruction2ranges_[instruction].push_back(range);
            }

            if (auto *expression = node->as<core::likec::Expression>()) {
                if (auto *identifier = expression->as<core::likec::FunctionIdentifier>()) {
                    document_->declaration2uses_[identifier->declaration()].push_back(identifier);
                } else if (auto *identifier = expression->as<core::likec::LabelIdentifier>()) {
                    document_->declaration2uses_[identifier->declaration()].push_back(identifier);
                } else if (auto *identifier = expression->as<core::likec::VariableIdentifier>()) {
                    document_->declaration2uses_[identifier->declaration()].push_back(identifier);
                }
            } else if (auto *statement = node->as<core::likec::Statement>()) {
                if (auto *labelStatement = statement->as<core::likec::LabelStatement>()) {
                    document_->declaration2uses_[labelStatement->label()].push_back(labelStatement);
                    document_->label2statement_[labelStatement->label()] = labelStatement;
                }
            }
        };
    };

    QString text;
    QTextStream stream(&text);

    if (context()->tree()) {
        Callback callback(stream, this);
        context()->tree()->print(stream, &callback);
    }

    setPlainText(text);
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

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
