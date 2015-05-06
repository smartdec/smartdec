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

#include "UniversalAnalyzer.h"

#include <QObject> /* For QObject::tr() */

#include <cstdint> /* uintptr_t */

#include <nc/common/Foreach.h>

#include <nc/core/Context.h>
#include <nc/core/Module.h>
#include <nc/core/arch/irgen/IRGenerator.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Functions.h>
#include <nc/core/ir/FunctionsGenerator.h>
#include <nc/core/ir/Program.h>
#include <nc/core/ir/calls/CallingConventionDetector.h>
#include <nc/core/ir/calls/CallsData.h>
#include <nc/core/ir/cflow/Graph.h>
#include <nc/core/ir/cflow/GraphBuilder.h>
#include <nc/core/ir/cflow/StructureAnalyzer.h>
#include <nc/core/ir/cgen/CodeGenerator.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/DataflowAnalyzer.h>
#include <nc/core/ir/misc/TermToFunction.h>
#include <nc/core/ir/types/TypeAnalyzer.h>
#include <nc/core/ir/types/Types.h>
#include <nc/core/ir/usage/Usage.h>
#include <nc/core/ir/usage/UsageAnalyzer.h>
#include <nc/core/ir/vars/VariableAnalyzer.h>
#include <nc/core/ir/vars/Variables.h>
#include <nc/core/likec/Tree.h>
#include <nc/core/mangling/Demangler.h>

#ifdef NC_TREE_CHECKS
#include <nc/core/ir/misc/CensusVisitor.h>
#include <nc/core/likec/Expression.h>
#include <nc/core/likec/Statement.h>
#endif

