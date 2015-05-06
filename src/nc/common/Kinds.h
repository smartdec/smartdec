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
#include <nc/common/CheckedCast.h>

#include <boost/mpl/int.hpp>

namespace nc { 
    /**
     * Compile-time mapping from class in some hierarchy to its kind.
     * 
     * \tparam T                       Class.
     * \tparam Base                    Base class of the hierarchy that this class belongs to.
     */
    template<class T, class Base>
    class class_kind;
}

/**
 * Macro defining a 'kind' property, its getter, is() and as() template methods.
 * 
 * Must appear in private section of a class.
 * 
 * \param CLASS         Class this macro is being used in.
 * \param KIND_PROPERTY Name of the kind property.
 */
#define NC_CLASS_WITH_KINDS(CLASS, KIND_PROPERTY)                       \
    public:                                                             \
                                                                        \
    int KIND_PROPERTY() const { return KIND_PROPERTY##_; }              \
                                                                        \
    template<class Class>                                               \
    bool is() const {                                                   \
        return KIND_PROPERTY##_ == class_kind<Class, CLASS>::value;     \
    }                                                                   \
                                                                        \
    template<class Class>                                               \
    Class *as() {                                                       \
        if (!is<Class>()) {                                             \
            return NULL;                                                \
        } else {                                                        \
            return checked_cast<Class *>(this);                         \
        }                                                               \
    }                                                                   \
                                                                        \
    template<class Class>                                               \
    const Class *as() const {                                           \
        if (!is<Class>()) {                                             \
            return NULL;                                                \
        } else {                                                        \
            return checked_cast<const Class *>(this);                   \
        }                                                               \
    }                                                                   \
                                                                        \
    private:                                                            \
                                                                        \
    int KIND_PROPERTY##_;

/**
 * Defines a compile-time mapping from class to class kind.
 * 
 * Must be used at global namespace.
 * 
 * This macro MUST be invoked for each class that you wish to use with <tt>is</tt>
 * and <tt>as</tt> methods.
 * 
 * \param BASE                         Base class of the hierarchy that this class belongs to.
 * \param CLASS                        Class.
 * \param KIND                         Class kind.
 */
#define NC_REGISTER_CLASS_KIND(BASE, CLASS, KIND)                               \
namespace nc {                                                                  \
    template<>                                                                  \
    struct class_kind<CLASS, BASE>: public boost::mpl::int_<KIND> {};           \
}

/* vim:set et sts=4 sw=4: */
