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

#include "MasterAnalyzer.h"

#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/core/Context.h>
#include <nc/core/arch/irgen/IRGenerator.h>
#include <nc/core/image/Image.h>
#include <nc/core/image/Symbols.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Functions.h>
#include <nc/core/ir/FunctionsGenerator.h>
#include <nc/core/ir/Program.h>
#include <nc/core/ir/calling/Conventions.h>
#include <nc/core/ir/calling/Hooks.h>
#include <nc/core/ir/calling/SignatureAnalyzer.h>
#include <nc/core/ir/calling/Signatures.h>
#include <nc/core/ir/cflow/Graphs.h>
#include <nc/core/ir/cflow/GraphBuilder.h>
#include <nc/core/ir/cflow/StructureAnalyzer.h>
#include <nc/core/ir/cgen/CodeGenerator.h>
#include <nc/core/ir/dflow/Dataflows.h>
#include <nc/core/ir/dflow/DataflowAnalyzer.h>
#include <nc/core/ir/liveness/Livenesses.h>
#include <nc/core/ir/liveness/LivenessAnalyzer.h>
#include <nc/core/ir/types/TypeAnalyzer.h>
#include <nc/core/ir/types/Types.h>
#include <nc/core/ir/vars/VariableAnalyzer.h>
#include <nc/core/ir/vars/Variables.h>
#include <nc/core/likec/Tree.h>
#include <nc/core/mangling/Demangler.h>

