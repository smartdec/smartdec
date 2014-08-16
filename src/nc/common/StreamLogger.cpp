/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "StreamLogger.h"

#include <QObject>

namespace nc {

void StreamLogger::log(LogLevel level, const QString &text) {
    stream_ << QObject::tr("[%1] %2", "log").arg(level.getName()).arg(text) << endl;
}

} // namespace nc

/* vim:set et sts=4 sw=4: */
