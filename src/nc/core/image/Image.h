/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

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

#include <memory>

#include <QString>

namespace nc { namespace core {

namespace arch {
    class Architecture;
}

namespace mangling {
    class Demangler;
}

namespace image {

class Sections;
class Symbols;

/**
 * An executable image.
 */
class Image {
    const arch::Architecture *architecture_; ///< Architecture.
    std::unique_ptr<Sections> sections_; ///< Sections.
    std::unique_ptr<Symbols> symbols_; ///< Symbols.
    std::unique_ptr<mangling::Demangler> demangler_; ///< Demangler.

public:
    /**
     * Constructor.
     */
    Image();

    /**
     * Destructor.
     */
    ~Image();

    /**
     * \return Pointer to the architecture. Can be NULL.
     */
    const arch::Architecture *architecture() const { return architecture_; }

    /**
     * Sets the architecture of this executable image.
     * The architecture must not have been set before.
     *
     * \param architecture Valid pointer to the architecture.
     */
    void setArchitecture(const arch::Architecture *architecture);

    /**
     * Sets the architecture of this executable image.
     * The architecture must not have been set before.
     *
     * \param name Name of the architecture.
     */
    void setArchitecture(const QString &name);

    /**
     * \return Valid pointer to the sections of the executable file.
     */
    Sections *sections() { return sections_.get(); }

    /**
     * \return Valid pointer to the sections of the executable file.
     */
    const Sections *sections() const { return sections_.get(); }

    /**
     * \return Valid pointer to the symbols of the image.
     */
    Symbols *symbols() { return symbols_.get(); }

    /**
     * \return Valid pointer to the symbols of the image.
     */
    const Symbols *symbols() const { return symbols_.get(); }

    /**
     * \return Valid pointer to a demangler.
     */
    const mangling::Demangler *demangler() const { return demangler_.get(); }

    /**
     * Sets the demangler.
     *
     * \param demangler Valid pointer to the new demangler.
     */
    void setDemangler(std::unique_ptr<mangling::Demangler> demangler);

    /**
     * Sets the demangler.
     *
     * \param name Name of the demangler.
     */
    void setDemangler(const QString &name);
};

}}} // namespace nc::core::image

/* vim:set et sts=4 sw=4: */
