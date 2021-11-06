/*
    SPDX-FileCopyrightText: 2012-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2006 Michel Marti <mma@objectxp.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bitbutton.h"
#include <QApplication>
#include <QPainter>

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

    setFixedSize(size);

    this->setAttribute(Qt::WA_Hover, true);
}

//------------------------------------------------------------------------------
// Name: paintEvent
// Desc: draws the button
//------------------------------------------------------------------------------
void BitButton::paintEvent(QPaintEvent *)
{
    uint8_t alpha = 0x60;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(palette().text(), 1);
    pen.setJoinStyle(Qt::MiterJoin);
    painter.setPen(pen);

    if (on_) {
        painter.setBrush(palette().text());
        alpha = 0xB0;
    } else {
        painter.setBrush(palette().base());
    }

    if (over_) {
        painter.setBrush(QColor(palette().text().color().red(), palette().text().color().green(), palette().text().color().blue(), alpha));
    }

    painter.drawRect(rect().adjusted(1, 1, -1, -1));
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
