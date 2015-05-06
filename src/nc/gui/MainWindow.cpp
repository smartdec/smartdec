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

#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QProgressDialog>
#include <QSettings>
#include <QStatusBar>
#include <QTextStream>
#include <QTreeView>

#include <nc/common/Exception.h>
#include <nc/common/Foreach.h>
#include <nc/common/GitSHA1.h>
#include <nc/common/make_unique.h>
#include <nc/common/SignalLogger.h>
#include <nc/core/Module.h>
#include <nc/core/Context.h>
#include <nc/core/arch/Instructions.h>
#include <nc/core/image/Image.h>
#include <nc/core/image/Section.h>
#include <nc/core/ir/Program.h>

#include "Command.h"
#include "CommandQueue.h"
#include "CxxDocument.h"
#include "CxxView.h"
#include "DisassemblyDialog.h"
#include "InspectorModel.h"
#include "InspectorView.h"
#include "InstructionsModel.h"
#include "InstructionsView.h"
#include "LogView.h"
#include "Project.h"
#include "SectionsModel.h"
#include "SectionsView.h"

namespace nc { namespace gui {

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent)
{
    setDockNestingEnabled(true);
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    createWidgets();
    createActions();
    createMenus();

    auto logger = std::make_shared<SignalLogger>();
    connect(logger.get(), SIGNAL(message(const QString &)), logView_, SLOT(log(const QString &)));
    connect(logger.get(), SIGNAL(message(const QString &)), progressDialog_, SLOT(setLabelText(const QString &)));
    connect(logger.get(), SIGNAL(message(const QString &)), this, SLOT(setStatusText(const QString &)));

    logToken_ = LogToken(logger);

    settings_ = new QSettings("decompilation.info", "SmartDec", this);
    loadSettings();

    updateGuiState();
}

MainWindow::~MainWindow() {}

void MainWindow::createWidgets() {
    statusLabel_ = new QLabel(statusBar());
    statusLabel_->setMargin(3);

    statusProgressBar_ = new QProgressBar(statusBar());
    statusProgressBar_->setTextVisible(false);
    statusProgressBar_->setRange(0, 0);

    statusBar()->addPermanentWidget(statusLabel_, 1);
    statusBar()->addPermanentWidget(statusProgressBar_, 0);

    instructionsView_ = new InstructionsView(this);
    instructionsView_->setModel(new InstructionsModel(this));
    instructionsView_->setObjectName("InstructionsView");
    addDockWidget(Qt::LeftDockWidgetArea, instructionsView_);

    connect(instructionsView_, SIGNAL(instructionSelectionChanged()), this, SLOT(highlightInstructionsInCxx()));
    connect(instructionsView_, SIGNAL(deleteSelectedInstructions()), this, SLOT(deleteSelectedInstructions()));
    connect(instructionsView_, SIGNAL(decompileSelectedInstructions()), this, SLOT(decompileSelectedInstructions()));
    connect(instructionsView_, SIGNAL(contextMenuCreated(QMenu *)), this, SLOT(populateInstructionsContextMenu(QMenu *)));

    cxxView_ = new CxxView(this);
    cxxView_->setDocument(new CxxDocument(this));
    cxxView_->setFeatures(QDockWidget::DockWidgetFeatures());
    cxxView_->setObjectName("CxxView");
    setCentralWidget(cxxView_);

    connect(cxxView_, SIGNAL(status(const QString &)), this, SLOT(setStatusText(const QString &)));
    connect(cxxView_, SIGNAL(instructionSelectionChanged()), this, SLOT(highlightCxxInInstructions()));
    connect(cxxView_, SIGNAL(nodeSelectionChanged()), this, SLOT(highlightCxxInTree()));
    connect(cxxView_, SIGNAL(contextMenuCreated(QMenu *)), this, SLOT(populateCxxContextMenu(QMenu *)));

    sectionsView_ = new SectionsView(this);
    sectionsView_->setModel(new SectionsModel(this));
    sectionsView_->setObjectName("SectionsView");
    addDockWidget(Qt::RightDockWidgetArea, sectionsView_);
    sectionsView_->hide();

    connect(sectionsView_, SIGNAL(contextMenuCreated(QMenu *)), this, SLOT(populateSectionsContextMenu(QMenu *)));

    inspectorView_ = new InspectorView(this);
    inspectorView_->setModel(new InspectorModel(this));
    inspectorView_->setObjectName("InspectorView");
    addDockWidget(Qt::RightDockWidgetArea, inspectorView_);
    inspectorView_->hide();

    connect(inspectorView_, SIGNAL(nodeSelectionChanged()), this, SLOT(highlightTreeInCxx()));
    connect(inspectorView_, SIGNAL(instructionSelectionChanged()), this, SLOT(highlightTreeInInstructions()));

    logView_ = new LogView(this);
    logView_->setObjectName("LogView");
    addDockWidget(Qt::BottomDockWidgetArea, logView_);
    logView_->hide();

    connect(logView_, SIGNAL(status(const QString &)), this, SLOT(setStatusText(const QString &)));

    disassemblyDialog_ = new DisassemblyDialog(this);
    connect(disassemblyDialog_, SIGNAL(accepted()), this, SLOT(disassembleSelectedSectionRange()));

    progressDialog_ = new QProgressDialog(this);
    QProgressBar *progressBar = new QProgressBar(progressDialog_);
    progressBar->setTextVisible(false);
    progressDialog_->setBar(progressBar);
    progressDialog_->setRange(0, 0);
    progressDialog_->setWindowModality(Qt::WindowModal);
    progressDialog_->setWindowTitle(windowTitle());
}

void MainWindow::createActions() {
    openAction_ = new QAction(tr("&Open..."), this);
    openAction_->setShortcuts(QKeySequence::Open);
    connect(openAction_, SIGNAL(triggered()), this, SLOT(open()));

    exportCfgAction_ = new QAction(tr("&Export CFG..."), this);
    connect(exportCfgAction_, SIGNAL(triggered()), this, SLOT(exportCfg()));

    quitAction_ = new QAction(tr("&Quit"), this);
    quitAction_->setShortcuts(QKeySequence::Quit);
    connect(quitAction_, SIGNAL(triggered()), this, SLOT(close()));

    disassembleAction_ = new QAction(tr("Di&sassemble..."), this);
    disassembleAction_->setShortcut(Qt::CTRL + Qt::Key_I);
    connect(disassembleAction_, SIGNAL(triggered()), this, SLOT(disassemble()));

    decompileAction_ = new QAction(tr("&Decompile"), this);
    decompileAction_->setShortcut(Qt::CTRL + Qt::Key_D);
    connect(decompileAction_, SIGNAL(triggered()), this, SLOT(decompile()));

    cancelAllAction_ = new QAction(tr("&Cancel All"), this);
    cancelAllAction_->setShortcut(Qt::CTRL + Qt::Key_Z);

    decompileAutomaticallyAction_ = new QAction(tr("Decompile &Automatically"), this);
    decompileAutomaticallyAction_->setCheckable(true);
    connect(decompileAutomaticallyAction_, SIGNAL(toggled(bool)), this, SLOT(setDecompileAutomatically(bool)));

    instructionsViewAction_ = instructionsView_->toggleViewAction();
    instructionsViewAction_->setText(tr("&Instructions"));
    instructionsViewAction_->setShortcut(Qt::ALT + Qt::Key_I);

    sectionsViewAction_ = sectionsView_->toggleViewAction();
    sectionsViewAction_->setText(tr("&Sections"));
    sectionsViewAction_->setShortcut(Qt::ALT + Qt::Key_S);

    inspectorViewAction_ = inspectorView_->toggleViewAction();
    inspectorViewAction_->setText(tr("Inspec&tor"));
    inspectorViewAction_->setShortcut(Qt::ALT + Qt::Key_T);

    logViewAction_ = logView_->toggleViewAction();
    logViewAction_->setText(tr("&Log"));
    logViewAction_->setShortcut(Qt::ALT + Qt::Key_L);

    aboutQtAction_ = new QAction(tr("About &Qt"), this);
    connect(aboutQtAction_, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    aboutAction_ = new QAction(tr("&About SmartDec"), this);
    connect(aboutAction_, SIGNAL(triggered()), this, SLOT(about()));

    deleteSelectedInstructionsAction_ = new QAction(tr("Delete"), this);
    deleteSelectedInstructionsAction_->setShortcut(Qt::Key_Delete);
    deleteSelectedInstructionsAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(deleteSelectedInstructionsAction_, SIGNAL(triggered()), this, SLOT(deleteSelectedInstructions()));
    instructionsView_->treeView()->addAction(deleteSelectedInstructionsAction_);

    decompileSelectedInstructionsAction_ = new QAction(tr("Decompile"), this);
    decompileSelectedInstructionsAction_->setShortcut(Qt::CTRL + Qt::Key_E);
    decompileSelectedInstructionsAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(decompileSelectedInstructionsAction_, SIGNAL(triggered()), this, SLOT(decompileSelectedInstructions()));
    instructionsView_->treeView()->addAction(decompileSelectedInstructionsAction_);
}

void MainWindow::createMenus() {
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAction_);
    fileMenu->addSeparator();
    fileMenu->addAction(exportCfgAction_);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction_);

