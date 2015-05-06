/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <memory> /* For std::unique_ptr. */

#include <boost/unordered_map.hpp>

#include <QString>

#include <nc/common/Range.h> /* nc::find */
#include <nc/common/Types.h>

namespace nc { namespace core {

namespace arch {
    class Architecture;
    class Instructions;
}

namespace image {
    class Image;
}

namespace mangling {
    class Demangler;
}

/**
 * An executable module: architecture, image, symbols.
 */
class Module {
public:
    Module();

    /**
     * Destructor.
     */
    ~Module();

    /**
     * \return Pointer to the architecture. Can be NULL.
     */
    arch::Architecture *architecture() { return mArchitecture.get(); }

    /**
     * \return Pointer to the architecture. Can be NULL.
     */
    const arch::Architecture *architecture() const { return mArchitecture.get(); }

    /**
     * Sets the architecture of this module.
     * The architecture must not have been set before.
     *
     * \param architecture Valid pointer to the architecture.
     */
    void setArchitecture(std::unique_ptr<arch::Architecture> architecture);

    /**
     * Sets the architecture of this module.
     * The architecture must not have been set before.
     *
     * \param name Name of the architecture.
     */
    void setArchitecture(const QString &name);

    /**
     * \return Valid pointer to the image of the executable file.
     */
    image::Image *image() { return mImage.get(); }

    /**
     * \return Valid pointer to the image of the executable file.
     */
    const image::Image *image() const { return mImage.get(); }

    /**
     * Sets the name of an address. The name can be taken, for example,
     * from the symbols table of the executable.
     *
     * \param[in] address              Address.
     * \param[in] name                 Name for the given address.
     */
    void addName(ByteAddr address, const QString &name) { mAddress2name[address] = name; }

    /**
     * \param[in] addr Address.
     *
     * \return Name for the given address, if any, and QString() otherwise.
     */
    const QString &getName(ByteAddr addr) const { return nc::find(mAddress2name, addr); }

    /**
     * \return All known pairs of addresses and its names.
     */
    const boost::unordered_map<ByteAddr, QString> &names() const { return mAddress2name; }

    /**
     * \return Valid pointer to a demangler.
     */
    const mangling::Demangler *demangler() const { return mDemangler.get(); }

    /**
     * Sets the demangler for the module.
     *
     * \param demangler Valid pointer to the new demangler.
     */
    void setDemangler(std::unique_ptr<mangling::Demangler> demangler);

    /**
     * Sets the demangler for the module.
     *
     * \param name Name of the demangler.
     */
    void setDemangler(const QString &name);

private:
    /** Architecture of the code being analyzed. */
    std::unique_ptr<arch::Architecture> mArchitecture;

    /** Image of the executable file. */
    std::unique_ptr<image::Image> mImage;

    /** Mapping of an address to its name. */
    boost::unordered_map<ByteAddr, QString> mAddress2name;

    /** Demangler. */
    std::unique_ptr<mangling::Demangler> mDemangler;
};

}} // namespace nc::core

/* vim:set et sts=4 sw=4: */
