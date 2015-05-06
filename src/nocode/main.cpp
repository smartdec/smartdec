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

#include <nc/config.h>

#include <nc/common/Conversions.h>
#include <nc/common/Exception.h>
#include <nc/common/Foreach.h>

#include <nc/core/Module.h>
#include <nc/core/Context.h> 
#include <nc/core/arch/Instruction.h>
#include <nc/core/arch/Instructions.h>
#include <nc/core/image/Image.h>
#include <nc/core/input/Parser.h>
#include <nc/core/input/ParserRepository.h>

#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Functions.h>
#include <nc/core/ir/Program.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/inlining/CallInliner.h>
#include <nc/core/ir/cflow/Graph.h>

#include <nc/core/likec/Tree.h>

#include <QFile>
#include <QStringList>
#include <QTextStream>

const char *self = "nocode";

QTextStream qin(stdin, QIODevice::ReadOnly);
QTextStream qout(stdout, QIODevice::WriteOnly);
QTextStream qerr(stderr, QIODevice::WriteOnly);

template<class T>
void openFileForWritingAndCall(const QString &filename, T functor) {
    if (filename.isEmpty()) {
        return;
    } else if (filename == "-") {
        functor(qout);
    } else {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly)) {
            throw nc::Exception("could not open file for writing");
        }
        QTextStream out(&file);
        functor(out);
    }
}

void inlineFunctions(nc::core::Context &context, std::vector<nc::ByteAddr> functionAddresses) {
    nc::core::ir::inlining::CallInliner inliner;

    foreach (nc::ByteAddr addr, functionAddresses) {
        auto &functions = context.functions()->getFunctionsAtAddress(addr);
        if (functions.empty()) {
            throw nc::Exception(QString("there is no function having address 0x%1 to inline").arg(addr, 0, 16));
        }
        const nc::core::ir::Function *inlinedFunction = functions.front();

        foreach (nc::core::ir::Function *function, context.functions()->functions()) {
            std::vector<const nc::core::ir::Call *> calls;

            foreach (nc::core::ir::BasicBlock *basicBlock, function->basicBlocks()) {
                foreach (nc::core::ir::Statement *statement, basicBlock->statements()) {
                    if (const nc::core::ir::Call *call = statement->asCall()) {
                        if (const nc::core::ir::Constant *constant = call->target()->asConstant()) {
                            if (constant->value().value() == nc::ConstantValue(addr)) {
                                calls.push_back(call);
                            }
                        }
                    }
                }
            }

            foreach (const nc::core::ir::Call *call, calls) {
                inliner.perform(function, call, inlinedFunction);
            }
        }
    }
}

void inlineCalls(nc::core::Context &context, std::vector<nc::ByteAddr> callAddresses) {
    if (callAddresses.empty()) {
        return;
    }

    nc::core::ir::inlining::CallInliner inliner;

    std::sort(callAddresses.begin(), callAddresses.end());

    foreach (nc::core::ir::Function *function, context.functions()->functions()) {
        std::vector<std::pair<const nc::core::ir::Call *, const nc::core::ir::Function *>> inlines;

        foreach (nc::core::ir::BasicBlock *basicBlock, function->basicBlocks()) {
            foreach (nc::core::ir::Statement *statement, basicBlock->statements()) {
                if (statement->instruction() &&
                    std::binary_search(callAddresses.begin(), callAddresses.end(), statement->instruction()->addr()))
                {
                    if (const nc::core::ir::Call *call = statement->asCall()) {
                        const nc::core::ir::Function *inlinedFunction = NULL;

                        if (const nc::core::ir::Constant *constant = call->target()->asConstant()) {
                            auto &functions = context.functions()->getFunctionsAtAddress(constant->value().value());
                            if (!functions.empty()) {
                                inlinedFunction = functions.front();
                            }
                        }

                        if (!inlinedFunction) {
                            throw nc::Exception(QString("can't detect the function being called at 0x%1").
                                arg(statement->instruction()->addr(), 0, 16));
                        }

                        inlines.push_back(std::make_pair(call, inlinedFunction));
                    }
                }
            }
        }

        foreach (const auto &pair, inlines) {
            inliner.perform(function, pair.first, pair.second);
        }
    }
}

void printSections(nc::core::Context &context, QTextStream &out) {
    foreach (const auto *section, context.module()->image()->sections()) {
        QString flags;
        if (section->isReadable()) {
            flags += "r";
        }
        if (section->isWritable()) {
            flags += "w";
        }
        if (section->isExecutable()) {
            flags += "x";
        }
        if (section->isCode()) {
            flags += ",code";
        }
        if (section->isData()) {
            flags += ",data";
        }
        if (section->isBss()) {
            flags += ",bss";
        }
        out << QString("section name = '%1', start = 0x%2, size = 0x%3, flags = %4")
            .arg(section->name()).arg(section->addr(), 0, 16).arg(section->size(), 0, 16).arg(flags) << endl;
    }
}

void printRegionGraphs(nc::core::Context &context, QTextStream &out) {
    out << "digraph Functions" << " { compound=true; " << endl;
    foreach (const auto *function, context.functions()->functions()) {
        context.getRegionGraph(function)->print(out);
    }
    out << "}" << endl;
}

