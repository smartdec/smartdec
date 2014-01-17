/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "Sections.h"

#include <cassert>

#include <nc/common/Foreach.h>

#include "Section.h"
#include "Symbols.h"

namespace nc {
namespace core {
namespace image {

Sections::Sections() {}

Sections::~Sections() {}

void Sections::add(std::unique_ptr<Section> section) {
    assert(section != NULL);
    section->setSections(this);
    sections_.push_back(std::move(section));
}

const Section *Sections::getSectionContainingAddress(ByteAddr addr) const {
    foreach (auto section, all()) {
        if (section->containsAddress(addr)) {
            return section;
        }
    }
    return NULL;
}

const Section *Sections::getSectionByName(const QString &name) const {
    foreach (auto section, all()) {
        if (section->name() == name) {
            return section;
        }
    }
    return NULL;
}

ByteSize Sections::readBytes(ByteAddr addr, void *buf, ByteSize size) const {
    if (externalByteSource()) {
        return externalByteSource()->readBytes(addr, buf, size);
    } else if (const Section *section = getSectionContainingAddress(addr)) {
        return section->readBytes(addr, buf, size);
    } else {
        return 0;
    }
}

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
