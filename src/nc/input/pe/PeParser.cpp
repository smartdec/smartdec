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

#include "PeParser.h"

#include <QCoreApplication> /* For Q_DECLARE_TR_FUNCTIONS. */
#include <QIODevice>

#include <nc/common/ByteOrder.h>
#include <nc/common/Conversions.h>
#include <nc/common/Foreach.h>
#include <nc/common/Warnings.h>
#include <nc/common/make_unique.h>

#include <nc/core/image/BufferByteSource.h>
#include <nc/core/image/Image.h>
#include <nc/core/image/Section.h>
#include <nc/core/input/ParseError.h>

#include "pe.h"

namespace nc {
namespace input {
namespace pe {

namespace {

const ByteOrder peByteOrder(ByteOrder::LittleEndian);

bool seekFileHeader(QIODevice *source) {
    IMAGE_DOS_HEADER dosHeader;

    if (source->read(reinterpret_cast<char *>(&dosHeader), sizeof(dosHeader)) != sizeof(dosHeader)) {
        return false;
    }

    peByteOrder.convertFrom(dosHeader.e_magic);
    peByteOrder.convertFrom(dosHeader.e_lfanew);

    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        return false;
    }

    if (!source->seek(dosHeader.e_lfanew)) {
        return false;
    }

    uint32_t signature;
    if (source->read(reinterpret_cast<char *>(&signature), sizeof(signature)) != sizeof(signature)) {
        return false;
    }
    peByteOrder.convertFrom(signature);

    if (signature != IMAGE_NT_SIGNATURE) {
        return false;
    }

    return true;
}

class PeParserPrivate {
    Q_DECLARE_TR_FUNCTIONS(PeParserPrivate)

    QIODevice *source_;
    core::image::Image *image_;

public:
    PeParserPrivate(QIODevice *source, core::image::Image *image):
        source_(source), image_(image)
    {}

    void parse() {
        if (!seekFileHeader(source_)) {
            throw core::input::ParseError(tr("PE signature doesn't match."));
        }

        IMAGE_FILE_HEADER fileHeader;
        if (source_->read(reinterpret_cast<char *>(&fileHeader), sizeof(fileHeader)) != sizeof(fileHeader)) {
            throw core::input::ParseError(tr("Cannot read the file header."));
        }
        peByteOrder.convertFrom(fileHeader.Machine);
        peByteOrder.convertFrom(fileHeader.PointerToSymbolTable);
        peByteOrder.convertFrom(fileHeader.NumberOfSymbols);
        peByteOrder.convertFrom(fileHeader.NumberOfSections);

        switch (fileHeader.Machine) {
            case IMAGE_FILE_MACHINE_I386:
                image_->setArchitecture(QLatin1String("i386"));
                break;
            case IMAGE_FILE_MACHINE_AMD64:
                image_->setArchitecture(QLatin1String("x86-64"));
                break;
            default:
                throw core::input::ParseError(tr("Unknown machine id: %1.").arg(fileHeader.Machine));
        }

        parseSections(fileHeader);
        parseSymbols(fileHeader);
    }

private:
    void parseSections(const IMAGE_FILE_HEADER &fileHeader) {
        if (!source_->seek(source_->pos() + fileHeader.SizeOfOptionalHeader)) {
            ncWarning("Cannot seek to the section header table.");
            return;
        }

        for (std::size_t i = 0; i < fileHeader.NumberOfSections; ++i) {
            IMAGE_SECTION_HEADER sectionHeader;
            if (source_->read(reinterpret_cast<char *>(&sectionHeader), sizeof(sectionHeader)) != sizeof(sectionHeader)) {
                ncWarning("Cannot read the section header %1.", i);
                return;
            }

            peByteOrder.convertFrom(sectionHeader.VirtualAddress);
            peByteOrder.convertFrom(sectionHeader.SizeOfRawData);
            peByteOrder.convertFrom(sectionHeader.Characteristics);
            peByteOrder.convertFrom(sectionHeader.PointerToRawData);
            peByteOrder.convertFrom(sectionHeader.SizeOfRawData);

            auto section = std::make_unique<core::image::Section>(
                getString(sectionHeader.Name), sectionHeader.VirtualAddress, sectionHeader.SizeOfRawData
            );

            section->setAllocated((sectionHeader.Characteristics & IMAGE_SCN_MEM_DISCARDABLE) == 0);
            section->setReadable(sectionHeader.Characteristics & IMAGE_SCN_MEM_READ);
            section->setWritable(sectionHeader.Characteristics & IMAGE_SCN_MEM_WRITE);
            section->setExecutable(sectionHeader.Characteristics & IMAGE_SCN_MEM_EXECUTE);

            section->setCode(sectionHeader.Characteristics & IMAGE_SCN_CNT_CODE);
            section->setData(sectionHeader.Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA);
            section->setBss(sectionHeader.Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA);

            auto pos = source_->pos();
            if (!section->isBss()) {
                if (source_->seek(sectionHeader.PointerToRawData)) {
                    section->setExternalByteSource(std::make_unique<core::image::BufferByteSource>(source_->read(sectionHeader.SizeOfRawData)));
                }
            }
            source_->seek(pos);

            image_->addSection(std::move(section));
        }
    }

