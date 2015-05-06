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

#include "Parser.h"

#include <cassert>

#include <QIODevice>

#include <nc/core/input/ParseError.h>

namespace nc { namespace core { namespace input {

bool Parser::canParse(QIODevice *source) const {
    assert(source != NULL);

    source->seek(0);
    return doCanParse(source);
}

void Parser::parse(QIODevice *source, Module *module) const {
    assert(source != NULL);
    assert(module != NULL);

    try {
        source->seek(0);
        doParse(source, module);
    } catch (nc::Exception &e) {
        if (!boost::get_error_info<ErrorOffset>(e)) {
            e << ErrorOffset(source->pos());
        }
        throw;
    }
}

}}} // namespace nc::core::input

/* vim:set et sts=4 sw=4: */
