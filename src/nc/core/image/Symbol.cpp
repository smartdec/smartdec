/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "Symbol.h"

#include <QObject>

#include <nc/common/Unreachable.h>

namespace nc {
namespace core {
namespace image {

QString SymbolType::getName(Type type) {
    switch (type) {
        case NOTYPE:
            return QObject::tr("none", "symbol");
        case FUNCTION:
            return QObject::tr("function", "symbol");
        case OBJECT:
            return QObject::tr("object", "symbol");
        case SECTION:
            return QObject::tr("section", "symbol");
    }
    unreachable();
}

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
