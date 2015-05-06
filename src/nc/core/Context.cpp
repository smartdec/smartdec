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

#include <QFile>
#include <QTextStream>

#include <nc/common/Foreach.h>
#include <nc/common/Exception.h>

#include <nc/core/Module.h>
#include <nc/core/UniversalAnalyzer.h>
#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/Instructions.h>
#include <nc/core/arch/disasm/Disassembler.h>
#include <nc/core/image/Image.h>
#include <nc/core/input/Parser.h>
#include <nc/core/input/ParserRepository.h>
#include <nc/core/ir/Functions.h>
#include <nc/core/ir/Program.h>
#include <nc/core/ir/calls/CallingConventionDetector.h>
#include <nc/core/ir/calls/CallsData.h>
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

void Context::setCallsData(std::unique_ptr<ir::calls::CallsData> callsData) {
    assert(callsData);
    assert(!callsData_);
    callsData_ = std::move(callsData);
}

void Context::setCallingConventionDetector(std::unique_ptr<ir::calls::CallingConventionDetector> detector) {
    assert(detector);
    assert(!callingConventionDetector_);
    callingConventionDetector_ = std::move(detector);
}

void Context::setTermToFunction(std::unique_ptr<ir::misc::TermToFunction> termToFunction) {
    assert(termToFunction);
    assert(!termToFunction_);
    termToFunction_ = std::move(termToFunction);
}

void Context::setDataflow(const ir::Function *function, std::unique_ptr<ir::dflow::Dataflow> dataflow) {
    assert(function);
    assert(dataflow);
    auto &entry = dataflows_[function];
    assert(!entry);
    entry = std::move(dataflow);
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

void Context::parse(const QString &filename) {
    // TODO: move to ParserRepository

    QFile source(filename);

    if (!source.open(QIODevice::ReadOnly)) {
        throw nc::Exception(tr("Could not open file \"%1\" for reading.").arg(filename));
    }

    logToken() << tr("Choosing a parser for %1...").arg(filename);

    input::Parser *suitableParser = NULL;

    foreach(input::Parser *parser, input::ParserRepository::instance()->parsers()) {
        logToken() << tr("Trying %1 parser...").arg(parser->name());
        if (parser->canParse(&source)) {
            suitableParser = parser;
            break;
        }
    }

    if (!suitableParser) {
        throw nc::Exception(tr("File %1 has unknown format.").arg(filename));
    }

    logToken() << tr("Parsing using %1 parser...").arg(suitableParser->name());

    suitableParser->parse(&source, module().get());

    logToken() << tr("Parsing completed.");
}

void Context::disassemble() {
    logToken() << tr("Disassembling code sections...");

    foreach (const image::Section *section, module()->image()->sections()) {
        if (section->isCode()) {
            disassemble(section);
        }
    }
}

void Context::disassemble(const image::Section *section) {
    assert(section != NULL);

    logToken() << tr("Disassembling section %1...").arg(section->name());

    disassemble(section, section->addr(), section->addr() + section->size());
}

void Context::disassemble(const image::ByteSource *source, ByteAddr begin, ByteAddr end) {
    assert(source != NULL);

    logToken() << tr("Disassembling addresses from 0x%2 to 0x%3...").arg(begin, 0, 16).arg(end, 0, 16);

    auto newInstructions = std::make_shared<arch::Instructions>(*instructions());

    arch::disasm::Disassembler disassembler(module()->architecture(), newInstructions.get());
    disassembler.disassemble(source, begin, end, cancellationToken());

    setInstructions(newInstructions);
}

void Context::decompile() {
    if (instructions()->all().empty()) {
        disassemble();
    }
    module()->architecture()->universalAnalyzer()->decompile(this);
}

} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
