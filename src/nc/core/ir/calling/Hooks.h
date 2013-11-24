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

#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>

#include <nc/common/Types.h>

#include "CalleeId.h"

namespace nc {
namespace core {
namespace ir {

class Call;
class Function;
class Return;

namespace calling {

class CallHook;
class Convention;
class Conventions;
class EntryHook;
class ReturnHook;
class Signatures;

/**
 * Calling conventions hooks.
 */
class Hooks {
    /** Assigned calling conventions. */
    const Conventions &conventions_;

    /** Signatures of functions. */
    const Signatures &signatures_;

public:
    /** Type for the calling convention detector callback. */
    typedef std::function<void(const CalleeId &)> ConventionDetector;

private:
    /** Calling convention detector. */
    ConventionDetector conventionDetector_;

    /** Mapping from a call to its destination address. */
    boost::unordered_map<const Call *, ByteAddr> call2address_;

public:
    /** All hooks of a certain callee id. */
    struct CalleeHooks {
        /** Mapping from a function to its entry hook. */
        boost::unordered_map<const Function *, std::unique_ptr<EntryHook>> entryHooks;

        /** Mapping from a return statement to its return hook. */
        boost::unordered_map<const Return *, std::unique_ptr<ReturnHook>> returnHooks;

        /** Mapping from a call statement to its call hook. */
        boost::unordered_map<const Call *, std::unique_ptr<CallHook>> callHooks;
    };

private:
    /** Hooks per callee id. */
    boost::unordered_map<CalleeId, CalleeHooks> calleeHooks_;

public:
    /**
     * Constructor.
     *
     * \param conventions Assigned calling conventions.
     * \param signatures Known signatures of functions.
     */
    Hooks(const Conventions &conventions, const Signatures &signatures);

    /**
     * Destructor.
     */
    ~Hooks();

    /**
     * \return Assigned calling conventions.
     */
    const Conventions &conventions() const { return conventions_; }

    /**
     * Sets the function being called when a calling convention for a particular
     * callee is requested, but currently unknown. It is assumed that this function
     * will detect this convention and modify Conventions object passed to the
     * constructor of this Hooks object.
     *
     * \param detector Calling convention detector.
     */
    void setConventionDetector(ConventionDetector detector) {
        conventionDetector_ = std::move(detector);
    }

    /**
     * \param function Valid pointer to a function.
     *
     * \return Id of the function.
     */
    CalleeId getCalleeId(const Function *function) const;

    /**
     * \param call Valid pointer to a call.
     *
     * \return Id of the function called.
     */
    CalleeId getCalleeId(const Call *call) const;

    /**
     * \param call Valid pointer to a Call instance.
     *
     * \return Address this call is a call to.
     */
    boost::optional<ByteAddr> getCalledAddress(const Call *call) const;

    /**
     * Sets the destination address of a call.
     *
     * \param call Valid pointer to a Call instance.
     * \param addr New destination address of the call.
     */
    void setCalledAddress(const Call *call, boost::optional<ByteAddr> addr);

    /**
     * \param calleeId Callee id.
     *
     * \return Pointer to the calling convention used for calls to given address. Can be NULL.
     */
    const Convention *getConvention(const CalleeId &calleeId);

    /**
     * \param function Valid pointer to a function.
     *
     * \return Pointer to a EntryHook instance for this function.
     * Can be NULL. Such instance is created when necessary and if possible.
     */
    EntryHook *getEntryHook(const Function *function);

    /**
     * \param function Valid pointer to a function.
     * \param ret Valid pointer to a return statement.
     *
     * \return Pointer to a ReturnHook instance for this return statement.
     * Can be NULL. Such instance is created when necessary and if possible.
     */
    ReturnHook *getReturnHook(const Function *function, const Return *ret);

    /**
     * \param call Valid pointer to a call statement.
     *
     * \return Pointer to a CallHook instance for this call statement.
     * Can be NULL. Such instance is created when necessary and if possible.
     */
    CallHook *getCallHook(const Call *call);

    /**
     * \return Mapping from a callee id to its hooks.
     */
    const boost::unordered_map<CalleeId, CalleeHooks> &calleeHooks() const { return calleeHooks_; }
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
