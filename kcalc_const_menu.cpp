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

#define NUM_CONST 16

const struct science_constant KCalcConstMenu::Constants[] = {
  {QString::fromUtf8("π"), i18n("Pi"), "", "3.14159265358979323846264338327950288", Mathematics},
  {"e", i18n("Euler Number"), "", "2.71828182845904523536028747135266249", Mathematics},
  {"c", i18n("Light Speed"), "", "2.99792458e8", Electromagnetic},
  {"h", i18n("Planck's Constant"), "", "6.6260693e-34", Nuclear},
  {"G", i18n("Constant of Gravitation"), "", "6.6742e-11", Gravitation},
  {"g", i18n("Earth Acceleration"), "", "9.80665", Gravitation},
  {"e", i18n("Elementary Charge"), "", "1.60217653e-19", Electromagnetic},
  {"Z_0", i18n("Impedance of Vacuum"), "", "376.730313461", Electromagnetic},
  {QString::fromUtf8("α"), i18n("Fine-Structure Constant"), "", "7.297352568e-3", Nuclear},
  {"e", i18n("Elementary Charge"), "", "1.60217653e-19", Nuclear},
  {"_0", i18n("Permeability of Vacuum"), "", "1.2566370614e-6", Electromagnetic},
  {QString::fromUtf8("ε")+"_0", i18n("Permittivity of vacuum"), "", "8.854187817e-12", Electromagnetic},
  {"k", i18n("Boltzmann Constant"), "", "1.3806505e-23", Thermodynamics},
  {"1u", i18n("Atomic Mass Unit"), "", "1.66053886e-27", Thermodynamics},
  {"R", i18n("Molar Gas Constant"), "", "8.314472", Thermodynamics},
  {QString::fromUtf8("σ"), i18n("Stefan-Boltzmann Constant"), "", "8.314472", Thermodynamics},
  {"N_A", i18n("Avogadro's Number"), "", "6.0221415e23", Thermodynamics}
};

KCalcConstMenu::KCalcConstMenu(QWidget * parent, const char * name)
  : QPopupMenu(parent, name)
{
  QPopupMenu *math_menu = new QPopupMenu(this, "mathematical constants");
  QPopupMenu *em_menu = new QPopupMenu(this, "electromagnetic constants");
  QPopupMenu *nuclear_menu = new QPopupMenu(this, "nuclear constants");
  QPopupMenu *thermo_menu = new QPopupMenu(this, "thermodynamics constants");
  QPopupMenu *gravitation_menu = new QPopupMenu(this, "gravitation constants");

  insertItem(i18n("Mathematics"), math_menu);
  insertItem(i18n("Electromagnetism"), em_menu);
  insertItem(i18n("Atomic && Nuclear"), nuclear_menu);
  insertItem(i18n("Thermodynamics"), thermo_menu);
  insertItem(i18n("Gravitation"), gravitation_menu);

  connect(math_menu, SIGNAL(activated(int)), this, SLOT(slotPassActivate(int)));
  connect(em_menu, SIGNAL(activated(int)), this, SLOT(slotPassActivate(int)));
  connect(nuclear_menu, SIGNAL(activated(int)), this, SLOT(slotPassActivate(int)));
  connect(thermo_menu, SIGNAL(activated(int)), this, SLOT(slotPassActivate(int)));
  connect(gravitation_menu, SIGNAL(activated(int)), this, SLOT(slotPassActivate(int)));


  for (int i = 0; i<NUM_CONST; i++)
    switch (Constants[i].category)
      {
      case Mathematics:
	math_menu->insertItem(Constants[i].name, i);
	break;
      case Electromagnetic:
	em_menu->insertItem(Constants[i].name, i);
	break;
      case Nuclear:
	nuclear_menu->insertItem(Constants[i].name, i);
	break;
      case Thermodynamics:
	thermo_menu->insertItem(Constants[i].name, i);
	break;
      case Gravitation:
	gravitation_menu->insertItem(Constants[i].name, i);
	break;
      }
}


#include "kcalc_const_menu.moc"
