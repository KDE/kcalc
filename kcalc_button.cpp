/*
SPDX-FileCopyrightText: 2001-2013 Evan Teran
    evan.teran@gmail.com

SPDX-FileCopyrightText: 1996-2000 Bernd Johannes Wuebben
    wuebben@kde.org

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_button.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QPainter>
#include <QStyleOptionButton>
#include <QTextDocument>

//------------------------------------------------------------------------------
// Name: KCalcButton
// Desc: constructor
//------------------------------------------------------------------------------
KCalcButton::KCalcButton(QWidget *parent)
    : QPushButton(parent)
    , mode_flags_(ModeNormal)
    , size_()
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
    , mode_flags_(ModeNormal)
    , size_()
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
    if (mode_.contains(mode)) {
        mode_.remove(mode);
    }

    mode_[mode] = ButtonMode(label, tooltip);
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
    ButtonModeFlags new_mode;

    if (flag) { // if the specified mode is to be set (i.e. flag = true)
        new_mode = ButtonModeFlags(mode_flags_ | mode);
    } else if (mode_flags_ && mode) { // if the specified mode is to be cleared (i.e. flag = false)
        new_mode = ButtonModeFlags(mode_flags_ - mode);
    } else {
        return; // nothing to do
    }

    if (mode_.contains(new_mode)) {
        // save shortcut, because setting label erases it
        QKeySequence current_shortcut = shortcut();

        setText(mode_[new_mode].label);
        this->setToolTip(mode_[new_mode].tooltip);
        mode_flags_ = new_mode;

        // restore shortcut
        setShortcut(current_shortcut);
    }

    // this is necessary for people pressing CTRL and changing mode at
    // the same time...
    if (show_shortcut_mode_) {
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
    show_shortcut_mode_ = flag;

    // save shortcut, because setting label erases it
    QKeySequence current_shortcut = shortcut();

    if (flag) {
        setText(shortcut().toString(QKeySequence::NativeText));
    } else {
        setText(mode_[mode_flags_].label);
    }

    // restore shortcut
    setShortcut(current_shortcut);
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
    const bool is_down = isDown() || isChecked();
    const int x_offset = is_down ? style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal, &option, this) : 0;
    const int y_offset = is_down ? style()->pixelMetric(QStyle::PM_ButtonShiftVertical, &option, this) : 0;

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
    QColor color = text_color_;
    if (!isEnabled()) {
        color.setAlphaF(0.6);
    }
    context.palette.setColor(QPalette::Text, color);

    p.translate((width() / 2 - doc.size().width() / 2) + x_offset, (height() / 2 - doc.size().height() / 2) + y_offset);
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
    return size_;
}

//------------------------------------------------------------------------------
// Name: calcSizeHint
// Desc:
//------------------------------------------------------------------------------
void KCalcButton::calcSizeHint()
{
    int margin = style()->pixelMetric(QStyle::PM_ButtonMargin, nullptr, this);

    // want narrow margin than normal
    margin = qMax(margin / 2, 3);

    // approximation because metrics doesn't account for richtext
    size_ = fontMetrics().size(0, mode_[ModeNormal].label);
    if (mode_.contains(ModeShift)) {
        size_ = size_.expandedTo(fontMetrics().size(0, mode_[ModeShift].label));
    }

    if (mode_.contains(ModeHyperbolic)) {
        size_ = size_.expandedTo(fontMetrics().size(0, mode_[ModeHyperbolic].label));
    }

    size_ += QSize(margin * 2, margin * 2);
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
    text_color_ = color;
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
    if (mode_[ModeNormal].label.isEmpty()) {
        mode_[ModeNormal].label = text;
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
    if (mode_[ModeNormal].tooltip.isEmpty()) {
        mode_[ModeNormal].tooltip = tip;
    }
}
