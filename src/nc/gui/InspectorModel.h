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

#include <boost/unordered_map.hpp>

#include <QAbstractItemModel>

namespace nc {

namespace core {
    class Context;

    namespace likec {
        class TreeNode;
    }
}

namespace gui {

class InspectorItem;

/**
 * Item model for TreeInspector.
 */
class InspectorModel: public QAbstractItemModel {
    Q_OBJECT

    public:

    /**
     * Constructor.
     *
     * \param parent  Pointer to the parent object. Can be NULL.
     */
    InspectorModel(QObject *parent = NULL);

    /**
     * Destructor.
     */
    ~InspectorModel();

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

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QModelIndex parent(const QModelIndex &index) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    /**
     * \return Valid pointer to the root tree item.
     */
    InspectorItem *root() const { return root_.get(); }

    /**
     * \param[in] index Index.
     *
     * \return Valid pointer to the InspectorItem for the given index.
     */
    InspectorItem *getItem(const QModelIndex &index) const;

    /**
     * \param[in] item Valid pointer to an InspectorItem.
     *
     * \return Index for the given InspectorItem.
     */
    QModelIndex getIndex(const InspectorItem *item) const;

    /**
     * \param[in] node Pointer to a LikeC tree node.
     *
     * \return Pointer to this node's parent. Can be NULL.
     */
    const core::likec::TreeNode *getParent(const core::likec::TreeNode *node);

    /**
     * Computes children of given tree item.
     *
     * \param item Valid pointer to a tree item.
     */
    void expand(InspectorItem *item) const;

    private:

    /** Associated immutable context instance. */
    std::shared_ptr<const core::Context> context_;

    /** Root tree item. */
    std::unique_ptr<InspectorItem> root_;

    /** Mapping from LikeC nodes to their parents. */
    boost::unordered_map<const core::likec::TreeNode *, const core::likec::TreeNode *> node2parent_;

    /**
     * Computes mapping from LikeC nodes to their parents, if not done yet.
     */
    void computeParentRelation();

    /**
     * Updates the contents of the model.
     * Currently it effectively clears the model.
     */
    void updateContents();
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
