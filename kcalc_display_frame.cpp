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
    QWidget::paintEvent(e);

    QPainter painter(this);

    QStyleOptionFrame option;
    initStyleOption(&option);

    style()->drawPrimitive(QStyle::PE_PanelLineEdit, &option, &painter, this);
}

//------------------------------------------------------------------------------
// Name: resizeEvent
// Desc: resize display and adjust font size
//------------------------------------------------------------------------------
void KCalcDisplayFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);

    // Update font size
    // updateFont();
}

#include "moc_kcalc_display_frame.cpp"
