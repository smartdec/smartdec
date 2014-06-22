/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <QMutex>
#include <QProcess>
#include <QString>
#include <QStringList>

#include "Demangler.h"

namespace nc {
namespace core {
namespace mangling {

/**
 * Demangler using c++filt.
 */
class ExternalDemangler: public Demangler {
    /** Demangler program. */
    QString program_;

    /** Arguments for the demangler program. */
    QStringList arguments_;

    /** Demangler process. */
    mutable QProcess process_;

    /** Mutex for handling queries in parallel. */
    mutable QMutex mutex_;

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
    ~ExternalDemangler();

    virtual QString demangle(const QString &symbol) const override;
};

}}} // namespace nc::core::mangling

/* vim:set et sts=4 sw=4: */
