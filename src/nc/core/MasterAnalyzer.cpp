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

#include <QObject> /* For QObject::tr() */

#include <cstdint> /* uintptr_t */

#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/core/Context.h>
#include <nc/core/Module.h>
#include <nc/core/arch/irgen/IRGenerator.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Functions.h>
#include <nc/core/ir/FunctionsGenerator.h>
#include <nc/core/ir/Program.h>
#include <nc/core/ir/calling/Conventions.h>
#include <nc/core/ir/calling/Hooks.h>
#include <nc/core/ir/calling/SignatureAnalyzer.h>
#include <nc/core/ir/calling/Signatures.h>
#include <nc/core/ir/cflow/Graph.h>
#include <nc/core/ir/cflow/GraphBuilder.h>
#include <nc/core/ir/cflow/StructureAnalyzer.h>
#include <nc/core/ir/cgen/CodeGenerator.h>
#include <nc/core/ir/dflow/Dataflows.h>
#include <nc/core/ir/dflow/DataflowAnalyzer.h>
#include <nc/core/ir/liveness/Liveness.h>
#include <nc/core/ir/liveness/LivenessAnalyzer.h>
#include <nc/core/ir/misc/TermToFunction.h>
#include <nc/core/ir/types/TypeAnalyzer.h>
#include <nc/core/ir/types/Types.h>
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

MasterAnalyzer::~MasterAnalyzer() {}

void MasterAnalyzer::createProgram(Context &context) const {
    std::unique_ptr<ir::Program> program(new ir::Program());

    core::arch::irgen::IRGenerator generator(context.module().get(), context.instructions().get(), program.get());
    generator.generate(context.cancellationToken());

    context.setProgram(std::move(program));
}

void MasterAnalyzer::createFunctions(Context &context) const {
    std::unique_ptr<ir::Functions> functions(new ir::Functions);
    ir::FunctionsGenerator generator;
    generator.makeFunctions(*context.program(), *functions);

    foreach (ir::Function *function, functions->functions()) {
        pickFunctionName(context, function);
    }

    context.setFunctions(std::move(functions));
}

void MasterAnalyzer::pickFunctionName(Context &context, ir::Function *function) const {
    /* If the function has an entry, and the entry has an address... */
    if (function->entry()&& function->entry()->address()) {
        QString name = context.module()->getName(*function->entry()->address());

        if (!name.isEmpty()) {
            /* Take the name of the corresponding symbol, if possible. */
            QString cleanName = likec::Tree::cleanName(name);
            function->setName(cleanName);

            if (name != cleanName) {
                function->comment().append(name);
            }

            QString demangledName = context.module()->demangler()->demangle(name);
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

void MasterAnalyzer::initializeHooks(Context &context) const {
    if (!context.signatures()) {
        context.setSignatures(std::make_unique<ir::calling::Signatures>());
    }
    if (!context.conventions()) {
        context.setConventions(std::make_unique<ir::calling::Conventions>());
    }

    context.setHooks(std::make_unique<ir::calling::Hooks>(*context.conventions(), *context.signatures()));
    context.hooks()->setConventionDetector([this, &context](const ir::calling::CalleeId &calleeId) {
        this->detectCallingConvention(context, calleeId);
    });
}

void MasterAnalyzer::detectCallingConvention(Context & /*context*/, const ir::calling::CalleeId &/*descriptor*/) const {
    /* Nothing to do. */
}

void MasterAnalyzer::computeTermToFunctionMapping(Context &context) const {
    context.setTermToFunction(std::unique_ptr<ir::misc::TermToFunction>(
        new ir::misc::TermToFunction(context.functions(), context.hooks())));
}

void MasterAnalyzer::analyzeDataflow(Context &context, const ir::Function *function) const {
    std::unique_ptr<ir::dflow::Dataflow> dataflow(new ir::dflow::Dataflow());

    ir::dflow::DataflowAnalyzer(*dataflow, context.module()->architecture(), function, context.hooks())
        .analyze(context.cancellationToken());

    if (!context.dataflows()) {
        context.setDataflows(std::make_unique<ir::dflow::Dataflows>());
    }
    context.dataflows()->setDataflow(function, std::move(dataflow));
}

void MasterAnalyzer::reconstructSignatures(Context &context) const {
    auto signatures = std::make_unique<ir::calling::Signatures>();

    ir::calling::SignatureAnalyzer(*signatures, *context.dataflows(), *context.hooks())
        .analyze(context.cancellationToken());

    context.setSignatures(std::move(signatures));
}

void MasterAnalyzer::computeLiveness(Context &context, const ir::Function *function) const {
    std::unique_ptr<ir::liveness::Liveness> liveness(new ir::liveness::Liveness());

    ir::liveness::LivenessAnalyzer(*liveness, function,
        *context.dataflows()->getDataflow(function), context.module()->architecture(),
        *context.getRegionGraph(function), *context.hooks(), *context.signatures())
    .analyze();

    context.setLiveness(function, std::move(liveness));
}

void MasterAnalyzer::reconstructTypes(Context &context, const ir::Function *function) const {
    std::unique_ptr<ir::types::Types> types(new ir::types::Types());

    ir::types::TypeAnalyzer(
        *types, *context.dataflows()->getDataflow(function), *context.getLiveness(function),
        *context.hooks(), *context.signatures())
    .analyze(function, context.cancellationToken());

    context.setTypes(function, std::move(types));
}

void MasterAnalyzer::doStructuralAnalysis(Context &context, const ir::Function *function) const {
    std::unique_ptr<ir::cflow::Graph> graph(new ir::cflow::Graph());

    ir::cflow::GraphBuilder()(*graph, function);
    ir::cflow::StructureAnalyzer(*graph, *context.dataflows()->getDataflow(function)).analyze();

    context.setRegionGraph(function, std::move(graph));
}

void MasterAnalyzer::reconstructVariables(Context &context) const {
    std::unique_ptr<ir::vars::Variables> variables(new ir::vars::Variables());

    ir::vars::VariableAnalyzer(*variables, *context.dataflows(), context.module()->architecture()).analyze();

    context.setVariables(std::move(variables));
}

void MasterAnalyzer::generateTree(Context &context) const {
    std::unique_ptr<nc::core::likec::Tree> tree(new nc::core::likec::Tree());

    ir::cgen::CodeGenerator generator(context, *tree);
    generator.makeCompilationUnit(context.cancellationToken());

    context.setTree(std::move(tree));
}

#ifdef NC_TREE_CHECKS
void MasterAnalyzer::checkTree(Context &context) const {
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

    ir::misc::CensusVisitor visitor(context.hooks());
    foreach (const ir::Function *function, context.functions()->functions()) {
        visitor(function);
    }

    TreeVisitor checker(visitor);
    checker(context.tree()->root());
}
#endif

} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
