/*
    SPDX-FileCopyrightText: 2012-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2006 Michel Marti <mma@objectxp.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_bitbutton.h"
#include <QEvent>
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
    setCheckable(true);

    // size button by font
    QSize size = fontMetrics().size(0, QStringLiteral("M"));

    if (size.width() < size.height()) {
        size.setHeight(size.width());
    } else {
        size.setWidth(size.height());
    }

    setMinimumSize(size);
    setRenderSize(size);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    this->setAttribute(Qt::WA_Hover, true);
}

//------------------------------------------------------------------------------
// Name: paintEvent
// Desc: draws the button
//------------------------------------------------------------------------------
void BitButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(palette().highlight(), 2);
    pen.setJoinStyle(Qt::MiterJoin);
    painter.setPen(m_over ? pen : Qt::NoPen);

    if (isDown()) {
        painter.setBrush(palette().highlight());
    } else {
        painter.setBrush(m_on ? palette().text() : palette().base());
    }

    // Prepare button size (should be square)
    QRect square = rect();
    square.setSize(renderSize());

    painter.translate(QPoint(0, (rect().height() - square.height()) / 2)); // center button
    painter.drawRoundedRect(square, 3.0, 3.0);
    pen.setColor(palette().color(isDown() ? QPalette::HighlightedText : (m_on ? QPalette::Window : QPalette::Text)));
    painter.setPen(pen);
    painter.drawText(square, Qt::AlignCenter, QString::number(m_on ? 1 : 0));
}

//------------------------------------------------------------------------------
// Name: isOn
// Desc: returns true if this bit-button is "on"
//------------------------------------------------------------------------------
bool BitButton::isOn() const
{
    return m_on;
}

//------------------------------------------------------------------------------
// Name: setOn
// Desc: changes the "on" value for the bitset
//------------------------------------------------------------------------------
void BitButton::setOn(bool value)
{
    m_on = value;
    update();
}

//------------------------------------------------------------------------------
// Name: renderSize
// Desc: returns current render size
//------------------------------------------------------------------------------
void BitButton::setRenderSize(const QSize &size)
{
    if (size == m_renderSize) {
        return;
    }

    m_renderSize = size;
    // resize font for painting 0/1
    auto actualFont = font();
    actualFont.setPixelSize(qFloor(m_renderSize.height() * 0.9));
    setFont(actualFont);
}

//------------------------------------------------------------------------------
// Name: renderSize
// Desc: returns current render size
//------------------------------------------------------------------------------
QSize BitButton::renderSize() const
{
    return m_renderSize;
}

//------------------------------------------------------------------------------
// Name: enterEvent
// Desc: sets to true the "over" variable on Enter event
//------------------------------------------------------------------------------
void BitButton::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event)
    m_over = true;
    update();
}

//------------------------------------------------------------------------------
// Name: leaveEvent
// Desc: sets to false the "over" variable on Leave event
//------------------------------------------------------------------------------
void BitButton::leaveEvent(QEvent *event)
{
    if (event->type() == QEvent::Leave) {
        m_over = false;
        update();
    }
}

//------------------------------------------------------------------------------
// Name: focusInEvent
// Desc: sets to true the "over" variable on Enter event
//------------------------------------------------------------------------------
void BitButton::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    m_over = true;
    update();
}

//------------------------------------------------------------------------------
// Name: focusOutEvent
// Desc: sets to false the "over" variable on Leave event
//------------------------------------------------------------------------------
void BitButton::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    m_over = false;
    update();
}

#include "moc_kcalc_bitbutton.cpp"
