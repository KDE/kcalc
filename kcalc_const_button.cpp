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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <qstring.h>


#include <kcmenumngr.h>
#include <kinputdialog.h>
#include <kpopupmenu.h>

#include "kcalc_const_button.h"
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
  return (KCalcSettings::valueUserConstants())[_button_num];
}

void KCalcConstButton::setLabelAndTooltip(void)
{
  QString new_label = QString("C") + QString().setNum(_button_num + 1);
  QString new_tooltip;
  
  // this is such a crappy work-around, because I don´t know how to
  // access the constants in a generic way
  switch(_button_num) {
  case 0:
    new_label = (KCalcSettings::nameConstant1().isNull() ? new_label : KCalcSettings::nameConstant1());
    break;
  case 1:
    new_label = (KCalcSettings::nameConstant2().isNull() ? new_label : KCalcSettings::nameConstant2());
    break;
  case 2:
    new_label = (KCalcSettings::nameConstant3().isNull() ? new_label : KCalcSettings::nameConstant3());
    break;
  case 3:
    new_label = (KCalcSettings::nameConstant4().isNull() ? new_label : KCalcSettings::nameConstant4());
    break;
  case 4:
    new_label = (KCalcSettings::nameConstant5().isNull() ? new_label : KCalcSettings::nameConstant5());
    break;
  case 5:
    new_label = (KCalcSettings::nameConstant6().isNull() ? new_label : KCalcSettings::nameConstant6());
    break;
  }
  
  new_tooltip = new_label + "=" + (KCalcSettings::valueUserConstants())[_button_num];
  
  addMode(ModeNormal, new_label, new_tooltip);
}

void KCalcConstButton::initPopupMenu(void)
{
  _popup = new KPopupMenu(this, "set const-cutton");
  _popup->insertItem(i18n("Set name"), 0);
  //_popup->insertItem(i18n("Choose from list"), 1);

  connect(_popup, SIGNAL(activated(int)), SLOT(slotConfigureButton(int)));

  KContextMenuManager::insert(this, _popup);
}

void KCalcConstButton::slotConfigureButton(int option)
{
  if (option == 0)
    {
      bool yes_no;
      QString input = KInputDialog::text(i18n("New name for constant"), i18n("New name:"),
					 text(), &yes_no, this, "nameUserConstants-Dialog");
      if(yes_no) {
	// this is such a crappy work-around, because I don´t know how to
	// access the constants in a generic way
	switch(_button_num) {
	case 0:
	  KCalcSettings::setNameConstant1(input);
	  break;
	case 1:
	  KCalcSettings::setNameConstant2(input);
	  break;
	case 2:
	  KCalcSettings::setNameConstant3(input);
	  break;
	case 3:
	  KCalcSettings::setNameConstant4(input);
	  break;
	case 4:
	  KCalcSettings::setNameConstant5(input);
	  break;
	case 5:
	  KCalcSettings::setNameConstant6(input);
	  break;
	}
	setLabelAndTooltip();
      }
    }
}

#include "kcalc_const_button.moc"