namespace nc {
namespace core {

MasterAnalyzer::~MasterAnalyzer() {}

void MasterAnalyzer::createProgram(Context &context) const {
    context.logToken() << tr("Creating intermediate representation of the program.");

    std::unique_ptr<ir::Program> program(new ir::Program());

    core::arch::irgen::IRGenerator(context.image().get(), context.instructions().get(), program.get())
        .generate(context.cancellationToken());

    context.setProgram(std::move(program));
}

void MasterAnalyzer::createFunctions(Context &context) const {
    context.logToken() << tr("Creating functions.");

    std::unique_ptr<ir::Functions> functions(new ir::Functions);

    ir::FunctionsGenerator().makeFunctions(*context.program(), *functions);

    context.setFunctions(std::move(functions));
}

void MasterAnalyzer::createHooks(Context &context) const {
    context.logToken() << tr("Creating hooks.");

    context.setSignatures(std::make_unique<ir::calling::Signatures>());
    context.setConventions(std::make_unique<ir::calling::Conventions>());
    context.setHooks(std::make_unique<ir::calling::Hooks>(*context.conventions(), *context.signatures()));

    context.hooks()->setConventionDetector([this, &context](const ir::calling::CalleeId &calleeId) {
        this->detectCallingConvention(context, calleeId);
    });
}

void MasterAnalyzer::detectCallingConvention(Context & /*context*/, const ir::calling::CalleeId &/*descriptor*/) const {
    /* Nothing to do. */
}

void MasterAnalyzer::dataflowAnalysis(Context &context) const {
    context.logToken() << tr("Dataflow analysis.");

    context.setDataflows(std::make_unique<ir::dflow::Dataflows>());

    foreach (auto function, context.functions()->list()) {
        dataflowAnalysis(context, function);
        context.cancellationToken().poll();
    }
}

void MasterAnalyzer::dataflowAnalysis(Context &context, ir::Function *function) const {
    context.logToken() << tr("Dataflow analysis of %1.").arg(getFunctionName(context, function));

    std::unique_ptr<ir::dflow::Dataflow> dataflow(new ir::dflow::Dataflow());

    context.hooks()->instrument(function, dataflow.get());

    ir::dflow::DataflowAnalyzer(*dataflow, context.image()->architecture())
        .analyze(function, context.cancellationToken());

    context.dataflows()->emplace(function, std::move(dataflow));
}

void MasterAnalyzer::reconstructSignatures(Context &context) const {
    context.logToken() << tr("Reconstructing function signatures.");

    ir::calling::SignatureAnalyzer(*context.signatures(), *context.image(), *context.functions(),
        *context.dataflows(), *context.hooks())
        .analyze(context.cancellationToken());
}

void MasterAnalyzer::reconstructVariables(Context &context) const {
    context.logToken() << tr("Reconstructing variables.");

    std::unique_ptr<ir::vars::Variables> variables(new ir::vars::Variables());

    ir::vars::VariableAnalyzer(*variables, *context.dataflows(), context.image()->architecture()).analyze();

    context.setVariables(std::move(variables));
}

void MasterAnalyzer::livenessAnalysis(Context &context) const {
    context.logToken() << tr("Liveness analysis.");

    context.setLivenesses(std::make_unique<ir::liveness::Livenesses>());

    foreach (const ir::Function *function, context.functions()->list()) {
        livenessAnalysis(context, function);
    }
}

void MasterAnalyzer::livenessAnalysis(Context &context, const ir::Function *function) const {
    context.logToken() << tr("Liveness analysis of %1.").arg(getFunctionName(context, function));

    std::unique_ptr<ir::liveness::Liveness> liveness(new ir::liveness::Liveness());

    ir::liveness::LivenessAnalyzer(*liveness, function,
        *context.dataflows()->at(function), context.image()->architecture(),
        *context.graphs()->at(function), *context.hooks(), *context.signatures())
    .analyze();

    context.livenesses()->emplace(function, std::move(liveness));
}

void MasterAnalyzer::reconstructTypes(Context &context) const {
    context.logToken() << tr("Reconstructing types.");

    std::unique_ptr<ir::types::Types> types(new ir::types::Types());

    ir::types::TypeAnalyzer(
        *types, *context.functions(), *context.dataflows(), *context.variables(),
        *context.livenesses(), *context.hooks(), *context.signatures())
    .analyze(context.cancellationToken());

    context.setTypes(std::move(types));
}

void MasterAnalyzer::structuralAnalysis(Context &context) const {
    context.logToken() << tr("Structural analysis.");

    context.setGraphs(std::make_unique<ir::cflow::Graphs>());

    foreach (auto function, context.functions()->list()) {
        structuralAnalysis(context, function);
        context.cancellationToken().poll();
    }
}

void MasterAnalyzer::structuralAnalysis(Context &context, const ir::Function *function) const {
    context.logToken() << tr("Structural analysis of %1.").arg(getFunctionName(context, function));

    std::unique_ptr<ir::cflow::Graph> graph(new ir::cflow::Graph());

    ir::cflow::GraphBuilder()(*graph, function);
    ir::cflow::StructureAnalyzer(*graph, *context.dataflows()->at(function)).analyze();

    context.graphs()->emplace(function, std::move(graph));
}

void MasterAnalyzer::generateTree(Context &context) const {
    context.logToken() << tr("Generating AST.");

    auto tree = std::make_unique<nc::core::likec::Tree>();

    ir::cgen::CodeGenerator(*tree, *context.image(), *context.functions(), *context.hooks(),
        *context.signatures(), *context.dataflows(), *context.variables(), *context.graphs(),
        *context.livenesses(), *context.types(), context.cancellationToken())
        .makeCompilationUnit();

    context.setTree(std::move(tree));
}

void MasterAnalyzer::decompile(Context &context) const {
    context.logToken() << tr("Decompiling.");

    createProgram(context);
    context.cancellationToken().poll();

    createFunctions(context);
    context.cancellationToken().poll();

    createHooks(context);
    context.cancellationToken().poll();

    dataflowAnalysis(context);
    context.cancellationToken().poll();

    reconstructSignatures(context);
    context.cancellationToken().poll();

    dataflowAnalysis(context);
    context.cancellationToken().poll();

    reconstructVariables(context);
    context.cancellationToken().poll();

    structuralAnalysis(context);
    context.cancellationToken().poll();

    livenessAnalysis(context);
    context.cancellationToken().poll();

    reconstructTypes(context);
    context.cancellationToken().poll();

    generateTree(context);
    context.cancellationToken().poll();

    context.logToken() << tr("Decompilation completed.");
}

QString MasterAnalyzer::getFunctionName(Context &context, const ir::Function *function) const {
    if (function->entry() && function->entry()->address()) {
        ir::calling::FunctionSignature *signature = NULL;
        if (context.signatures()) {
            signature = context.signatures()->getSignature(function).get();
        }
        if (signature) {
            return tr("function at address %1 (%2)").arg(*function->entry()->address(), 0, 16).arg(signature->name());
        } else {
            return tr("function at address %1").arg(*function->entry()->address(), 0, 16);
        }
    } else {
        return tr("function with unknown address");
    }
}

} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
