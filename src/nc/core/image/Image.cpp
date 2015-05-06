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

#include <nc/common/Foreach.h>
#include <nc/common/Warnings.h>

namespace nc {
namespace core {
namespace image {

Image::Image(const Module *module): Reader(module) {}

Image::~Image() {
    foreach (Section *section, sections_) {
        delete section;
    }
}

Section *Image::createSection(const QString &name, ByteAddr addr, ByteSize size) {
    std::unique_ptr<Section> result(new Section(module(), name, addr, size));
    sections_.push_back(result.get());
    return result.release();
}

const Section *Image::getSectionContainingAddress(ByteAddr addr) const {
    foreach (Section *section, sections_) {
        if (section->containsAddress(addr)) {
            return section;
        }
    }
    return NULL;
}

const Section *Image::getSectionByName(const QString &name) const {
    foreach (Section *section, sections_) {
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
