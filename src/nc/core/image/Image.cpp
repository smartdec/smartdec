/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "Image.h"

#include <cassert>

#include <nc/common/Foreach.h>

#include "Section.h"
#include "Symbols.h"

namespace nc {
namespace core {
namespace image {

Image::Image():
    symbols_(new Symbols())
{}

Image::~Image() {}

void Image::addSection(std::unique_ptr<Section> section) {
    assert(section != NULL);
    section->setImage(this);
    sections_.push_back(std::move(section));
}

const Section *Image::getSectionContainingAddress(ByteAddr addr) const {
    foreach (auto section, sections()) {
        if (section->containsAddress(addr)) {
            return section;
        }
    }
    return NULL;
}

const Section *Image::getSectionByName(const QString &name) const {
    foreach (auto section, sections()) {
        if (section->name() == name) {
            return section;
        }
    }
    return NULL;
}

ByteSize Image::readBytes(ByteAddr addr, void *buf, ByteSize size) const {
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
