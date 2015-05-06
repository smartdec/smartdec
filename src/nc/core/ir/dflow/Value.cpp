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

#include "Value.h"

#include <QTextStream>

#include <nc/common/Unused.h>
#include <nc/common/BitTwiddling.h>

namespace nc {
namespace core {
namespace ir {
namespace dflow {

#ifndef NDEBUG
namespace {

bool test() {
    {
        int value = 0xf;
        assert(signExtend(value, 4) == -1);
    }

    {
        int value = 0x4;
        assert(signExtend(value, 5) == 4);
    }

    {
        int value = 0x3;
        assert(bitAbs(value, 2) == 1);
    }

    {
        int value = 0x3;
        assert(bitAbs(value, 10) == 3);
    }

    {
        unsigned value = -1;
        assert(bitTruncate(value, 5) == 0x1f);
    }

    {
        unsigned value = -1;
        assert(bitTruncate(value, sizeof(value) * CHAR_BIT) == value);
    }

    return true;
}

bool success = test();

} // anonymous namespace
#endif

Value::Value(SmallBitSize size):
    size_(size),
    isConstant_(false), isNonconstant_(false), isStackOffset_(false), isNotStackOffset_(false),
    isMultiplication_(false), isNotMultiplication_(false)
{
    if (size_ > MAX_SIZE) {
        /* We don't track values in too large registers yet. */
        makeNonconstant();
        makeNotStackOffset();
        makeNotMultiplication();
    }
}

void Value::makeConstant(const SizedValue &value) {
    SizedValue val = value.resized(size());

    if (isConstant_) {
        if (constantValue_.value() != val.value()) {
            makeNonconstant();
        }
    } else {
        isConstant_ = true;
        constantValue_ = val;
    }
}

void Value::forceConstant(const SizedValue &value) {
    isConstant_ = true;
    isNonconstant_ = false;

    constantValue_ = value.resized(size());
}

void Value::makeStackOffset(SizedValue offset) {
    SizedValue off = SizedValue(offset.value(), size());

    if (isStackOffset_) {
        /*
         * If we get different values of stack pointer from different
         * branches, most likely we screwed up at detecting a stdcall
         * function or a function that never returns.
         *
         * The heuristic is to choose the bigger stack pointer value,
         * i.e. the value corresponding to the branch with less pushes.
         *
         * Note: this assumes a stack growing down.
         */
        if (stackOffset_.signedValue() < off.signedValue()) {
            stackOffset_ = off;
        }
    } else {
        isStackOffset_ = true;
        stackOffset_ = off;
    }
}

void Value::join(const Value &that) {
    if (that.isConstant()) {
        makeConstant(that.constantValue());
    } else if (that.isNonconstant()) {
        makeNonconstant();
    }

    if (that.isStackOffset()) {
        makeStackOffset(that.stackOffset());
    } else if (that.isNotStackOffset()) {
        makeNotStackOffset();
    }

    if (that.isMultiplication()) {
        makeMultiplication();
    } else if (that.isNotMultiplication()) {
        makeNotMultiplication();
    }
}

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
