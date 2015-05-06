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

#include "IntelRegisters.h"

namespace nc { namespace arch { namespace intel {

IntelRegisters::IntelRegisters() {
#define REGISTER_TABLE <nc/arch/intel/IntelRegisterTable.i>
#include <nc/core/arch/RegistersConstructor.i>

    registerStackPointer(RSP);
    registerStackPointer(ESP);
    registerStackPointer(SP);
}

}}} // namespace nc::arch::intel

/* vim:set et sts=4 sw=4: */