    QMenu *analyseMenu = menuBar()->addMenu(tr("&Analyse"));
    analyseMenu->addAction(disassembleAction_);
    analyseMenu->addSeparator();
    analyseMenu->addAction(decompileAction_);
    analyseMenu->addAction(decompileAutomaticallyAction_);
    analyseMenu->addSeparator();
    analyseMenu->addAction(cancelAllAction_);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(instructionsViewAction_);
    viewMenu->addAction(sectionsViewAction_);
    viewMenu->addAction(inspectorViewAction_);
    viewMenu->addAction(logViewAction_);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutQtAction_);
    helpMenu->addAction(aboutAction_);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::loadSettings() {
    if (parent() == NULL) {
        restoreGeometry(settings_->value("geometry").toByteArray());
    }
    restoreState(settings_->value("windowState").toByteArray());
    setDecompileAutomatically(settings_->value("decompileAutomatically", true).toBool());
}

void MainWindow::saveSettings() {
    if (parent() == NULL) {
        settings_->setValue("geometry", saveGeometry());
    }
    settings_->setValue("windowState", saveState());
    settings_->setValue("decompileAutomatically", decompileAutomatically());
}

void MainWindow::updateGuiState() {
    exportCfgAction_->setEnabled(project() != NULL);
    disassembleAction_->setEnabled(project() != NULL);
    decompileAction_->setEnabled(project() != NULL);
    cancelAllAction_->setEnabled(project() != NULL && project()->commandQueue()->front() != NULL);

    if (project() && !project()->name().isEmpty()) {
        setWindowTitle(tr("%1 - SmartDec").arg(project()->name()));
    } else {
        setWindowTitle("SmartDec");
    }

    if (project() && project()->commandQueue()->front() && !project()->commandQueue()->front()->isBackground()) {
        progressDialog_->show();
    } else {
        progressDialog_->hide();
    }

    if (project() && project()->commandQueue()->front()) {
        statusProgressBar_->show();
    } else {
        statusProgressBar_->hide();
    }
}

