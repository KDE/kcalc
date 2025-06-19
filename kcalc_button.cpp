/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 1996-2000 Bernd Johannes Wuebben <wuebben@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_button.h"

#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QStyleOptionButton>
#include <QTextDocument>

//------------------------------------------------------------------------------
// Name: KCalcButton
// Desc: constructor
//------------------------------------------------------------------------------
KCalcButton::KCalcButton(QWidget *parent)
    : QPushButton(parent)
    , m_modeFlags(ModeNormal)

{
    setAcceptDrops(true); // allow color drops
    setFocusPolicy(Qt::TabFocus);
    setAutoDefault(false);

    // use preferred size policy for vertical
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    setAttribute(Qt::WA_LayoutUsesWidgetRect);
}

//------------------------------------------------------------------------------
// Name: KCalcButton
// Desc: constructor
//------------------------------------------------------------------------------
KCalcButton::KCalcButton(const QString &label, QWidget *parent, const QString &tooltip)
    : QPushButton(label, parent)
    , m_modeFlags(ModeNormal)

{
    setAutoDefault(false);
    addMode(ModeNormal, label, tooltip);

    // use preferred size policy for vertical
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    setAttribute(Qt::WA_LayoutUsesWidgetRect);
}

//------------------------------------------------------------------------------
// Name: addMode
// Desc:
//------------------------------------------------------------------------------
void KCalcButton::addMode(ButtonModeFlags mode, const QString &label, const QString &tooltip)
{
    if (this->m_mode.contains(mode)) {
        this->m_mode.remove(mode);
    }

    this->m_mode[mode] = ButtonMode(label, tooltip);
    calcSizeHint();

    // Need to put each button into default mode first
    if (mode == ModeNormal) {
        slotSetMode(ModeNormal, true);
    }
}

//------------------------------------------------------------------------------
// Name: slotSetMode
// Desc:
//------------------------------------------------------------------------------
void KCalcButton::slotSetMode(ButtonModeFlags mode, bool flag)
{
    ButtonModeFlags newMode;

    if (flag) { // if the specified mode is to be set (i.e. flag = true)
        newMode = ButtonModeFlags(m_modeFlags | mode);
    } else if (m_modeFlags & mode) { // if the specified mode is to be cleared (i.e. flag = false)
        newMode = ButtonModeFlags(m_modeFlags - mode);
    } else {
        return; // nothing to do
    }

    if (this->m_mode.contains(newMode)) {
        // save shortcut, because setting label erases it
        QKeySequence currentShortcut = shortcut();

        setText(this->m_mode[newMode].label);
        this->setToolTip(this->m_mode[newMode].tooltip);
        m_modeFlags = newMode;

        // restore shortcut
        setShortcut(currentShortcut);
    }

    // this is necessary for people pressing CTRL and changing mode at
    // the same time...
    if (m_showShortcutMode) {
        slotSetAccelDisplayMode(true);
    }

    update();
}

//------------------------------------------------------------------------------
// Name: slotSetAccelDisplayMode
// Desc:
//------------------------------------------------------------------------------
void KCalcButton::slotSetAccelDisplayMode(bool flag)
{
    m_showShortcutMode = flag;

    // save shortcut, because setting label erases it
    QKeySequence currentShortcut = shortcut();

    if (flag) {
        setText(shortcut().toString(QKeySequence::NativeText));
    } else {
        setText(m_mode[m_modeFlags].label);
    }

    // restore shortcut
    setShortcut(currentShortcut);
    update();
}

//------------------------------------------------------------------------------
// Name: paintEvent
// Desc: draws the button
//------------------------------------------------------------------------------
void KCalcButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOptionButton option;
    initStyleOption(&option);
    const bool isDownFlag = isDown() || isChecked();
    const int xOffset = isDownFlag ? style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal, &option, this) : 0;
    const int yOffset = isDownFlag ? style()->pixelMetric(QStyle::PM_ButtonShiftVertical, &option, this) : 0;

    // draw bevel
    style()->drawControl(QStyle::CE_PushButtonBevel, &option, &p, this);

    // draw label...
    p.save();

    // rant: Qt4 needs QSimpleRichText, dammit!
    QTextDocument doc;
    QAbstractTextDocumentLayout::PaintContext context;
    doc.setHtml(QLatin1String("<center>") + text() + QLatin1String("</center>"));
    doc.setDefaultFont(font());
    context.palette = palette();
    QColor color = m_textColor;
    if (!isEnabled()) {
        color.setAlphaF(0.6);
    }
    context.palette.setColor(QPalette::Text, color);

    p.translate((width() / 2 - doc.size().width() / 2) + xOffset, (height() / 2 - doc.size().height() / 2) + yOffset);
    doc.documentLayout()->draw(&p, context);
    p.restore();

    // draw focus
    if (hasFocus()) {
        QStyleOptionFocusRect fropt;
        fropt.QStyleOption::operator=(option);
        fropt.rect = style()->subElementRect(QStyle::SE_PushButtonFocusRect, &option, this);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &fropt, &p, this);
    }
}

//------------------------------------------------------------------------------
// Name: sizeHint
// Desc:
//------------------------------------------------------------------------------
QSize KCalcButton::sizeHint() const
{
    // reimplemented to provide a smaller button
    return m_size;
}

//------------------------------------------------------------------------------
// Name: calcSizeHint
// Desc:
//------------------------------------------------------------------------------
void KCalcButton::calcSizeHint()
{
    int margin = style()->pixelMetric(QStyle::PM_ButtonMargin, nullptr, this);

    // want narrower margin than normal
    margin = qMax(margin / 2, 3);

    // simply use font size of a single letter
    m_size = fontMetrics().size(0, QStringLiteral("M"));

    m_size += QSize(margin * 2, margin * 2);
}

//------------------------------------------------------------------------------
// Name: setFont
// Desc:
//------------------------------------------------------------------------------
void KCalcButton::setFont(const QFont &fnt)
{
    QPushButton::setFont(fnt);
    calcSizeHint();
}

//------------------------------------------------------------------------------
// Name: setTextColor
// Desc:
//------------------------------------------------------------------------------
void KCalcButton::setTextColor(const QColor &color)
{
    m_textColor = color;
    update();
}

//------------------------------------------------------------------------------
// Name: setText
// Desc:
//------------------------------------------------------------------------------
void KCalcButton::setText(const QString &text)
{
    QPushButton::setText(text);

    // normal mode may not have been explicitly set
    if (m_mode[ModeNormal].label.isEmpty()) {
        m_mode[ModeNormal].label = text;
    }

    calcSizeHint();
}

//------------------------------------------------------------------------------
// Name: setToolTip
// Desc:
//------------------------------------------------------------------------------
void KCalcButton::setToolTip(const QString &tip)
{
    QPushButton::setToolTip(tip);

    // normal mode may not have been explicitly set
    if (m_mode[ModeNormal].tooltip.isEmpty()) {
        m_mode[ModeNormal].tooltip = tip;
    }
}

#include "moc_kcalc_button.cpp"
