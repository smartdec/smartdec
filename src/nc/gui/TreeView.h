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

QT_BEGIN_NAMESPACE
class QMenu;
class QTreeView;
QT_END_NAMESPACE

namespace nc { namespace gui {

/**
 * Dock widget for showing trees or lists.
 */
class TreeView: public QDockWidget {
    Q_OBJECT

    /** Tree widget. */
    QTreeView *treeView_;

    /** Action for copying the currently selected text. */
    QAction *copyAction_;

    /** Action for showing the text search widget. */
    QAction *openSearchAction_;

    /** Action for finding a next occurrence of a string. */
    QAction *findNextAction_;

    /** Action for finding a previous occurrence of a string. */
    QAction *findPreviousAction_;

    public:

    /**
     * Class constructor.
     *
     * \param[in] title     Title of the widget.
     * \param[in] parent    Parent widget.
     */
    TreeView(const QString &title, QWidget *parent = NULL);

    /**
     * \return Valid pointer to the tree widget.
     */
    QTreeView *treeView() const { return treeView_; }

    public Q_SLOTS:

    /**
     * Copies currently selected text in the clipboard.
     */
    void copy();

    Q_SIGNALS:

    /**
     * This signal is emitted when a context menu is being created.
     * Intercept this signal to populate the menu with some actions.
     *
     * \param menu Valid pointer to the context menu being created.
     */
    void contextMenuCreated(QMenu *menu);

    private Q_SLOTS:

    /**
     * Shows a context menu for the child tree widget.
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
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
