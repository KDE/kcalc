/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_display_frame.h"

//------------------------------------------------------------------------------
// Name: KCalcDisplayFrame
// Desc: constructor
//------------------------------------------------------------------------------
KCalcDisplayFrame::KCalcDisplayFrame(QWidget *parent)
    : QFrame(parent)
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags(Qt::BottomEdge)));
}

#include "moc_kcalc_display_frame.cpp"
