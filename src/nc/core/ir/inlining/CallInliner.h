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

namespace nc { namespace core { namespace ir {

class BasicBlock;
class Call;
class Function;

namespace inlining {

class CallInliner {
public:

    /**
     * Virtual destructor.
     */
    virtual ~CallInliner() {}

    /**
     * Inlines one function into another.
     *
     * \param[in] receivingFunction Function to inline into.
     * \param[in] call Call to replace with inlined function.
     * \param[in] inlinedFunction Inlined function itself.
     */
    virtual void perform(Function *receivingFunction, const Call *call, const Function *inlinedFunction) const;
};

}}}} // namespace nc::core::ir::inlining

/* vim:set et sts=4 sw=4: */
