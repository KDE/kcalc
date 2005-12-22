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

#include <qstring.h>


#include <kcmenumngr.h>
#include <kinputdialog.h>
#include <kpopupmenu.h>

#include "kcalc_const_button.h"
#include "kcalc_const_menu.h"
#include "kcalc_settings.h"


KCalcConstButton::KCalcConstButton(QWidget *parent, int but_num, const char * name)
  : KCalcButton(parent, name), _button_num(but_num)
{
  addMode(ModeInverse, "Store", i18n("Write display data into memory"));
  
  initPopupMenu();
}


KCalcConstButton::KCalcConstButton(const QString &label, QWidget *parent, int but_num,
                                   const char * name, const QString &tooltip)
  : KCalcButton(label, parent, name, tooltip), _button_num(but_num)
{
  addMode(ModeInverse, "Store", i18n("Write display data into memory"));
  
  initPopupMenu();
}

QString KCalcConstButton::constant(void) const
{
  return KCalcSettings::valueConstant(_button_num);
}

void KCalcConstButton::setLabelAndTooltip(void)
{
  QString new_label = QString("C") + QString().setNum(_button_num + 1);
  QString new_tooltip;
  
  new_label = (KCalcSettings::nameConstant(_button_num).isNull() ? new_label : KCalcSettings::nameConstant(_button_num));
  
  new_tooltip = new_label + "=" + KCalcSettings::valueConstant(_button_num);
  
  addMode(ModeNormal, new_label, new_tooltip);
}

void KCalcConstButton::initPopupMenu(void)
{
  KCalcConstMenu *tmp_menu = new KCalcConstMenu(this);
  
  _popup = new KPopupMenu(this, "set const-cutton");
  _popup->insertItem(i18n("Set Name"), 0);
  _popup->insertItem(i18n("Choose From List"), tmp_menu, 1);
  
  connect(_popup, SIGNAL(activated(int)), SLOT(slotConfigureButton(int)));
  connect(tmp_menu, SIGNAL(activated(int)), SLOT(slotChooseScientificConst(int)));

  KContextMenuManager::insert(this, _popup);
}

void KCalcConstButton::slotConfigureButton(int option)
{
  if (option == 0)
    {
      bool yes_no;
      QString input = KInputDialog::text(i18n("New Name for Constant"), i18n("New name:"),
					 text(), &yes_no, this, "nameUserConstants-Dialog");
      if(yes_no) {
	KCalcSettings::setNameConstant(_button_num, input);
	setLabelAndTooltip();
      }
    }
}

void KCalcConstButton::slotChooseScientificConst(int option)
{
  KCalcSettings::setValueConstant(_button_num,
				  KCalcConstMenu::Constants[option].value);

  KCalcSettings::setNameConstant(_button_num,
				 KCalcConstMenu::Constants[option].label);
  
  setLabelAndTooltip();
}

#include "kcalc_const_button.moc"

