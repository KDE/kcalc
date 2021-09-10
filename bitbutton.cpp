/*
    SPDX-FileCopyrightText: 2012-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2006 Michel Marti <mma@objectxp.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bitbutton.h"
#include <QApplication>

//------------------------------------------------------------------------------
// Name: BitButton
// Desc: constructor
//------------------------------------------------------------------------------
BitButton::BitButton(QWidget *parent)
    : QAbstractButton(parent)
{
    // too many bits for tab focus
    setFocusPolicy(Qt::ClickFocus);

    // size button by font
    QSize size = fontMetrics().size(0, QStringLiteral("M"));

    if (size.width() < size.height()) {
        size.setHeight(size.width());
    } else {
        size.setWidth(size.height());
    }

    setMinimumSize(size);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    this->setAttribute(Qt::WA_Hover, true);
}

//------------------------------------------------------------------------------
// Name: isOn
// Desc: returns true if this bit-button is "on"
//------------------------------------------------------------------------------
bool BitButton::isOn() const
{
    return on_;
}

//------------------------------------------------------------------------------
// Name: setOn
// Desc: changes the "on" value for the bitset
//------------------------------------------------------------------------------
void BitButton::setOn(bool value)
{
    on_ = value;
    update();
}

//------------------------------------------------------------------------------
// Name: enterEvent
// Desc: sets to true the "over" variable on Enter event
//------------------------------------------------------------------------------
void BitButton::enterEvent(QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        over_ = true;
        update();
    }
}

//------------------------------------------------------------------------------
// Name: leaveEvent
// Desc: sets to false the "over" variable on Leave event
//------------------------------------------------------------------------------
void BitButton::leaveEvent(QEvent *event)
{
    if (event->type() == QEvent::Leave) {
        over_ = false;
        update();
    }
}
