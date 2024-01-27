/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_display_frame.h"
#include "kcalc_settings.h"

#include <QFrame>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

//------------------------------------------------------------------------------
// Name: KCalcDisplayFrame
// Desc: constructor
//------------------------------------------------------------------------------
KCalcDisplayFrame::KCalcDisplayFrame(QWidget *parent)
    : QFrame(parent)
{
}

//------------------------------------------------------------------------------
// Name: ~KCalcDisplayFrame
// Desc: destructor
//------------------------------------------------------------------------------
KCalcDisplayFrame::~KCalcDisplayFrame() = default;

//------------------------------------------------------------------------------
// Name: paintEvent
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplayFrame::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(palette().color(QPalette::Base));
    painter.drawRect(rect());
}

#include "moc_kcalc_display_frame.cpp"
