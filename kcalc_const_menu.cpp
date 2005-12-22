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

#include <klocale.h>

#include "kcalc_const_menu.h"

#define NUM_CONST 17

const struct science_constant KCalcConstMenu::Constants[] = {
  {QString::fromUtf8("π"), I18N_NOOP("Pi"), "",
   "3.1415926535897932384626433832795028841971693993751"
   "05820974944592307816406286208998628034825342117068", Mathematics},
  {"e", I18N_NOOP("Euler Number"), "",
   "2.7182818284590452353602874713526624977572470936999"
   "59574966967627724076630353547594571382178525166427", Mathematics},
  {QString::fromUtf8("φ"), I18N_NOOP("Golden Ratio"), "", "1.61803398874989484820458683436563811", Mathematics},
  {"c", I18N_NOOP("Light Speed"), "", "2.99792458e8", Electromagnetic},
  {"h", I18N_NOOP("Planck's Constant"), "", "6.6260693e-34", Nuclear},
  {"G", I18N_NOOP("Constant of Gravitation"), "", "6.6742e-11", Gravitation},
  {"g", I18N_NOOP("Earth Acceleration"), "", "9.80665", Gravitation},
  {"e", I18N_NOOP("Elementary Charge"), "", "1.60217653e-19", ConstantCategory(Electromagnetic|Nuclear)},
  {"Z_0", I18N_NOOP("Impedance of Vacuum"), "", "376.730313461", Electromagnetic},
  {QString::fromUtf8("α"), I18N_NOOP("Fine-Structure Constant"), "", "7.297352568e-3", Nuclear},
  {QString::fromUtf8("μ")+"_0", I18N_NOOP("Permeability of Vacuum"), "", "1.2566370614e-6", Electromagnetic},
  {QString::fromUtf8("ε")+"_0", I18N_NOOP("Permittivity of vacuum"), "", "8.854187817e-12", Electromagnetic},
  {"k", I18N_NOOP("Boltzmann Constant"), "", "1.3806505e-23", Thermodynamics},
  {"1u", I18N_NOOP("Atomic Mass Unit"), "", "1.66053886e-27", Thermodynamics},
  {"R", I18N_NOOP("Molar Gas Constant"), "", "8.314472", Thermodynamics},
  {QString::fromUtf8("σ"), I18N_NOOP("Stefan-Boltzmann Constant"), "", "5.670400e-8", Thermodynamics},
  {"N_A", I18N_NOOP("Avogadro's Number"), "", "6.0221415e23", Thermodynamics}
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


  for (int i = 0; i<NUM_CONST; i++) {
    if(Constants[i].category  &  Mathematics)
      math_menu->insertItem(i18n(Constants[i].name), i);
    if(Constants[i].category  &  Electromagnetic)
      em_menu->insertItem(i18n(Constants[i].name), i);
    if(Constants[i].category  &  Nuclear)
      nuclear_menu->insertItem(i18n(Constants[i].name), i);
    if(Constants[i].category  &  Thermodynamics)
      thermo_menu->insertItem(i18n(Constants[i].name), i);
    if(Constants[i].category  &  Gravitation)
      gravitation_menu->insertItem(i18n(Constants[i].name), i);
  }
}


#include "kcalc_const_menu.moc"
