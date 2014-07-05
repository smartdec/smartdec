/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <QtGlobal>

#include <memory>
#include <vector>

#include "ByteSource.h"

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

namespace nc {
namespace core {
namespace image {

class Section;
class Symbols;

/**
 * Sections of an executable file.
 */
class Sections: public ByteSource {
    std::vector<std::unique_ptr<Section>> sections_; ///< The list of sections.
    std::unique_ptr<ByteSource> externalByteSource_; ///< External source of this image's bytes.

public:
    /**
     * Constructor.
     */
    Sections();

    /**
     * Destructor.
     */
    ~Sections();

    /**
     * Adds a new section.
     *
     * \param section Valid pointer to a section.
     */
    void add(std::unique_ptr<Section> section);

    /**
     * \return List of all sections.
     */
    const std::vector<Section *> &all() {
        return reinterpret_cast<const std::vector<Section *> &>(sections_);
    }

    /**
     * \return List of all sections.
     */
    const std::vector<const Section *> &all() const {
        return reinterpret_cast<const std::vector<const Section *> &>(sections_);
    }

    /**
     * \param[in] addr Linear address.
     *
     * \return A valid pointer to allocated section containing given virtual address
     *         or NULL if there is no such section.
     */
    const Section *getSectionContainingAddress(ByteAddr addr) const;
    
    /**
     * \param[in] name Section name.
     * 
     * \return Section with the given name, or NULL if there is no such section.
     */
    const Section *getSectionByName(const QString &name) const;

    /**
     * \return Pointer to the external byte source. Can be NULL.
     */
    ByteSource *externalByteSource() const { return externalByteSource_.get(); }

    /**
     * Sets the external byte source.
     *
     * \param byteSource Pointer to the new external byte source. Can be NULL.
     */
    void setExternalByteSource(std::unique_ptr<ByteSource> byteSource) { externalByteSource_ = std::move(byteSource); }

    virtual ByteSize readBytes(ByteAddr addr, void *buf, ByteSize size) const override;
};

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
