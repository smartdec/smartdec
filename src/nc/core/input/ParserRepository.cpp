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

#include "ParserRepository.h"

#include <cassert>

#include <QHash>

#include <nc/common/Foreach.h>
#include <nc/common/Warnings.h>
#include <nc/common/make_unique.h>

#include <nc/input/elf/ElfParser.h>
#include <nc/input/pe/PeParser.h>

#include "Parser.h"

namespace nc { namespace core { namespace input {

Q_GLOBAL_STATIC_WITH_INITIALIZER(ParserRepository, parserRepository, {
    /* Register default parsers. */
    x->registerParser(std::make_unique<nc::input::elf::ElfParser>());
    x->registerParser(std::make_unique<nc::input::pe::PeParser>());
});

class ParserRepositoryPrivate {
public:
    ~ParserRepositoryPrivate() {
        foreach(Parser *parser, parsers) {
            delete parser;
        }
    }

    QHash<QString, Parser *> name2parser;
    std::vector<Parser *> parsers;
};

ParserRepository::ParserRepository(): 
    d(new ParserRepositoryPrivate()) 
{}

ParserRepository *ParserRepository::instance() {
    return parserRepository();
}

void ParserRepository::registerParser(std::unique_ptr<Parser> parser) {
    assert(parser != NULL);
    assert(!getParser(parser->name()) && "Cannot register two parsers with the same name.");

    d->name2parser[parser->name()] = parser.get();
    d->parsers.push_back(parser.release());
}

Parser *ParserRepository::getParser(const QString &name) {
    return d->name2parser.value(name);
}

const std::vector<Parser *> &ParserRepository::parsers() const {
    return d->parsers;
}

}}} // namespace nc::core::input

/* vim:set et sts=4 sw=4: */
