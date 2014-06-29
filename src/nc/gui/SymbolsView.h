/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include "TreeView.h"

namespace nc {
namespace gui {

class SymbolsModel;

class SymbolsView: public TreeView {
    Q_OBJECT

    /** The model being shown. */
    SymbolsModel *model_;

public:
    /**
     * Constructor.
     *
     * \param parent Pointer to the parent widget. Can be NULL.
     */
    SymbolsView(QWidget *parent = 0);

    /**
     * \return Pointer to the model being viewed. Can be NULL.
     */
    SymbolsModel *model() const { return model_; }

    /**
     * Sets the model being viewed.
     *
     * \param model Pointer to the new model. Can be NULL.
     */
    void setModel(SymbolsModel *model);
};

} // namespace gui
} // namespace nc

/* vim:set et sts=4 sw=4: */
