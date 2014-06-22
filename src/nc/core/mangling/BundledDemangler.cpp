/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "BundledDemangler.h"

#include <QCoreApplication>
#include <QFileInfo>

#include <nc/common/Foreach.h>

#include "ExternalDemangler.h"

namespace nc {
namespace core {
namespace mangling {

BundledDemangler::BundledDemangler() {
    QStringList locations;
    locations << (QCoreApplication::applicationDirPath() + QLatin1String("/demangler"));
    locations << (QCoreApplication::applicationDirPath() + QLatin1String("/demangler.exe"));
    locations << (QCoreApplication::applicationDirPath() + QLatin1String("/../demangler/demangler"));
    locations << (QCoreApplication::applicationDirPath() + QLatin1String("/../demangler/demangler.exe"));
    locations << (QCoreApplication::applicationDirPath() + QLatin1String("/../../demangler/Release/demangler.exe"));
    locations << (QCoreApplication::applicationDirPath() + QLatin1String("/../../demangler/Debug/demangler.exe"));
    
    foreach (const QString &filename, locations) {
        QFileInfo fileInfo(filename);
        if (fileInfo.isFile() && fileInfo.isExecutable()) {
            demangler_.reset(new ExternalDemangler(filename));
            break;
        }
    }

    if (!demangler_) {
        demangler_.reset(new ExternalDemangler(QLatin1String("demangler")));
    }
}

QString BundledDemangler::demangle(const QString &symbol) const {
    return demangler_->demangle(symbol);
}

}}} // namespace nc::core::mangling

/* vim:set et sts=4 sw=4: */
