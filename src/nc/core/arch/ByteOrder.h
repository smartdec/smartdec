/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <QSysInfo>

namespace nc {
namespace core {
namespace arch {

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
        Current      = QSysInfo::ByteOrder     ///< Current byte order.
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

    private:

    /** Actual value. */
    Type value_;
};

} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
