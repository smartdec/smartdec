/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory> /* std::shared_ptr */

#include <QAbstractItemModel>

namespace nc {

namespace core {
    namespace image {
        class Symbol;
        class Image;
    }
}

namespace gui {

/**
 * Item model for SymbolsView.
 */
class SymbolsModel: public QAbstractItemModel {
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * \param parent  Pointer to the parent object. Can be NULL.
     */
    SymbolsModel(QObject *parent = NULL);

    /**
     * Sets the associated executable image.
     *
     * \param image Pointer to the executable image. Can be NULL.
     */
    void setImage(const std::shared_ptr<const core::image::Image> &image = std::shared_ptr<const core::image::Image>());

    /**
     * \return Pointer to the associated executable image. Can be NULL.
     */
    const std::shared_ptr<const core::image::Image> &image() const { return image_; }

    /**
     * \param index Model index.
     *
     * \return Pointer to the section associated with the index. Can be NULL.
     */
    const core::image::Symbol *getSymbol(const QModelIndex &index) const;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QModelIndex parent(const QModelIndex &index) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    /** Associated executable image. */
    std::shared_ptr<const core::image::Image> image_;

    /**
     * Updates the contents of the model.
     */
    void updateContents();
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
