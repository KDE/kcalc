/*
    Copyright (C) 2006  Michel Marti <mma@objectxp.com>

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

#include "kcalc_bitset.h"

#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QPainter>

#include "kcalc_bitset.moc"

BitButton::BitButton(QWidget *parent) : QAbstractButton(parent), on(false)
{
	setFocusPolicy(Qt::ClickFocus); // too many bits for tab focus

	// size button by font
	QSize size = fontMetrics().size(0, "M");
	if (size.width() < size.height()) {
		size.setHeight(size.width());
	} else {
		size.setWidth(size.height());
	}
	setFixedSize(size.expandedTo(QApplication::globalStrut()));
}

bool BitButton::isOn() const
{
	return on;
}

void BitButton::setOn(bool value)
{
	on = value;
	update();
}

void BitButton::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	QPen pen(palette().text(), 2);
	pen.setJoinStyle(Qt::MiterJoin);
	painter.setPen(pen);

	if (on) painter.setBrush(palette().text());
	else    painter.setBrush(palette().base());

	painter.drawRect(rect().adjusted(1, 1, -1, -1));
}


KCalcBitset::KCalcBitset(QWidget *parent)
   : QFrame(parent), mValue(0)
{
	setFrameStyle(QFrame::Panel | QFrame::Sunken);
	bitButtonGroup = new QButtonGroup(this);
	connect(bitButtonGroup, SIGNAL(buttonClicked (int)),
		SLOT(slotToggleBit(int)));

	// smaller label font
	QFont fnt = font();
	if (fnt.pointSize() > 6) fnt.setPointSize(fnt.pointSize() - 1);

	// main layout
	QGridLayout *layout = new QGridLayout(this);
	layout->setMargin(2);
	layout->setSpacing(0);

	// create bits
	int bitCounter = 63;
	for (int rows=0; rows<2; rows++) {
		for (int cols=0; cols<4; cols++) {
			// two rows of four words
			QHBoxLayout *wordlayout = new QHBoxLayout();
			wordlayout->setMargin(2);
			wordlayout->setSpacing(2);
			layout->addLayout(wordlayout, rows, cols);

			for (int bit=0; bit<8; bit++) {
				QAbstractButton *tmpBitButton = new BitButton(this);
				wordlayout->addWidget(tmpBitButton);
				bitButtonGroup->addButton(tmpBitButton, bitCounter);
				bitCounter--;
			}

			// label word
			QLabel *label = new QLabel(this);
			label->setText(QString::number(bitCounter+1));
			label->setFont(fnt);
			wordlayout->addWidget(label);

		}
	}
}

void KCalcBitset::setValue(unsigned long long value)
{
	mValue = value;
	for(int i=0; i<64; i++) {
		BitButton *bb = qobject_cast<BitButton*>(bitButtonGroup->button(i));
		if (bb) bb->setOn(value & 1);
		value >>= 1;
	}
}

unsigned long long KCalcBitset::getValue()
{
	return mValue;
}

void KCalcBitset::slotToggleBit( int bit )
{
  unsigned long long nv = getValue() ^ (1LL << bit);
  setValue(nv);
  emit valueChanged(mValue);
}

