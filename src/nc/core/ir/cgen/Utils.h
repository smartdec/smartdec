/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

namespace nc {
namespace core {
namespace ir {

class Dominators;
class Statement;

namespace cgen {

/**
* \param[in] first Valid pointer to a statement in a CFG.
* \param[in] second Valid pointer to a statement in a CFG.
* \param[in] dominators Dominator sets for the CFG.
*
* \return True if the first statement dominates the second statement in the CFG.
*/
bool isDominating(const Statement *first, const Statement *second, const Dominators &dominators);

} // namespace cgen
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
