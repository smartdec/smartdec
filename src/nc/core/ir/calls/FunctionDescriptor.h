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
namespace calls {

/**
 * An immutable class with value semantics used to identify functions even when their addresses are not known.
 */
class FunctionDescriptor {
    public:

    /**
     * Kind of the descriptor.
     */
    enum Kind {
        INVALID,       ///< Invalid descriptor not identifying anything.
        ENTRY_ADDRESS, ///< Identifier by function's entry address.
        CALL_ADDRESS   ///< Identifier by the call instruction address.
    };

    private:

    Kind kind_; ///< Kind of the descriptor.
    ByteAddr address_; ///< Address.

    public:

    /**
     * Constructor of an invalid descriptor.
     */
    FunctionDescriptor(): kind_(INVALID), address_() {}

    /**
     * Constructor.
     *
     * \param kind Kind of the descriptor.
     * \param address Function's entry or call instruction address (depends on the kind).
     */
    FunctionDescriptor(Kind kind, ByteAddr address):
        kind_(kind), address_(address)
    {
        assert(kind == ENTRY_ADDRESS || kind == CALL_ADDRESS);
    }

    /**
     * \return Kind of the descriptor.
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
    bool operator==(const FunctionDescriptor &that) const {
        return kind_ == that.kind_ && address_ == that.address_;
    }

    /**
     * \return True if this is not equal to that, false otherwise.
     */
    bool operator!=(const FunctionDescriptor &that) const {
        return !(*this == that);
    }

    /**
     * \return NULL if the descriptor is invalid, non-null pointer otherwise.
     */
    operator const void *() const { return kind_ == INVALID ? NULL : this; }

    friend struct std::hash<FunctionDescriptor>;
};

} // namespace calls
} // namespace ir
} // namespace core
} // namespace nc

namespace std {

/**
 * Specialization of std::hash for function descriptors.
 * 
 * This makes it possible to use function descriptors as keys in hash maps.
 */
template<>
struct hash<nc::core::ir::calls::FunctionDescriptor>: public unary_function<nc::core::ir::calls::FunctionDescriptor, size_t> {
public:
    result_type operator()(const argument_type &descriptor) const {
        return hash_value(static_cast<int>(descriptor.kind_)) ^ hash_value(descriptor.address_);
    }

protected:
    template<class T>
    result_type hash_value(const T &value) const {
        return hash<T>()(value);
    }
};

} // namespace std

namespace nc { namespace core { namespace ir { namespace calls {

/**
 * Qt hash function for memory locations.
 */
inline unsigned int qHash(const FunctionDescriptor &value) {
    return static_cast<unsigned int>(std::hash<FunctionDescriptor>()(value));
}

/**
 * Boost hash function for function descriptors.
 */
inline std::size_t hash_value(const FunctionDescriptor &value) {
    return std::hash<FunctionDescriptor>()(value);
}

}}}} // namespace nc::core::ir::calls

/* vim:set et sts=4 sw=4: */
