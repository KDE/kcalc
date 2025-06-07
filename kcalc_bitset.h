/*
    SPDX-FileCopyrightText: 2012-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2006 Michel Marti <mma@objectxp.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QWidget>

class QButtonGroup;
class QAbstractButton;
class KCalcBitset : public QWidget
{
    Q_OBJECT

public:
    explicit KCalcBitset(QWidget *parent = nullptr);
    Q_REQUIRED_RESULT qint64 getValue() const;

    void calculateMaxSize();

protected:
    void resizeEvent(QResizeEvent *event) override;

public Q_SLOTS:
    void setValue(qint64 value);
    void slotToggleBit(QAbstractButton *button);
    void setReadOnly(bool readOnly);
    void clear();

Q_SIGNALS:
    void valueChanged(qint64 value);

private:
    QButtonGroup *const bit_button_group_;
    qint64 value_;
    bool m_readOnly;
};
