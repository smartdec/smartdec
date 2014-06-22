/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory>

#include "Demangler.h"

namespace nc {
namespace core {
namespace mangling {

/**
 * Demangler using bundled demangler.
 */
class BundledDemangler: public Demangler {
    std::unique_ptr<const Demangler> demangler_;

public:
    /**
     * Constructor.
     *
     * \param program Demangler executable.
     * \param arguments Demangler arguments.
     */
    BundledDemangler();

    virtual QString demangle(const QString &symbol) const override;
};

}}} // namespace nc::core::mangling

/* vim:set et sts=4 sw=4: */
