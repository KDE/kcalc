/*
 *  Copyright (c) 2021 Antonio Prcela <antonio.prcela@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
}

//------------------------------------------------------------------------------
// Name: KCalcHistory
// Desc: destructor
//------------------------------------------------------------------------------
KCalcHistory::~KCalcHistory()
{
}

//------------------------------------------------------------------------------
// Name: addToHistory
// Desc: Adds the latest calculations to  history window
//------------------------------------------------------------------------------
void KCalcHistory::addToHistory(const QString &str, bool set_new_line_)
{
    // add_new_line is false on launch and after clearHistory()
    // so the first line doesn't get an unnecessary new line
    if (add_new_line) {
        insertHtml(QStringLiteral("<br>"));
        moveCursor(QTextCursor::Start);
        add_new_line = false;
    }

    insertHtml(str);

    if (set_new_line_) {
        moveCursor(QTextCursor::Start);
        add_new_line = true;
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
    add_new_line = false;
}

void KCalcHistory::changeSettings()
{
    QPalette pal = palette();

    pal.setColor(QPalette::Text, KCalcSettings::foreColor());
    pal.setColor(QPalette::Base, KCalcSettings::backColor());

    setPalette(pal);

    setFont(KCalcSettings::historyFont());
}
