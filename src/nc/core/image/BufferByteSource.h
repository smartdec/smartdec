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

#include <QByteArray>

#include "ByteSource.h"

namespace nc {
namespace core {
namespace image {

/**
 * ByteSource reading the bytes from a QIODevice.
 */
class BufferByteSource: public ByteSource {
    QByteArray buffer_; ///< Buffer to read from.

    public:

    /**
     * Default constructor.
     */
    BufferByteSource() {}

    /**
     * Constructor.
     *
     * \param buffer Buffer to read from.
     */
    BufferByteSource(const QByteArray &buffer): buffer_(buffer) {}

    /**
     * Buffer to read from.
     */
    QByteArray &buffer() { return buffer_; }

    /**
     * Buffer to read from.
     */
    const QByteArray &buffer() const { return buffer_; }

    virtual ByteSize readBytes(ByteAddr addr, void *buf, ByteSize size) const override;
};

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
