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

#include <nc/common/Types.h>

namespace nc {
namespace core {
namespace ir {
namespace calling {

/**
 * An immutable class with value semantics used to identify functions
 * being called, even when their addresses are not known.
 */
class CalleeId {
    public:

    /**
     * Kind of the id.
     */
    enum Kind {
        INVALID,       ///< Invalid, identifies nothing.
        ENTRY_ADDRESS, ///< Identifies a function by its entry address.
        CALL_ADDRESS   ///< Identifies a function by the address of a call to it.
    };

    private:

    Kind kind_; ///< Kind of this id.
    ByteAddr address_; ///< Address of callee's entry or call instruction.

    public:

    /**
     * Constructs an invalid id.
     */
    CalleeId(): kind_(INVALID), address_() {}

    /**
     * Constructs a valid id.
     *
     * \param kind Kind of the id.
     * \param address Function's entry or call instruction address (depends on the kind).
     */
    CalleeId(Kind kind, ByteAddr address):
        kind_(kind), address_(address)
    {
        assert(kind == ENTRY_ADDRESS || kind == CALL_ADDRESS);
    }

    /**
     * \return Kind of this id.
     */
    Kind kind() const { return kind_; }

    /**
     * \return Valid pointer to the address being called, if kind is ENTRY_ADDRESS, or NULL otherwise.
     */
    const ByteAddr *entryAddress() const { return kind_ == ENTRY_ADDRESS ? &address_ : NULL; }

    /**
     * \return Valid pointer to the address of the call instruction, if kind is CALL_ADDRESS, or NULL otherwise.
     */
    const ByteAddr *callAddress() const { return kind_ == CALL_ADDRESS ? &address_ : NULL; }

    /**
     * \return True if this is equal to that, false otherwise.
     */
    bool operator==(const CalleeId &that) const {
        return kind_ == that.kind_ && address_ == that.address_;
    }

    /**
     * \return True if this is not equal to that, false otherwise.
     */
    bool operator!=(const CalleeId &that) const {
        return !(*this == that);
    }

    /**
     * \return NULL if this id is invalid, non-null pointer otherwise.
     */
    operator const void *() const { return kind_ == INVALID ? NULL : this; }

    friend struct std::hash<CalleeId>;
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

namespace std {

/**
 * Specialization of std::hash for callee ids.
 *
 * This makes it possible to use callee ids as keys in hash maps.
 */
template<>
struct hash<nc::core::ir::calling::CalleeId>: public unary_function<nc::core::ir::calling::CalleeId, size_t> {
public:
    result_type operator()(const argument_type &value) const {
        return hash_value(static_cast<int>(value.kind_)) ^ hash_value(value.address_);
    }

protected:
    template<class T>
    result_type hash_value(const T &value) const {
        return hash<T>()(value);
    }
};

} // namespace std

namespace nc { namespace core { namespace ir { namespace calling {

/**
 * Qt hash function for memory locations.
 */
inline unsigned int qHash(const CalleeId &value) {
    return static_cast<unsigned int>(std::hash<CalleeId>()(value));
}

/**
 * Boost hash function for callee ids.
 */
inline std::size_t hash_value(const CalleeId &value) {
    return std::hash<CalleeId>()(value);
}

}}}} // namespace nc::core::ir::calling

/* vim:set et sts=4 sw=4: */
