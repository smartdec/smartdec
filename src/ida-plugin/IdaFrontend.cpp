/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

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

#include "IdaFrontend.h"

#include "IdaWorkaroundStart.h"
#include <ida.hpp>
#include <segment.hpp>
#include <bytes.hpp>
#include <ua.hpp>
#include <funcs.hpp>  /* For get_func_name. */
#include <demangle.hpp>
#include <idp.hpp>    /* Required by intel.hpp. */
#include <intel.hpp>
#include <nalt.hpp>   /* For import_node. */
#include "IdaWorkaroundEnd.h"

#include <nc/common/CheckedCast.h>
#include <nc/common/make_unique.h>
#include <nc/core/image/Image.h>
#include <nc/core/image/Section.h>

#include "IdaByteSource.h"

#if IDA_SDK_VERSION >= 700
extern "C" plugin_t PLUGIN;
#endif

namespace nc { namespace ida {

bool IdaFrontend::jumpToAddress(ByteAddr address) {
    return ::jumpto(checked_cast<ea_t>(address));
}

QString IdaFrontend::functionName(ByteAddr address) {
#if IDA_SDK_VERSION >= 700
    ::qstring result;
    ::get_func_name(&result, checked_cast<ea_t>(address));
    return QLatin1String(result.c_str());
#else
    char buffer[MAXSTR];
    if(::get_func_name(checked_cast<ea_t>(address), buffer, sizeof(buffer)) == NULL)
        return QString();
    return QLatin1String(buffer);
#endif
}

QString IdaFrontend::addressName(ByteAddr address) {
#if IDA_SDK_VERSION >= 700
    ::qstring result;
    ::get_ea_name(&result, checked_cast<ea_t>(address));
    return QLatin1String(result.c_str());
#else
    char buffer[MAXSTR];
    if(::get_true_name(BADADDR, checked_cast<ea_t>(address), buffer, sizeof(buffer)) == NULL)
        return QString();
    return QLatin1String(buffer);
#endif
}

#if IDA_SDK_VERSION >= 700
namespace {

int idaapi enumImportNamesCallback(ea_t ea, const char *name, ::uval_t, void *param) {
    reinterpret_cast<
        std::vector<std::pair<ByteAddr, QString>> *
    >(param)->emplace_back(ea, QLatin1String(name));
    return 1;
}

} // unnamed namespace
#endif

std::vector<std::pair<ByteAddr, QString>> IdaFrontend::importedFunctions() {
    std::vector<std::pair<ByteAddr, QString>> result;

#if IDA_SDK_VERSION >= 700
    for(int i = 0, n = ::get_import_module_qty(); i < n; i++) {
        ::enum_import_names(i, enumImportNamesCallback, &result);
    }
#else
    for(int i = 0, n = import_node.altval(-1); i < n; i++) {
        netnode module(import_node.altval(i));

        for(ea_t address = module.sup1st(); address != BADNODE; address = module.supnxt(address)) {
            char functionName[MAXSTR];
            ssize_t size = module.supval(address, functionName, sizeof(functionName));
            if(size != -1)
                result.push_back(std::make_pair(address, QLatin1String(functionName)));
        }
    }
#endif
    return result;
}

QString IdaFrontend::demangle(const QString &mangled) {
    QString preprocessed = mangled;
    if(preprocessed.startsWith("j_?"))
        preprocessed = preprocessed.mid(2); /* Handle thunks. */

    char buffer[MAXSTR];
    if(::demangle(buffer, sizeof(buffer), preprocessed.toLatin1().data(), 0) > 0)
        return QLatin1String(buffer);

    return preprocessed;
}

std::vector<ByteAddr> IdaFrontend::functionStarts() {
    std::vector<ByteAddr> result;

    for(std::size_t i = 0, n = get_func_qty(); i < n; i++) {
        func_t *func = getn_func(i);
        if (func != NULL) {
            result.push_back(func->
#if IDA_SDK_VERSION >= 700
                start_ea
#else
                startEA
#endif
            );
        }
    }

    return result;
}

void IdaFrontend::createSections(core::image::Image *image) {
    for (int i = 0; i < get_segm_qty(); i++) {
        segment_t *idaSegment = getnseg(i);

        assert(idaSegment != NULL);

#if IDA_SDK_VERSION >= 700
        ::qstring buffer;
        get_segm_name(&buffer, idaSegment);
        QString segName = QLatin1String(buffer.c_str());
#else
        char buffer[MAXSTR];
        ssize_t ret = get_segm_name(idaSegment, buffer, sizeof(buffer) - 1);
        assert(ret >= 0);
        buffer[ret] = '\0';
        QString segName = QLatin1String(buffer);
#endif
        if (!segName.isEmpty() && segName[0] == '_') {
            segName[0] = '.';
        }

        auto section = std::make_unique<core::image::Section>(
            segName,
            checked_cast<ByteAddr>(idaSegment->
#if IDA_SDK_VERSION >= 700
                start_ea
#else
                startEA
#endif
            ),
            checked_cast<ByteSize>(idaSegment->size())
        );

        section->setReadable(idaSegment->perm & SEGPERM_READ);
        section->setWritable(idaSegment->perm & SEGPERM_WRITE);
        section->setExecutable(idaSegment->perm & SEGPERM_EXEC);
        section->setCode(idaSegment->type == SEG_CODE);
        section->setData(idaSegment->type == SEG_DATA);
        section->setBss(idaSegment->type == SEG_BSS);
        section->setAllocated(section->isCode() || section->isData() || section->isBss());
        section->setExternalByteSource(std::make_unique<IdaByteSource>());

        image->addSection(std::move(section));
    }
}

ByteOrder IdaFrontend::byteOrder() {
    return
#if IDA_SDK_VERSION >= 700
        inf.is_be()
#else
        inf.mf
#endif
        ? ByteOrder::BigEndian : ByteOrder::LittleEndian;
}

QString IdaFrontend::architecture() {
    QLatin1String procName(
#if IDA_SDK_VERSION >= 700
        inf.procname
#else
        inf.procName
#endif
    );

    if (procName == "ARM") {
        return QLatin1String(byteOrder() == ByteOrder::LittleEndian ? "arm-le" : "arm-be");
    } else if (procName == "ARMB") {
        return QLatin1String("arm-be");
    } else {
        /* Assume x86 by default. */
        if (segment_t *segment = get_segm_by_name(".text")) {
            switch (segment->bitness) {
                case 0: return QLatin1String("8086");
                case 1: return QLatin1String("i386");
                case 2: return QLatin1String("x86-64");
            }
        }
        return QLatin1String("i386");
    }
}

core::image::Platform::OperatingSystem IdaFrontend::operatingSystem() {
    if (inf.filetype == f_WIN || inf.filetype == f_COFF || inf.filetype == f_PE) {
        return core::image::Platform::Windows;
    }
    return core::image::Platform::UnknownOS;
}

std::vector<AddressRange> IdaFrontend::functionAddresses(ByteAddr address) {
    std::vector<AddressRange> result;

    ea_t func_ea = checked_cast<ea_t>(address);
    func_t *function = ::get_func(func_ea);

    if (!function) {
        return result;
    }

    auto startAddr = checked_cast<ByteAddr>(function->
#if IDA_SDK_VERSION >= 700
        start_ea
#else
        startEA
#endif
);
    auto endAddr = checked_cast<ByteAddr>(function->
#if IDA_SDK_VERSION >= 700
        end_ea
#else
        endEA
#endif
);
    result.push_back(AddressRange(startAddr, endAddr));

    for (int i = 0; i < function->tailqty; ++i) {
        auto chunkStartAddr = checked_cast<ByteAddr>(function->tails[i].
#if IDA_SDK_VERSION >= 700
            start_ea
#else
            startEA
#endif
        );
        auto chunkEndAddr = checked_cast<ByteAddr>(function->tails[i].
#if IDA_SDK_VERSION >= 700
            end_ea
#else
            endEA
#endif
        );
        result.push_back(AddressRange(chunkStartAddr, chunkEndAddr));
    }

    return result;
}

ByteAddr IdaFrontend::screenAddress() {
    return checked_cast<ByteAddr>(get_screen_ea());
}

#if IDA_SDK_VERSION >= 700
struct IdaFrontend::MenuItem: public ::action_handler_t {
    QString path_;
    QString name_;
    std::function<void()> handler_;
    QString actionName_;

