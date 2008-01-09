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

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QStyleOptionButton>
#include <QStylePainter>
#include <QTextDocument>

#include "kcalc_button.h"


KCalcButton::KCalcButton(QWidget * parent)
  : KPushButton(parent), _show_shortcut_mode(false),
    _mode_flags(ModeNormal)
{
	setFocusPolicy(Qt::TabFocus);
	setAutoDefault(false);

	// use preferred size policy for vertical
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
}

KCalcButton::KCalcButton(const QString &label, QWidget * parent,
			 const QString &tooltip)
  : KPushButton(label, parent), _show_shortcut_mode(false),
    _mode_flags(ModeNormal)
{
  setAutoDefault(false);
  addMode(ModeNormal, label, tooltip);

  // use preferred size policy for vertical
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
}

void KCalcButton::addMode(ButtonModeFlags mode, const QString &label,
						  const QString &tooltip, const KIcon &icon)
{
  if (_mode.contains(mode)) _mode.remove(mode);

  _mode[mode] = ButtonMode(label, tooltip, icon);

  // Need to put each button into default mode first
  if (mode == ModeNormal) slotSetMode(ModeNormal, true);
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

	setText(_mode[new_mode].label);
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

void KCalcButton::slotSetAccelDisplayMode(bool flag)
{
  _show_shortcut_mode = flag;

  // save shortcut, because setting label erases it
  QKeySequence _shortcut = shortcut();
  
  if (flag == true) {
    setText(QString(shortcut()));
  } else {
    setText(_mode[_mode_flags].label);
  }

  // restore shortcut
  setShortcut(_shortcut);
  update();
}

void KCalcButton::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	QStyleOptionButton option;
	initStyleOption(&option);

	// draw bevel
	style()->drawControl(QStyle::CE_PushButtonBevel, &option, &p, this);

	// draw label...
	p.save();

	// rant: Qt4 needs QSimpleRichText, dammit!
	QTextDocument doc;
	QAbstractTextDocumentLayout::PaintContext context;
	doc.setHtml("<center>" + text() + "</center>");
	context.palette = palette();
        context.palette.setColor(QPalette::Text, context.palette.buttonText().color());

	p.translate(width()/2 - doc.size().width()/2,
				height()/2 - doc.size().height()/2);
	doc.documentLayout()->draw(&p, context);
	p.restore();

	// draw focus
	if (hasFocus()) {
		QStyleOptionFocusRect fropt;
		fropt.QStyleOption::operator=(option);
		fropt.rect = style()->subElementRect(QStyle::SE_PushButtonFocusRect,
											 &option, this);
		style()->drawPrimitive(QStyle::PE_FrameFocusRect, &fropt, &p, this);
	}
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

void KCalcButton::setText(const QString &text)
{
	KPushButton::setText(text);

	// normal mode may not have been explicitly set
	if (_mode[ModeNormal].label.isEmpty()) {
		_mode[ModeNormal].label = text;
	}
}

void KCalcButton::setToolTip(const QString &tip)
{
	KPushButton::setToolTip(tip);

	// normal mode may not have been explicitly set
	if (_mode[ModeNormal].tooltip.isEmpty()) {
		_mode[ModeNormal].tooltip = tip;
	}
}

#include "kcalc_button.moc"

