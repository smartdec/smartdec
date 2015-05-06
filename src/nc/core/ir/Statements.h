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

#include <memory>
#include <vector>

#include <QString>

#include "Statement.h"
#include "Term.h"

namespace nc {
namespace core {
namespace ir {

/**
 * Statement doing really nothing, but capable of delivering some user-defined text.
 * Useful mainly for debugging.
 */
class Comment: public Statement {
    QString text_; ///< The comment.

public:
    /**
     * Constructor.
     *
     * \param[in] text                 The comment.
     */
    Comment(const QString &text): Statement(COMMENT), text_(text) {}

    /**
     * \return                         The comment.
     */
    const QString &text() const { return text_; }

    virtual void print(QTextStream &out) const override;

protected:
    virtual Comment *doClone() const override;
};

class InlineAssembly: public Statement {
    public:

    /**
     * Constructor.
     */
    InlineAssembly(): Statement(INLINE_ASSEMBLY) {}

    virtual void print(QTextStream &out) const override;

protected:
    virtual InlineAssembly *doClone() const override;
};

/**
 * Assignment statement.
 */
class Assignment: public Statement {
    std::unique_ptr<Term> left_; ///< Term being the left-hand side.
    std::unique_ptr<Term> right_; ///< Term being the right-hand side.

public:
    /**
     * Class constructor.
     *
     * \param[in] left                 Valid pointer to the term being the left-hand side.
     * \param[in] right                Valid pointer to the term being the right-hand side.
     *
     * Arguments have to have the same size. Use UnaryOperator for size conversions.
     */
    Assignment(std::unique_ptr<Term> left, std::unique_ptr<Term> right);

    /**
     * \return                         Valid pointer to the term being the left-hand side.
     */
    Term *left() { return left_.get(); }

    /**
     * \return                         Valid pointer to the term being the left-hand side.
     */
    const Term *left() const { return left_.get(); }

    /**
     * \return                         Valid pointer to the term being the right-hand side.
     */
    Term *right() { return right_.get(); }

    /**
     * \return                         Valid pointer to the term being the right-hand side.
     */
    const Term *right() const { return right_.get(); }

    virtual void visitChildTerms(Visitor<Term> &visitor) override;
    virtual void visitChildTerms(Visitor<const Term> &visitor) const override;

    virtual void print(QTextStream &out) const override;

protected:
    virtual Assignment *doClone() const override;
};

/**
 * Statement killing reaching definitions of a term.
 */
class Kill: public Statement {
    std::unique_ptr<Term> term_; ///< Term whose definitions are killed.

public:
    /**
     * Class constructor.
     *
     * \param[in] term                 Valid pointer to the term whose definitions are killed.
     */
    Kill(std::unique_ptr<Term> term);

    /**
     * \return                         Valid pointer to the term whose definitions are killed.
     */
    Term *term() const { return term_.get(); }

    virtual void visitChildTerms(Visitor<Term> &visitor) override;
    virtual void visitChildTerms(Visitor<const Term> &visitor) const override;

    virtual void print(QTextStream &out) const override;

protected:
    virtual Kill *doClone() const override;
};

/**
 * Function call statement.
 */
class Call: public Statement {
    std::unique_ptr<Term> target_; ///< Call target (function address).

public:
    /**
     * Class constructor.
     *
     * \param[in] target    Call target (function address).
     */
    Call(std::unique_ptr<Term> target);

    /**
     * \return Call target (function address).
     */
    Term *target() { return target_.get(); }

    /**
     * \return Call target (function address).
     */
    const Term *target() const { return target_.get(); }

    virtual void visitChildTerms(Visitor<Term> &visitor) override;
    virtual void visitChildTerms(Visitor<const Term> &visitor) const override;

    virtual void print(QTextStream &out) const override;

protected:
    virtual Call *doClone() const override { return new Call(target()->clone()); }
};

/**
 * Return from a function statement.
 */
class Return: public Statement {
public:
    /**
     * Default constructor.
     */
    Return(): Statement(RETURN) {}

    virtual void print(QTextStream &out) const override;

protected:
    virtual Return *doClone() const override;
};


/*
 * Statement implementation follows.
 */

const Comment *Statement::asComment() const {
    return as<Comment>();
}

const Assignment *Statement::asAssignment() const {
    return as<Assignment>();
}

const Kill *Statement::asKill() const {
    return as<Kill>();
}

const Call *Statement::asCall() const {
    return as<Call>();
}

const Return *Statement::asReturn() const {
    return as<Return>();
}

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
