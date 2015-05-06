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

#include "SmartDecPlugin.h"

#include <QApplication>
#include <QAction>
#include <QMessageBox>
#include <QStackedLayout>

#include <nc/common/Foreach.h>
#include <nc/common/GitSHA1.h>
#include <nc/common/make_unique.h>
#include <nc/core/Context.h>
#include <nc/core/Module.h>
#include <nc/core/arch/Instruction.h>
#include <nc/core/image/Image.h>
#include <nc/gui/MainWindow.h>
#include <nc/gui/Project.h>

#include "IdaByteSource.h"
#include "IdaDemangler.h"
#include "IdaFrontend.h"
#include "NavigationHelper.h"

namespace nc { namespace ida {

namespace {

const char *decompileFunctionHotkey = "F4";
const char *decompileProgramHotkey  = "Ctrl-F4";

const char *subviewsMenuPath = "View/Open subviews";
const char *hexDumpMenuItem = "Hex dump";
const char *decompileFunctionMenuItem = "Decompile a function";
const char *decompileProgramMenuItem  = "Decompile a program";

SmartDecPlugin *globalPluginInstance = NULL;

} // anonymous namespace

extern "C" {
    static bool decompileFunctionCallback() {
        globalPluginInstance->decompileFunction();
        return true;
    }

    static bool decompileAllCallback() {
        globalPluginInstance->decompileProgram();
        return true;
    }
}

SmartDecPlugin::SmartDecPlugin():
    programWindow_(NULL)
{
    if (QApplication::instance() == NULL) {
        static int argc = 1;
        static char *argv[] = {const_cast<char *>("dummy.exe")};

        mApplication.reset(new QApplication(argc, argv));
    }

    globalPluginInstance = this;

    IdaFrontend::addMenuItem(
        tr("%1/%2").arg(subviewsMenuPath).arg(hexDumpMenuItem),
        tr(decompileProgramMenuItem),
        decompileProgramHotkey,
        &decompileAllCallback);
    IdaFrontend::addMenuItem(
        tr("%1/%2").arg(subviewsMenuPath).arg(hexDumpMenuItem),
        tr(decompileFunctionMenuItem),
        decompileFunctionHotkey,
        &decompileFunctionCallback);

    IdaFrontend::print(tr(
        "SmartDec plugin (revision %1) loaded.\n"
        "  Press %2 to decompile the function under cursor, %3 to decompile the whole program.\n"
        "  Press %2 (%3) again to jump to the address under cursor.\n")
        .arg(QString(git_sha1).left(5)).arg(decompileFunctionHotkey).arg(decompileProgramHotkey));
}

SmartDecPlugin::~SmartDecPlugin() {
    IdaFrontend::deleteMenuItem(tr("%1/%2").arg(subviewsMenuPath).arg(decompileFunctionMenuItem));
    IdaFrontend::deleteMenuItem(tr("%1/%2").arg(subviewsMenuPath).arg(decompileProgramMenuItem));
}

void SmartDecPlugin::decompileFunction() {
    auto functionRanges = IdaFrontend::functionAddresses(IdaFrontend::screenAddress());
    if (functionRanges.empty()) {
        QMessageBox::warning(NULL, tr("SmartDec"), tr("Please, put the text cursor inside a function that you would like to decompile."));
        return;
    }

    ByteAddr functionAddress = functionRanges.front().start();

    if (auto window = function2window_[functionAddress]) {
        activateWindow(window);
        window->jumpToAddress(IdaFrontend::screenAddress());
        return;
    }

    auto window = createWindow();
    window->open(createIdaProject());

    window->project()->setName(IdaFrontend::functionName(functionAddress));

    foreach (const AddressRange &range, functionRanges) {
        window->project()->disassemble(window->project()->module()->image(), range.start(), range.end());
    }

    if (window->decompileAutomatically()) {
        window->project()->decompile();
    }

    function2window_[functionAddress] = window;
    window2function_[window] = functionAddress;
}
    
void SmartDecPlugin::decompileProgram() {
    if (programWindow_) {
        activateWindow(programWindow_);
        programWindow_->jumpToAddress(IdaFrontend::screenAddress());
        return;
    }

    auto window = createWindow();
    window->open(createIdaProject());
    window->project()->disassemble();
    if (window->decompileAutomatically()) {
        window->project()->decompile();
    }
    programWindow_ = window;
}

gui::MainWindow *SmartDecPlugin::createWindow() {
    gui::MainWindow *window = new gui::MainWindow();
    window->setWindowFlags(Qt::Widget);
    window->setAttribute(Qt::WA_DeleteOnClose);

    connect(window, SIGNAL(destroyed(QObject *)), this, SLOT(windowDestroyed(QObject *)));

    QWidget *widget = IdaFrontend::createWidget(window->windowTitle());
    widget->setLayout(new QStackedLayout());
    widget->layout()->addWidget(window);

    window->openAction()->setEnabled(false);
    window->openAction()->setVisible(false);

    window->quitAction()->setEnabled(false);
    window->quitAction()->setVisible(false);
    
    connect(window, SIGNAL(windowTitleChanged(const QString &)), widget, SLOT(setWindowTitle(const QString &)));

    window2widget_[window] = widget;

    /* Owned by MainWindow. */
    new NavigationHelper(window);

    return window;
}

void SmartDecPlugin::activateWindow(gui::MainWindow *window) {
    assert(window != NULL);

    IdaFrontend::activateWidget(window2widget_[window]);
}

void SmartDecPlugin::windowDestroyed(QObject *object) {
    /*
     * checked_cast is not applicable here: destructor of
     * gui::MainWindow was already called.
     */
    auto window = static_cast<gui::MainWindow *>(object);

    if (programWindow_ == window) {
        programWindow_ = NULL;
    } else {
        function2window_.erase(window2function_[window]);
        window2function_.erase(window);
    }

    window2widget_.erase(window);
}

std::unique_ptr<gui::Project> SmartDecPlugin::createIdaProject() const {
    auto project = std::make_unique<gui::Project>();
    auto module = project->module().get();

    /* Set architecture. */
    module->setArchitecture(IdaFrontend::architecture());

    /* Set demangler. */
    module->setDemangler(std::make_unique<IdaDemangler>());

    /* Set image byte source. */
    module->image()->setExternalByteSource(std::make_unique<IdaByteSource>());

    /* Create sections. */
    IdaFrontend::createSections(module);

    /* Add function names. */
    foreach(ByteAddr address, IdaFrontend::functionStarts()) {
        QString name = IdaFrontend::functionName(address);

        if (!name.isEmpty()) {
            module->addName(address, name);
        }
    }

    /* Add imported function names. */
    typedef std::pair<ByteAddr, QString> Import;
    foreach(const Import &import, IdaFrontend::importedFunctions()) {
        module->addName(import.first, import.second);
    }

    return project;
}

NC_IDA_REGISTER_PLUGIN(SmartDecPlugin);

}} // namespace nc::ida

/* vim:set et sts=4 sw=4: */
