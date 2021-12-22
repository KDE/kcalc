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
    void setRenderSize(const QSize &size);
    const QSize &renderSize() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    bool on_ = false;
    bool over_ = false;
    QSize* renderSize_ = nullptr;
};

