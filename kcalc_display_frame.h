/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 1996-2000 Bernd Johannes Wuebben <wuebben@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QFrame>
#include <QStyle>

class KCalcDisplayFrame : public QFrame
{
    Q_OBJECT

public:
    explicit KCalcDisplayFrame(QWidget *parent = nullptr);
    ~KCalcDisplayFrame() override;

protected:
    void paintEvent(QPaintEvent *p) override;
    void resizeEvent(QResizeEvent *e) override;
};
