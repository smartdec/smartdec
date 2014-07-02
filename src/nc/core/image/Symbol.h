/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/common/Types.h>

#include <QString>

namespace nc {
namespace core {
namespace image {

class Section;

class Symbol {
public:
    /**
     * Symbol type.
     */
    enum Type {
        NOTYPE,   ///< Not specified or unknown.
        FUNCTION, ///< Function.
        OBJECT,   ///< Data object.
        SECTION   ///< Section object.
    };

private:
    Type type_; ///< Type of the symbol.
    QString name_; ///< Name of the symbol.
    ConstantValue value_; ///< Value of the symbol.
    const Section *section_; ///< Section referenced by the symbol.

public:
    /**
     * Constructor.
     *
     * \param type Type of the symbol.
     * \param name Name of the symbol.
     * \param value Value of the symbol.
     * \param section Pointer to the section referenced by the symbol.
     */
    Symbol(Type type, QString name, ConstantValue value, const Section *section = NULL):
        type_(type), name_(std::move(name)), value_(value), section_(section)
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

    /**
     * \return Pointer to the section references by the symbol. Can be NULL.
     */
    const Section *section() const { return section_; }
};

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
