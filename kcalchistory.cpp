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
    setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::LeftEdge}));

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
    // QTextEdit's cursor location might be changed by mouse clicks.
    // We have to move the cursor to correct position.

    //  Ensures the cursor goes back to topmost line's end during a calculation.
    //  1 + 1 + _
    if (!add_new_line_ && !set_new_line_) {
        moveCursor(QTextCursor::Start);
        moveCursor(QTextCursor::EndOfLine);
    }

    // add_new_line_ is false on launch and after clearHistory()
    // so the first line doesn't get an unnecessary new line
    if (add_new_line_) {
        moveCursor(QTextCursor::Start);
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
    addToHistory(/*QStringLiteral("&nbsp;=&nbsp;") +*/ display_content, true);
}

//------------------------------------------------------------------------------
// Name: addFuncToHistory
// Desc: Adds the current function symbol the history window
//------------------------------------------------------------------------------
void KCalcHistory::addFuncToHistory(const QString &func)
{
    QString textToHistory = QStringLiteral("&nbsp;") + func + QStringLiteral("&nbsp;");
    addToHistory(textToHistory, false);
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
    baseFont_ = font;
    updateFont();
}

//------------------------------------------------------------------------------
// Name: updateFont
// Desc: Update font using baseFont to better fit
//------------------------------------------------------------------------------
void KCalcHistory::updateFont(double zoomFactor)
{
    // Make a working copy of the font
    QFont newFont(baseFont());

    // Calculate actual font size by keeping the ratio, keeping previous zoomFactor, using historyFont as minimum size
    double ratio = (minimumSize().width() - contentsMargins().left() - contentsMargins().right()) / baseFont().pointSizeF();
    idealPointSizeF_ = contentsRect().width() / ratio;
    newFont.setPointSizeF(qMax(double(baseFont().pointSizeF()), idealPointSizeF_) * zoomFactor);

    // Apply font
    QTextEdit::setFont(newFont);
}

//------------------------------------------------------------------------------
// Name: baseFont
// Desc: Returns current baseFont
//------------------------------------------------------------------------------
const QFont& KCalcHistory::baseFont() const
{
    return baseFont_;
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

#include "moc_kcalchistory.cpp"
