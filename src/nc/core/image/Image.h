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
#include <vector>

#include <boost/unordered_map.hpp>

#include <QString>

#include "ByteSource.h"
#include "Symbol.h"

namespace nc { namespace core {

namespace arch {
    class Architecture;
}

namespace mangling {
    class Demangler;
}

namespace image {

class Section;
class Relocations;

/**
 * An executable image.
 */
class Image: public ByteSource {
    const arch::Architecture *architecture_; ///< Architecture.
    std::vector<std::unique_ptr<Section>> sections_; ///< The list of sections.
    std::vector<std::unique_ptr<Symbol>> symbols_; ///< The list of symbols.
    boost::unordered_map<std::pair<ConstantValue, Symbol::Type>, Symbol *> value2symbol_; ///< Mapping from value and symbol type to a symbol with this value and type.
    std::unique_ptr<Relocations> relocations_; ///< Relocations.
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
     * Adds a new section.
     *
     * \param section Valid pointer to the section.
     */
    void addSection(std::unique_ptr<Section> section);

    /**
     * \return List of all sections.
     */
    const std::vector<Section *> &sections() {
        return reinterpret_cast<const std::vector<Section *> &>(sections_);
    }

    /**
     * \return List of all sections.
     */
    const std::vector<const Section *> &sections() const {
        return reinterpret_cast<const std::vector<const Section *> &>(sections_);
    }

    /**
     * \param[in] addr Linear address.
     *
     * \return A valid pointer to allocated section containing given
     *         virtual address or NULL if there is no such section.
     */
    const Section *getSectionContainingAddress(ByteAddr addr) const;

    /**
     * \param[in] name Section name.
     *
     * \return Valid pointer to a section with the given name,
     *         NULL if there is no such section.
     */
    const Section *getSectionByName(const QString &name) const;

    /**
     * Reads a sequence of bytes from the section containing
     * the given address and allocated during program execution.
     */
    ByteSize readBytes(ByteAddr addr, void *buf, ByteSize size) const override;

    /**
     * Adds a symbol.
     *
     * \param symbol Valid pointer to the symbol.
     */
    void addSymbol(std::unique_ptr<Symbol> symbol);

    /**
     * \return List of all symbols.
     */
    const std::vector<const Symbol *> &symbols() const {
        return reinterpret_cast<const std::vector<const Symbol *> &>(symbols_);
    }

    /**
     * Finds a symbol with a given type and value.
     *
     * \param value Value of the symbol.
     * \param type Type of the symbol.
     *
     * \return Pointer to a symbol with the given type and value. Can be NULL.
     */
    const Symbol *getSymbol(ConstantValue value, Symbol::Type type) const;

    /**
     * \return Valid pointer to the information about relocations.
     */
    Relocations *relocations() { return relocations_.get(); }

    /**
     * \return Valid pointer to the information about relocations.
     */
    const Relocations *relocations() const { return relocations_.get(); }

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
