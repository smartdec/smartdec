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

#include <QString>

namespace nc { namespace core { namespace input {
class Parser;

class ParserRepositoryPrivate;

/**
 * Static repository that stores parser instances.
 */
class ParserRepository {
public:
    /**
     * \returns Parser repository instance.
     */
    static ParserRepository *instance();

    /**
     * Default constructor. 
     */
    ParserRepository();

    /**
     * Registers a parser, if there is no parser with the same name yet.
     *
     * \param[in] parser Valid pointer to the parser to register.
     *
     * \return True if the parser was registered, false otherwise.
     */
    void registerParser(std::unique_ptr<Parser> parser);

    /**
     * \param[in] name Name of the parser.
     *
     * \returns Valid pointer to the parser for the given name, or NULL if no such parser exists.
     */
    Parser *getParser(const QString &name);

    /**
     * \returns List of all registered parsers.
     */
    const std::vector<Parser *> &parsers() const;

private:
    std::unique_ptr<ParserRepositoryPrivate> d;
};

}}} // namespace nc::core::input

/* vim:set et sts=4 sw=4: */
