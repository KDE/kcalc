/*
Copyright (C) 2001 - 2013 Evan Teran
                          evan.teran@gmail.com

Copyright (C) 1996 - 2000 Bernd Johannes Wuebben
                          wuebben@kde.org

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KCALC_CONST_BUTTON_H_
#define KCALC_CONST_BUTTON_H_

#include <klocale.h>
#include "kcalc_button.h"

struct science_constant;


class KCalcConstButton : public KCalcButton
{
    Q_OBJECT

public:

    explicit KCalcConstButton(QWidget *parent);

    KCalcConstButton(const QString &label, QWidget *parent,
                     const QString &tooltip = QString());

    QString constant() const;

    void setButtonNumber(int num);

    void setLabelAndTooltip();

signals:
    void clicked(int num);

private slots:
    void slotConfigureButton();
    void slotChooseScientificConst(const science_constant &const_chosen);
    void slotClicked();

private:
    void initPopupMenu();

    int button_num_;
};


#endif
