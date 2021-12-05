/*
    SPDX-FileCopyrightText: 2021 Antonio Prcela <antonio.prcela@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalchistory.h"
#include "kcalc_settings.h"

//------------------------------------------------------------------------------
// Name: KCalcHistory
// Desc: constructor
//------------------------------------------------------------------------------
KCalcHistory::KCalcHistory(QWidget *parent)
    : QTextEdit(parent)
{
    setReadOnly(true);
    setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

    // Initialize idealPointSizeF_
    idealPointSizeF_ = currentFont().pointSizeF();
}

//------------------------------------------------------------------------------
// Name: KCalcHistory
// Desc: destructor
//------------------------------------------------------------------------------
KCalcHistory::~KCalcHistory() = default;

//------------------------------------------------------------------------------
// Name: addToHistory
// Desc: Adds the latest calculations to  history window
//------------------------------------------------------------------------------
void KCalcHistory::addToHistory(const QString &str, bool set_new_line_)
{
    // add_new_line_ is false on launch and after clearHistory()
    // so the first line doesn't get an unnecessary new line
    if (add_new_line_) {
        insertHtml(QStringLiteral("<br>"));
        moveCursor(QTextCursor::Start);
        add_new_line_ = false;
    }

    insertHtml(str);

    if (set_new_line_) {
        moveCursor(QTextCursor::Start);
        add_new_line_ = true;
    }

    setAlignment(Qt::AlignRight);
    ensureCursorVisible();
}

//------------------------------------------------------------------------------
// Name: addResultToHistory
// Desc: Used mostly for functions that are not in CalcEngine::Operation
//       adds "=" and the result with newline endings
//------------------------------------------------------------------------------
void KCalcHistory::addResultToHistory(const QString &display_content)
{
    addToHistory(QStringLiteral("&nbsp;=&nbsp;") + display_content, true);
}

//------------------------------------------------------------------------------
// Name: addFuncToHistory
// Desc: Adds the current function symbol, taken via CalcEngine::Operation
//       to the history window
//------------------------------------------------------------------------------
void KCalcHistory::addFuncToHistory(const CalcEngine::Operation FUNC)
{
    QString textToHistroy = QStringLiteral("&nbsp;");

    if (FUNC == CalcEngine::FUNC_PERCENT) {
        textToHistroy += QStringLiteral("%");
    } else if (FUNC == CalcEngine::FUNC_OR) {
        textToHistroy += QStringLiteral("OR");
    } else if (FUNC == CalcEngine::FUNC_XOR) {
        textToHistroy += QStringLiteral("XOR");
    } else if (FUNC == CalcEngine::FUNC_AND) {
        textToHistroy += QStringLiteral("AND");
    } else if (FUNC == CalcEngine::FUNC_LSH) {
        textToHistroy += QStringLiteral("Lsh");
    } else if (FUNC == CalcEngine::FUNC_RSH) {
        textToHistroy += QStringLiteral("Rsh");
    } else if (FUNC == CalcEngine::FUNC_ADD) {
        textToHistroy += QStringLiteral("+");
    } else if (FUNC == CalcEngine::FUNC_SUBTRACT) {
        textToHistroy += QStringLiteral("-");
    } else if (FUNC == CalcEngine::FUNC_MULTIPLY) {
        textToHistroy += QStringLiteral("×");
    } else if (FUNC == CalcEngine::FUNC_DIVIDE) {
        textToHistroy += QStringLiteral("÷");
    } else if (FUNC == CalcEngine::FUNC_MOD) {
        textToHistroy += QStringLiteral("Mod");
    } else if (FUNC == CalcEngine::FUNC_INTDIV) {
        textToHistroy += QStringLiteral("IntDiv");
    } else if (FUNC == CalcEngine::FUNC_BINOM) {
        textToHistroy += QStringLiteral("Binom");
    }

    textToHistroy += QStringLiteral("&nbsp;");
    addToHistory(textToHistroy, false);
}

//------------------------------------------------------------------------------
// Name: addFuncToHistory
// Desc: Adds the current function symbol the history window
//------------------------------------------------------------------------------
void KCalcHistory::addFuncToHistory(const QString &func)
{
    QString textToHistroy = QStringLiteral("&nbsp;") + func + QStringLiteral("&nbsp;");
    addToHistory(textToHistroy, false);
}

//------------------------------------------------------------------------------
// Name: clearHistory
// Desc: Clears the content of the history window
//------------------------------------------------------------------------------
void KCalcHistory::clearHistory()
{
    clear();
    add_new_line_ = false;
}

//------------------------------------------------------------------------------
// Name: changeSettings
// Desc:
//------------------------------------------------------------------------------
void KCalcHistory::changeSettings()
{
    QPalette pal = palette();

    pal.setColor(QPalette::Text, KCalcSettings::foreColor());
    pal.setColor(QPalette::Base, KCalcSettings::backColor());

    setPalette(pal);

    setFont(KCalcSettings::historyFont());
}

//------------------------------------------------------------------------------
// Name: setFont
// Desc: Set the base font and recalculate the font size to better fit
//------------------------------------------------------------------------------
void KCalcHistory::setFont(const QFont &font)
{
    // Overwrite current baseFont
    if (baseFont_) {
        delete baseFont_;
    }
    baseFont_ = new QFont(font);
    
    updateFont();
}

//------------------------------------------------------------------------------
// Name: updateFont
// Desc: Update font using baseFont to better fit
//------------------------------------------------------------------------------
void KCalcHistory::updateFont(double zoomFactor)
{
    // Make a working copy of the font
    QFont* newFont = new QFont(baseFont());

    // Calculate actual font size by keeping the ratio, keeping previous zoomFactor, using historyFont as minimum size
    double ratio = (minimumSize().width() - contentsMargins().left() - contentsMargins().right()) / baseFont().pointSizeF();
    idealPointSizeF_ = contentsRect().width() / ratio;
    newFont->setPointSizeF(qMax(double(baseFont().pointSizeF()), idealPointSizeF_) * zoomFactor);

    // Apply font
    QTextEdit::setFont(*newFont);
    
    // Free the memory
    delete newFont;
}

//------------------------------------------------------------------------------
// Name: baseFont
// Desc: Returns current baseFont
//------------------------------------------------------------------------------
const QFont& KCalcHistory::baseFont() const
{
    return *baseFont_;
}

//------------------------------------------------------------------------------
// Name: resizeEvent
// Desc: resize history and adjust font size
//------------------------------------------------------------------------------
void KCalcHistory::resizeEvent(QResizeEvent* event)
{
    QTextEdit::resizeEvent(event);

    // Determine current zoom
    double zoomFactor = currentFont().pointSizeF() / idealPointSizeF_;

    // Update font size
    updateFont(zoomFactor);

    updateGeometry();
}
