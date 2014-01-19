/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

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

#include <cassert>
#include <memory>

#include <QString>

namespace nc {
namespace core {
namespace ir {

class Term;

namespace calling {

/**
 * Signature of a function: name, arguments, return value.
 */
class Signature {
    QString name_; ///< Name of the function.
    std::vector<std::unique_ptr<Term>> arguments_; ///< Function's arguments.
    bool variadic_; ///< True if a function is variadic.
    std::unique_ptr<Term> returnValue_; ///< Term containing the return value.
    QString comment_; ///< Comment to generate before the function's declaration.

public:
    /**
     * Constructs an empty signature.
     */
    Signature();

    /**
     * Destructor.
     */
    ~Signature();

    /**
     * \return Name of the function.
     */
    const QString &name() const { return name_; }

    /**
     * Sets the function name.
     *
     * \param name New name.
     */
    void setName(QString name) { name_ = std::move(name); }

    /**
     * \return List of function's arguments.
     */
    const std::vector<const Term *> &arguments() const {
        return reinterpret_cast<const std::vector<const Term *> &>(arguments_);
    }

    /**
     * Adds a term representing function's argument.
     *
     * \param term Valid pointer to a term.
     */
    void addArgument(std::unique_ptr<Term> term);

    /**
     * \return True if the function is variadic.
     */
    bool variadic() const { return variadic_; }

    /**
     * Sets whether the function is variadic.
     *
     * \param value Flag whether the function is variadic.
     */
    void setVariadic(bool value = true) { variadic_ = value; }

    /**
     * \return Pointer to the term containing the return value. Can be NULL.
     */
    const Term *returnValue() const { return returnValue_.get(); }

    /**
     * Sets the pointer to the term containing the return value.
     *
     * \param term Valid pointer to the term.
     */
    void setReturnValue(std::unique_ptr<Term> term);

    /**
     * \return Comment to generate before the function's declaration.
     */
    const QString &comment() const { return comment_; }

    /**
     * Appends some text to the comment which will be generated before
     * the function's declaration.
     *
     * \param text Text to append.
     */
    void addComment(QString text);
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