void MainWindow::setWindowTitle(const QString &title) {
    if (windowTitle() != title) {
        QMainWindow::setWindowTitle(title);
        Q_EMIT windowTitleChanged(windowTitle());
    }
}

void MainWindow::open() {
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Which file or files should I decompile?"));

    open(filenames);
}

void MainWindow::open(const QStringList &filenames) {
    if (filenames.empty()) {
        return;
    }

    auto context = std::make_shared<core::Context>();
    context->setLogToken(logToken_);

    foreach (const QString &filename, filenames) {
        try {
            context->parse(filename);
        } catch (const nc::Exception &e) {
            QMessageBox::critical(this, tr("Error"), e.unicodeWhat());
            return;
        } catch (const std::exception &e) {
            QMessageBox::critical(this, tr("Error"), e.what());
            return;
        }
    }

    auto project = std::make_unique<gui::Project>();
    project->setName(QFileInfo(filenames.front()).fileName());
    project->setContext(context);
    project->setModule(context->module());
    project->setInstructions(context->instructions());

    open(std::move(project));

    if (project_->instructions()->empty()) {
        project_->disassemble();
    }

    if (decompileAutomatically()) {
        project_->decompile();
    }
}

void MainWindow::open(std::unique_ptr<Project> project) {
    assert(project);

    project_ = std::move(project);

    sectionsView_->model()->setModule();
    disassemblyDialog_->setModule();
    instructionsView_->model()->setInstructions();
    cxxView_->document()->setContext();
    inspectorView_->model()->setContext();

    /* Log messages to the log window. */
    project_->setLogToken(logToken_);

    /* Connect the project to the slots for updating views. */
    connect(project_.get(), SIGNAL(nameChanged()), this, SLOT(updateGuiState()));
    connect(project_.get(), SIGNAL(moduleChanged()), this, SLOT(moduleChanged()));
    connect(project_.get(), SIGNAL(instructionsChanged()), this, SLOT(instructionsChanged()));
    connect(project_.get(), SIGNAL(treeChanged()), this, SLOT(treeChanged()));

    /* Connect the project to the progress dialog. */
    connect(project_->commandQueue(), SIGNAL(nextCommand()), this, SLOT(updateGuiState()));
    connect(project_->commandQueue(), SIGNAL(idle()), this, SLOT(updateGuiState()));
    connect(progressDialog_, SIGNAL(canceled()), project_.get(), SLOT(cancelAll()));

    /* Delegate "Cancel All" to the project. */
    connect(cancelAllAction_, SIGNAL(triggered()), project_.get(), SLOT(cancelAll()));

    updateGuiState();

    moduleChanged();
    instructionsChanged();
    treeChanged();
}

