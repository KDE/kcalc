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
    , base_indicator_(addIndicator(QList<QString>() << QStringLiteral("DEC") << QStringLiteral("BIN") << QStringLiteral("OCT") << QStringLiteral("HEX")))
    , angle_mode_indicator_(addIndicator(QList<QString>() << QStringLiteral("DEG") << QStringLiteral("RAD") << QStringLiteral("GRA")))
    , memory_indicator_(addIndicator(QList<QString>() << QString() << i18nc("Memory indicator in status bar", "M")))
{
    setSizeGripEnabled(false);
}

QLabel *KCalcStatusBar::addIndicator(const QList<QString> &indicatorTexts)
{
    auto l = new QLabel(indicatorTexts.at(0), this);

    // find widest indicator text
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
        text = QStringLiteral("BIN");
        break;
    case 8:
        text = QStringLiteral("OCT");
        break;
    case 10:
    default:
        text = QStringLiteral("DEC");
        break;
    case 16:
        text = QStringLiteral("HEX");
        break;
    }
    base_indicator_->setText(text);
}

void KCalcStatusBar::setAngleMode(AngleMode mode)
{
    QString text;

    switch (mode) {
    case DegMode:
        text = QStringLiteral("DEG");
        break;
    case RadMode:
        text = QStringLiteral("RAD");
        break;
    case GradMode:
        text = QStringLiteral("GRA");
        break;
    }
    angle_mode_indicator_->setText(text);
}

void KCalcStatusBar::setMemoryIndicator(bool memory)
{
    memory_indicator_->setText(memory ? i18nc("Memory indicator in status bar", "M") : QString());
}
