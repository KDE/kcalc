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

#define NUM_CONST 9

const struct science_constant KCalcConstMenu::Constants[] = {
  {QString::fromUtf8("Ï€"), i18n("Pi"), "", "3.14159265358979323846264338327950288", Mathematics},
  {"e", i18n("Euler number"), "", "2.71828182845904523536028747135266249", Mathematics},
  {"c", i18n("Light speed"), "", "2.99792458e8", Physics},
  {"h", i18n("Planck's constant"), "", "6.6260693e-34", Physics},
  {"G", i18n("Constant of gravitation"), "", "6.6742e-11", Physics},
  {"e", i18n("Elementary charge"), "", "1.60217653e-19", Physics},
  {"µ0", i18n("Permeability of vacuum"), "", "1.2566370614e-6", Physics},
  {QString::fromUtf8("\0x3B5"), i18n("Permittivity of vacuum"), "", "8.854187817e-12", Physics},
  {"NA", i18n("Avogadro's number"), "", "6.0221415e23", Chemistry}
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
