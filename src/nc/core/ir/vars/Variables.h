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

#include <memory> /* unique_ptr */

#include <boost/unordered_map.hpp>

#include "Variable.h"

namespace nc {
namespace core {
namespace ir {

class Term;

namespace vars {

/**
 * Container for information about which term realizes which variable of reconstructed program.
 */
class Variables {
    mutable boost::unordered_map<const Term *, std::unique_ptr<Variable> > variables_; ///< Mapping of terms to variables.

    public:

    /**
     * \param[in] term Term.
     *
     * \return Variable represented by this term.
     */
    Variable *getVariable(const Term *term);

    /**
     * \param[in] term Term.
     *
     * \return Variable represented by this term.
     */
    const Variable *getVariable(const Term *term) const;
};

} // namespace vars
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
