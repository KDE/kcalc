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

#include <QApplication>
#include <QStyle>

#include "kcalc_button.h"


KCalcButton::KCalcButton(QWidget * parent)
  : KPushButton(parent), _show_shortcut_mode(false),
    _mode_flags(ModeNormal), _label(this)
{
  setAutoDefault(false);

  // use preferred size policy for vertical
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
}

KCalcButton::KCalcButton(const QString &label, QWidget * parent,
			 const QString &tooltip)
  : KPushButton(label, parent), _show_shortcut_mode(false),
    _mode_flags(ModeNormal), _label(this)
{
  setAutoDefault(false);
  addMode(ModeNormal, label, tooltip);
}

void KCalcButton::addMode(ButtonModeFlags mode, const QString &label,
	const QString &tooltip, bool is_label_richtext, const KIcon &icon)
{
  if (_mode.contains(mode)) _mode.remove(mode);

  _mode[mode] = ButtonMode(label, tooltip, is_label_richtext, icon);

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
    // save shortcut, because setting label erases it
    QKeySequence _shortcut = shortcut();

    if(_mode[new_mode].is_label_richtext) {
      _label.setText(_mode[new_mode].label);
      setText(QString::null);	//krazy:exclude=nullstrassign for old broken gcc
      _label.show();
    }  else {
      setText(_mode[new_mode].label);
      _label.hide();
    }
    this->setToolTip( _mode[new_mode].tooltip);
	setIcon(_mode[new_mode].icon);
    _mode_flags = new_mode;


    // restore shortcut
    setShortcut(_shortcut);
  }

  // this is necessary for people pressing CTRL and changing mode at
  // the same time...
  if (_show_shortcut_mode) slotSetAccelDisplayMode(true);
  
  update();
}

static QString escape(QString str)
{
  str.replace('&', "&&");
  return str;
}


void KCalcButton::slotSetAccelDisplayMode(bool flag)
{
  _show_shortcut_mode = flag;

  // save shortcut, because setting label erases it
  QKeySequence _shortcut = shortcut();
  
  if (flag == true) {
    setText(escape(QString(shortcut())));
    _label.hide();
  } else {
    if(_mode[_mode_flags].is_label_richtext) {
      _label.setText(_mode[_mode_flags].label);
      setText(QString::null);	//krazy:exclude=nullstrassign for old broken gcc
      _label.show();
    }  else {
      setText(_mode[_mode_flags].label);
      _label.hide();
    }
  }

  // restore shortcut
  setShortcut(_shortcut);
  update();
}

void KCalcButton::paintEvent(QPaintEvent *p)
{
  // draw first button frame (and label, when not using richtext)
  KPushButton::paintEvent(p);

  if(_mode[_mode_flags].is_label_richtext)
    _label.move((width()-_label.width())/2, (height()-_label.height())/2);
  
}

QSize KCalcButton::sizeHint() const
{
	// reimplemented to provide a shorter button
    int margin = style()->pixelMetric(QStyle::PM_ButtonMargin, 0, this);
    int h = fontMetrics().lineSpacing() + margin*2 + 4;

	QSize sz = QPushButton::sizeHint();
	sz.setHeight(qMin(sz.height(), h));
	return sz.expandedTo(QApplication::globalStrut());
}

#include "kcalc_button.moc"

