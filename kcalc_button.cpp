/*
    kCalculator, a simple scientific calculator for KDE

    Copyright (C) 1996-2000 Bernd Johannes Wuebben
                            wuebben@kde.org

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

#include <qsimplerichtext.h>

#include "qdom.h"
#include "kcalc_button.h"


KCalcButton::KCalcButton(QWidget * parent, const char * name)
  : KPushButton(parent, name), _inverse_mode(false), _show_accel_mode(false)
{
  setAutoDefault(false);
}

KCalcButton::KCalcButton(const QString &label, QWidget * parent,
			 const char * name)
  : KPushButton(label, parent, name), _inverse_mode(false),
    _show_accel_mode(false)
{
  setAutoDefault(false);
  setText(label);
}

void KCalcButton::slotSetInverseMode(bool flag)
{
  _inverse_mode = flag;
  update();
}

void KCalcButton::slotSetAccelDisplayMode(bool flag)
{
  _show_accel_mode = flag;

  // save accel, because setting label erases accel
  QKeySequence _accel = accel();

  KPushButton::setText(QString(accel()));
  update();

  // set back deleted accel
  setAccel(_accel);
}

void KCalcButton::setText(const QString &label)
{
  _label = "<qt type=\"page\"><center>" + label + "</center></qt>";
  KPushButton::setText(_label);
  update();
}

void KCalcButton::setInvText(const QString &label)
{
  _inv_label = "<qt type=\"page\"><center>" + label + "</center></qt>";
  KPushButton::setText(_label);
  update();
}

void KCalcButton::drawButtonLabel(QPainter *paint)
{
  if (_show_accel_mode) {
    KPushButton::drawButtonLabel(paint);
  } else if(pixmap()) {
    KPushButton::drawButtonLabel(paint);
    return;
  } else {
    if (_inverse_mode == false  || _inv_label.isNull()) {
      QSimpleRichText _text(_label, font());
      _text.draw(paint, width()/2-_text.width()/2, 0, childrenRegion(), colorGroup());
    } else {
      QSimpleRichText _text(_inv_label, font());
      _text.draw(paint, width()/2-_text.width()/2, 0, childrenRegion(), colorGroup());
    }
  }
}

#include "kcalc_button.moc"

