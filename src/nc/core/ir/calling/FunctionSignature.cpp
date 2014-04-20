/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "FunctionSignature.h"

namespace nc {
namespace core {
namespace ir {
namespace calling {

void FunctionSignature::addComment(QString text) {
    if (text.trimmed().isEmpty()) {
        return;
    } else if (comment_.isEmpty()) {
        comment_ = std::move(text);
    } else {
        comment_.append('\n');
        comment_.append(std::move(text));
    }
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
