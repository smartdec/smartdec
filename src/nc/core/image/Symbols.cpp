/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "Symbols.h"

#include <nc/common/Range.h>

namespace nc {
namespace core {
namespace image {

void Symbols::add(std::unique_ptr<Symbol> symbol) {
    typeValue2symbol_[std::make_pair(symbol->type(), symbol->value())] = symbol.get();
    symbols_.push_back(std::move(symbol));
}

const Symbol *Symbols::find(Symbol::Type type, ConstantValue value) const {
    return nc::find(typeValue2symbol_, std::make_pair(type, value));
}

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
