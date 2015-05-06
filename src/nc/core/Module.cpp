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

#include "Module.h"

#include <nc/common/Warnings.h>
#include <nc/common/make_unique.h>

#include <nc/core/arch/Architecture.h>
#include <nc/core/image/Image.h>
#include <nc/core/mangling/Demangler.h>
#include <nc/core/mangling/CxxFiltDemangler.h>

#include <nc/arch/intel/IntelArchitecture.h>

#ifdef NC_WITH_LIBIBERTY
    #include <nc/core/mangling/GnuDemangler.h>
#endif

#ifdef NC_WITH_UNDNAME
    #include <nc/core/mangling/MsvcDemangler.h>
#endif

namespace nc { namespace core {

Module::Module():
    mImage(new image::Image(this)),
    mDemangler(new mangling::Demangler)
{}

Module::~Module() {}

void Module::setArchitecture(std::unique_ptr<arch::Architecture> architecture) {
    assert(architecture != NULL);
    assert(mArchitecture == NULL && "Can't set architecture twice.");

    mArchitecture = std::move(architecture);
}

void Module::setArchitecture(const QString &name) {
    // TODO: ideally, there should be an ArchitectureRepository.

    if (name == QLatin1String("8086")) {
        setArchitecture(std::make_unique<nc::arch::intel::IntelArchitecture>(nc::arch::intel::IntelArchitecture::REAL_MODE));
    } else if (name == QLatin1String("i386")) {
        setArchitecture(std::make_unique<nc::arch::intel::IntelArchitecture>(nc::arch::intel::IntelArchitecture::PROTECTED_MODE));
    } else if (name == QLatin1String("x86-64")) {
        setArchitecture(std::make_unique<nc::arch::intel::IntelArchitecture>(nc::arch::intel::IntelArchitecture::LONG_MODE));
    } else {
        ncWarning("Architecture not supported: %1", name);
    }
}

void Module::setDemangler(std::unique_ptr<mangling::Demangler> demangler) {
    assert(demangler != NULL);

    mDemangler = std::move(demangler);
}

void Module::setDemangler(const QString &format) {
    // TODO: ideally, there should be a DemanglerRepository.

#ifdef NC_WITH_LIBIBERTY
    if (format == QLatin1String("gnu-v3")) {
        setDemangler(std::make_unique<mangling::GnuDemangler>());
        return;
    }
#endif

#ifdef NC_WITH_UNDNAME
    if (format == QLatin1String("msvc")) {
        setDemangler(std::make_unique<mangling::MsvcDemangler>());
        return;
    }
#endif

    setDemangler(std::make_unique<mangling::CxxFiltDemangler>(format));
}

}} // namespace nc::core

/* vim:set et sts=4 sw=4: */
