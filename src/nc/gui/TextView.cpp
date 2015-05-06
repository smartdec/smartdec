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

#include "TextView.h"

#include <algorithm>

#include <QAction>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTextStream>
#include <QVBoxLayout>

#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include "GotoLineWidget.h"
#include "SearchWidget.h"
#include "TextEditSearcher.h"

namespace nc { namespace gui {

TextView::TextView(const QString &title, QWidget *parent):
    QDockWidget(title, parent)
{
    textEdit_ = new QPlainTextEdit(this);
    textEdit_->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
    textEdit_->setLineWrapMode(QPlainTextEdit::NoWrap);
    textEdit_->installEventFilter(this);
    textEdit_->viewport()->installEventFilter(this);
    textEdit_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(textEdit_, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));
    connect(this, SIGNAL(contextMenuCreated(QMenu *)), this, SLOT(populateContextMenu(QMenu *)));

    auto searchWidget = new SearchWidget(std::make_unique<TextEditSearcher>(textEdit_), this);
    searchWidget->hide();

    GotoLineWidget *gotoLineWidget = new GotoLineWidget(textEdit_, this);
    gotoLineWidget->hide();

    QWidget *widget = new QWidget(this);

    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(QMargins());
    layout->addWidget(textEdit_);
    layout->addWidget(searchWidget);
    layout->addWidget(gotoLineWidget);

    setWidget(widget);

    saveAsAction_ = new QAction(tr("Save As..."), this);
    saveAsAction_->setShortcut(QKeySequence::Save);
    saveAsAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(saveAsAction_);

    connect(saveAsAction_, SIGNAL(triggered()), this, SLOT(saveAs()));

    QList<QKeySequence> searchShortcuts;
    searchShortcuts.append(QKeySequence::Find);
    searchShortcuts.append(tr("/"));

    openSearchAction_ = new QAction(tr("Find..."), this);
    openSearchAction_->setShortcuts(searchShortcuts);
    openSearchAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(openSearchAction_);

    connect(openSearchAction_, SIGNAL(triggered()), searchWidget, SLOT(activate()));

    findNextAction_ = new QAction(tr("Find Next"), this);
    findNextAction_->setShortcut(QKeySequence::FindNext);
    findNextAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(findNextAction_);

    connect(findNextAction_, SIGNAL(triggered()), searchWidget, SLOT(findNext()));

    findPreviousAction_ = new QAction(tr("Find Previous"), this);
    findPreviousAction_->setShortcut(QKeySequence::FindPrevious);
    findPreviousAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(findPreviousAction_);

    connect(findPreviousAction_, SIGNAL(triggered()), searchWidget, SLOT(findPrevious()));

    QList<QKeySequence> gotoLineShortcuts;
    gotoLineShortcuts.append(Qt::CTRL + Qt::Key_L);
    gotoLineShortcuts.append(Qt::CTRL + Qt::Key_G);

    openGotoLineAction_ = new QAction(tr("Go to Line..."), this);
    openGotoLineAction_->setShortcuts(gotoLineShortcuts);
    openGotoLineAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(openGotoLineAction_);

    connect(openGotoLineAction_, SIGNAL(triggered()), gotoLineWidget, SLOT(activate()));

    QAction *closeEverythingAction = new QAction(this);
    closeEverythingAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    closeEverythingAction->setShortcut(Qt::Key_Escape);
    addAction(closeEverythingAction);

