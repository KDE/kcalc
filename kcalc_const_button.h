/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 1996-2000 Bernd Johannes Wuebben <wuebben@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "kcalc_button.h"

struct ScienceConstant;

class KCalcConstButton : public KCalcButton
{
    Q_OBJECT

public:
    explicit KCalcConstButton(QWidget *parent);

    KCalcConstButton(const QString &label, QWidget *parent, const QString &tooltip = QString());

    QString constant() const;

    void setButtonNumber(int num);

    void setLabelAndTooltip();

Q_SIGNALS:
    void constButtonClicked(int num);

private Q_SLOTS:
    void slotConfigureButton();
    void slotChooseScientificConst(const ScienceConstant &constChosen);
    void slotClicked();

private:
    void initPopupMenu();

private:
    int m_buttonNum = -1;
};
