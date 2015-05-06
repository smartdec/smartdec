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

#include <vector>

#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>

#include <nc/common/Types.h>

#include "FunctionDescriptor.h"

namespace nc {
namespace core {
namespace ir {

class Call;
class Function;
class Return;

namespace calls {

class CallAnalyzer;
class CallingConvention;
class CallingConventionDetector;
class DescriptorAnalyzer;
class FunctionAnalyzer;
class FunctionSignature;
class ReturnAnalyzer;

/**
 * Information about how functions call each other.
 */
class CallsData {
    /** Detector of calling conventions. */
    const CallingConventionDetector *callingConventionDetector_;

    /** Mapping from a call to its destination address. */
    boost::unordered_map<const Call *, ByteAddr> call2address_;

    /** Mapping from a function's descriptor to the associated calling convention. */
    boost::unordered_map<FunctionDescriptor, const CallingConvention *> descriptor2convention_;

    /** Mapping from a function's descriptor to the associated address analyzer. */
    boost::unordered_map<FunctionDescriptor, std::unique_ptr<DescriptorAnalyzer>> descriptor2analyzer_;

    /** Mapping from a function to its analyzer. */
    boost::unordered_map<std::pair<FunctionDescriptor, const Function *>, std::unique_ptr<FunctionAnalyzer>> function2analyzer_;

    /** Mapping from a call to its analyzer. */
    boost::unordered_map<std::pair<FunctionDescriptor, const Call *>, std::unique_ptr<CallAnalyzer>> call2analyzer_;

    /** Mapping from a return to its analyzer. */
    boost::unordered_map<std::pair<FunctionDescriptor, const Return *>, std::unique_ptr<ReturnAnalyzer>> return2analyzer_;

    /** Mapping from a function's descriptor to its signature. */
    boost::unordered_map<FunctionDescriptor, std::unique_ptr<FunctionSignature>> descriptor2signature_;

    public:

    /**
     * Constructor.
     */
    CallsData();

    /**
     * Destructor.
     */
    ~CallsData();

    /**
     * \return Pointer to the calling convention detector. Can be NULL.
     */
    const CallingConventionDetector *callingConventionDetector() const { return callingConventionDetector_; }

    /**
     * Sets a calling convention detector.
     *
     * \param detector Pointer to the new calling convention detector. Can be NULL.
     */
    void setCallingConventionDetector(const CallingConventionDetector *detector) { callingConventionDetector_ = detector; }

    /**
     * \param function Valid pointer to a function.
     *
     * \return Function's descriptor.
     */
    FunctionDescriptor getDescriptor(const Function *function) const;

    /**
     * \param call Valid pointer to a call.
     *
     * \return Descriptor of the called function.
     */
    FunctionDescriptor getDescriptor(const Call *call) const;

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
    void setCalledAddress(const Call *call, ByteAddr addr);

    /**
     * Sets function's calling convention.
     *
     * \param descriptor Function's descriptor.
     * \param convention Pointer to a calling convention of the function. Can be NULL.
     */
    void setCallingConvention(const FunctionDescriptor &descriptor, const CallingConvention *convention);

    /**
     * \param descriptor Function's descriptor.
     *
     * \return Pointer to the calling convention used for calls to given address. Can be NULL.
     */
    const CallingConvention *getCallingConvention(const FunctionDescriptor &descriptor);

    /**
     * \param descriptor Function's descriptor.
     *
     * \return Pointer to the associated address analyzer. Can be NULL.
     */
    DescriptorAnalyzer *getDescriptorAnalyzer(const FunctionDescriptor &descriptor);

    /**
     * \param function Valid pointer to a function.
     *
     * \return Pointer to a FunctionAnalyzer instance for this function.
     * Can be NULL. Such instance is created when necessary and if possible.
     */
    FunctionAnalyzer *getFunctionAnalyzer(const Function *function);

    /**
     * \param call Valid pointer to a call statement.
     *
     * \return Pointer to a CallAnalyzer instance for this call statement.
     * Can be NULL. Such instance is created when necessary and if possible.
     */
    CallAnalyzer *getCallAnalyzer(const Call *call);

    /**
     * \param function Valid pointer to a function.
     * \param ret Valid pointer to a return statement.
     *
     * \return Pointer to a ReturnAnalyzer instance for this return statement.
     * Can be NULL. Such instance is created when necessary and if possible.
     */
    ReturnAnalyzer *getReturnAnalyzer(const Function *function, const Return *ret);

    /**
     * \return Pointer to the signature of the function with given address. Can be NULL.
     *
     * \param descriptor Descriptor of a function.
     */
    const FunctionSignature *getFunctionSignature(const FunctionDescriptor &descriptor);

    /**
     * \return Pointer to the signature of the function. Can be NULL.
     *
     * \param function Valid pointer to the function.
     */
    const FunctionSignature *getFunctionSignature(const Function *function);

    /**
     * \return Pointer to the signature of the function being called. Can be NULL.
     *
     * \param call Valid pointer to the call.
     */
    const FunctionSignature *getFunctionSignature(const Call *call);

    /**
     * \param function Valid pointer to a function.
     *
     * \return List of all Return statements in the function.
     */
    std::vector<const Return *> getReturns(const Function *function) const;
};

} // namespace calls
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
