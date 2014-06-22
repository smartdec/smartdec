/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "ExternalDemangler.h"

namespace nc {
namespace core {
namespace mangling {

ExternalDemangler::ExternalDemangler(QString program, QStringList arguments):
    program_(std::move(program)), arguments_(std::move(arguments))
{}

ExternalDemangler::~ExternalDemangler() {
    process_.kill();
    process_.waitForFinished();
}

QString ExternalDemangler::demangle(const QString &symbol) const {
    QMutexLocker lock(&mutex_);

    if (process_.state() == QProcess::NotRunning) {
        process_.start(program_, arguments_);
    }
    if (process_.state() == QProcess::Starting) {
        process_.waitForStarted();
    }
    if (process_.state() != QProcess::Running) {
        return QString();
    }

    process_.write(symbol.toAscii().append('\n'));

    QByteArray response;
    while (process_.waitForReadyRead()) {
        response += process_.readLine();
        if (response.endsWith('\n')) {
            break;
        }
    }

    QString result = response.trimmed();

    if (result != symbol.trimmed()) {
        return result;
    } else {
        return QString();
    }
}

}}} // namespace nc::core::mangling

/* vim:set et sts=4 sw=4: */
