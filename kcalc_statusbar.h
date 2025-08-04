/*
    SPDX-FileCopyrightText: 2014 Christoph Feck <christoph@maxiom.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QStatusBar>

class QLabel;

class KCalcStatusBar : public QStatusBar
{
    Q_OBJECT

public:
    explicit KCalcStatusBar(QWidget *parent = nullptr);
    ~KCalcStatusBar() override = default;

    enum AngleMode {
        DegMode,
        RadMode,
        GradMode
    };

    void setBaseIndicatorVisible(bool visible);
    void setAngleModeIndicatorVisible(bool visible);

    void setShiftIndicator(bool shift);
    void setBase(int base);
    void setAngleMode(AngleMode mode);
    void setMemoryIndicator(bool memory);

private:
    QLabel *addIndicator(const QList<QString> &indicatorTexts);

    QLabel *const shift_indicator_;
    QLabel *const base_indicator_;
    QLabel *const angle_mode_indicator_;
    QLabel *const memory_indicator_;
};
