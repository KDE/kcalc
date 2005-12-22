/*
    KCalc, a scientific calculator for the X window system using the
    Qt widget libraries, available at no cost at http://www.troll.no

    Copyright (C) 1996 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

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

#ifndef _KCALC_CONST_BUTTON_H
#define _KCALC_CONST_BUTTON_H

#include <klocale.h>
#include "kcalc_button.h"


class KCalcConstButton : public KCalcButton
{
Q_OBJECT

  public:

  KCalcConstButton(QWidget *parent, int but_num, const char * name = 0);
  
  KCalcConstButton(const QString &label, QWidget *parent, int but_num, const char * name = 0,
		   const QString &tooltip = QString());

  QString constant(void) const;

  void setLabelAndTooltip(void);

  private slots:
  void slotConfigureButton(int option);
  void slotChooseScientificConst(int option);

  private:
  void initPopupMenu(void);
  
  KPopupMenu* _popup;
  int _button_num;
};


#endif  // _KCALC_CONST_BUTTON_H
