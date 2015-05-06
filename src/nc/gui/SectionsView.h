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

#include "TreeView.h"

QT_BEGIN_NAMESPACE
class QTreeView;
QT_END_NAMESPACE

namespace nc {

namespace core {
    namespace image {
        class Section;
    }
}

namespace gui {

class SectionsModel;

class SectionsView: public TreeView {
    Q_OBJECT

    /** The model being shown. */
    SectionsModel *model_;

    public:

    /**
     * Constructor.
     *
     * \param parent Pointer to the parent widget. Can be NULL.
     */
    SectionsView(QWidget *parent = 0);

    /**
     * \return Pointer to the model being viewed. Can be NULL.
     */
    SectionsModel *model() const { return model_; }

    /**
     * Sets the model being viewed.
     *
     * \param model Pointer to the new model. Can be NULL.
     */
    void setModel(SectionsModel *model);

    /**
     * \returns Pointer to the currently selected section. Can be NULL.
     */
    const core::image::Section *selectedSection() const;
};

} // namespace gui
} // namespace nc

/* vim:set et sts=4 sw=4: */