    connect(closeEverythingAction, SIGNAL(triggered()), searchWidget, SLOT(deactivate()));
    connect(closeEverythingAction, SIGNAL(triggered()), gotoLineWidget, SLOT(deactivate()));
    connect(closeEverythingAction, SIGNAL(triggered()), textEdit(), SLOT(setFocus()));
}

void TextView::updatePositionStatus() {
    QTextCursor cursor = textEdit()->textCursor();
    int line = cursor.blockNumber() + 1;
    int column = cursor.columnNumber() + 1;

    Q_EMIT status(tr("Line %1, Column %2").arg(line).arg(column));
}

void TextView::showContextMenu(const QPoint &pos) {
    std::unique_ptr<QMenu> menu(textEdit()->createStandardContextMenu());

    Q_EMIT contextMenuCreated(menu.get());

    if (!menu->isEmpty()) {
        menu->exec(textEdit()->mapToGlobal(pos));
    }
}

void TextView::populateContextMenu(QMenu *menu) {
    menu->addSeparator();
    menu->addAction(saveAsAction_);
    menu->addSeparator();
    menu->addAction(openSearchAction_);
    menu->addAction(findNextAction_);
    menu->addAction(findPreviousAction_);
    menu->addSeparator();
    menu->addAction(openGotoLineAction_);
}

void TextView::highlight(const std::vector<TextRange> &ranges, bool ensureVisible) {
    QList<QTextEdit::ExtraSelection> selections;

    foreach (const TextRange &range, ranges) {
        QTextEdit::ExtraSelection selection;
        selection.cursor = textEdit()->textCursor();
        selection.cursor.setPosition(range.start());
        selection.cursor.setPosition(range.end(), QTextCursor::KeepAnchor);
        selection.format.setBackground(QBrush(QColor(Qt::lightGray)));
        selections.append(selection);
    }

    if (!ranges.empty()) {
        std::vector<TextRange> sortedRanges(ranges.begin(), ranges.end());
        std::sort(sortedRanges.begin(), sortedRanges.end());

        if (ensureVisible) {
            std::vector<TextRange> difference;

            set_difference(
                sortedRanges.begin(), sortedRanges.end(),
                highlighting_.begin(), highlighting_.end(),
                std::back_inserter(difference));

            if (difference.empty()) {
                set_difference(
                    highlighting_.begin(), highlighting_.end(),
                    sortedRanges.begin(), sortedRanges.end(),
                    std::back_inserter(difference));
            }

            if (!difference.empty()) {
                textEdit()->blockSignals(true);
                moveCursor(difference.front().end(), true);
                moveCursor(difference.front().start(), true);
                textEdit()->blockSignals(false);
            }
        }

        highlighting_.swap(sortedRanges);
    } else {
        highlighting_.clear();
    }

    textEdit()->setExtraSelections(selections);
}

void TextView::moveCursor(int position, bool ensureVisible) {
    QTextCursor cursor = textEdit()->textCursor();
    cursor.setPosition(position);
    textEdit()->setTextCursor(cursor);
    if (ensureVisible) {
        textEdit()->ensureCursorVisible();
    }
}

void TextView::saveAs() {
    QString filename = QFileDialog::getSaveFileName(this, tr("Where should I save the text?"));
    if (!filename.isEmpty()) {
        QFile file(filename);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QMessageBox::critical(this, tr("Error"), tr("File %1 could not be opened for writing.").arg(filename));
        }

        QTextStream out(&file);
        out << textEdit()->toPlainText();
    }
}

void TextView::zoomIn(int delta) {
    QFont font = textEdit()->document()->defaultFont();
    font.setPointSize(std::max(font.pointSize() + delta, 1));
    textEdit()->document()->setDefaultFont(font);
}

void TextView::zoomOut(int delta) {
    zoomIn(-delta);
}

bool TextView::eventFilter(QObject *watched, QEvent *event) {
    if (watched == textEdit()) {
        if (event->type() == QEvent::FocusIn) {
            connect(textEdit_, SIGNAL(cursorPositionChanged()), this, SLOT(updatePositionStatus()));
            updatePositionStatus();
        } else if (event->type() == QEvent::FocusOut) {
            disconnect(textEdit_, SIGNAL(cursorPositionChanged()), this, SLOT(updatePositionStatus()));
        }
    }
    if (watched == textEdit() || watched == textEdit()->viewport()) {
        if (event->type() == QEvent::Wheel) {
            auto wheelEvent = static_cast<QWheelEvent *>(event);

            if (wheelEvent->orientation() == Qt::Vertical && wheelEvent->modifiers() & Qt::ControlModifier) {
                if (wheelEvent->delta() > 0) {
                    zoomIn(1 + wheelEvent->delta() / 360);
                } else {
                    zoomOut(1 - wheelEvent->delta() / 360);
                }
                return true;
            }
        }
    }
    return QDockWidget::eventFilter(watched, event);
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
