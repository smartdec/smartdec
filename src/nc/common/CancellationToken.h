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

#include <memory> /* std::shared_ptr */

namespace nc {

/**
 * Class for propagating cancellation notifications.
 */
class CancellationToken {
    /** Flag whether the cancellation is requested. */
    std::shared_ptr<volatile bool> cancellationRequested_;

    public:

    /**
     * Creates a not canceled token.
     */
    CancellationToken():
        cancellationRequested_(std::make_shared<volatile bool>(false))
    {}

    /**
     * Notifies the token and all its copies about the cancellation.
     */
    void cancel() { *cancellationRequested_ = true; }

    /**
     * \return True if the cancellation is requested and false otherwise.
     */
    bool cancellationRequested() const
#ifdef NC_USE_THREADS
    { return *cancellationRequested_; }
#else
    ;
#endif

    /**
     * \return Non-zero pointer of cancellation is requested, NULL otherwise.
     */
    operator const void *() const { return cancellationRequested() ? this : NULL; }
};

} // namespace nc

/* vim:set et sts=4 sw=4: */
