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

#include <klocale.h>

#include "kcalc_const_menu.h"

#define NUM_CONST 3

const struct science_constant KCalcConstMenu::Constants[] = {
  {"Pi", i18n("Pi"), "", "3.14", Mathematics},
  {"e", i18n("Euler number"), "", "2.71", Mathematics},
  {"c", i18n("Light speed"), "", "3e8", Physics}
};

KCalcConstMenu::KCalcConstMenu(QWidget * parent, const char * name)
  : QPopupMenu(parent, name)
{
  QPopupMenu *math_menu = new QPopupMenu(this, "mathematical constants");
  QPopupMenu *phys_menu = new QPopupMenu(this, "physical constants");
  QPopupMenu *chem_menu = new QPopupMenu(this, "chemical constants");

  insertItem(i18n("Mathematics"), math_menu);
  insertItem(i18n("Physics"), phys_menu);
  insertItem(i18n("Chemistry"), chem_menu);

  connect(math_menu, SIGNAL(activated(int)), this, SLOT(slotPassActivate(int)));
  connect(phys_menu, SIGNAL(activated(int)), this, SLOT(slotPassActivate(int)));
  connect(chem_menu, SIGNAL(activated(int)), this, SLOT(slotPassActivate(int)));

  

  for (int i = 0; i<NUM_CONST; i++)
    switch (Constants[i].category)
      {
      case Mathematics:
	math_menu->insertItem(Constants[i].name, i);
	break;
      case Physics:
	phys_menu->insertItem(Constants[i].name, i);
	break;
      case Chemistry:
	chem_menu->insertItem(Constants[i].name, i);
	break;
      }
}


#include "kcalc_const_menu.moc"
