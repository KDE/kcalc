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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <qsimplerichtext.h>
#include <qtooltip.h>
#include <qpainter.h>


#include "qdom.h"

#include "kcalc_button.h"


KCalcButton::KCalcButton(QWidget * parent, const char * name)
  : KPushButton(parent, name), _show_accel_mode(false),
    _mode_flags(ModeNormal)
{
  setAutoDefault(false);
}

KCalcButton::KCalcButton(const QString &label, QWidget * parent,
			 const char * name, const QString &tooltip)
  : KPushButton(label, parent, name), _show_accel_mode(false), _mode_flags(ModeNormal)
{
  setAutoDefault(false);
  addMode(ModeNormal, label, tooltip);
}

void KCalcButton::addMode(ButtonModeFlags mode, QString label, QString tooltip, bool is_label_richtext)
{
  if (_mode.contains(mode)) _mode.remove(mode);

  _mode[mode] = ButtonMode(label, tooltip, is_label_richtext);

  // Need to put each button into default mode first
  if(mode == ModeNormal) slotSetMode(ModeNormal, true);
}

void KCalcButton::slotSetMode(ButtonModeFlags mode, bool flag)
{
  ButtonModeFlags new_mode;

  if (flag) { // if the specified mode is to be set (i.e. flag = true)
  	new_mode = ButtonModeFlags(_mode_flags | mode);
  } else if (_mode_flags && mode) { // if the specified mode is to be cleared (i.e. flag = false)
  	new_mode = ButtonModeFlags(_mode_flags - mode);
  } else {
  	return; // nothing to do
  }

  if (_mode.contains(new_mode)) {
    // save accel, because setting label erases accel
    QKeySequence _accel = accel();

    if(_mode[new_mode].is_label_richtext)
      _label = _mode[new_mode].label;
    else
      setText(_mode[new_mode].label);
	QToolTip::remove(this);
    QToolTip::add(this, _mode[new_mode].tooltip);
    _mode_flags = new_mode;

    // restore accel
    setAccel(_accel);
  }

  // this is necessary for people pressing CTRL and changing mode at
  // the same time...
  if (_show_accel_mode) slotSetAccelDisplayMode(true);
  
  update();
}

static QString escape(QString str)
{
  str.replace('&', "&&");
  return str;
}


void KCalcButton::slotSetAccelDisplayMode(bool flag)
{
  _show_accel_mode = flag;

  // save accel, because setting label erases accel
  QKeySequence _accel = accel();
  
  if (flag == true) {
    setText(escape(QString(accel())));
  } else {
    setText(_mode[_mode_flags].label);
  }

  // restore accel
  setAccel(_accel);
}

void KCalcButton::paintLabel(QPainter *paint)
{
  if (_mode[_mode_flags].is_label_richtext) {
    QSimpleRichText _text(_label, font());
    _text.draw(paint, width()/2-_text.width()/2, 0, childrenRegion(), colorGroup());
  } else {
    KPushButton::drawButtonLabel(paint);
  }
}

void KCalcButton::drawButtonLabel(QPainter *paint)
{
  if (_show_accel_mode) {
    KPushButton::drawButtonLabel(paint);
  } else if (_mode.contains(_mode_flags)) {
    paintLabel(paint);
  }
}


void KSquareButton::paintLabel(QPainter *paint)
{
  int w = width();
  int w2 = w/2 - 13;
  int h = height();
  int h2 = h/2 - 7;
  // in some KDE-styles (.NET, Phase,...) we have to set the painter back to the right color
  paint->setPen(foregroundColor());
  // these statements are for the improved
  // representation of the sqrt function
  paint->drawLine(w2, 11 + h2, w2 + 2, 7 + h2);
  paint->drawLine(w2 + 2, 7 + h2, w2 + 4, 14 + h2);
  paint->drawLine(w2 + 4, 14 + h2, w2 + 6, 1 + h2);
  paint->drawLine(w2 + 6, 1 + h2, w2 + 27, 1 + h2);
  paint->drawLine(w2 + 27, 1 + h2, w2 + 27, 4 + h2);
  // add a three for the cube root
  if (_mode_flags & ModeInverse) {
    paint->drawText(w2-2, 9 + h2, "³");
  }
}

void KSquareButton::drawButtonLabel(QPainter *paint)
{
  if (_show_accel_mode) {
    KPushButton::drawButtonLabel(paint);
  } else if (_mode.contains(_mode_flags)) {
    paintLabel(paint);
  }
}

#include "kcalc_button.moc"

