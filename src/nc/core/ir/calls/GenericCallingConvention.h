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

#include <cassert>
#include <vector>

#include <QString>

#include <nc/common/Types.h>

#include <nc/core/ir/MemoryLocation.h>

#include "CallingConvention.h"

namespace nc {
namespace core {

namespace arch {
    class RegisterOperand;
}

namespace ir {

class Statement;
class Term;

namespace calls {

/**
 * A set of locations where an argument can be stored.
 *
 * For example, first argument in AMD64 calling convention can be stored in rdi, edi or di registers.
 */
class Argument {
    std::vector<MemoryLocation> locations_; ///< The set of memory locations where an argument can be stored.

    public:

    /**
     * \return The set of memory locations where an argument can be stored.
     */
    const std::vector<MemoryLocation> &locations() const { return locations_; }

    /**
     * Adds given memory location to the list of locations where the argument can be stored.
     *
     * \param location                  Memory location.
     *
     * \return                          *this.
     */
    Argument &operator<<(const MemoryLocation &location) {
        locations_.push_back(location);
        return *this;
    }

    /**
     * Adds memory location of given register to the list of locations where the argument can be stored.
     *
     * \param reg                       Valid pointer to register operand.
     *
     * \return                          *this.
     */
    Argument &operator<<(const core::arch::RegisterOperand *reg);
};

/**
 * A group of arguments.
 *
 * For example, the AMD64 calling convention has two groups: integer/pointer arguments and floating-point arguments.
 */
class ArgumentGroup {
    QString name_; ///< Name of the group.
    std::vector<Argument> arguments_; ///< Arguments in the group.

    public:

    /**
     * Constructor.
     *
     * \param name                      Name of the group.
     */
    ArgumentGroup(const QString &name): name_(name) {}

    /**
     * \return Name of the group.
     */
    const QString &name() const { return name_; }

    /**
     * \return Arguments in the group.
     */
    const std::vector<Argument> &arguments() const { return arguments_; }

    /**
     * Adds an argument to the group.
     *
     * \param argument                  Argument to add.
     *
     * \return                          *this.
     */
    ArgumentGroup &operator<<(const Argument &argument) {
        arguments_.push_back(argument);
        return *this;
    }
};

/**
 * A typical calling convention using registers and stack for passing arguments.
 */
class GenericCallingConvention: public CallingConvention {
    MemoryLocation stackPointer_; ///< Memory location of stack pointer register.

    BitOffset firstArgumentOffset_; ///< Offset of the first argument in a function's stack frame.
    BitSize argumentAlignment_; ///< Alignment of stack arguments in bits.

    std::vector<ArgumentGroup> argumentGroups_; ///< Argument groups.
    std::vector<const Term *> returnValues_; ///< Terms where return values may be kept.

    bool calleeCleanup_; ///< Callee cleans up arguments.

    std::vector<const Statement *> entryStatements_; ///< Statements executed when a function is entered.

public:

    /**
     * Class constructor.
     */
    GenericCallingConvention():
        firstArgumentOffset_(0), argumentAlignment_(0), calleeCleanup_(false)
    {}

    /**
     * Virtual destructor.
     */
    virtual ~GenericCallingConvention();

    virtual std::unique_ptr<DescriptorAnalyzer> createDescriptorAnalyzer() const override;

    /**
     * \return MemoryLocation of stack pointer register.
     */
    const MemoryLocation &stackPointer() const { return stackPointer_; }

    /**
     * \return Offset of the first argument in a function's stack frame.
     */
    BitOffset firstArgumentOffset() const { return firstArgumentOffset_; }

    /**
     * \return Alignment of stack arguments in bits.
     */
    BitSize argumentAlignment() const { return argumentAlignment_; };

    /**
     * \return List of argument groups.
     */
    const std::vector<ArgumentGroup> &argumentGroups() const { return argumentGroups_; }

    /**
     * \return Factories for terms where return values may be kept.
     */
    const std::vector<const Term *> &returnValues() const { return returnValues_; }

    /**
     * \return True if callee cleans up arguments.
     */
    bool calleeCleanup() const { return calleeCleanup_; }
    
    /**
     * Statements executed when a function is entered.
     *
     * Such statements are typically used for setting various flags to the values
     * they are guaranteed to be set by calling convention, i.e. set Intel's direction
     * flag to zero.
     */
    const std::vector<const Statement *> &entryStatements() const { return entryStatements_; }

protected:

    /**
     * Sets MemoryLocation of stack pointer register.
     *
     * \param[in] location MemoryLocation.
     */
    void setStackPointer(const MemoryLocation &location) { stackPointer_ = location; }

    /**
     * Sets the offset of the first argument in a function's stack frame.
     *
     * \param firstArgumentOffset The offset.
     */
    void setFirstArgumentOffset(BitOffset firstArgumentOffset) { firstArgumentOffset_ = firstArgumentOffset; }

    /**
     * Sets alignment of stack arguments.
     *
     * \param[in] argumentAlignment Alignment in bits.
     */
    void setArgumentAlignment(BitSize argumentAlignment) { argumentAlignment_ = argumentAlignment; };

    /**
     * Adds memory location to the list of possible locations for last added argument position.
     *
     * \param[in] argumentGroup Memory location.
     */
    void addArgumentGroup(const ArgumentGroup &argumentGroup) { argumentGroups_.push_back(argumentGroup); }

    /**
     * Adds a term in which return values may be kept.
     *
     * \param term Valid pointer to a term.
     */
    void addReturnValue(std::unique_ptr<Term> term);

    /**
     * Sets whether callee cleans up arguments.
     *
     * \param[in] calleeCleanup Whether callee cleans up arguments.
     */
    void setCalleeCleanup(bool calleeCleanup) { calleeCleanup_ = calleeCleanup; }

    /**
     * Adds a statement executed when a function is entered.
     *
     * \param statement Valid pointer to a statement.
     */
    void addEnterStatement(std::unique_ptr<Statement> statement);
};

} // namespace calls
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
