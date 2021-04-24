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

#include <boost/array.hpp>

#include <QSyntaxHighlighter>
#include <QSet>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

namespace nc { namespace gui {

/**
 * An object storing the formatting information used for C++ highlighting. It
 * has to be inherited from QWidget, otherwise, styling via Qt style sheets
 * does not work.
 */
class CxxFormatting: public QWidget {
    Q_OBJECT

public:
    enum Element {
        /** Normal text. */
        TEXT, 

        /* Following elements can be matched with simple regexps. */
        SINGLE_LINE_COMMENT,
        KEYWORD, 
        OPERATOR,
        NUMBER,
        ESCAPE_CHAR,

        /* Following elements need states on lines. */
        MACRO, 
        MULTI_LINE_COMMENT, 
        STRING,

        ITEM_COUNT
    };

    CxxFormatting(QWidget *parent);

    /**
     * \param[in] element Text element.
     *
     * \return Formatting that must be used for that text element.
     */
    const QTextCharFormat &getFormat(Element element) const {
        return formats_[element];
    }

private:
    /** Formats of the text elements. */
    boost::array<QTextCharFormat, ITEM_COUNT> formats_;

    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor STORED false)
    Q_PROPERTY(QColor singleLineCommentColor READ singleLineCommentColor WRITE setSingleLineCommentColor STORED false)
    Q_PROPERTY(QColor multiLineLineCommentColor READ multiLineCommentColor WRITE setMultiLineCommentColor STORED false)
    Q_PROPERTY(QColor keywordColor READ keywordColor WRITE setKeywordColor STORED false)
    Q_PROPERTY(QColor operatorColor READ operatorColor WRITE setOperatorColor STORED false)
    Q_PROPERTY(QColor numberColor READ numberColor WRITE setNumberColor STORED false)
    Q_PROPERTY(QColor macroColor READ macroColor WRITE setMacroColor STORED false)
    Q_PROPERTY(QColor stringColor READ stringColor WRITE setStringColor STORED false)
    Q_PROPERTY(QColor escapeCharColor READ escapeCharColor WRITE setEscapeCharColor STORED false)

    void setTextColor(QColor color) { formats_[TEXT].setForeground(color); }
    QColor textColor() const { return formats_[TEXT].foreground().color(); }

    void setSingleLineCommentColor(QColor color) { formats_[SINGLE_LINE_COMMENT].setForeground(color); }
    QColor singleLineCommentColor() const { return formats_[SINGLE_LINE_COMMENT].foreground().color(); }

    void setMultiLineCommentColor(QColor color) { formats_[MULTI_LINE_COMMENT].setForeground(color); }
    QColor multiLineCommentColor() const { return formats_[MULTI_LINE_COMMENT].foreground().color(); }

    void setKeywordColor(QColor color) { formats_[KEYWORD].setForeground(color); }
    QColor keywordColor() const { return formats_[KEYWORD].foreground().color(); }

    void setOperatorColor(QColor color) { formats_[OPERATOR].setForeground(color); }
    QColor operatorColor() const { return formats_[OPERATOR].foreground().color(); }

    void setNumberColor(QColor color) { formats_[NUMBER].setForeground(color); }
    QColor numberColor() const { return formats_[NUMBER].foreground().color(); }

    void setMacroColor(QColor color) { formats_[MACRO].setForeground(color); }
    QColor macroColor() const { return formats_[MACRO].foreground().color(); }

    void setStringColor(QColor color) { formats_[STRING].setForeground(color); }
    QColor stringColor() const { return formats_[STRING].foreground().color(); }

    void setEscapeCharColor(QColor color) { formats_[ESCAPE_CHAR].setForeground(color); }
    QColor escapeCharColor() const { return formats_[ESCAPE_CHAR].foreground().color(); }
};

/**
 * Syntax highlighter for C++.
 */
class CppSyntaxHighlighter: public QSyntaxHighlighter {
    Q_OBJECT
public:
    /**
     * Constructor.
     * 
     * \param[in] parent Pointer to the parent object. Can be nullptr.
     * \param[in] formatting Valid pointer to the formatting information.
     */
    explicit CppSyntaxHighlighter(QObject *parent, const CxxFormatting *formatting);

    /**
     * Virtual destructor.
     */
    virtual ~CppSyntaxHighlighter();

protected:
    virtual void highlightBlock(const QString &text) override;

private:
    bool processState(const QString &text, int *startPos, int *endPos);

    void processRegexp(QRegExp &regexp, CxxFormatting::Element element, const QString &text, int startPos = 0);

    void processRegexps(const QString &text, int startPos = 0);

    bool processPreprocessor(const QString &text);

    void processEscapeChar(const QString &text, int start = 0, int len = 0);

    int findStringEnd(const QString &text, int startPos = 0, QChar strChar = '"');

    int findMultilineCommentEnd(const QString &text, int startPos = 0);

    /** Keywords. */ 
    QSet<QString> mKeywords;

    /* Regular expressions. */
    QRegExp mIncludeRegexp;
    QRegExp mMacroRegexp;
    QRegExp mMultilineMacroRegexp;
    QRegExp mSpecialRegexp;
    QRegExp mNumberRegexp;
    QRegExp mOperatorRegexp;
    QRegExp mTextRegexp;

    const CxxFormatting *formatting_;
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
