/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

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
 * Binary file image.
 */
class Image: public ByteSource {
    std::vector<std::unique_ptr<Section>> sections_; ///< Sections of the executable file.
    std::unique_ptr<ByteSource> externalByteSource_; ///< External source of this image's bytes.
    std::unique_ptr<Symbols> symbols_; ///< Symbols of the image.

public:
    /**
     * Constructor.
     */
    Image();

    /**
     * Destructor.
     */
    ~Image();

    /**
     * Adds a new section.
     *
     * \param section Valid pointer to a section.
     */
    void addSection(std::unique_ptr<Section> section);

    /**
     * \return Sections of the executable file.
     */
    const std::vector<Section *> &sections() const { return reinterpret_cast<const std::vector<Section *> &>(sections_); }

    /**
     * \param[in] addr  Linear address.
     *
     * \return Section containing given virtual address, or NULL if there is no such section.
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

    /**
     * \return Symbols of the image.
     */
    Symbols &symbols() { return *symbols_; }

    /**
     * \return Symbols of the image.
     */
    const Symbols &symbols() const { return *symbols_; }
};

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