    MenuItem(
        const QString &path,
        const QString &name,
        const QString &after,
        const QString &hotkey,
        std::function<void()> handler
    ):
        path_(path),
        name_(name),
        handler_(handler),
        actionName_(QString("snowman:%1").arg(name))
    {
        action_desc_t desc ACTION_DESC_LITERAL(
            actionName_.toLocal8Bit().constData(),
            name.toLocal8Bit().constData(),
            this,
            hotkey.toLocal8Bit().constData(),
            nullptr,
            -1
        );
        ::register_action(desc);
        ::attach_action_to_menu(
            QString("%1/%2").arg(path).arg(after).toLocal8Bit().constData(),
            desc.name,
            SETMENU_APP
        );
    }

    ~MenuItem() {
        ::detach_action_from_menu(
            QString("%1/%2").arg(path_).arg(name_).toLocal8Bit().constData(),
            actionName_.toLocal8Bit().constData()
        );
        ::unregister_action(actionName_.toLocal8Bit().constData());
    }

    int idaapi activate(action_activation_ctx_t *) override {
        handler_();
        return 0;
    }

    action_state_t idaapi update(action_update_ctx_t *) override {
        return AST_ENABLE_ALWAYS;
    }
};

#else

struct IdaFrontend::MenuItem {
    QString path_;
    QString name_;
    std::function<void()> handler_;

