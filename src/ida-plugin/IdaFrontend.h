/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

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

#include <functional>
#include <utility> /* For std::pair. */
#include <vector>

#include <QString>

#include <nc/common/ByteOrder.h>
#include <nc/common/RangeClass.h>
#include <nc/common/Types.h>
#include <nc/core/image/Platform.h>

QT_BEGIN_NAMESPACE
class QApplication;
class QWidget;
QT_END_NAMESPACE

QT_USE_NAMESPACE

namespace nc {

namespace core {
    namespace image {
        class Image;
    }
}

namespace ida {

typedef Range<ByteAddr> AddressRange;

/**
 * This class exposes some of IDA functionality while not polluting the global
 * namespace with IDA includes.
 */    
class IdaFrontend {
public:
    /**
     * \param[in] address              Address to move cursor in IDA code view to.
     *
     * \return True on success, false otherwise.
     */
    static bool jumpToAddress(ByteAddr address);

    /**
     * \param[in] address              Address of a function.
     * \returns                        Mangled name of a function starting at 
     *                                 the given address.
     */
    static QString functionName(ByteAddr address);

    /**
     * \param[in] address              Address.
     * \returns                        Name for the given address, or empty string
     *                                 if the given address does not have a name.
     */
    static QString addressName(ByteAddr address);

    /**
     * \returns                        Addresses and names of imported functions.
     */
    static std::vector<std::pair<ByteAddr, QString> > importedFunctions();

    /**
     * \returns                        Addresses of all function starts.
     */
    static std::vector<ByteAddr> functionStarts();

    /**
     * \param[in] mangled              Mangled name.
     * \returns                        Demangled name.
     */
    static QString demangle(const QString &mangled);

    /**
     * \returns The name of the archtecture of the current binary.
     */
    static QString architecture();

    /**
     * \returns The byte order of the currently opened executable.
     */
    static ByteOrder byteOrder();

    /**
     * \returns The operating system of the current binary.
     */
    static core::image::Platform::OperatingSystem operatingSystem();

    /**
     * Converts the current image file to nc's internal representation.
     *
     * \param[in, out] image Valid pointer to the executable file image.
     */
    static void createSections(core::image::Image *image);

    /**
     * \param[in] address              Any address in a function.
     * \returns                        All address ranges of a function, including function's tail chunks.
     */
    static std::vector<AddressRange> functionAddresses(ByteAddr address);

    /**
     * \returns                        The current address of a screen cursor.
     */
    static ByteAddr screenAddress();

    struct MenuItem;

    /**
     * Adds a menu item in the IDA's UI.
     *
     * param[in] path       Path to the menu into which the item must be inserted.
     * param[in] name       Name of the menu item.
     * param[in] after      Name of the menu item after which to insert the item.
     * param[in] hotkey     Hotkey for the menu item.
     * param[in] handler    The function to be called when the user selects the menu item.
     *
     * \returns A valid pointer to an opaque object to be passed to deleteMenuItem().
     */
    static MenuItem *addMenuItem(
        const QString &path,
        const QString &name,
        const QString &after,
        const QString &hotkey,
        std::function<void()> handler);

    /**
     * Deletes a menu item in the IDA's UI.
     *
     * param[in] menuItem Valid pointer to the menu item to delete.
     */
    static void deleteMenuItem(MenuItem *menuItem);

    /**
     * Prints a message to IDA console.
     */
    static void print(const QString &message);

    /**
     * Creates a child MDI widget using IDA's API.
     *
     * \param caption Title of the widget.
     *
     * \return Valid pointer to the created widget.
     */
    static QWidget *createWidget(const QString &caption);

    /**
     * Switches focus to the given widget.
     *
     * \param widget Valid pointer to the IDA's widget.
     */
    static void activateWidget(QWidget *widget);

    /**
     * Closes given IDA's widget.
     *
     * \param widget Valid pointer to the IDA's widget.
     */
    static void closeWidget(QWidget *widget);
};

}} // namespace nc::ida

/* vim:set et sts=4 sw=4: */
