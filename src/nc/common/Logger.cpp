/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "Logger.h"

#include <QObject>

namespace nc {

QString LogLevel::getName(Level level) {
    switch (level) {
        case DEBUG:
            return QObject::tr("Debug", "log");
        case INFO:
            return QObject::tr("Info", "log");
        case WARNING:
            return QObject::tr("Warning", "log");
        case ERROR:
            return QObject::tr("Error", "log");
    }
    return QString();
}

} // namespace nc

/* vim:set et sts=4 sw=4: */
