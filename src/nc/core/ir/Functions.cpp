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

#include "Functions.h"

#include <QTextStream>

#include <nc/common/Foreach.h>
#include <nc/common/Range.h>

#include "BasicBlock.h"
#include "Function.h"

namespace nc {
namespace core {
namespace ir {

Functions::~Functions() {
    foreach(Function *function, functions_) {
        delete function;
    }
}

void Functions::addFunction(std::unique_ptr<Function> function) {
    if (function->entry() && function->entry()->address()) {
        entry2functions_[*function->entry()->address()].push_back(function.get());
    }

    functions_.reserve(functions_.size() + 1);
    functions_.push_back(function.release());
}

const std::vector<Function *> &Functions::getFunctionsAtAddress(ByteAddr address) const {
    return nc::find(entry2functions_, address);
}

void Functions::print(QTextStream &out) const {
    out << "digraph Functions" << this << " {" << endl;
    
    foreach (const Function *function, functions_) {
        out << *function;
    }

    out << "}" << endl;
}

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
