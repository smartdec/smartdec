/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "Uses.h"

#include <nc/common/Foreach.h>

#include "Dataflow.h"

namespace nc {
namespace core {
namespace ir {
namespace dflow {

Uses::Uses(const Dataflow &dataflow) {
    foreach (auto &termAndDefinitions, dataflow.term2definitions()) {
        /* Ignore REACHING_SNAPSHOT intrinsics which do not have
         * a memory location, but have definitions. */
        if (dataflow.getMemoryLocation(termAndDefinitions.first)) {
            foreach (const auto &chunk, termAndDefinitions.second.chunks()) {
                foreach (const Term *definition, chunk.definitions()) {
                    term2uses_[definition].push_back(termAndDefinitions.first);
                }
            }
        }
    }
}

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
