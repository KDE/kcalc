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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef _KCALC_BUTTON_H
#define _KCALC_BUTTON_H

#include <kpushbutton.h>

class QDomNode;

class KCalcButton : public KPushButton
{
Q_OBJECT

 public:
  KCalcButton(QWidget *parent, const char * name = 0); 
  KCalcButton(const QString &label, QWidget *parent, const char * name = 0);

 private:
};


#endif  // _KCALC_BUTTON_H
