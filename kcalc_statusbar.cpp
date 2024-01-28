/*
    SPDX-FileCopyrightText: 2014 Christoph Feck <christoph@maxiom.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_statusbar.h"

#include <QLabel>

#include <KLocalizedString>

KCalcStatusBar::KCalcStatusBar(QWidget *parent)
    : QStatusBar(parent)
    , shift_indicator_(
          addIndicator(QList<QString>() << i18nc("Normal button functions are active", "NORM") << i18nc("Second button functions are active", "SHIFT")))
    , base_indicator_(addIndicator(QList<QString>() << i18nc("Decimal indicator in status bar", "DEC") << i18nc("Binary indicator in status bar", "BIN")
                                                    << i18nc("Octal indicator in status bar", "OCT") << i18nc("Hexadecimal indicator in status bar", "HEX")))
    , angle_mode_indicator_(addIndicator(QList<QString>() << i18nc("Degree indicator in status bar", "DEG") << i18nc("Radian indicator in status bar", "RAD")
                                                          << i18nc("Grad indicator in status bar", "GRA")))
    , memory_indicator_(addIndicator(QList<QString>() << QString() << i18nc("Memory indicator in status bar", "M")))
{
    setSizeGripEnabled(false);
}

QLabel *KCalcStatusBar::addIndicator(const QList<QString> &indicatorTexts)
{
    auto l = new QLabel(indicatorTexts.at(0), this);

    // find the widest indicator text
    QFontMetrics fm(l->font());
    int maxWidth = 0;
    for (const QString &text : indicatorTexts) {
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
        text = i18n("BIN");
        break;
    case 8:
        text = i18n("OCT");
        break;
    case 10:
    default:
        text = i18n("DEC");
        break;
    case 16:
        text = i18n("HEX");
        break;
    }
    base_indicator_->setText(text);
}

void KCalcStatusBar::setAngleMode(AngleMode mode)
{
    QString text;

    switch (mode) {
    case DegMode:
        text = i18n("DEG");
        break;
    case RadMode:
        text = i18n("RAD");
        break;
    case GradMode:
        text = i18n("GRA");
        break;
    }
    angle_mode_indicator_->setText(text);
}

void KCalcStatusBar::setMemoryIndicator(bool memory)
{
    memory_indicator_->setText(memory ? i18nc("Memory indicator in status bar", "M") : QString());
}

#include "moc_kcalc_statusbar.cpp"
