/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "Utils.h"

#include <cassert>

#include <nc/common/Range.h>
#include <nc/core/arch/Instruction.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Dominators.h>
#include <nc/core/ir/Statement.h>

namespace nc {
namespace core {
namespace ir {
namespace cgen {

bool isDominating(const Statement *first, const Statement *second, const Dominators &dominators) {
    assert(first);
    assert(second);

    if (first == second) {
        return false;
    }

    if (first->basicBlock() == second->basicBlock()) {
        if (first->instruction() && second->instruction() &&
            first->instruction() != second->instruction())
        {
            return first->instruction()->addr() < second->instruction()->addr();
        } else {
            const auto &statements = second->basicBlock()->statements();
            assert(nc::contains(statements, first));
            assert(nc::contains(statements, second));
            return std::find(
                std::find(statements.begin(), statements.end(), first),
                statements.end(),
                second) != statements.end();
        }
    }

    return dominators.isDominating(first->basicBlock(), second->basicBlock());
}

} // namespace cgen
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
