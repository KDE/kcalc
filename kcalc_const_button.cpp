/*
    kCalculator, a simple scientific calculator for KDE

    Copyright (C) 2003 Klaus Niederkrueger <kniederk@math.uni-koeln.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "kcalc_const_button.h"
#include "kcalc_const_menu.h"
#include "kcalc_settings.h"

#include <kinputdialog.h>
#include <kmenu.h>

KCalcConstButton::KCalcConstButton(QWidget *parent)
        : KCalcButton(parent), button_num_(-1)
{
    addMode(ModeShift, i18nc("Write display data into memory", "Store"), i18n("Write display data into memory"));

    initPopupMenu();

    connect(this, SIGNAL(clicked()), SLOT(slotClicked()));
}


KCalcConstButton::KCalcConstButton(const QString &label, QWidget *parent,
                                   const QString &tooltip)
        : KCalcButton(label, parent, tooltip), button_num_(-1)
{
    addMode(ModeShift, i18nc("Write display data into memory", "Store"), i18n("Write display data into memory"));

    initPopupMenu();
}

QString KCalcConstButton::constant() const
{
    return KCalcSettings::valueConstant(button_num_);
}

void KCalcConstButton::setButtonNumber(int num)
{
    button_num_ = num;
}

void KCalcConstButton::setLabelAndTooltip()
{
    QString new_label = QString("C") + QString().setNum(button_num_ + 1);
    QString new_tooltip;

    new_label = (KCalcSettings::nameConstant(button_num_).isNull() ? new_label : KCalcSettings::nameConstant(button_num_));

    new_tooltip = new_label + '=' + KCalcSettings::valueConstant(button_num_);

    addMode(ModeNormal, new_label, new_tooltip);
}

void KCalcConstButton::initPopupMenu()
{
    KCalcConstMenu *tmp_menu = new KCalcConstMenu(this);

    QAction *a = new QAction(this);
    a->setText(i18n("Set Name"));
    connect(a, SIGNAL(triggered()), this, SLOT(slotConfigureButton()));
    addAction(a);
    tmp_menu->menuAction()->setText(i18n("Choose From List"));
    addAction(tmp_menu->menuAction());
    setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(tmp_menu,
            SIGNAL(triggeredConstant(const science_constant &)),
            SLOT(slotChooseScientificConst(const science_constant &)));

}

void KCalcConstButton::slotConfigureButton()
{
    bool yes_no;
    QString input = KInputDialog::getText(i18n("New Name for Constant"), i18n("New name:"),
                                          text(), &yes_no, this);  // "nameUserConstants-Dialog"
    if (yes_no) {
        KCalcSettings::setNameConstant(button_num_, input);
        setLabelAndTooltip();
    }
}

void KCalcConstButton::slotChooseScientificConst(const struct science_constant &const_chosen)
{
    KCalcSettings::setValueConstant(button_num_, const_chosen.value);

    KCalcSettings::setNameConstant(button_num_, const_chosen.label);

    setLabelAndTooltip();
}

void KCalcConstButton::slotClicked()
{
    emit clicked(button_num_);
}

#include "kcalc_const_button.moc"

