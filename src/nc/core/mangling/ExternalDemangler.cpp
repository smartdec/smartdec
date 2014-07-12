/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "ExternalDemangler.h"

#include <QProcess>

namespace nc {
namespace core {
namespace mangling {

ExternalDemangler::ExternalDemangler(QString program, QStringList arguments):
    program_(std::move(program)), arguments_(std::move(arguments))
{}

QString ExternalDemangler::demangle(const QString &symbol) const {
    /*
     * Create a new process for each request. This is stupid, but works.
     * If one tries to share the same QProcess among multiple requests
     * (and, consequently, among multiple threads), one gets the following
     * messages:
     *
     * [Warning] QObject: Cannot create children for a parent that is in a different thread.
     * (Parent is QProcess(0xe26a18), parent's thread is QThread(0xd04090), current thread is QThread(0xf0db10)
     */

    QProcess process;
    process.start(program_, arguments_);

    if (!process.waitForStarted()) {
        return QString();
    }

    process.write(symbol.toAscii().append('\n'));
    process.closeWriteChannel();

    if (!process.waitForFinished()) {
        process.kill();
        return QString();
    }

    auto result = QString(process.readAllStandardOutput()).trimmed();

    if (result != symbol.trimmed()) {
        return result;
    } else {
        return QString();
    }
}

}}} // namespace nc::core::mangling

/* vim:set et sts=4 sw=4: */
