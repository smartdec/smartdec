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

    std::unique_ptr<char[]> stringTable_;
    uint32_t stringTableSize_;

public:
    PeParserPrivate(QIODevice *source, core::image::Image *image):
        source_(source), image_(image), stringTableSize_(0)
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
                parseHeaders<IMAGE_OPTIONAL_HEADER32, IMAGE_NT_OPTIONAL_HDR32_MAGIC>(fileHeader);
                break;
            case IMAGE_FILE_MACHINE_AMD64:
                image_->setArchitecture(QLatin1String("x86-64"));
                parseHeaders<IMAGE_OPTIONAL_HEADER64, IMAGE_NT_OPTIONAL_HDR64_MAGIC>(fileHeader);
                break;
            default:
                throw core::input::ParseError(tr("Unknown machine id: %1.").arg(fileHeader.Machine));
        }

        if (fileHeader.PointerToSymbolTable && fileHeader.NumberOfSymbols) {
            if (!source_->seek(fileHeader.PointerToSymbolTable)) {
                throw core::input::ParseError(tr("Cannot seek to the symbol table."));
            }

            /*
             * http://www.delorie.com/djgpp/doc/coff/symtab.html
             */
            std::vector<IMAGE_SYMBOL> symbols(fileHeader.NumberOfSymbols);

            if (source_->read(reinterpret_cast<char *>(&symbols[0]), sizeof(IMAGE_SYMBOL) * fileHeader.NumberOfSymbols) !=
                static_cast<qint64>(sizeof(IMAGE_SYMBOL) * fileHeader.NumberOfSymbols))
            {
                throw core::input::ParseError(tr("Cannot read the symbol table."));
            }

            if (source_->read(reinterpret_cast<char *>(&stringTableSize_), sizeof(stringTableSize_)) != sizeof(stringTableSize_)) {
                throw core::input::ParseError(tr("Cannot read the size of string table."));
            }

            stringTable_.reset(new char[stringTableSize_]);
            memset(stringTable_.get(), 0, 4);
            if (source_->read(stringTable_.get() + 4, stringTableSize_ - 4) != stringTableSize_ - 4) {
                throw core::input::ParseError(tr("Cannot read the string table."));
            }

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
    }

    template<class OptionalHeader, WORD OptionalHeaderMagic>
    void parseHeaders(const IMAGE_FILE_HEADER &fileHeader) {
        OptionalHeader optionalHeader;
        if (source_->read(reinterpret_cast<char *>(&optionalHeader), sizeof(optionalHeader)) != sizeof(optionalHeader)) {
            throw core::input::ParseError(tr("Cannot read the optional header."));
        }
        peByteOrder.convertFrom(optionalHeader.Magic);
        if (optionalHeader.Magic != OptionalHeaderMagic) {
            throw core::input::ParseError(tr("Magic of the optional header doesn't match."));
        }

        for (std::size_t i = 0; i < fileHeader.NumberOfSections; ++i) {
            IMAGE_SECTION_HEADER sectionHeader;
            if (source_->read(reinterpret_cast<char *>(&sectionHeader), sizeof(sectionHeader)) != sizeof(sectionHeader)) {
                throw core::input::ParseError(tr("Cannot read section header #%1.").arg(i));
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

private:
    template<std::size_t size>
    QString getString(const char (&buffer)[size]) const {
        return QString::fromLatin1(buffer, qstrnlen(buffer, size));
    }

    QString getStringFromTable(uint32_t offset) const {
        if (offset < stringTableSize_) {
            return QString::fromLatin1(stringTable_.get() + offset, qstrnlen(stringTable_.get() + offset, stringTableSize_ - offset));
        } else {
            return QString();
        }
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
