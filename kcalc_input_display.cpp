/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <kcalc_input_display.h>

#include <QLineEdit>
// #include <kcalc_parser.h>

//------------------------------------------------------------------------------
// Name: KCalcInputDisplay
// Desc: constructor
//------------------------------------------------------------------------------
KCalcInputDisplay::KCalcInputDisplay(QWidget *parent)
    : QLineEdit(parent)
{
    overwrite_ = false;
    hard_overwrite_ = false;
}

//------------------------------------------------------------------------------
// Name: ~KCalcInputDisplay
// Desc:
//------------------------------------------------------------------------------
KCalcInputDisplay::~KCalcInputDisplay() = default;

//------------------------------------------------------------------------------
// Name: insertToken
// Desc:
//------------------------------------------------------------------------------
void KCalcInputDisplay::insertToken(const QString &token)
{
    if (overwrite_ || hard_overwrite_) {
        this->clear();
        overwrite_ = false;
        hard_overwrite_ = false;
    }

    this->insert(token);
}

//------------------------------------------------------------------------------
// Name: slotSetOverwrite
// Desc: when set, inserting to the display will clear it, and then insert.
//       (It can be reset refocusing the display)
//------------------------------------------------------------------------------
void KCalcInputDisplay::slotSetOverwrite()
{
    overwrite_ = true;
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
    hard_overwrite_ = true;
    overwrite_ = true;
    this->focusNextChild(); /* workaround to defocus the QWidget,
                               although it works, it can lead to
                               issues if the GUI changes */
}

//------------------------------------------------------------------------------
// Name: slotClearOverwrite
// Desc: resets Overwrite/hardOverwrite flags,
//       deletes finishing '=' when required
//------------------------------------------------------------------------------
void KCalcInputDisplay::slotClearOverwrite()
{
    overwrite_ = false;
    int tmp_cursorPosition = this->cursorPosition();
    if (this->text().endsWith(QLatin1String("="))) {
        this->end(false);
        this->backspace();
    }
    this->setCursorPosition(tmp_cursorPosition);

    if (hard_overwrite_) {
        hard_overwrite_ = false;
        this->clear();
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
}
