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

double KCalcConstButton::toDouble(void) const
{
  return (KCalcSettings::valueUserConstants())[_button_num].toDouble();
}

void KCalcConstButton::setName(void)
{
  QString new_label = ((KCalcSettings::nameUserConstants())[_button_num].isEmpty() ?
		       QString("C") + QString().setNum(_button_num + 1) :
		       (KCalcSettings::nameUserConstants())[_button_num]);
  QString new_tooltip = new_label + "=" + (KCalcSettings::valueUserConstants())[_button_num];
  
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
      if(yes_no)
	{
	  QStringList tmp_list = KCalcSettings::nameUserConstants();
	  tmp_list[_button_num] = input;
	  KCalcSettings::setNameUserConstants(tmp_list);
	  setName();
	}
    }
}

#include "kcalc_const_button.moc"

