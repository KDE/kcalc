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

#include "kcalc_statusbar.h"

#include <QLabel>

#include <KLocalizedString>



KCalcStatusBar::KCalcStatusBar(QWidget *parent)
    : QStatusBar(parent)
{
    setSizeGripEnabled(false);

    shift_indicator_ = addIndicator(QList<QString>() << i18nc("Normal button functions are active", "NORM") << i18nc("Second button functions are active", "SHIFT"));
    base_indicator_ = addIndicator(QList<QString>() << "DEC" << "BIN" << "OCT" << "HEX");
    angle_mode_indicator_ = addIndicator(QList<QString>() << "DEG" << "RAD" << "GRA");
    memory_indicator_ = addIndicator(QList<QString>() << QString() << i18nc("Memory indicator in status bar", "M"));
}

KCalcStatusBar::~KCalcStatusBar()
{
}

QLabel *KCalcStatusBar::addIndicator(QList<QString> indicatorTexts)
{
    QLabel *l = new QLabel(indicatorTexts.at(0), this);

    // find widest indicator text
    QFontMetrics fm(l->font());
    int maxWidth = 0;
    foreach (const QString &text, indicatorTexts) {
        maxWidth = qMax(maxWidth, fm.boundingRect(text).width());
    }
    // add some padding
    maxWidth += fm.height();

    l->setFixedSize(maxWidth, fm.height());
    l->setAlignment(Qt::AlignCenter);

    addPermanentWidget(l);
    return l;
}

void KCalcStatusBar::setBaseIndicatorVisible(bool visible)
{
    base_indicator_->setVisible(visible);
}

void KCalcStatusBar::setAngleModeIndicatorVisible(bool visible)
{
    angle_mode_indicator_->setVisible(visible);
}

void KCalcStatusBar::setShiftIndicator(bool shift)
{
    shift_indicator_->setText(shift ? i18nc("Second button functions are active", "SHIFT") : i18nc("Normal button functions are active", "NORM"));
}

void KCalcStatusBar::setBase(int base)
{
    QString text;

    switch (base) {
    case 2:
        text = "BIN";
        break;
    case 8:
        text = "OCT";
        break;
    case 10:
    default:
        text = "DEC";
        break;
    case 16:
        text = "HEX";
        break;
    }
    base_indicator_->setText(text);
}

void KCalcStatusBar::setAngleMode(AngleMode mode)
{
    QString text;

    switch (mode) {
    case DegMode:
        text = "DEG";
        break;
    case RadMode:
        text = "RAD";
        break;
    case GradMode:
        text = "GRA";
        break;
    }
    angle_mode_indicator_->setText(text);
}

void KCalcStatusBar::setMemoryIndicator(bool memory)
{
    memory_indicator_->setText(memory ? i18nc("Memory indicator in status bar", "M") : QString());
}

