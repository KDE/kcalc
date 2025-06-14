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
    KColorScheme schemeFonts(QPalette::Active, KColorScheme::Button);
    setTextColor(KCalcSettings::followSystemTheme() ? schemeFonts.foreground().color() : KCalcSettings::constantsFontsColor());
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
    KColorScheme schemeFonts(QPalette::Active, KColorScheme::Button);
    setTextColor(KCalcSettings::followSystemTheme() ? schemeFonts.foreground().color() : KCalcSettings::constantsFontsColor());
    addMode(ModeShift, i18nc("Write display data into memory", "Store"), i18n("Write display data into memory"));
    initPopupMenu();
}

//------------------------------------------------------------------------------
// Name: constant
// Desc: get the value of the const as a QString
//------------------------------------------------------------------------------
QString KCalcConstButton::constant() const
{
    return KCalcSettings::valueConstant(m_buttonNum);
}

//------------------------------------------------------------------------------
// Name: setButtonNumber
// Desc: remembers the "index" of the const button
//------------------------------------------------------------------------------
void KCalcConstButton::setButtonNumber(int num)
{
    m_buttonNum = num;
}

//------------------------------------------------------------------------------
// Name: setLabelAndTooltip
// Desc: sets both the label and the tooltip for the const button
//------------------------------------------------------------------------------
void KCalcConstButton::setLabelAndTooltip()
{
    QString newLabel = QLatin1String("C") + QString::number(m_buttonNum + 1);
    QString newTooltip;

    newLabel = (KCalcSettings::nameConstant(m_buttonNum).isNull() ? newLabel : KCalcSettings::nameConstant(m_buttonNum));

    newTooltip = newLabel + QLatin1Char('=') + KCalcSettings::valueConstant(m_buttonNum);

    addMode(ModeNormal, newLabel, newTooltip);
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

    auto const tmpMenu = new KCalcConstMenu(this);
    tmpMenu->menuAction()->setText(i18n("Choose From List"));
    addAction(tmpMenu->menuAction());
    setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(tmpMenu, &KCalcConstMenu::triggeredConstant, this, &KCalcConstButton::slotChooseScientificConst);
}

//------------------------------------------------------------------------------
// Name: slotConfigureButton
// Desc: lets the user set the name for a constant
//------------------------------------------------------------------------------
void KCalcConstButton::slotConfigureButton()
{
    bool yesNo = false;
    const QString input =
        QInputDialog::getText(this, i18n("New Name for Constant"), i18n("New name:"), QLineEdit::Normal, text(), &yesNo); // "nameUserConstants-Dialog"
    if (yesNo && !input.isEmpty()) {
        KCalcSettings::setNameConstant(m_buttonNum, input);
        setLabelAndTooltip();
    }
}

//------------------------------------------------------------------------------
// Name: slotChooseScientificConst
// Desc: set the button's scientific constant
//------------------------------------------------------------------------------
void KCalcConstButton::slotChooseScientificConst(const ScienceConstant &constChosen)
{
    KCalcSettings::setValueConstant(m_buttonNum, constChosen.value);
    KCalcSettings::setNameConstant(m_buttonNum, constChosen.label);
    setLabelAndTooltip();
}

//------------------------------------------------------------------------------
// Name: slotClicked
// Desc: constant button was clicked
//------------------------------------------------------------------------------
void KCalcConstButton::slotClicked()
{
    Q_EMIT constButtonClicked(m_buttonNum);
}

#include "moc_kcalc_const_button.cpp"
