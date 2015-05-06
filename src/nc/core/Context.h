/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <memory> /* For std::unique_ptr. */

#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>

#include <QObject>

#include <nc/common/CancellationToken.h>
#include <nc/common/LogToken.h>
#include <nc/common/Range.h> /* For nc::find(). */
#include <nc/common/Types.h>

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

namespace nc {
namespace core {

namespace arch {
    class Instructions;
}

namespace image {
    class ByteSource;
    class Section;
}

namespace ir {
    class Function;
    class Functions;
    class Program;

    namespace calls {
        class CallsData;
        class CallingConventionDetector;
    }
    namespace cflow {
        class Graph;
    }
    namespace dflow {
        class Dataflow;
    }
    namespace misc {
        class TermToFunction;
    }
    namespace types {
        class Types;
    }
    namespace usage {
        class Usage;
    }
    namespace vars {
        class Variables;
    }
}

namespace likec {
    class Tree;
}

class Module;

/**
 * Decompilation context.
 *
 * This class and manages all the information known about the decompiled program.
 */
class Context: public QObject {
    Q_OBJECT

    std::shared_ptr<Module> module_; ///< Module being decompiled.
    std::shared_ptr<const arch::Instructions> instructions_; ///< Instructions being decompiled.
    std::unique_ptr<ir::Program> program_; ///< Program.
    std::unique_ptr<ir::Functions> functions_; ///< Functions.
    std::unique_ptr<ir::misc::TermToFunction> termToFunction_; ///< Term to function mapping.
    std::unique_ptr<ir::calls::CallsData> callsData_; ///< Calls data.
    std::unique_ptr<ir::calls::CallingConventionDetector> callingConventionDetector_; ///< Detector of calling conventions.
    boost::unordered_map<const ir::Function *, std::unique_ptr<ir::dflow::Dataflow> > dataflows_; ///< Dataflow information.
    boost::unordered_map<const ir::Function *, std::unique_ptr<ir::usage::Usage> > usages_; ///< Term usage information.
    boost::unordered_map<const ir::Function *, std::unique_ptr<ir::types::Types> > types_; ///< Information about types.
    boost::unordered_map<const ir::Function *, std::unique_ptr<ir::vars::Variables> > variables_; ///< Reconstructed variables.
    boost::unordered_map<const ir::Function *, std::unique_ptr<ir::cflow::Graph> > regionGraphs_; ///< Region graphs.
    std::unique_ptr<likec::Tree> tree_; ///< Representation of LikeC program.
    LogToken logToken_; ///< Log token.
    CancellationToken cancellationToken_; ///< Cancellation token.

public:
    /**
     * Class constructor.
     */
    Context();

    /**
     * Class destructor.
     */
    ~Context();

    /**
     * \return Valid pointer to the module being decompiled.
     */
    std::shared_ptr<Module> module() const { return module_; }

    /**
     * Sets the module.
     *
     * \param module Valid pointer to a module.
     */
    void setModule(const std::shared_ptr<Module> &module);

    /**
     * \returns Valid pointer to the instructions being decompiled.
     */
    const std::shared_ptr<const arch::Instructions> &instructions() const { return instructions_; }

    /**
     * Sets the set instructions of the executable file.
     *
     * \param instructions New set of instructions.
     */
    void setInstructions(const std::shared_ptr<const arch::Instructions> &instructions);

    /**
     * Sets the control flow graph of the program.
     *
     * \param program Valid pointer to the program CFG.
     */
    void setProgram(std::unique_ptr<ir::Program> program);

    /**
     * \return Pointer to the program. Can be NULL.
     */
    const ir::Program *program() const { return program_.get(); }

    /**
     * Sets the set of functions.
     *
     * \param functions Valid pointer to the set of functions.
     */
    void setFunctions(std::unique_ptr<ir::Functions> functions);

    /**
     * \return Pointer to the set of functions. Can be NULL.
     */
    ir::Functions *functions() const { return functions_.get(); }

    /**
     * Sets the calls data.
     *
     * \param callsData Valid pointer to a calls data.
     */
    void setCallsData(std::unique_ptr<ir::calls::CallsData> callsData);

    /**
     * \return Valid pointer to the information on calling conventions of functions.
     */
    ir::calls::CallsData *callsData() { return callsData_.get(); }

    /**
     * Sets the calling convention detector.
     *
     * \param detector Valid pointer to the detector.
     */
    void setCallingConventionDetector(std::unique_ptr<ir::calls::CallingConventionDetector> detector);

    /**
     * \return Valid pointer to the calling convention detector.
     */
    ir::calls::CallingConventionDetector *callingConventionDetector() { return callingConventionDetector_.get(); }

