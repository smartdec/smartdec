/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <vector>

#include <boost/unordered_map.hpp>

#include "Relocation.h"

namespace nc {
namespace core {
namespace image {

/**
 * Information about relocations.
 */
class Relocations {
    std::vector<std::unique_ptr<Relocation>> relocations_;
    boost::unordered_map<ByteAddr, Relocation *> address2relocation_;

public:
    /**
     * Adds an information about relocation.
     *
     * \param relocation Valid pointer to a relocation information.
     */
    void add(std::unique_ptr<Relocation> relocation);

    /**
     * \param address Address.
     * \return Pointer to a relocation for this address. Can be NULL.
     */
    const Relocation *find(ByteAddr address) const;
};

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
