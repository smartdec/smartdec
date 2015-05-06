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

namespace nc {

/**
 * Template base class for visitor classes (like in visitor pattern).
 *
 * \tparam T Type of objects this visitor is for.
 */
template <class T>
class Visitor {
public:
    /**
     * Visit an object of type T.
     *
     * \param[in] obj Valid pointer to an object of type T.
     */
    virtual void operator()(T *obj) = 0;
};


namespace detail {
template<class T, class Function>
class FunctionVisitor: public Visitor<T> {
public:
    FunctionVisitor(const Function &function): mFunction(function) {}

    virtual void operator()(T *obj) override {
        mFunction(obj);
    }

private:
    Function mFunction;
};

} // namespace detail

/**
 * Factory function that creates visitor from the given functor.
 * 
 * \param function                     Functor to create visitor from.
 */
template<class T, class Function>
detail::FunctionVisitor<T, Function> makeVisitor(const Function &function) {
    return detail::FunctionVisitor<T, Function>(function);
}

} // namespace nc

/* vim:set et sts=4 sw=4: */
