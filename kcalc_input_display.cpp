/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_input_display.h"

//------------------------------------------------------------------------------
// Name: KCalcInputDisplay
// Desc: constructor
//------------------------------------------------------------------------------
KCalcInputDisplay::KCalcInputDisplay(QWidget *parent)
    : QLineEdit(parent)
    , m_overwrite(false)
    , m_hardOverwrite(false)
    , m_hasResult(false)
{
    setFrame(false);
}

//------------------------------------------------------------------------------
// Name: ~KCalcInputDisplay
// Desc:
//------------------------------------------------------------------------------
KCalcInputDisplay::~KCalcInputDisplay() = default;

//------------------------------------------------------------------------------
// Name: reset
// Desc:
//------------------------------------------------------------------------------
void KCalcInputDisplay::reset(bool clearRedoUndo /*=false*/)
{
    this->clear();
    m_overwrite = false;
    m_hardOverwrite = false;
    m_hasResult = false;
    if (clearRedoUndo) {
        this->setText(QStringLiteral(""));
    }
}

//------------------------------------------------------------------------------
// Name: insertToken
// Desc:
//------------------------------------------------------------------------------
void KCalcInputDisplay::insertToken(const QString &token)
{
    if (m_overwrite || m_hardOverwrite) {
        this->clear();
        m_overwrite = false;
        m_hardOverwrite = false;
    }

    this->insert(token);
    m_hasResult = false;
}

//------------------------------------------------------------------------------
// Name: insertTokenNumeric
// Desc:
//------------------------------------------------------------------------------
void KCalcInputDisplay::insertTokenNumeric(const QString &token)
{
    if (m_overwrite || m_hardOverwrite || m_hasResult) {
        this->clear();
        m_overwrite = false;
        m_hardOverwrite = false;
        m_hasResult = false;
    }

    this->insert(token);
}

//------------------------------------------------------------------------------
// Name: insertTokenFunction
// Desc:
//------------------------------------------------------------------------------
void KCalcInputDisplay::insertTokenFunction(const QString &token)
{
    if (m_overwrite || m_hardOverwrite) {
        m_overwrite = false;
        m_hardOverwrite = false;
        this->clear();
        this->insert(token);
        this->insert(QStringLiteral("("));
    } else if (m_hasResult) {
        this->home(false);
        this->insert(token);
        this->insert(QStringLiteral("("));
        this->end(false);
        this->insert(QStringLiteral(")"));
    } else {
        this->insert(token);
        this->insert(QStringLiteral("("));
    }
}

//------------------------------------------------------------------------------
// Name: setHasResult
// Desc:
//------------------------------------------------------------------------------
void KCalcInputDisplay::setHasResult()
{
    m_hasResult = true;
    this->clearFocus();
}

//------------------------------------------------------------------------------
// Name: slotSetOverwrite
// Desc: when set, inserting to the display will clear it, and then insert.
//       (It can be reset refocusing the display)
//------------------------------------------------------------------------------
void KCalcInputDisplay::slotSetOverwrite()
{
    m_overwrite = true;
    this->clearFocus();
    // this->focusNextChild(); /* workaround to defocus the QWidget,
    // although it works, it can lead to
    // issues if the GUI changes */
}

//------------------------------------------------------------------------------
// Name: slotSetHardOverwrite
// Desc: when set, focusing or inserting to the display will clear it,
//       and then insert (if requested),
//------------------------------------------------------------------------------
void KCalcInputDisplay::slotSetHardOverwrite()
{
    m_hardOverwrite = true;
    m_overwrite = true;
    this->focusNextChild(); /* workaround to defocus the QWidget,
                               although it works, it can lead to
                               issues if the GUI changes */
}

//------------------------------------------------------------------------------
// Name: clearHasResult
// Desc:
//------------------------------------------------------------------------------
void KCalcInputDisplay::clearHasResult()
{
    m_hasResult = false;
}

//------------------------------------------------------------------------------
// Name: slotClearOverwrite
// Desc: resets Overwrite/hardOverwrite flags,
//       deletes finishing '=' when required
//------------------------------------------------------------------------------
void KCalcInputDisplay::slotClearOverwrite()
{
    m_overwrite = false;
    int tmpCursorPosition = this->cursorPosition();
    if (this->text().endsWith(QLatin1String("="))) {
        this->end(false);
        this->backspace();
    }
    this->setCursorPosition(tmpCursorPosition);

    if (m_hardOverwrite) {
        m_hardOverwrite = false;
        this->clear();
    }
}

//------------------------------------------------------------------------------
// Name: keyPressEvent
// Desc:
//------------------------------------------------------------------------------
void KCalcInputDisplay::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Undo)) {
        undoCalculation();
    } else if (event->matches(QKeySequence::Redo)) {
        redoCalculation();
    } else {
        QLineEdit::keyPressEvent(event);
    }
}

//------------------------------------------------------------------------------
// Name: undoCalculation
// Desc:
//------------------------------------------------------------------------------
void KCalcInputDisplay::undoCalculation()
{
    QLineEdit::undo();
    if (this->isUndoAvailable() && text().isEmpty()) {
        QLineEdit::undo();
        QLineEdit::deselect();
    }
}

//------------------------------------------------------------------------------
// Name: redoCalculation
// Desc:
//------------------------------------------------------------------------------
void KCalcInputDisplay::redoCalculation()
{
    QLineEdit::redo();
    if (this->isRedoAvailable() && text().isEmpty()) {
        QLineEdit::redo();
        QLineEdit::deselect();
    }
}

//------------------------------------------------------------------------------
// Name: focusInEvent
// Desc: focusIn, if hardOverwrite, display is cleared,
//       then clears overwrite and hardOverwrite flags.
//------------------------------------------------------------------------------
void KCalcInputDisplay::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);
    slotClearOverwrite();
    clearHasResult();
}

#include "moc_kcalc_input_display.cpp"
