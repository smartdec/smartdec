/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <QTextStream>

#include "Logger.h"

namespace nc {

/**
 * Logger printing messages to a stream.
 */
class StreamLogger: public nc::Logger {
    QTextStream &stream_;

    public:

    /**
     * Constructor.
     *
     * \param stream Reference to the stream to print messages to.
     */
    StreamLogger(QTextStream &stream): stream_(stream) {}

    virtual void log(const QString &text) { stream_ << text << endl; }
};

} // namespace nc

/* vim:set et sts=4 sw=4: */
