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

#include "TermToFunction.h"

#include <cassert>

#include <nc/common/Foreach.h>

#include <nc/core/ir/Functions.h>

#include "CensusVisitor.h"

namespace nc {
namespace core {
namespace ir {
namespace misc {

TermToFunction::TermToFunction(const Functions *functions, calls::CallsData *callsData) {
    assert(functions != NULL);

    foreach (const ir::Function *function, functions->functions()) {
        ir::misc::CensusVisitor visitor(callsData);
        visitor(function);

        foreach (const ir::Term *term, visitor.terms()) {
            term2function_[term] = function;
        }
    }
}

} // namespace misc
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
