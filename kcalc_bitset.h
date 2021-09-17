/*
SPDX-FileCopyrightText: 2012-2013 Evan Teran
    evan.teran@gmail.com

SPDX-FileCopyrightText: 2006 Michel Marti
    mma@objectxp.com

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QFrame>

class QButtonGroup;
class QAbstractButton;
class KCalcBitset : public QFrame
{
    Q_OBJECT

public:
    explicit KCalcBitset(QWidget *parent = nullptr);
    Q_REQUIRED_RESULT quint64 getValue() const;

public Q_SLOTS:
    void setValue(quint64 value);
    void slotToggleBit(QAbstractButton *button);

Q_SIGNALS:
    void valueChanged(quint64 value);

private:
    QButtonGroup *const bit_button_group_;
    quint64 value_;
};

