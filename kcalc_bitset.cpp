/*
Copyright (C) 2012 - 2013 Evan Teran
                          evan.teran@gmail.com

Copyright (C) 2006        Michel Marti
                          mma@objectxp.com

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

#include "kcalc_bitset.h"
#include "bitbutton.h"

#include <QGridLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QPainter>

#include "kcalc_bitset.moc"

// TODO: I think it would actually be appropriate to use a std::bitset<64>
//       for the internal representation of this class perhaps
//       the only real caveat is the conversion to/from quint64

//------------------------------------------------------------------------------
// Name: paintEvent
// Desc: draws the button
//------------------------------------------------------------------------------
void BitButton::paintEvent(QPaintEvent *) {

	QPainter painter(this);
	QPen pen(palette().text(), 2);
	pen.setJoinStyle(Qt::MiterJoin);
	painter.setPen(pen);

	if (on_) {
		painter.setBrush(palette().text());
	} else {
		painter.setBrush(palette().base());
	}

	painter.drawRect(rect().adjusted(1, 1, -1, -1));
}

//------------------------------------------------------------------------------
// Name: KCalcBitset
// Desc: constructor
//------------------------------------------------------------------------------
KCalcBitset::KCalcBitset(QWidget *parent) : QFrame(parent), value_(0) {

	setFrameStyle(QFrame::Panel | QFrame::Sunken);
	
	bit_button_group_ = new QButtonGroup(this);
	connect(bit_button_group_, SIGNAL(buttonClicked(int)), SLOT(slotToggleBit(int)));

	// smaller label font
	QFont fnt = font();
	if (fnt.pointSize() > 6) {
		fnt.setPointSize(fnt.pointSize() - 1);
	}

	// main layout
	QGridLayout *layout = new QGridLayout(this);
	layout->setMargin(2);
	layout->setSpacing(0);

	// create bits
	int bitCounter = 63;
	for (int rows = 0; rows < 2; rows++) {
		for (int cols = 0; cols < 4; cols++) {
			// two rows of four words
			QHBoxLayout *const wordlayout = new QHBoxLayout();
			wordlayout->setMargin(2);
			wordlayout->setSpacing(2);
			layout->addLayout(wordlayout, rows, cols);

			for (int bit = 0; bit < 8; bit++) {
				BitButton *const tmpBitButton = new BitButton(this);
				wordlayout->addWidget(tmpBitButton);
				bit_button_group_->addButton(tmpBitButton, bitCounter);
				bitCounter--;
			}

			// label word
			QLabel *label = new QLabel(this);
			label->setText(QString::number(bitCounter + 1));
			label->setFont(fnt);
			wordlayout->addWidget(label);
		}
	}
}

//------------------------------------------------------------------------------
// Name: setValue
// Desc: set the value of the bitset based on an unsigned 64-bit number
//------------------------------------------------------------------------------
void KCalcBitset::setValue(quint64 value) {

	if (value_ == value) {
		// don't waste time if there was no change..
		return;
	}

	value_ = value;
	
	// set each bit button
	for (int i = 0; i < 64; i++) {	
		if(BitButton *bb = qobject_cast<BitButton*>(bit_button_group_->button(i))) {
			bb->setOn(value & 1);
		}
		value >>= 1;
	}
}

//------------------------------------------------------------------------------
// Name: getValue
// Desc: returns the bitset value as an unsigned 64-bit number
//------------------------------------------------------------------------------
quint64 KCalcBitset::getValue() const {
    return value_;
}

//------------------------------------------------------------------------------
// Name: slotToggleBit
// Desc: inverts the value of a single bit
//------------------------------------------------------------------------------
void KCalcBitset::slotToggleBit(int bit) {

    const quint64 nv = getValue() ^(1LL << bit);
    setValue(nv);
    emit valueChanged(value_);
}

