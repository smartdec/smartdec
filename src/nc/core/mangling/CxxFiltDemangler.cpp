//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "CxxFiltDemangler.h"

#include <QProcess>
#include <QTextStream>

namespace nc {
namespace core {
namespace mangling {

CxxFiltDemangler::CxxFiltDemangler(const QString &format):
    format_(format)
{}

QString CxxFiltDemangler::demangle(const QString &symbol) const {
    /*
     * Current implementation create a new process for each request.
     * This is naive, but thread-safe. However, can be improved.
     */

    QProcess process;
    process.start(QLatin1String("c++filt"), QStringList() << QLatin1String("-s") << format_ << symbol);

    if (!process.waitForStarted()) {
        return QString();
    }

    if (!process.waitForFinished()) {
        process.kill();
        return QString();
    }

    auto result = QString(process.readAllStandardOutput()).trimmed();

    if (result != symbol) {
        return result;
    } else {
        return QString();
    }
}

}}} // namespace nc::core::mangling

/* vim:set et sts=4 sw=4: */
