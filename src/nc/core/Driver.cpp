/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "Driver.h"

#include <QFile>

#include <nc/common/Foreach.h>
#include <nc/common/Exception.h>

#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/Instructions.h>
#include <nc/core/arch/disasm/Disassembler.h>
#include <nc/core/image/Image.h>
#include <nc/core/image/Section.h>
#include <nc/core/input/Parser.h>
#include <nc/core/input/ParserRepository.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Functions.h>

#include "Context.h"
#include "MasterAnalyzer.h"
#include "Module.h"

namespace nc {
namespace core {

void Driver::parse(Context &context, const QString &filename) {
    QFile source(filename);

    if (!source.open(QIODevice::ReadOnly)) {
        throw nc::Exception(tr("Could not open file \"%1\" for reading.").arg(filename));
    }

    context.logToken() << tr("Choosing a parser for %1...").arg(filename);

    const input::Parser *suitableParser = NULL;

    foreach(const input::Parser *parser, input::ParserRepository::instance()->parsers()) {
        context.logToken() << tr("Trying %1 parser...").arg(parser->name());
        if (parser->canParse(&source)) {
            suitableParser = parser;
            break;
        }
    }

    if (!suitableParser) {
        context.logToken() << tr("No suitable parser found.");
        throw nc::Exception(tr("File %1 has unknown format.").arg(filename));
    }

    context.logToken() << tr("Parsing using %1 parser...").arg(suitableParser->name());

    suitableParser->parse(&source, context.module().get());

    context.logToken() << tr("Parsing completed.");
}

void Driver::disassemble(Context &context) {
    context.logToken() << tr("Disassembling code sections...");

    foreach (auto section, context.module()->image()->sections()) {
        if (section->isCode()) {
            disassemble(context, section);
        }
    }
}

void Driver::disassemble(Context &context, const image::Section *section) {
    assert(section != NULL);

    context.logToken() << tr("Disassembling section %1...").arg(section->name());

    disassemble(context, section, section->addr(), section->endAddr());
}

void Driver::disassemble(Context &context, const image::ByteSource *source, ByteAddr begin, ByteAddr end) {
    assert(source != NULL);

    context.logToken() << tr("Disassembling addresses from 0x%2 to 0x%3...").arg(begin, 0, 16).arg(end, 0, 16);

    try {
        auto newInstructions = std::make_shared<arch::Instructions>(*context.instructions());

        arch::disasm::Disassembler disassembler(context.module()->architecture(), newInstructions.get());
        disassembler.disassemble(source, begin, end, context.cancellationToken());

        context.setInstructions(newInstructions);

        context.logToken() << tr("Disassembly completed.");
    } catch (const CancellationException &e) {
        context.logToken() << tr("Disassembly canceled.");
    }
}

void Driver::decompile(Context &context) {
    try {
        auto masterAnalyzer = context.module()->architecture()->masterAnalyzer();

        context.logToken() << tr("Creating the program IR...");
        masterAnalyzer->createProgram(context);
        context.cancellationToken().poll();

        context.logToken() << tr("Creating functions...");
        masterAnalyzer->createFunctions(context);
        context.cancellationToken().poll();

        context.logToken() << tr("Creating the calls data...");
        masterAnalyzer->createCallsData(context);
        context.cancellationToken().poll();

        foreach (const ir::Function *function, context.functions()->functions()) {
            context.logToken() << tr("Running dataflow analysis on %1...").arg(function->name());
            masterAnalyzer->analyzeDataflow(context, function);
            context.cancellationToken().poll();
        }

        context.logToken() << tr("Reconstructing signatures of functions...");
        masterAnalyzer->reconstructSignatures(context);
        context.cancellationToken().poll();

        foreach (const ir::Function *function, context.functions()->functions()) {
            context.logToken() << tr("Running structural analysis on %1...").arg(function->name());
            masterAnalyzer->doStructuralAnalysis(context, function);
            context.cancellationToken().poll();

            context.logToken() << tr("Running liveness analysis on %1...").arg(function->name());
            masterAnalyzer->computeUsage(context, function);
            context.cancellationToken().poll();

            context.logToken() << tr("Running type reconstruction on %1...").arg(function->name());
            masterAnalyzer->reconstructTypes(context, function);
            context.cancellationToken().poll();

            context.logToken() << tr("Running reconstruction of variables on %1...").arg(function->name());
            masterAnalyzer->reconstructVariables(context, function);
            context.cancellationToken().poll();
        }

        context.logToken() << tr("Computing term to function mapping...");
        masterAnalyzer->computeTermToFunctionMapping(context);
        context.cancellationToken().poll();

        context.logToken() << tr("Generating AST...");
        masterAnalyzer->generateTree(context);
        context.cancellationToken().poll();

#ifdef NC_TREE_CHECKS
        context.logToken() << tr("Checking AST...");
        masterAnalyzer->checkTree(context);
#endif

        context.logToken() << tr("Decompilation completed.");
    } catch (const CancellationException &) {
        context.logToken() << tr("Decompilation canceled.");
        throw;
    }
}

} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
