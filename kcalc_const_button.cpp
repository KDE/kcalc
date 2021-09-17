/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2003-2005 Klaus Niederkrueger <kniederk@math.uni-koeln.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_const_button.h"
#include "kcalc_const_menu.h"
#include "kcalc_settings.h"

#include <QInputDialog>

#include <KLocalizedString>

//------------------------------------------------------------------------------
// Name: KCalcConstButton
// Desc: constructor
//------------------------------------------------------------------------------
KCalcConstButton::KCalcConstButton(QWidget *parent)
    : KCalcButton(parent)
{
    setTextColor(KCalcSettings::constantsFontsColor());
    addMode(ModeShift, i18nc("Write display data into memory", "Store"), i18n("Write display data into memory"));
    initPopupMenu();
    connect(this, &QAbstractButton::clicked, this, &KCalcConstButton::slotClicked);
}

//------------------------------------------------------------------------------
// Name: KCalcConstButton
// Desc: constructor
//------------------------------------------------------------------------------
KCalcConstButton::KCalcConstButton(const QString &label, QWidget *parent, const QString &tooltip)
    : KCalcButton(label, parent, tooltip)
{
    setTextColor(KCalcSettings::constantsFontsColor());
    addMode(ModeShift, i18nc("Write display data into memory", "Store"), i18n("Write display data into memory"));
    initPopupMenu();
}

//------------------------------------------------------------------------------
// Name: constant
// Desc: get the value of the const as a QString
//------------------------------------------------------------------------------
QString KCalcConstButton::constant() const
{
    return KCalcSettings::valueConstant(button_num_);
}

//------------------------------------------------------------------------------
// Name: setButtonNumber
// Desc: remembers the "index" of the const button
//------------------------------------------------------------------------------
void KCalcConstButton::setButtonNumber(int num)
{
    button_num_ = num;
}

//------------------------------------------------------------------------------
// Name: setLabelAndTooltip
// Desc: sets both the label and the tooltip for the const button
//------------------------------------------------------------------------------
void KCalcConstButton::setLabelAndTooltip()
{
    QString new_label = QLatin1String("C") + QString::number(button_num_ + 1);
    QString new_tooltip;

    new_label = (KCalcSettings::nameConstant(button_num_).isNull() ? new_label : KCalcSettings::nameConstant(button_num_));

    new_tooltip = new_label + QLatin1Char('=') + KCalcSettings::valueConstant(button_num_);

    addMode(ModeNormal, new_label, new_tooltip);
}

//------------------------------------------------------------------------------
// Name: initPopupMenu
// Desc: initializes the const button popup
//------------------------------------------------------------------------------
void KCalcConstButton::initPopupMenu()
{
    auto const a = new QAction(this);
    a->setText(i18n("Set Name"));
    connect(a, &QAction::triggered, this, &KCalcConstButton::slotConfigureButton);
    addAction(a);

    auto const tmp_menu = new KCalcConstMenu(this);
    tmp_menu->menuAction()->setText(i18n("Choose From List"));
    addAction(tmp_menu->menuAction());
    setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(tmp_menu, &KCalcConstMenu::triggeredConstant, this, &KCalcConstButton::slotChooseScientificConst);
}

//------------------------------------------------------------------------------
// Name: slotConfigureButton
// Desc: lets the user set the name for a constant
//------------------------------------------------------------------------------
void KCalcConstButton::slotConfigureButton()
{
    bool yes_no;
    const QString input =
        QInputDialog::getText(this, i18n("New Name for Constant"), i18n("New name:"), QLineEdit::Normal, text(), &yes_no); // "nameUserConstants-Dialog"
    if (yes_no && !input.isEmpty()) {
        KCalcSettings::setNameConstant(button_num_, input);
        setLabelAndTooltip();
    }
}

//------------------------------------------------------------------------------
// Name: slotChooseScientificConst
// Desc: set the buttons's scientific constant
//------------------------------------------------------------------------------
void KCalcConstButton::slotChooseScientificConst(const science_constant &const_chosen)
{
    KCalcSettings::setValueConstant(button_num_, const_chosen.value);
    KCalcSettings::setNameConstant(button_num_, const_chosen.label);
    setLabelAndTooltip();
}

//------------------------------------------------------------------------------
// Name: slotClicked
// Desc: constant button was clicked
//------------------------------------------------------------------------------
void KCalcConstButton::slotClicked()
{
    Q_EMIT constButtonClicked(button_num_);
}
