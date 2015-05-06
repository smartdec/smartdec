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

#include <memory>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>

#include <nc/common/Printable.h>
#include <nc/common/Types.h>

#include "CommentText.h"

namespace nc {
namespace core {
namespace ir {

class Function;

/**
 * Collection of functions in intermediate representation.
 */
class Functions: public PrintableBase<Functions>, boost::noncopyable {
    /** The functions. */
    std::vector<Function *> functions_;

    /** Mapping from an entry address to the list of functions with this address. */
    boost::unordered_map<ByteAddr, std::vector<Function *>> entry2functions_;

    /** Comment for the whole set of functions. */
    CommentText comment_;

public:
    /**
     * Destructor.
     */
    ~Functions();

    /**
     * \return Intermediate representations of functions.
     */
    const std::vector<Function *> &functions() const { return functions_; }

    /**
     * Adds intermediate representation of a function and takes ownership of it.
     *
     * \param[in] function Valid pointer to a function.
     */
    void addFunction(std::unique_ptr<Function> function);

    /**
     * \param address Entry address.
     *
     * \return Pointer to the functions with given entry address.
     */
    const std::vector<Function *> &getFunctionsAtAddress(ByteAddr address) const;

    /**
     * Prints the intermediate representation of all functions into a stream in DOT format.
     *
     * \param[in] out Output stream.
     */
    void print(QTextStream &out) const;

    /**
     * \return Comment for this set of functions.
     */
    CommentText &comment() { return comment_; }

    /**
     * \return Comment for this set of functions.
     */
    const CommentText &comment() const { return comment_; }
};

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
