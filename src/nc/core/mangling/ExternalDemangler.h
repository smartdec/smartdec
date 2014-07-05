/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <QString>
#include <QStringList>

#include "Demangler.h"

namespace nc {
namespace core {
namespace mangling {

/**
 * Demangler using an external program.
 */
class ExternalDemangler: public Demangler {
    /** Demangler program. */
    QString program_;

    /** Arguments for the demangler program. */
    QStringList arguments_;

public:
    /**
     * Constructor.
     *
     * \param program Demangler executable.
     * \param arguments Demangler arguments.
     */
    ExternalDemangler(QString program, QStringList arguments = QStringList());

    /**
     * Destructor.
     *
     * Kills the demangler process.
     */
    ~ExternalDemangler() override;

    virtual QString demangle(const QString &symbol) const override;
};

}}} // namespace nc::core::mangling

/* vim:set et sts=4 sw=4: */
