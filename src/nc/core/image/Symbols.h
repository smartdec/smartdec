/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <vector>

#include <boost/unordered_map.hpp>

#include "Symbol.h"

namespace nc {
namespace core {
namespace image {

class Symbol;

/**
 * Symbols in an executable file.
 */
class Symbols {
    std::vector<std::unique_ptr<Symbol>> symbols_;
    boost::unordered_map<std::pair<Symbol::Type, ConstantValue>, Symbol *> typeValue2symbol_;

public:
    /**
     * Adds a symbol.
     *
     * \param symbol Valid pointer to the symbol.
     */
    void add(std::unique_ptr<Symbol> symbol);

    /**
     * Finds a symbol with a given type and value.
     *
     * \param type Type of the symbol.
     * \param value Value of the symbol.
     *
     * \return Pointer to a symbol with the given type and value. Can be NULL.
     */
    const Symbol *find(Symbol::Type type, ConstantValue value) const;

    /**
     * \return List of all symbols.
     */
    const std::vector<const Symbol *> &all() const { return reinterpret_cast<const std::vector<const Symbol *> &>(symbols_); }
};

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
