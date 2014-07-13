/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include "ByteSource.h"

namespace nc {
namespace core {
namespace image {

/**
 * ByteSource reading only zeroes.
 */
class ZeroByteSource: public ByteSource {
    ByteAddr address_; ///< Virtual address of the first byte.
    ByteSize size_; ///< Size of the zero zone.

public:
    /**
     * Constructor.
     *
     * \param address Virtual address of the buffer's beginning.
     * \param size Size of the zero memory region.
     */
    ZeroByteSource(ByteAddr address, ByteSize size): address_(address), size_(size) {}

    virtual ByteSize readBytes(ByteAddr addr, void *buf, ByteSize size) const override;
};

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
