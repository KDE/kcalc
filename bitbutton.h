/*
SPDX-FileCopyrightText: 2012-2013 Evan Teran <evan.teran@gmail.com>

SPDX-FileCopyrightText: 2006 Michel Marti <mma@objectxp.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractButton>

class BitButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit BitButton(QWidget *parent = nullptr);
    Q_REQUIRED_RESULT bool isOn() const;
    void setOn(bool value);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    bool on_ = false;
    bool over_ = false;
};

