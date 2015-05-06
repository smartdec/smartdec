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

#include "Reader.h"

#include <cassert>

#include <nc/core/Module.h>
#include <nc/core/arch/Architecture.h>

namespace nc {
namespace core {
namespace image {

QString Reader::readAsciizString(ByteAddr addr, ByteSize maxSize) const {
    assert(maxSize >= 0);

    if (maxSize == 0) {
        return QString();
    }

    std::unique_ptr<char[]> buf(new char[maxSize + 1]);

    ByteSize size = readBytes(addr, buf.get(), maxSize);
    assert(size <= maxSize);

    if (size == 0) {
        return QString();
    } else {
        buf.get()[size] = '\0';
        return QString::fromLatin1(buf.get());
    }
}

boost::optional<ByteAddr> Reader::readPointer(ByteAddr addr) const {
    return readPointer(addr, module()->architecture()->bitness() / 8);
}

boost::optional<ByteAddr> Reader::readPointer(ByteAddr addr, ByteSize size) const {
    ByteAddr result = 0;
    size = std::min(static_cast<ByteSize>(sizeof(result)), size);

    // TODO: this works only if compiled on little-endian architecture.
    if (readBytes(addr, &result, size) == size) {
        return result;
    } else {
        return boost::none;
    }
}

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
