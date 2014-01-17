/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/common/Types.h>

#include <QString>

namespace nc {
namespace core {
namespace image {

class Symbol {
public:
    /**
     * Symbol type.
     */
    enum Type {
        None,     ///< Not specified or unknown.
        Function, ///< Function.
        Data      ///< Data object.
    };

private:
    Type type_; ///< Type of the symbol.
    QString name_; ///< Name of the symbol.
    ConstantValue value_; ///< Value of the symbol.

public:
    /**
     * Constructor.
     *
     * \param type Type of the symbol.
     * \param name Name of the symbol.
     * \param value Value of the symbol.
     */
    Symbol(Type type, QString name, ConstantValue value):
        type_(type), name_(std::move(name)), value_(value)
    {}

    /**
     * \return Type of the object.
     */
    Type type() const { return type_; }

    /**
     * \return Name of the object.
     */
    QString name() const { return name_; }

    /**
     * \return Value of the symbol.
     */
    ConstantValue value() const { return value_; }
};

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
