/*
Copyright (C) 2001 - 2013 Evan Teran
                          evan.teran@gmail.com

Copyright (C) 1996 - 2000 Bernd Johannes Wuebben
                          wuebben@kde.org

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kcalc_button.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QStyleOptionButton>
#include <QStylePainter>
#include <QTextDocument>

#include "kcalc_button.moc"

//------------------------------------------------------------------------------
// Name: KCalcButton
// Desc: constructor
//------------------------------------------------------------------------------
KCalcButton::KCalcButton(QWidget *parent) : KPushButton(parent), show_shortcut_mode_(false), mode_flags_(ModeNormal), size_() {

	setAcceptDrops(true);   // allow color drops
	setFocusPolicy(Qt::TabFocus);
	setAutoDefault(false);

	// use preferred size policy for vertical
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
}

//------------------------------------------------------------------------------
// Name: KCalcButton
// Desc: constructor
//------------------------------------------------------------------------------
KCalcButton::KCalcButton(const QString &label, QWidget *parent, const QString &tooltip) : KPushButton(label, parent), show_shortcut_mode_(false), mode_flags_(ModeNormal), size_() {

	setAutoDefault(false);
	addMode(ModeNormal, label, tooltip);

	// use preferred size policy for vertical
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
}

//------------------------------------------------------------------------------
// Name: addMode
// Desc: 
//------------------------------------------------------------------------------
void KCalcButton::addMode(ButtonModeFlags mode, const QString &label, const QString &tooltip) {

	if (mode_.contains(mode)) {
		mode_.remove(mode);
	}

	mode_[mode] = ButtonMode(label, tooltip);
	calcSizeHint();

	// Need to put each button into default mode first
	if (mode == ModeNormal) {
		slotSetMode(ModeNormal, true);
	}
}

//------------------------------------------------------------------------------
// Name: slotSetMode
// Desc: 
//------------------------------------------------------------------------------
void KCalcButton::slotSetMode(ButtonModeFlags mode, bool flag) {

	ButtonModeFlags new_mode;

	if (flag) {   // if the specified mode is to be set (i.e. flag = true)
		new_mode = ButtonModeFlags(mode_flags_ | mode);
	} else if (mode_flags_ && mode) {   // if the specified mode is to be cleared (i.e. flag = false)
		new_mode = ButtonModeFlags(mode_flags_ - mode);
	} else {
		return; // nothing to do
	}

	if (mode_.contains(new_mode)) {
		// save shortcut, because setting label erases it
		QKeySequence current_shortcut = shortcut();

		setText(mode_[new_mode].label);
		this->setToolTip(mode_[new_mode].tooltip);
		mode_flags_ = new_mode;

		// restore shortcut
		setShortcut(current_shortcut);
	}

	// this is necessary for people pressing CTRL and changing mode at
	// the same time...
	if (show_shortcut_mode_) {
		slotSetAccelDisplayMode(true);
	}

	update();
}

//------------------------------------------------------------------------------
// Name: slotSetAccelDisplayMode
// Desc: 
//------------------------------------------------------------------------------
void KCalcButton::slotSetAccelDisplayMode(bool flag) {

	show_shortcut_mode_ = flag;

	// save shortcut, because setting label erases it
	QKeySequence current_shortcut = shortcut();

	if (flag) {
		setText(QString(shortcut()));
	} else {
		setText(mode_[mode_flags_].label);
	}

	// restore shortcut
	setShortcut(current_shortcut);
	update();
}

//------------------------------------------------------------------------------
// Name: paintEvent
// Desc: draws the button
//------------------------------------------------------------------------------
void KCalcButton::paintEvent(QPaintEvent *) {

	QPainter p(this);
	QStyleOptionButton option;
	initStyleOption(&option);
	const bool is_down = isDown() || isChecked();
	const int x_offset = is_down ? style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal, &option, this) : 0;
	const int y_offset = is_down ? style()->pixelMetric(QStyle::PM_ButtonShiftVertical, &option, this) : 0;

	// draw bevel
	style()->drawControl(QStyle::CE_PushButtonBevel, &option, &p, this);

	// draw label...
	p.save();

	// rant: Qt4 needs QSimpleRichText, dammit!
	QTextDocument doc;
	QAbstractTextDocumentLayout::PaintContext context;
	doc.setHtml(QLatin1String("<center>") + text() + QLatin1String("</center>"));
	doc.setDefaultFont(font());
	context.palette = palette();
	context.palette.setColor(QPalette::Text, context.palette.buttonText().color());

	p.translate((width() / 2 - doc.size().width() / 2) + x_offset, (height() / 2 - doc.size().height() / 2) + y_offset);
	doc.documentLayout()->draw(&p, context);
	p.restore();

	// draw focus
	if (hasFocus()) {
		QStyleOptionFocusRect fropt;
		fropt.QStyleOption::operator=(option);
		fropt.rect = style()->subElementRect(QStyle::SE_PushButtonFocusRect, &option, this);
		style()->drawPrimitive(QStyle::PE_FrameFocusRect, &fropt, &p, this);
	}
}

//------------------------------------------------------------------------------
// Name: sizeHint
// Desc: 
//------------------------------------------------------------------------------
QSize KCalcButton::sizeHint() const {
	// reimplemented to provide a smaller button
	return size_;
}

//------------------------------------------------------------------------------
// Name: calcSizeHint
// Desc: 
//------------------------------------------------------------------------------
void KCalcButton::calcSizeHint() {

	int margin = style()->pixelMetric(QStyle::PM_ButtonMargin, 0, this);
	
	// want narrow margin than normal
	margin = qMax(margin / 2, 3);

	// approximation because metrics doesn't account for richtext
	size_ = fontMetrics().size(0, mode_[ModeNormal].label);
	if (mode_.contains(ModeShift)) {
		size_ = size_.expandedTo(fontMetrics().size(0, mode_[ModeShift].label));
	}
	
	if (mode_.contains(ModeHyperbolic)) {
		size_ = size_.expandedTo(fontMetrics().size(0, mode_[ModeHyperbolic].label));
	}

	size_ += QSize(margin * 2, margin * 2);
	size_ = size_.expandedTo(QApplication::globalStrut());
}

//------------------------------------------------------------------------------
// Name: setFont
// Desc: 
//------------------------------------------------------------------------------
void KCalcButton::setFont(const QFont &fnt) {

	KPushButton::setFont(fnt);
	calcSizeHint();
}

//------------------------------------------------------------------------------
// Name: setText
// Desc: 
//------------------------------------------------------------------------------
void KCalcButton::setText(const QString &text) {

	KPushButton::setText(text);

	// normal mode may not have been explicitly set
	if (mode_[ModeNormal].label.isEmpty()) {
		mode_[ModeNormal].label = text;
	}
	
	calcSizeHint();
}

//------------------------------------------------------------------------------
// Name: setToolTip
// Desc: 
//------------------------------------------------------------------------------
void KCalcButton::setToolTip(const QString &tip) {

	KPushButton::setToolTip(tip);

	// normal mode may not have been explicitly set
	if (mode_[ModeNormal].tooltip.isEmpty()) {
		mode_[ModeNormal].tooltip = tip;
	}
}
