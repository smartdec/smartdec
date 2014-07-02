/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "Relocations.h"

#include <nc/common/Range.h>

namespace nc {
namespace core {
namespace image {

void Relocations::add(std::unique_ptr<Relocation> relocation) {
    address2relocation_[relocation->address()] = relocation.get();
    relocations_.push_back(std::move(relocation));
}

const Relocation *Relocations::find(ByteAddr address) const {
    return nc::find(address2relocation_, address);
}

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
