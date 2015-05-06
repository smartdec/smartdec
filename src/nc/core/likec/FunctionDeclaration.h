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
#include <memory> /* unique_ptr */

#include "Commentable.h"
#include "Declaration.h"
#include "FunctionPointerType.h"
#include "ArgumentDeclaration.h"

namespace nc {
namespace core {
namespace likec {

/**
 * Function declaration.
 */
class FunctionDeclaration: public Declaration, public Commentable {
    std::unique_ptr<FunctionPointerType> type_; ///< Type of pointer to this function.
    std::vector<std::unique_ptr<ArgumentDeclaration> > arguments_; ///< Function arguments.

public:
    /**
     * Class constructor.
     *
     * \param[in] tree Owning tree.
     * \param[in] identifier Function name.
     * \param[in] returnType Function return type.
     * \param[in] variadic Whether function has variable number of arguments.
     */
    FunctionDeclaration(Tree &tree, const QString &identifier, const Type *returnType = 0, bool variadic = false);

    protected:

    /**
     * Class constructor having declaration kind argument used by FunctionDefinition.
     *
     * \param[in] tree Owning tree.
     * \param[in] declarationKind Declaration kind.
     * \param[in] identifier Function name.
     * \param[in] returnType Function return type.
     * \param[in] variadic Whether function has variable number of arguments.
     */
    FunctionDeclaration(Tree &tree, int declarationKind, const QString &identifier, const Type *returnType = 0, bool variadic = false);

    public:

    /**
     * \return Type of pointer to this function.
     */
    const FunctionPointerType *type() const { return type_.get(); }

    /**
     * Function arguments.
     */
    const std::vector<std::unique_ptr<ArgumentDeclaration> > &arguments() const { return arguments_; }

    /**
     * Adds argument to the function.
     */
    void addArgument(ArgumentDeclaration *argument);

    virtual void visitChildNodes(Visitor<TreeNode> &visitor) override;

    virtual FunctionDeclaration *rewrite() override { return this; }

    virtual void doPrint(PrintContext &context) const override;
};

} // namespace likec
} // namespace core
} // namespace nc

NC_REGISTER_CLASS_KIND(nc::core::likec::Declaration, nc::core::likec::FunctionDeclaration, nc::core::likec::Declaration::FUNCTION_DECLARATION)

/* vim:set et sts=4 sw=4: */