namespace nc {
namespace core {

UniversalAnalyzer::~UniversalAnalyzer() {}

namespace {
    /* MSVC 2010 fails to find the type, if one defines it inside the function. */
    struct CancellationException {};
}

void UniversalAnalyzer::decompile(Context *context) const {
    auto checkForCancellation = [context]() { if (context->cancellationToken()) { throw CancellationException(); } };

    try {
        context->logToken() << QObject::tr("Creating the program IR...");
        createProgram(context);
        checkForCancellation();

        context->logToken() << QObject::tr("Creating functions...");
        createFunctions(context);
        checkForCancellation();

        context->logToken() << QObject::tr("Creating the calls data...");
        createCallsData(context);
        checkForCancellation();

        context->logToken() << QObject::tr("Computing term to function mapping...");
        computeTermToFunctionMapping(context);
        checkForCancellation();

        foreach (const ir::Function *function, context->functions()->functions()) {
            context->logToken() << QObject::tr("Running dataflow analysis on %1...").arg(function->name());
            analyzeDataflow(context, function);
            checkForCancellation();
        }

        foreach (const ir::Function *function, context->functions()->functions()) {
            context->logToken() << QObject::tr("Running structural analysis on %1...").arg(function->name());
            doStructuralAnalysis(context, function);
            checkForCancellation();

            context->logToken() << QObject::tr("Running liveness analysis on %1...").arg(function->name());
            computeUsage(context, function);
            checkForCancellation();

            context->logToken() << QObject::tr("Running type reconstruction on %1...").arg(function->name());
            reconstructTypes(context, function);
            checkForCancellation();

            context->logToken() << QObject::tr("Running reconstruction of variables on %1...").arg(function->name());
            reconstructVariables(context, function);
            checkForCancellation();
        }

        context->logToken() << QObject::tr("Generating AST...");
        generateTree(context);

#ifdef NC_TREE_CHECKS
        context->logToken() << QObject::tr("Checking AST...");
        checkTree(context);
#endif

        context->logToken() << QObject::tr("Decompilation completed.");
    } catch (const CancellationException &) {
        context->logToken() << QObject::tr("Decompilation canceled.");
    }
}

void UniversalAnalyzer::createProgram(Context *context) const {
    std::unique_ptr<ir::Program> program(new ir::Program());

    core::arch::irgen::IRGenerator generator(context->module().get(), context->instructions().get(), program.get());
    generator.generate(context->cancellationToken());

    context->setProgram(std::move(program));
}

void UniversalAnalyzer::createFunctions(Context *context) const {
    std::unique_ptr<ir::Functions> functions(new ir::Functions);
    ir::FunctionsGenerator generator;
    generator.makeFunctions(*context->program(), *functions);

    foreach (ir::Function *function, functions->functions()) {
        pickFunctionName(context, function);
    }

    context->setFunctions(std::move(functions));
}

void UniversalAnalyzer::pickFunctionName(Context *context, ir::Function *function) const {
    /* If the function has an entry, and the entry has an address... */
    if (function->entry()&& function->entry()->address()) {
        QString name = context->module()->getName(*function->entry()->address());

        if (!name.isEmpty()) {
            /* Take the name of the corresponding symbol, if possible. */
            QString cleanName = likec::Tree::cleanName(name);
            function->setName(cleanName);

            if (name != cleanName) {
                function->comment().append(name);
            }

            QString demangledName = context->module()->demangler()->demangle(name);
            if (demangledName.contains('(')) {
                /* What we demangled has really something to do with a function. */
                function->comment().append(demangledName);
            }
        } else {
            /* Invent a name based on the entry address. */
            function->setName(QString("func_%1").arg(*function->entry()->address(), 0, 16));
        }
    } else {
        /* If there are no other options, invent some random unique name. */
        function->setName(QString("func_noentry_%1").arg(reinterpret_cast<std::uintptr_t>(function), 0, 16));
    }
}

void UniversalAnalyzer::createCallsData(Context *context) const {
    std::unique_ptr<ir::calls::CallsData> callsData(new ir::calls::CallsData());

    class Detector: public ir::calls::CallingConventionDetector {
        const UniversalAnalyzer *universalAnalyzer_;
        Context *context_;

        public:

        Detector(const UniversalAnalyzer *universalAnalyzer, Context *context):
            universalAnalyzer_(universalAnalyzer), context_(context)
        {}

        virtual void detectCallingConvention(const ir::calls::FunctionDescriptor &descriptor) const override {
            universalAnalyzer_->detectCallingConvention(context_, descriptor);
        }
    };

    std::unique_ptr<ir::calls::CallingConventionDetector> detector(new Detector(this, context));
    callsData->setCallingConventionDetector(detector.get());

    context->setCallsData(std::move(callsData));
    context->setCallingConventionDetector(std::move(detector));
}

void UniversalAnalyzer::detectCallingConvention(Context * /*context*/, const ir::calls::FunctionDescriptor &/*descriptor*/) const {
    /* Nothing to do. */
}

void UniversalAnalyzer::computeTermToFunctionMapping(Context *context) const {
    context->setTermToFunction(std::unique_ptr<ir::misc::TermToFunction>(
        new ir::misc::TermToFunction(context->functions(), context->callsData())));
}

void UniversalAnalyzer::analyzeDataflow(Context *context, const ir::Function *function) const {
    std::unique_ptr<ir::dflow::Dataflow> dataflow(new ir::dflow::Dataflow());

    ir::dflow::DataflowAnalyzer analyzer(*dataflow, context->module()->architecture(), context->callsData());
    analyzer.analyze(function, context->cancellationToken());

    context->setDataflow(function, std::move(dataflow));
}

void UniversalAnalyzer::computeUsage(Context *context, const ir::Function *function) const {
    std::unique_ptr<ir::usage::Usage> usage(new ir::usage::Usage());

    ir::usage::UsageAnalyzer(*usage, function,
        context->getDataflow(function), context->module()->architecture(),
        context->getRegionGraph(function), context->callsData())
    .analyze();

    context->setUsage(function, std::move(usage));
}

void UniversalAnalyzer::reconstructTypes(Context *context, const ir::Function *function) const {
    std::unique_ptr<ir::types::Types> types(new ir::types::Types());

    ir::types::TypeAnalyzer analyzer(*types, *context->getDataflow(function), *context->getUsage(function), context->callsData());
    analyzer.analyze(function, context->cancellationToken());

    context->setTypes(function, std::move(types));
}

void UniversalAnalyzer::reconstructVariables(Context *context, const ir::Function *function) const {
    std::unique_ptr<ir::vars::Variables> variables(new ir::vars::Variables());

    ir::vars::VariableAnalyzer analyzer(*variables, *context->getDataflow(function), context->callsData());
    analyzer.analyze(function);

    context->setVariables(function, std::move(variables));
}

void UniversalAnalyzer::doStructuralAnalysis(Context *context, const ir::Function *function) const {
    std::unique_ptr<ir::cflow::Graph> graph(new ir::cflow::Graph());

    ir::cflow::GraphBuilder()(*graph, function);
    ir::cflow::StructureAnalyzer(*graph, *context->getDataflow(function)).analyze();

    context->setRegionGraph(function, std::move(graph));
}

void UniversalAnalyzer::generateTree(Context *context) const {
    std::unique_ptr<nc::core::likec::Tree> tree(new nc::core::likec::Tree());

    ir::cgen::CodeGenerator generator(*context, *tree);
    generator.makeCompilationUnit(context->cancellationToken());

    context->setTree(std::move(tree));
}

#ifdef NC_TREE_CHECKS
void UniversalAnalyzer::checkTree(Context *context) const {
    class TreeVisitor: public Visitor<likec::TreeNode> {
        boost::unordered_set<const ir::Statement *> statements_;
        boost::unordered_set<const ir::Term *> terms_;

        public:

        TreeVisitor(const ir::misc::CensusVisitor &visitor):
            statements_(visitor.statements().begin(), visitor.statements().end()),
            terms_(visitor.terms().begin(), visitor.terms().end())
        {}

        virtual void operator()(likec::TreeNode *node) override {
            if (const likec::Statement *statement = node->as<likec::Statement>()) {
                if (statement->statement()) {
                    assert(contains(statements_, statement->statement()));
                }
            } else if (const likec::Expression *expression = node->as<likec::Expression>()) {
                if (expression->term()) {
                    assert(contains(terms_, expression->term()));
                }

                auto type = expression->getType();
                assert(type != NULL);
#if 0
                // TODO: make decompiler not to emit warnings here
                if (type == expression->tree().makeErroneousType()) {
                    ncWarning(typeid(*node).name());
                }
#endif
            }
            node->visitChildNodes(*this);
        }
    };

    ir::misc::CensusVisitor visitor(context->callsData());
    foreach (const ir::Function *function, context->functions()->functions()) {
        visitor(function);
    }

    TreeVisitor checker(visitor);
    checker(context->tree()->root());
}
#endif

} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