    /**
     * Sets the term to function mapping.
     *
     * \param termToFunction Valid pointer to the term to function mapping.
     */
    void setTermToFunction(std::unique_ptr<ir::misc::TermToFunction> termToFunction);

    /**
     * \return Valid pointer to the term to function mapping.
     */
    const ir::misc::TermToFunction *termToFunction() const { return termToFunction_.get(); }

    /**
     * Sets the dataflow information for a function.
     *
     * \param[in] function Valid pointer to a function.
     * \param[in] dataflow Dataflow information.
     */
    void setDataflow(const ir::Function *function, std::unique_ptr<ir::dflow::Dataflow> dataflow);

    /**
     * \param[in] function Valid pointer to a function.
     *
     * \return Pointer to the dataflow information for a given function. Can be NULL.
     */
    const ir::dflow::Dataflow *getDataflow(const ir::Function *function) const;

    /**
     * Sets the dataflow information for a function.
     *
     * \param[in] function Valid pointer to a function.
     * \param[in] usage Valid pointer to the usage information.
     */
    void setUsage(const ir::Function *function, std::unique_ptr<ir::usage::Usage> usage);

    /**
     * \param[in] function Valid pointer to a function.
     *
     * \return Pointer to the usage information. Can be NULL.
     */
    const ir::usage::Usage *getUsage(const ir::Function *function) const;

    /**
     * Sets the type information for a function.
     *
     * \param[in] function Valid pointer to a function.
     * \param[in] types Valid pointer to the type information.
     */
    void setTypes(const ir::Function *function, std::unique_ptr<ir::types::Types> types);

    /**
     * \param[in] function Valid pointer to a function.
     *
     * \return Pointer to the type information for a given function. Can be NULL.
     */
    const ir::types::Types *getTypes(const ir::Function *function) const;

    /**
     * Sets the information about variables for a function.
     *
     * \param[in] function Valid pointer to a function.
     * \param[in] variables Valid pointer to the information about variables.
     */
    void setVariables(const ir::Function *function, std::unique_ptr<ir::vars::Variables> variables);

    /**
     * \param[in] function Valid pointer to a function.
     *
     * \return Pointer to the information about variables for a given function. Can be NULL.
     */
    const ir::vars::Variables *getVariables(const ir::Function *function) const;

    /**
     * Sets the region graph for a function.
     *
     * \param[in] function Valid pointer to a function.
     * \param[in] graph Valid pointer to the region graph.
     */
    void setRegionGraph(const ir::Function *function, std::unique_ptr<ir::cflow::Graph> graph);

    /**
     * \param[in] function Valid pointer to a function.
     *
     * \return Valid pointer to the region graph of the given function.
     */
    const ir::cflow::Graph *getRegionGraph(const ir::Function *function) const;

    /**
     * Sets the LikeC tree.
     *
     * \param tree Valid pointer to the LikeC tree.
     */
    void setTree(std::unique_ptr<likec::Tree> tree);

    /**
     * \return The LikeC tree. Can be NULL.
     */
    likec::Tree *tree() const { return tree_.get(); }

    /**
     * Sets cancellation token.
     *
     * \param token Cancellation token.
     */
    void setCancellationToken(const CancellationToken &token) { cancellationToken_ = token; }

    /**
     * \return Cancellation token.
     */
    const CancellationToken &cancellationToken() const { return cancellationToken_; }

    /**
     * Sets the log token.
     *
     * \param token log token.
     */
    void setLogToken(const LogToken &token) { logToken_ = token; }

    /**
     * \return Log token.
     */
    const LogToken &logToken() const { return logToken_; }

    public Q_SLOTS:

    // TODO: remove all functions in this section.

    /**
     * Parse file.
     *
     * \param filename Name of the file to parse.
     */
    void parse(const QString &filename);

    /**
     * Disassembles all code sections.
     */
    void disassemble();

    /**
     * Disassembles an image section.
     *
     * \param section Valid pointer to the image section.
     */
    void disassemble(const image::Section *section);

    /*
     * Disassembles all instructions in the given range of addresses.
     *
     * \param source Valid pointer to a byte source.
     * \param begin First address in the range.
     * \param end First address past the range.
     */
    void disassemble(const image::ByteSource *source, ByteAddr begin, ByteAddr end);

    /**
     * Decompile everything.
     * The context must be clean, i.e. not decompiled before.
     */
    void decompile();

    Q_SIGNALS:

    /**
     * Signal emitted when the set of instructions is changed.
     */
    void instructionsChanged();

    /**
     * Signal emitted when C tree is computed.
     */
    void treeChanged();
};

} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
