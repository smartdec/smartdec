/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

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

#include "Context.h"

#include <cstdint> /* For std::uintptr_t. */

#include <nc/common/Foreach.h>

#include <nc/core/Module.h>
#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/Instructions.h>
#include <nc/core/ir/Functions.h>
#include <nc/core/ir/Program.h>
#include <nc/core/ir/calling/Conventions.h>
#include <nc/core/ir/calling/Hooks.h>
#include <nc/core/ir/calling/Signatures.h>
#include <nc/core/ir/cflow/Graph.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/misc/TermToFunction.h>
#include <nc/core/ir/types/Types.h>
#include <nc/core/ir/usage/Usage.h>
#include <nc/core/ir/vars/Variables.h>
#include <nc/core/likec/Tree.h>

namespace nc {
namespace core {

Context::Context():
    module_(std::make_shared<Module>()),
    instructions_(std::make_shared<const arch::Instructions>())
{}

Context::~Context() {}

void Context::setModule(const std::shared_ptr<Module> &module) {
    assert(module);
    module_ = module;
}

void Context::setInstructions(const std::shared_ptr<const arch::Instructions> &instructions) {
    assert(instructions);
    instructions_ = instructions;
    Q_EMIT instructionsChanged();
}

void Context::setProgram(std::unique_ptr<ir::Program> program) {
    assert(program);
    assert(!program_);
    program_ = std::move(program);
}

void Context::setFunctions(std::unique_ptr<ir::Functions> functions) {
    assert(functions);
    assert(!functions_);
    functions_ = std::move(functions);
}

void Context::setConventions(std::unique_ptr<ir::calling::Conventions> conventions) {
    conventions_ = std::move(conventions);
}

void Context::setHooks(std::unique_ptr<ir::calling::Hooks> hooks) {
    assert(hooks);
    assert(!hooks_);
    hooks_ = std::move(hooks);
}

void Context::setSignatures(std::unique_ptr<ir::calling::Signatures> signatures) {
    assert(signatures);
    assert(!signatures_);
    signatures_ = std::move(signatures);
}

void Context::setDataflow(const ir::Function *function, std::unique_ptr<ir::dflow::Dataflow> dataflow) {
    assert(function);
    assert(dataflow);
    dataflows_[function] = std::move(dataflow);
}

const ir::dflow::Dataflow *Context::getDataflow(const ir::Function *function) const {
    return nc::find(dataflows_, function).get();
}

void Context::setUsage(const ir::Function *function, std::unique_ptr<ir::usage::Usage> usage) {
    assert(function);
    assert(usage);
    auto &entry = usages_[function];
    assert(!entry);
    entry = std::move(usage);
}

const ir::usage::Usage *Context::getUsage(const ir::Function *function) const {
    return nc::find(usages_, function).get();
}

void Context::setTypes(const ir::Function *function, std::unique_ptr<ir::types::Types> types) {
    assert(function);
    assert(types);
    auto &entry = types_[function];
    assert(!entry);
    entry = std::move(types);
}

const ir::types::Types *Context::getTypes(const ir::Function *function) const {
    return nc::find(types_, function).get();
}

void Context::setVariables(const ir::Function *function, std::unique_ptr<ir::vars::Variables> variables) {
    assert(function);
    assert(variables);
    auto &entry = variables_[function];
    assert(!entry);
    entry = std::move(variables);
}

const ir::vars::Variables *Context::getVariables(const ir::Function *function) const {
    return nc::find(variables_, function).get();
}

void Context::setRegionGraph(const ir::Function *function, std::unique_ptr<ir::cflow::Graph> graph) {
    assert(function);
    assert(graph);
    auto &entry = regionGraphs_[function];
    assert(!entry);
    entry = std::move(graph);
}

const ir::cflow::Graph *Context::getRegionGraph(const ir::Function *function) const {
    return nc::find(regionGraphs_, function).get();
}

void Context::setTree(std::unique_ptr<likec::Tree> tree) {
    assert(tree);
    assert(!tree_);
    tree_ = std::move(tree);
    Q_EMIT treeChanged();
}

void Context::setTermToFunction(std::unique_ptr<ir::misc::TermToFunction> termToFunction) {
    assert(termToFunction);
    assert(!termToFunction_);
    termToFunction_ = std::move(termToFunction);
}

} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
