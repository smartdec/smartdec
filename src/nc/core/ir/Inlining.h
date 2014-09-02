/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

namespace nc { namespace core { namespace ir {

class Call;
class Function;

/**
 * Inlines a function to the place of the call.
 *
 * \param function Valid pointer to the function being inlined.
 * \param call Valid pointer to the call to which the function must be inlined.
 */
void inlineFunction(const Function *function, Call *call);

}}} // namespace nc::core::ir

/* vim:set et sts=4 sw=4: */
