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

#include <qpainter.h>
#include <qpixmap.h>

#include "qdom.h"
#include "kcalc_button.h"


KCalcButton::KCalcButton(QWidget * parent, const char * name)
  : KPushButton(parent, name)
{
  setAutoDefault(false);
}

KCalcButton::KCalcButton(const QString &label, QWidget * parent,
			 const char * name)
  : KPushButton(label, parent, name), _label(label)
{
  setAutoDefault(false);
}

void KCalcButton::setExponent(const QString &exponent)
{
  _exponent = exponent;
  update();
}

void KCalcButton::setText(const QString &label)
{
  _label = label;
  KPushButton::setText(_label);
  update();
}

void KCalcButton::setTextWithExponent(const QString &label,
				      const QString &exponent)
{
  _label = label;
  _exponent = exponent;
  KPushButton::setText(_label);
  update();
}

void KCalcButton::drawButtonLabel(QPainter *paint)
{
  if(pixmap()) {
    KPushButton::drawButtonLabel(paint);
    return;
  }

  // set either standard label or label with exponent
  if(_exponent.isNull())  {
    KPushButton::drawButtonLabel(paint);
  } else {
    QFont fontExponent = font();
    int fontsizeExponent = fontExponent.pointSize() - 2;
    fontExponent.setPointSize( fontsizeExponent );
    fontExponent.setWeight( 0 );
    paint->drawText( 18, 15, _label );
    paint->setFont( fontExponent );
    paint->drawText( 26, 11, _exponent );
  }
}

#include "kcalc_button.moc"