void MainWindow::moduleChanged() {
    sectionsView_->model()->setModule(project()->module());
    disassemblyDialog_->setModule(project()->module());
}

void MainWindow::instructionsChanged() {
    instructionsView_->model()->setInstructions(project()->instructions());
}

void MainWindow::treeChanged() {
    cxxView_->document()->setContext(project()->context());
    inspectorView_->model()->setContext(project()->context());
}

void MainWindow::populateInstructionsContextMenu(QMenu *menu) {
    if (!instructionsView_->selectedInstructions().empty()) {
        menu->addSeparator();
        menu->addAction(deleteSelectedInstructionsAction_);
        menu->addSeparator();
        menu->addAction(decompileSelectedInstructionsAction_);
    }
}

void MainWindow::populateCxxContextMenu(QMenu *menu) {
    if (auto address = cxxView_->getSelectedInteger()) {
        menu->addAction(tr("Jump to address %1").arg(*address, 0, 16), this, SLOT(jumpToAddress()));
    }
}

void MainWindow::populateSectionsContextMenu(QMenu *menu) {
    if (sectionsView_->selectedSection()) {
        menu->addSeparator();
        menu->addAction(tr("Disassemble..."), this, SLOT(disassembleSelectedSection()));
    }
}

void MainWindow::exportCfg() {
    if (!project()) {
        return;
    }

    std::shared_ptr<const core::Context> context = project()->context();

    if (!context->program()) {
        QMessageBox::critical(this, tr("Error"), tr("Sorry, no control flow graph was built yet. Try to decompile something first."));
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this, tr("Where should I save the control flow graph?"), QString(), tr("Graphviz (*.dot);;All Files(*)"));
    if (!filename.isEmpty()) {
        QFile file(filename);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QMessageBox::critical(this, tr("Error"), tr("File %1 could not be opened for writing.").arg(filename));
        }

        QTextStream out(&file);
        out << *context->program();
    }
}

void MainWindow::disassemble() {
    if (!project()) {
        return;
    }
    if (project()->module()->image()->sections().empty()) {
        QMessageBox::critical(this, tr("Error"), tr("Sorry, the file you are currently working on does not contain any information about sections. There is nothing I could disassemble."));
        return;
    }
    disassemblyDialog_->show();
}

void MainWindow::disassembleSelectedSectionRange() {
    if (!project()) {
        return;
    }
    if (decompileAutomatically()) {
        project()->cancelAll();
    }
    project()->disassemble(disassemblyDialog_->selectedSection(), *disassemblyDialog_->startAddress(), *disassemblyDialog_->endAddress());
    if (decompileAutomatically()) {
        project()->decompile();
    }
}