void help() {
    qout << "Usage: " << self << " [options] [--] file..." << endl;
    qout << endl;
    qout << "Options:" << endl;
    qout << "  --help, -h                  Produce this help message and exit." << endl;
    qout << "  --list-parsers              List available parsers and exit." << endl;
    qout << "  --inline-function=ADDR      Inline a function with given address everywhere." << endl;
    qout << "  --inline-call=ADDR          Inline a call at given address." << endl;
    qout << "  --print-instructions[=FILE] Dump parsed instructions to the file." << endl;
    qout << "  --print-cfg[=FILE]          Dump control flow graph in DOT language to the file." << endl;
    qout << "  --print-ir[=FILE]           Dump intermediate representation in DOT language to the file." << endl;
    qout << "  --print-regions[=FILE]      Dump results of structural analysis in DOT language to the file." << endl;
    qout << "  --print-cxx[=FILE]          Print reconstructed program into given file." << endl;
    qout << endl;
    qout << "Program loads a disassembly text or executable image from given file or files" << endl;
    qout << "and prints what it is said to (by default, it prints C++ code). When output" << endl;
    qout << "file name is '-', stdout is used." << endl;
}

void listParsers() {
    qout << "Available parsers:" << endl;
    foreach(nc::core::input::Parser *parser, nc::core::input::ParserRepository::instance()->parsers()) {
        qout << "  " << parser->name() << endl;
    }
}

int main(int argc, char *argv[]) {
    try {
        QStringList args;

        for (int i = 1; i < argc; ++i) {
            args.append(QString(argv[i]));
        }

        QString sectionsFile;
        QString instructionsFile;
        QString cfgFile;
        QString irFile;
        QString regionsFile;
        QString cxxFile;
        bool autoDefault = true;

        std::vector<nc::ByteAddr> functionAddresses;
        std::vector<nc::ByteAddr> callAddresses;

        QStringList files;

        for (int i = 0; i < args.size(); ++i) {
            QString arg = args[i];
            if (arg == "--help" || arg == "-h") {
                help();
                return 1;
            } else if (arg == "--list-parsers") {
                listParsers();
                return 1;

            #define FILE_OPTION(option, variable)       \
            } else if (arg == option) {                 \
                variable = "-";                         \
                autoDefault = false;                    \
            } else if (arg.startsWith(option "=")) {    \
                variable = arg.section('=', 1);         \
                autoDefault = false;

            FILE_OPTION("--print-sections", sectionsFile)
            FILE_OPTION("--print-instructions", instructionsFile)
            FILE_OPTION("--print-cfg", cfgFile)
            FILE_OPTION("--print-ir", irFile)
            FILE_OPTION("--print-regions", regionsFile)
            FILE_OPTION("--print-cxx", cxxFile)

            #undef FILE_OPTION

            #define ADDR_OPTION(option, variable)                                   \
            } else if (arg.startsWith(option "=")) {                                \
                QString s = arg.section('=', 1);                                    \
                nc::ByteAddr address;                                               \
                if (!nc::stringToInt<nc::ByteAddr>(s, &address)) {                  \
                    throw nc::Exception(QString("bad address value: %1").arg(s));   \
                }                                                                   \
                variable.push_back(address);                                        \

            ADDR_OPTION("--inline-function", functionAddresses)
            ADDR_OPTION("--inline-call",     callAddresses)

            #undef ADDR_OPTION

            } else if (arg == "--") {
                while (++i < args.size()) {
                    files.append(args[i]);
                }
            } else if (arg.startsWith("-")) {
                throw nc::Exception(QString("unknown argument: %1").arg(arg));
            } else {
                files.append(args[i]);
            }
        }

        if (autoDefault) {
            cxxFile = "-";
        }

        if (files.empty()) {
            throw nc::Exception("no input files");
        }

        nc::core::Context context;

        foreach (const QString &filename, files) {
            try {
                context.parse(filename);
            } catch (const nc::Exception &e) {
                throw nc::Exception(filename + ":" + e.unicodeWhat());
            } catch (const std::exception &e) {
                throw nc::Exception(filename + ":" + e.what());
            }
        }

        // FIXME
        #if 0
        inlineFunctions(context, functionAddresses);
        inlineCalls(context, callAddresses);
        #endif

        openFileForWritingAndCall(sectionsFile,     [&](QTextStream &out) { printSections(context, out); });

        if (!instructionsFile.isEmpty()) {
            context.disassemble();
        }
        openFileForWritingAndCall(instructionsFile, [&](QTextStream &out) { context.instructions()->print(out); });

        if (!cfgFile.isEmpty() || !irFile.isEmpty() || !regionsFile.isEmpty() || !cxxFile.isEmpty()) {
            context.decompile();
        }
        openFileForWritingAndCall(cfgFile,          [&](QTextStream &out) { context.program()->print(out); });
        openFileForWritingAndCall(irFile,           [&](QTextStream &out) { context.functions()->print(out); });
        openFileForWritingAndCall(regionsFile,      [&](QTextStream &out) { printRegionGraphs(context, out); });
        openFileForWritingAndCall(cxxFile,          [&](QTextStream &out) { context.tree()->print(out); });
    } catch (const nc::Exception &e) {
        qerr << self << ": " << e.unicodeWhat() << endl;
        return 1;
    }

    return 0;
}

/* vim:set et sts=4 sw=4: */
