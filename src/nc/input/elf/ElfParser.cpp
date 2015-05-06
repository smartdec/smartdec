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

#include "ElfParser.h"

#include <QCoreApplication> /* For Q_DECLARE_TR_FUNCTIONS. */
#include <QIODevice>

#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/core/Module.h>
#include <nc/core/image/Image.h>
#include <nc/core/image/BufferByteSource.h>
#include <nc/core/input/ParseError.h>

#include "elf32.h"
#include "elf64.h"

namespace nc {
namespace input {
namespace elf {

namespace {

class ElfParserPrivate {
    Q_DECLARE_TR_FUNCTIONS(ElfParserPrivate)

    QIODevice *source_;
    core::Module *module_;

    public:

    ElfParserPrivate(QIODevice *source, core::Module *module):
        source_(source), module_(module)
    {}

    void parse() {
        union {
            Elf32_Ehdr ehdr32;
            Elf64_Ehdr ehdr64;
        } ehdr;

        std::size_t bytesRead = source_->read(reinterpret_cast<char *>(&ehdr), sizeof(ehdr));
        if (bytesRead < sizeof(ehdr.ehdr32.e_ident) || !IS_ELF(ehdr.ehdr32)) {
            throw core::input::ParseError(tr("ELF signature doesn't match."));
        }

        switch (ehdr.ehdr32.e_ident[EI_CLASS]) {
            case ELFCLASS32: {
                if (bytesRead < sizeof(ehdr.ehdr32)) {
                    throw core::input::ParseError(tr("Cannot read ELF32 header."));
                }
                parseHeaders<Elf32_Ehdr, Elf32_Shdr, Elf32_Sym>(ehdr.ehdr32);
                break;
            }
            case ELFCLASS64: {
                if (bytesRead < sizeof(ehdr.ehdr64)) {
                    throw core::input::ParseError(tr("Cannot read ELF64 header."));
                }
                parseHeaders<Elf64_Ehdr, Elf64_Shdr, Elf64_Sym>(ehdr.ehdr64);
                break;
            }
            default: {
                throw core::input::ParseError(tr("Unknown ELF class: %1.").arg(ehdr.ehdr32.e_ident[EI_CLASS]));
            }
        }
    }

    private:

    template<class Ehdr, class Shdr, class Sym>
    void parseHeaders(const Ehdr &ehdr) {
        switch (ehdr.e_machine) {
            case EM_386:
                module_->setArchitecture(QLatin1String("i386"));
                break;
            case EM_X86_64:
                module_->setArchitecture(QLatin1String("x86-64"));
                break;
            default:
                throw core::input::ParseError(tr("Unknown machine id: %1.").arg(ehdr.e_machine));
        }

        source_->seek(ehdr.e_shoff);

        std::vector<Shdr> shdrs(ehdr.e_shnum);
        if (source_->read(reinterpret_cast<char *>(&shdrs[0]), sizeof(Shdr) * shdrs.size()) != static_cast<qint64>(sizeof(Shdr) * shdrs.size())) {
            throw core::input::ParseError(tr("Cannot read section headers."));
        }

        std::size_t initialSectionsCount = module_->image()->sections().size();

        foreach (const Shdr &shdr, shdrs) {
            core::image::Section *section = module_->image()->createSection(QString(), shdr.sh_addr, shdr.sh_size);

            section->setAllocated(shdr.sh_flags & SHF_ALLOC);
            section->setReadable();
            section->setWritable(shdr.sh_flags & SHF_WRITE);
            section->setExecutable(shdr.sh_flags & SHF_EXECINSTR);

            section->setCode(shdr.sh_type == SHT_PROGBITS && section->isExecutable());
            section->setBss(shdr.sh_type == SHT_NOBITS);
            section->setData(section->isAllocated() && !section->isCode() && !section->isBss());

            if (source_->seek(shdr.sh_offset)) {
                section->setExternalByteSource(std::make_unique<core::image::BufferByteSource>(source_->read(shdr.sh_size)));
            }
        }

        if (ehdr.e_shstrndx < shdrs.size()) {
            const core::image::Section *shstrtab = module_->image()->sections()[initialSectionsCount + ehdr.e_shstrndx];

            for (std::size_t i = 0; i < shdrs.size(); ++i) {
                module_->image()->sections()[initialSectionsCount + i]->setName(
                    shstrtab->readAsciizString(shdrs[i].sh_name, shstrtab->size()));
            }
        }

        if (const core::image::Section *symtab = module_->image()->getSectionByName(".symtab")) {
            if (const core::image::Section *strtab = module_->image()->getSectionByName(".strtab")) {
                Sym sym;
                for (ByteAddr addr = symtab->addr(); addr < symtab->endAddr(); addr += sizeof(sym)) {
                    if (symtab->readBytes(addr, &sym, sizeof(sym)) != sizeof(sym)) {
                        break;
                    }
                    module_->addName(sym.st_value, strtab->readAsciizString(sym.st_name, strtab->size()));
                }
            }
        }
    }
};

} // anonymous namespace

ElfParser::ElfParser():
    core::input::Parser("ELF")
{}

bool ElfParser::doCanParse(QIODevice *source) const {
    Elf32_Ehdr ehdr;
    if (source->read(reinterpret_cast<char *>(&ehdr), sizeof(ehdr)) != sizeof(ehdr)) {
        return false;
    }
    return IS_ELF(ehdr);
}

void ElfParser::doParse(QIODevice *source, core::Module *module) const {
    ElfParserPrivate parser(source, module);
    parser.parse();
    module->setDemangler("gnu-v3");
}

} // namespace elf
} // namespace input
} // namespace nc

/* vim:set et sts=4 sw=4: */
