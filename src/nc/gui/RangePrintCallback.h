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

#include <QTextStream>

#include <nc/common/PrintCallback.h>

#include "TextRange.h"

namespace nc { namespace gui {

/**
 * PrintCallback for tracking positions of printed objects.
 *
 * \tparam T Type of printed objects.
 */
template<class T>
class RangePrintCallback: public PrintCallback<T> {
    QTextStream &stream_; ///< Stream in which objects are printed.
    std::vector<int> stack_; ///< Start positions of objects.

    public:

    /**
     * Constructor.
     *
     * \param stream    Stream in which objects will be printed.
     */
    RangePrintCallback(QTextStream &stream): stream_(stream) {}

    virtual void onStartPrinting(T * /*node*/) override {
        stack_.push_back(getPosition());
    }

    virtual void onEndPrinting(T *node) override {
        assert(!stack_.empty());

        onRange(node, TextRange(stack_.back(), getPosition()));
        stack_.pop_back();
    }

    /**
     * Callback function called when a range of positions
     * occupied by an object becomes known.
     *
     * \param[in] object    Valid pointer to an object.
     * \param[in] range     Range of positions occupied by the object.
     */
    virtual void onRange(T *object, const TextRange &range) = 0;

    private:

    int getPosition() const {
        if (stream_.string()) {
            return stream_.string()->size();
        } else {
            stream_.flush();
            return stream_.pos();
        }
    }
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
