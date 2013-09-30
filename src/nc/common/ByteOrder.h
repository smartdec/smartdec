/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <algorithm>

#include <QSysInfo>

#include <nc/common/Types.h>

namespace nc {

/**
 * Byte order of an architecture.
 */
class ByteOrder {
    public:

    /**
     * Enum with the byte orders.
     */
    enum Type {
        Unknown      = -1,                     ///< Unknown/undefined.
        BigEndian    = QSysInfo::BigEndian,    ///< Highest byte first.
        LittleEndian = QSysInfo::LittleEndian, ///< Lowest byte first.
        Current      = QSysInfo::ByteOrder     ///< Host byte order.
    };

    /**
     * Constructor from the enum.
     *
     * \param value Value of the byte order.
     */
    ByteOrder(Type value): value_(value) {}

    /**
     * Implicit cast operator to enum.
     *
     * \return Byte order value contained in the object.
     */
    operator Type() const { return value_; }

    /**
     * Converts a sequence of bytes representing an integer value in 'from'
     * byte order to a sequence of bytes representing this value in 'to'
     * byte order.
     *
     * \param[in,out] buf   Valid pointer to the sequence of bytes.
     * \param[in]     size  Size of the byte sequence.
     * \param[in]     from  Valid source byte order.
     * \param[in]     to    Valid destination byte order.
     */
    static void convert(void *buf, ByteSize size, ByteOrder from, ByteOrder to) {
        assert(from != Unknown);
        assert(to != Unknown);

        if (from != to) {
            std::reverse(static_cast<char *>(buf), static_cast<char *>(buf) + size);
        }
    }

    private:

    /** Actual value. */
    Type value_;
};

} // namespace nc

/* vim:set et sts=4 sw=4: */
