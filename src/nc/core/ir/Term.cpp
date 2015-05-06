//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "Term.h"

#include "Statement.h"

namespace nc { namespace core { namespace ir {

namespace {

class SetStatementVisitor: public Visitor<Term> {
    const Statement *statement_;

    public:

    SetStatementVisitor(const Statement *statement): statement_(statement) {}

    void operator()(Term *term) override {
        term->setStatement(statement_);
        term->visitChildTerms(*this);
    }
};

} // anonymous namespace

void Term::setStatementRecursively(const Statement *statement) {
    SetStatementVisitor visitor(statement);
    visitor(this);
}

void Term::visitChildTerms(Visitor<Term> & /*visitor*/) {
    /* Nothing to do */
} 

void Term::visitChildTerms(Visitor<const Term> & /*visitor*/) const {
    /* Nothing to do */
} 

}}} // namespace nc::core::ir

/* vim:set et sts=4 sw=4: */
