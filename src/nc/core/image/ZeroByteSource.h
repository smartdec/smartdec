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
    ByteSize size_;

public:
    /**
     * Default constructor.
     */
    ZeroByteSource() {}

    /**
     * Constructor.
     *
     * \param size Size of the zero memory region.
     */
    ZeroByteSource(ByteSize size): size_(size) {}

    virtual ByteSize readBytes(ByteAddr addr, void *buf, ByteSize size) const override;
};

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
