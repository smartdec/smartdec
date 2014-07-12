/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "ZeroByteSource.h"

#include <cassert>
#include <cstring> /* memset */

#include <nc/common/CheckedCast.h>

namespace nc {
namespace core {
namespace image {

ByteSize ZeroByteSource::readBytes(ByteAddr addr, void *buf, ByteSize size) const {
    assert(size >= 0);

    if (addr < 0 || addr >= size_) {
        return 0;
    }
    if (addr + size > size_) {
        size = size_ - addr;
    }

    memset(buf, 0, checked_cast<size_t>(size));

    return size;
}

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