    void parseSymbols(const IMAGE_FILE_HEADER &fileHeader) {
        if (!fileHeader.PointerToSymbolTable || !fileHeader.NumberOfSymbols) {
            return;
        }

        if (!source_->seek(fileHeader.PointerToSymbolTable)) {
            ncWarning("Cannot seek to the symbol table.");
            return;
        }

        /*
         * http://www.delorie.com/djgpp/doc/coff/symtab.html
         */
        std::vector<IMAGE_SYMBOL> symbols(fileHeader.NumberOfSymbols);

        if (source_->read(reinterpret_cast<char *>(&symbols[0]), sizeof(IMAGE_SYMBOL) * fileHeader.NumberOfSymbols) !=
            static_cast<qint64>(sizeof(IMAGE_SYMBOL) * fileHeader.NumberOfSymbols))
        {
            ncWarning("Cannot read the symbol table.");
            return;
        }

        uint32_t stringTableSize;
        if (source_->read(reinterpret_cast<char *>(&stringTableSize), sizeof(stringTableSize)) != sizeof(stringTableSize)) {
            ncWarning("Cannot read the size of the string table.");
            return;
        }

        std::unique_ptr<char[]> stringTable(new char[stringTableSize]);
        memset(stringTable.get(), 0, 4);
        if (source_->read(stringTable.get() + 4, stringTableSize - 4) != stringTableSize - 4) {
            ncWarning("Cannot read the string table.");
            return;
        }

        auto getStringFromTable = [&](uint32_t offset) -> QString {
            if (offset < stringTableSize) {
                return QString::fromLatin1(
                    stringTable.get() + offset, qstrnlen(stringTable.get() + offset, stringTableSize - offset));
            } else {
                return QString();
            }
        };

        foreach (IMAGE_SYMBOL &symbol, symbols) {
            peByteOrder.convertFrom(symbol.Type);
            peByteOrder.convertFrom(symbol.Value);

            using core::image::Symbol;

            Symbol::Type type;
            /*
             * Microsoft tools set this field to 0x20 (function) or 0x0 (not a function).
             * -- http://www.kishorekumar.net/pecoff_v8.1.htm
             */
            if (symbol.Type == 0x20) {
                type = Symbol::FUNCTION;
            } else {
                type = Symbol::NOTYPE;
            }

            QString name;
            if (symbol.N.Name.Short) {
                name = getString(symbol.N.ShortName);
            } else {
                name = getStringFromTable(symbol.N.Name.Long);
            }

            image_->addSymbol(std::make_unique<Symbol>(type, name, symbol.Value));
        }

        foreach (auto section, image_->sections()) {
            if (section->name().startsWith('/')) {
                uint32_t offset;
                if (stringToInt(section->name().mid(1), &offset)) {
                    QString newName = getStringFromTable(offset);
                    if (!newName.isEmpty()) {
                        section->setName(newName);
                    }
                }
            }
        }
    }

    template<std::size_t size>
    QString getString(const char (&buffer)[size]) const {
        return QString::fromLatin1(buffer, qstrnlen(buffer, size));
    }
};

} // anonymous namespace

PeParser::PeParser():
    core::input::Parser("PE")
{}

bool PeParser::doCanParse(QIODevice *source) const {
    return seekFileHeader(source);
}

void PeParser::doParse(QIODevice *source, core::image::Image *image) const {
    PeParserPrivate parser(source, image);
    parser.parse();
    image->setDemangler("msvc");
}

} // namespace pe
} // namespace input
} // namespace nc

/* vim:set et sts=4 sw=4: */
