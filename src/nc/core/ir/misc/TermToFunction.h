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

#include <boost/unordered_map.hpp>

#include <nc/common/Range.h> /* nc::find */

namespace nc {
namespace core {
namespace ir {

class Function;
class Functions;
class Term;

namespace calls {
    class CallsData;
}

namespace misc {

/**
 * Mapping from terms to functions owning them.
 */
class TermToFunction {
    /** Mapping from terms to functions owning them. */
    boost::unordered_map<const Term *, const Function *> term2function_;

    public:

    /**
     * Constructor.
     *
     * \param functions Valid pointer to the functions.
     * \param callsData Pointer to the calls data. Can be NULL.
     */
    TermToFunction(const Functions *functions, calls::CallsData *callsData);

    /**
     * \param term Valid pointer to a term.
     * \return Function owning the term.
     */
    const Function *getFunction(const Term *term) const { return nc::find(term2function_, term); }
};

} // namespace misc
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
