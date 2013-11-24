/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/common/Range.h>

#include "Dataflow.h"

namespace nc {
namespace core {
namespace ir {

class Function;

namespace dflow {

/**
 * Dataflow for each function.
 */
class Dataflows {
    boost::unordered_map<const Function *, std::unique_ptr<Dataflow>> dataflows_;

public:
    /**
     * \param function Valid pointer to a function.
     *
     * \return Pointer to the dataflow of a given function.
     *         Will be NULL, if not set before.
     */
    const Dataflow *getDataflow(const Function *function) const {
        assert(function != NULL);
        return nc::find(dataflows_, function).get();
    }

    /**
     * Sets dataflow of a function.
     *
     * \param function Valid pointer to a function.
     * \param dataflow Pointer to the dataflow can be NULL.
     */
    void setDataflow(const Function *function, std::unique_ptr<Dataflow> dataflow) {
        assert(function != NULL);
        if (dataflow) {
            dataflows_[function] = std::move(dataflow);
        } else {
            dataflows_.erase(function);
        }
    }
};

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
