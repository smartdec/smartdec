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
#include <nc/core/image/Sections.h>
#include <nc/core/input/Parser.h>
#include <nc/core/input/ParserRepository.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Functions.h>

#include "Context.h"
#include "MasterAnalyzer.h"

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

    suitableParser->parse(&source, context.image().get());

    context.logToken() << tr("Parsing completed.");
}

void Driver::disassemble(Context &context) {
    context.logToken() << tr("Disassembling code sections...");

    foreach (auto section, context.image()->sections()->all()) {
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

        arch::disasm::Disassembler disassembler(context.image()->architecture(), newInstructions.get());
        disassembler.disassemble(source, begin, end, context.cancellationToken());

        context.setInstructions(newInstructions);

        context.logToken() << tr("Disassembly completed.");
    } catch (const CancellationException &) {
        context.logToken() << tr("Disassembly canceled.");
    }
}

void Driver::decompile(Context &context) {
    try {
        context.image()->architecture()->masterAnalyzer()->decompile(context);
    } catch (const CancellationException &) {
        context.logToken() << tr("Decompilation canceled.");
        throw;
    }
}

} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
