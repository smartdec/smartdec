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

/**
 * Symbol type.
 */
class SymbolType {
public:
    /**
     * Symbol type values.
     */
    enum Type {
        NOTYPE,   ///< Not specified or unknown.
        FUNCTION, ///< Function.
        OBJECT,   ///< Data object.
        SECTION   ///< Section object.
    };

    /**
     * Constructor.
     *
     * \param type Symbol type value.
     */
    SymbolType(Type type = NOTYPE): type_(type) {}

    /**
     * \return Symbol type value.
     */
    operator Type() const { return type_; }

    /**
     * \param type Symbol type value.
     *
     * \return Name of the given symbol type.
     */
    static QString getName(Type type);

    /**
     * \return Name of the symbol type.
     */
    QString getName() const { return getName(type_); }

private:
    Type type_;
};

class Symbol {
private:
    SymbolType type_; ///< Type of the symbol.
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
    Symbol(SymbolType type, QString name, ConstantValue value, const Section *section = NULL):
        type_(type), name_(std::move(name)), value_(value), section_(section)
    {}

    /**
     * \return Type of the symbol.
     */
    SymbolType type() const { return type_; }

    /**
     * \return Name of the symbol.
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
