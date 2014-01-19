/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "Signature.h"

#include <nc/config.h>

#include <nc/core/ir/Term.h>

#include <memory>

namespace nc {
namespace core {
namespace ir {
namespace calling {

Signature::Signature(): variadic_(false) {}

Signature::~Signature() {}

void Signature::addComment(QString text) {
    if (text.trimmed().isEmpty()) {
        return;
    } else if (comment_.isEmpty()) {
        comment_ = std::move(text);
    } else {
        comment_.append('\n');
        comment_.append(std::move(text));
    }
}

void Signature::addArgument(std::unique_ptr<Term> term) {
    assert(term != NULL);
    arguments_.push_back(std::move(term));
}

void Signature::setReturnValue(std::unique_ptr<Term> term) {
    assert(term != NULL);
    returnValue_ = std::move(term);
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
