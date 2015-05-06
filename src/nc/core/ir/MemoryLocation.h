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
#include <functional> /* For std::hash. */

#include <boost/operators.hpp>

#include <nc/common/CheckedCast.h>
#include <nc/common/Printable.h>
#include <nc/common/Types.h>

#include "MemoryDomain.h"

namespace nc { namespace core { namespace ir {

/**
 * Some abstract memory location.
 */
class MemoryLocation: public boost::equality_comparable1<MemoryLocation>, public PrintableBase<MemoryLocation> {
    Domain domain_; ///< Id of the universe where the memory region is located.
    BitAddr addr_; ///< Address of the memory region in bits.
    BitSize size_; ///< Size of the memory region in bits.

    public:

    /**
     * Class default constructor.
     */
    MemoryLocation(): domain_(MemoryDomain::UNKNOWN), addr_(0), size_(0) {}

    /**
     * Class constructor.
     *
     * \param[in] domain Id of the universe where the memory region is located.
     * \param[in] addr Address of the memory region in bits.
     * \param[in] size Size of the memory region in bits.
     */
    MemoryLocation(Domain domain, BitAddr addr, BitSize size):
        domain_(domain), addr_(addr), size_(size)
    {
        assert(size_ > 0);
    }

    /**
     * \return Id of the universe where the memory region is located.
     */
    Domain domain() const { return domain_; }

    /**
     * \return Address of the memory region in bits.
     */
    BitAddr addr() const { return addr_; }

    /**
     * \return Size of the memory region in bits.
     */
    BitSize size() const { return size_; }

    /**
     * \return Size of the memory region in bits, safely converted to type T.
     */
    template<class T>
    T size() const {
        return checked_cast<T>(size_);
    }

    /**
     * \return Address + size.
     */
    BitAddr endAddr() const { return addr_ + size_; }

    /**
     * \return True if the object stores valid memory location, false otherwise.
     */
    operator const void *() const { return size() ? this : 0; }

    /**
     * Prints the location into a stream.
     *
     * \param[in] out Output stream.
     */
    void print(QTextStream &out) const;

    /**
     * \param offset Offset in bits.
     * \return New memory location.
     */
    MemoryLocation shifted(BitOffset offset) const {
        return MemoryLocation(domain_, addr_ + offset, size_);
    }
};

/**
 * \param[in] a Memory location.
 * \param[in] b Memory location.
 *
 * \return True, if a and b are equal memory locations, i.e. have the same domains, addresses, and sizes.
 */
inline bool operator==(const MemoryLocation &a, const MemoryLocation &b) {
    return a.domain() == b.domain() && a.addr() == b.addr() && a.size() == b.size();
}

/**
 * \param[in] a Memory location.
 * \param[in] b Memory location.
 *
 * \return True, if memory location a is in some sense is before memory location b.
 */
inline bool operator<(const MemoryLocation &a, const MemoryLocation &b) {
    return a.domain() < b.domain() || (a.domain() == b.domain() && (a.addr() < b.addr() || (a.addr() == b.addr() && a.size() < b.size())));
}

}}} // namespace nc::core::ir

namespace std {

/**
 * Specialization of std::hash for memory locations.
 * 
 * This makes it possible to use memory locations as keys in hash maps.
 */
template<>
class hash<nc::core::ir::MemoryLocation>: public unary_function<nc::core::ir::MemoryLocation, size_t> {
public:
    result_type operator()(const argument_type &location) const {
        return hash_value(location.domain()) ^ hash_value(location.addr()) ^ hash_value(location.size());
    }

protected:
    template<class T>
    result_type hash_value(const T &value) const {
        return hash<T>()(value);
    }
};

} // namespace std

namespace nc { namespace core { namespace ir {
/**
 * Qt hash function for memory locations.
 */
inline unsigned int qHash(const MemoryLocation &value) {
    return static_cast<unsigned int>(std::hash<MemoryLocation>()(value));
}

/**
 * Boost hash function for memory locations.
 */
inline std::size_t hash_value(const MemoryLocation &value) {
    return std::hash<MemoryLocation>()(value);
}

}}} // namespace nc::core::ir

/* vim:set et sts=4 sw=4: */
