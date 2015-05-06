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

#include <cassert>

#include <QString>

#include <boost/optional.hpp>

#include <nc/common/Types.h>

#include "ByteSource.h"

namespace nc {
namespace core {

class Module;

namespace image {

class Reader: public ByteSource {
    const Module *module_; ///< Module.

    public:

    /**
     * Constructor.
     *
     * \param module Valid pointer to a module.
     */
    Reader(const Module *module):
        module_(module)
    {
        assert(module != NULL);
    }

    /**
     * \return Valid pointer to the module.
     */
    const Module *module() const { return module_; }

    /**
     * \param[in] addr                 Linear address.
     * \tparam T                       Type of value to read.
     *
     * \return                         Value of type T at given linear address, 
     *                                 or boost::none if reading has failed.
     */
    template<class T> boost::optional<T> readType(ByteAddr addr) const {
        // TODO: architecture, byte order.

        T result;
        if (readBytes(addr, &result, sizeof(result)) == sizeof(T)) {
            return result;
        } else {
            return boost::none;
        }
    }

    /**
     * Reads an ASCIIZ string.
     *
     * \param[in] addr                 Linear address of the first byte to read.
     * \param[in] maxSize              Max number of bytes to read.
     *
     * \return                         ASCIIZ string without zero char terminator on success, NULL string on failure.
     */
    QString readAsciizString(ByteAddr addr, ByteSize maxSize) const;

    /**
     * Reads a pointer of the size equal to the bitness of the module's architecture.
     *
     * \param[in] addr                 Linear address of the pointer.
     * \return                         Pointer value, or boost::none in case of a failure.
     */
    boost::optional<ByteAddr> readPointer(ByteAddr addr) const;

    /**
     * Reads a pointer of the given size.
     *
     * \param[in] addr                 Linear address of the pointer.
     * \param[in] size                 Pointer size.
     * \return                         Pointer value, or boost::none in case of a failure.
     */
    boost::optional<ByteAddr> readPointer(ByteAddr addr, ByteSize size) const;
};

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
