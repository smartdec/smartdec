/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <QString>

#include <memory> /* std::unique_ptr */

namespace nc {
namespace core {
namespace ir {
namespace calls {

class DescriptorAnalyzer;

/**
 * An interface for a factory of address analyzers.
 */
class CallingConvention {
    QString name_; ///< Name of the calling convention.

    public:

    /**
     * Constructor.
     *
     * \paran name Name of the calling convention.
     */
    CallingConvention(QString name): name_(std::move(name)) {}

    /**
     * Virtual destructor.
     */
    virtual ~CallingConvention() {}

    /**
     * \return Name of the calling convention.
     */
    const QString &name() const { return name_; }

    /**
     * \return Pointer to a new instance of DescriptorAnalyzer. Can be NULL.
     */
    virtual std::unique_ptr<DescriptorAnalyzer> createDescriptorAnalyzer() const = 0;
};

} // namespace calls
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