    MenuItem(
        const QString &path,
        const QString &name,
        const QString &after,
        const QString &hotkey,
        std::function<void()> handler
    ): path_(path), name_(name), handler_(std::move(handler)) {
        ::add_menu_item(
            QString("%1/%2").arg(path).arg(after).toLocal8Bit().constData(),
            name.toLocal8Bit().constData(),
            hotkey.toLocal8Bit().constData(),
            SETMENU_APP,
            callback,
            this
        );
    }

    ~MenuItem() {
        ::del_menu_item(QString("%1/%2").arg(path_).arg(name_).toLocal8Bit().constData());
    }

    static bool idaapi callback(void *menuItem) {
        reinterpret_cast<MenuItem *>(menuItem)->handler_();
        return false;
    }
};
#endif

IdaFrontend::MenuItem *IdaFrontend::addMenuItem(
    const QString &path,
    const QString &name,
    const QString &after,
    const QString &hotkey,
    std::function<void()> handler
) {
    return new MenuItem(path, name, after, hotkey, std::move(handler));
}

void IdaFrontend::deleteMenuItem(MenuItem *menuItem) {
    delete menuItem;
}

void IdaFrontend::print(const QString &message) {
    ::msg(message.toLocal8Bit());
}

QWidget *IdaFrontend::createWidget(const QString &caption) {
#if IDA_SDK_VERSION >= 700
    TWidget *widget = ::create_empty_widget(caption.toLocal8Bit().constData());
    ::display_widget(widget, WOPN_MDI | WOPN_TAB | WOPN_MENU);
    return reinterpret_cast<QWidget *>(widget);
#else
    HWND hwnd;
    TForm *form = ::create_tform(caption.toLocal8Bit().constData(), &hwnd);
    ::open_tform(form, FORM_MDI | FORM_TAB | FORM_MENU | FORM_QWIDGET);
    return reinterpret_cast<QWidget *>(form);
#endif
}

void IdaFrontend::activateWidget(QWidget *widget) {
#if IDA_SDK_VERSION >= 700
    ::activate_widget(reinterpret_cast<TWidget *>(widget), true);
#else
    ::switchto_tform(reinterpret_cast<TForm *>(widget), true);
#endif
}

void IdaFrontend::closeWidget(QWidget *widget) {
#if IDA_SDK_VERSION >= 700
    ::close_widget(reinterpret_cast<TWidget *>(widget), 0);
#else
    ::close_tform(reinterpret_cast<TForm *>(widget), 0);
#endif
}

}} // namespace nc::ida

/* vim:set et sts=4 sw=4: */
