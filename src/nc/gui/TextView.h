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

#include <QDockWidget>

#include <memory>

#include "TextRange.h"

QT_BEGIN_NAMESPACE
class QMenu;
class QPlainTextEdit;
QT_END_NAMESPACE

namespace nc { namespace gui {

/**
 * Dock widget for showing text.
 */
class TextView: public QDockWidget {
    Q_OBJECT

    /** QPlainTextEdit instance used for showing text. */
    QPlainTextEdit *textEdit_;

    /** Current highlighting. */
    std::vector<TextRange> highlighting_;

    /** Action for saving the text being shown. */
    QAction *saveAsAction_;

    /** Action for showing the text search widget. */
    QAction *openSearchAction_;

    /** Action for finding a next occurrence of a string. */
    QAction *findNextAction_;

    /** Action for finding a previous occurrence of a string. */
    QAction *findPreviousAction_;

    /** Action for showing the go to line widget. */
    QAction *openGotoLineAction_;

    public:

    /**
     * Class constructor.
     *
     * \param[in] title     Title of the widget.
     * \param[in] parent    Parent widget.
     */
    TextView(const QString &title, QWidget *parent = 0);

    /**
     * \return Valid pointer to the text widget.
     */
    QPlainTextEdit *textEdit() const { return textEdit_; }

    public Q_SLOTS:

    /**
     * Highlights text ranges.
     *
     * \param[in] ranges        Vector of ranges to be highlighted.
     * \param[in] ensureVisible Ensure that changes in highlighting are visible.
     */
    void highlight(const std::vector<TextRange> &ranges, bool ensureVisible = true);

    /**
     * Moves cursor to given position.
     *
     * \param[in] position      Position.
     * \param[in] ensureVisible Ensure that the cursor is visible.
     */
    void moveCursor(int position, bool ensureVisible = true);

    public Q_SLOTS:

    /**
     * Lets the user choose a file name and saves the text into this file.
     */
    void saveAs();

    /**
     * Makes the default font of text edit delta points larger.
     *
     * \param delta Points size difference.
     */
    void zoomIn(int delta = 1);

    /**
     * Makes the default font of text edit delta points smaller.
     *
     * \param delta Points size difference.
     */
    void zoomOut(int delta = 1);

    Q_SIGNALS:

    /**
     * This signal is emitted when a context menu is being created.
     * Intercept this signal to populate the menu with some actions.
     *
     * \param menu Valid pointer to the context menu being created.
     */
    void contextMenuCreated(QMenu *menu);

    /**
     * This signal is emitted when the widget has a message to be displayed in status bar.
     *
     * \param message Message to be displayed.
     */
    void status(const QString &message = QString());

    private Q_SLOTS:

    /**
     * Generates a status signal reporting current position.
     */
    void updatePositionStatus();

    /**
     * Shows a context menu for the child QTextEdit widget.
     *
     * \param pos Position at which the menu is requested.
     */
    void showContextMenu(const QPoint &pos);

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
