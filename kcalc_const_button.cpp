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
  : KCalcButton(parent), _button_num(-1)
{
  addMode(ModeInverse, i18nc("Write display data into memory", "Store"), i18n("Write display data into memory"));

  initPopupMenu();

  connect(this, SIGNAL(clicked()), SLOT(slotClicked()));
}


KCalcConstButton::KCalcConstButton(const QString &label, QWidget *parent,
				   const QString &tooltip)
  : KCalcButton(label, parent, tooltip), _button_num(-1)
{
  addMode(ModeInverse, i18nc("Write display data into memory", "Store"), i18n("Write display data into memory"));

  initPopupMenu();
}

QString KCalcConstButton::constant(void) const
{
  return KCalcSettings::valueConstant(_button_num);
}

void KCalcConstButton::setButtonNumber(int num)
{
	_button_num = num;
}

void KCalcConstButton::setLabelAndTooltip(void)
{
  QString new_label = QString("C") + QString().setNum(_button_num + 1);
  QString new_tooltip;

  new_label = (KCalcSettings::nameConstant(_button_num).isNull() ? new_label : KCalcSettings::nameConstant(_button_num));
  
  new_tooltip = new_label + '=' + KCalcSettings::valueConstant(_button_num);
  
  addMode(ModeNormal, new_label, new_tooltip);
}

void KCalcConstButton::initPopupMenu(void)
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
	  SIGNAL(triggeredConstant(science_constant const &)),
	  SLOT(slotChooseScientificConst(science_constant const &)));

}

void KCalcConstButton::slotConfigureButton()
{
    bool yes_no;
    QString input = KInputDialog::getText(i18n("New Name for Constant"), i18n("New name:"),
					 text(), &yes_no, this); // "nameUserConstants-Dialog"
    if(yes_no) {
        KCalcSettings::setNameConstant(_button_num, input);
        setLabelAndTooltip();
    }
}

void KCalcConstButton::slotChooseScientificConst(struct science_constant const &const_chosen)
{
  KCalcSettings::setValueConstant(_button_num, const_chosen.value);

  KCalcSettings::setNameConstant(_button_num,const_chosen.label);
  
  setLabelAndTooltip();
}

void KCalcConstButton::slotClicked()
{
	emit clicked(_button_num);
}

#include "kcalc_const_button.moc"

