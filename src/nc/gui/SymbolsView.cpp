/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "SymbolsView.h"

#include <QMenu>
#include <QTreeView>

#include "SymbolsModel.h"

namespace nc {
namespace gui {

SymbolsView::SymbolsView(QWidget *parent):
    TreeView(tr("Symbols"), parent),
    model_(NULL)
{
    treeView()->setItemsExpandable(false);
    treeView()->setRootIsDecorated(false);
    treeView()->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView()->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeView()->setUniformRowHeights(true);
}

void SymbolsView::setModel(SymbolsModel *model) {
    if (model != model_) {
        model_ = model;
        treeView()->setModel(model);
    }
}

const core::image::Symbol *SymbolsView::selectedSymbol() const {
    return model_->getSymbol(treeView()->currentIndex());
}

} // namespace gui
} // namespace nc

/* vim:set et sts=4 sw=4: */
