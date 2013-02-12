/*
Copyright (C) 2001 - 2013 Evan Teran
                          evan.teran@gmail.com
						  
Copyright (C) 2003 - 2005 Klaus Niederkrueger
                          kniederk@math.uni-koeln.de

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kcalc_const_button.h"
#include "kcalc_const_menu.h"
#include "kcalc_settings.h"

#include <kinputdialog.h>
#include <kmenu.h>

#include "kcalc_const_button.moc"

//------------------------------------------------------------------------------
// Name: KCalcConstButton
// Desc: constructor
//------------------------------------------------------------------------------
KCalcConstButton::KCalcConstButton(QWidget *parent) : KCalcButton(parent), button_num_(-1) {

	addMode(ModeShift, i18nc("Write display data into memory", "Store"), i18n("Write display data into memory"));
	initPopupMenu();
	connect(this, SIGNAL(clicked()), SLOT(slotClicked()));
}

//------------------------------------------------------------------------------
// Name: KCalcConstButton
// Desc: constructor
//------------------------------------------------------------------------------
KCalcConstButton::KCalcConstButton(const QString &label, QWidget *parent, const QString &tooltip) : KCalcButton(label, parent, tooltip), button_num_(-1) {

	addMode(ModeShift, i18nc("Write display data into memory", "Store"), i18n("Write display data into memory"));
	initPopupMenu();
}

//------------------------------------------------------------------------------
// Name: constant
// Desc: get the value of the const as a QString
//------------------------------------------------------------------------------
QString KCalcConstButton::constant() const {

	return KCalcSettings::valueConstant(button_num_);
}

//------------------------------------------------------------------------------
// Name: setButtonNumber
// Desc: remembers the "index" of the const button
//------------------------------------------------------------------------------
void KCalcConstButton::setButtonNumber(int num) {

	button_num_ = num;
}

//------------------------------------------------------------------------------
// Name: setLabelAndTooltip
// Desc: sets both the label and the tooltip for the const button
//------------------------------------------------------------------------------
void KCalcConstButton::setLabelAndTooltip() {

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
void KCalcConstButton::initPopupMenu() {

    QAction *const a = new QAction(this);
    a->setText(i18n("Set Name"));
    connect(a, SIGNAL(triggered()), this, SLOT(slotConfigureButton()));
    addAction(a);
	
	KCalcConstMenu *const tmp_menu = new KCalcConstMenu(this);
    tmp_menu->menuAction()->setText(i18n("Choose From List"));
    addAction(tmp_menu->menuAction());
    setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(tmp_menu, SIGNAL(triggeredConstant(science_constant)), SLOT(slotChooseScientificConst(science_constant)));

}

//------------------------------------------------------------------------------
// Name: slotConfigureButton
// Desc: lets the user set the name for a constant
//------------------------------------------------------------------------------
void KCalcConstButton::slotConfigureButton() {

	bool yes_no;
	const QString input = KInputDialog::getText(i18n("New Name for Constant"), i18n("New name:"), text(), &yes_no, this);  // "nameUserConstants-Dialog"
	if (yes_no) {
		KCalcSettings::setNameConstant(button_num_, input);
		setLabelAndTooltip();
	}
}

//------------------------------------------------------------------------------
// Name: slotChooseScientificConst
// Desc: set the buttons's scientific constant
//------------------------------------------------------------------------------
void KCalcConstButton::slotChooseScientificConst(const science_constant &const_chosen) {

    KCalcSettings::setValueConstant(button_num_, const_chosen.value);
    KCalcSettings::setNameConstant(button_num_, const_chosen.label);
    setLabelAndTooltip();
}

//------------------------------------------------------------------------------
// Name: slotClicked
// Desc: constant button was clicked
//------------------------------------------------------------------------------
void KCalcConstButton::slotClicked() {

    emit clicked(button_num_);
}

