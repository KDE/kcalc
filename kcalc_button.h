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

typedef enum {ModeNormal = 0, ModeInverse = 1, ModeHyperbolic = 2} ButtonModeFlags;

class QDomNode;

// Each kcalc button can be in one of several modes.
// The following class describes label, tooltip etc. for each mode...
class ButtonMode
{
public:
  ButtonMode(void) {};
  ButtonMode(QString &label, QString &tooltip)
    : label(label), tooltip(tooltip) { };

  QString label;
  QString tooltip;
};


class KCalcButton : public KPushButton
{
Q_OBJECT

public:
 KCalcButton(QWidget *parent, const char * name = 0); 
 KCalcButton(const QString &label, QWidget *parent, const char * name = 0);

 void addMode(ButtonModeFlags mode, QString label, QString tooltip);

 virtual void setText(const QString &label);
 
public slots: 
  void slotSetInverseMode(bool flag);
  void slotSetMode(ButtonModeFlags mode, bool flag); 
  void slotSetAccelDisplayMode(bool flag);

protected:
 virtual void drawButtonLabel(QPainter *paint);

private:
 bool _inverse_mode;
 bool _show_accel_mode;
 QString _label;

 ButtonModeFlags _mode_flags;

 QMap<ButtonModeFlags, ButtonMode> _mode;
};


#endif  // _KCALC_BUTTON_H
