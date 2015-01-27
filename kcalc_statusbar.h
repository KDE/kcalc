/*
 *  Copyright (c) 2014 Christoph Feck <christoph@maxiom.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KCALC_STATUSBAR_H_
#define KCALC_STATUSBAR_H_

#include <QStatusBar>

class QLabel;

class KCalcStatusBar : public QStatusBar
{
    Q_OBJECT

public:
    KCalcStatusBar(QWidget *parent = 0);
    ~KCalcStatusBar();

public:
    enum AngleMode {
        DegMode,
        RadMode,
        GradMode
    };

public:
    void setBaseIndicatorVisible(bool visible);
    void setAngleModeIndicatorVisible(bool visible);

    void setShiftIndicator(bool shift);
    void setBase(int base);
    void setAngleMode(AngleMode mode);
    void setMemoryIndicator(bool memory);

private:
    QLabel *addIndicator(QList<QString> indicatorTexts);

private:
    QLabel *shift_indicator_;
    QLabel *base_indicator_;
    QLabel *angle_mode_indicator_;
    QLabel *memory_indicator_;
};

#endif