void MainWindow::disassembleSelectedSection() {
    disassemblyDialog_->selectSection(sectionsView_->selectedSection());
    disassemblyDialog_->show();
}

void MainWindow::deleteSelectedInstructions() {
    if (!project()) {
        return;
    }
    if (decompileAutomatically()) {
        project()->cancelAll();
    }
    project()->deleteInstructions(instructionsView_->selectedInstructions());
    if (decompileAutomatically()) {
        project()->decompile();
    }
}

void MainWindow::decompile() {
    if (!project()) {
        return;
    }
    project()->cancelAll();
    project()->decompile();
}

void MainWindow::decompileSelectedInstructions() {
    if (!project()) {
        return;
    }
    project()->cancelAll();
    project()->decompile(instructionsView_->selectedInstructions());
}

bool MainWindow::decompileAutomatically() const {
    return decompileAutomaticallyAction_->isChecked();
}

void MainWindow::setDecompileAutomatically(bool value) {
    decompileAutomaticallyAction_->setChecked(value);
}

void MainWindow::highlightInstructionsInCxx() {
    if (cxxView_->isVisible()) {
        /* Block signals, in order to avoid backfire. */
        cxxView_->blockSignals(true);
        cxxView_->highlightInstructions(instructionsView_->selectedInstructions());
        cxxView_->blockSignals(false);
    }
}

void MainWindow::highlightCxxInInstructions() {
    if (instructionsView_->isVisible()) {
        instructionsView_->blockSignals(true);
        instructionsView_->highlightInstructions(cxxView_->selectedInstructions());
        instructionsView_->blockSignals(false);
    }
}

void MainWindow::highlightTreeInCxx() {
    if (cxxView_->isVisible()) {
        cxxView_->blockSignals(true);
        cxxView_->highlightNodes(inspectorView_->selectedNodes());
        cxxView_->blockSignals(false);
    }
}

void MainWindow::highlightTreeInInstructions() {
    if (instructionsView_->isVisible()) {
        instructionsView_->blockSignals(true);
        instructionsView_->highlightInstructions(inspectorView_->selectedInstructions());
        instructionsView_->blockSignals(false);
    }
}

void MainWindow::highlightCxxInTree() {
    if (inspectorView_->isVisible()) {
        /* Prevent things from getting slow... */
        if (cxxView_->selectedNodes().size() < 100) {
            inspectorView_->blockSignals(true);
            inspectorView_->highlightNodes(cxxView_->selectedNodes());
            inspectorView_->blockSignals(false);
        }
    }
}

void MainWindow::jumpToAddress() {
    if (auto address = cxxView_->getSelectedInteger()) {
        if (jumpToAddress(*address)) {
            instructionsView_->show();
        }
    }
}

bool MainWindow::jumpToAddress(ByteAddr address) {
    if (!project()) {
        return false;
    }

    if (auto instruction = project()->instructions()->getCovering(address)) {
        instructionsView_->highlightInstructions(std::vector<const core::arch::Instruction *>(1, instruction.get()));
        return true;
    } else {
        setStatusText(tr("There is no instruction at address %1.").arg(address, 0, 16));
        return false;
    }
}

void MainWindow::setStatusText(const QString &text) {
    statusLabel_->setText(text);
}

void MainWindow::about() {
    QMessageBox::about(this, tr("About SmartDec"), tr(
        "<h3>About SmartDec</h3>"
        "<p>SmartDec is a retargetable native code to C/C++ decompiler.</p>"
        "<p>This is revision %1 built on %2.</p>"
        "<p>SmartDec supports the following architectures:<ul>"
        "<li>Intel x86,</li>"
        "<li>Intel x86-64.</li>"
        "</ul></p>"
        "<p>SmartDec supports the following input file formats:<ul>"
        "<li>ELF (32 and 64-bit),</li>"
        "<li>PE (32 and 64-bit).</li>"
        "</ul></p>"
        "<p>Report bugs to <a href=\"mailto:yegor.derevenets@gmail.com\">yegor.derevenets@gmail.com</a>.</p>"
        "<p>SmartDec home page: <a href=\"http://decompilation.info/\">decompilation.info</a>.</p>"
        ).arg(QString(git_sha1).left(5)).arg(__DATE__));
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
