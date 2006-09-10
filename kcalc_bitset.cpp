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

#include <QButtonGroup>
#include <QLayout>
#include <QEvent>

#include "kcalc_bitset.h"
#include "kcalc_bitset.moc"

static QPushButton *createBitButton(QWidget *parent = 0)
{
  QPushButton *tmp = new QPushButton("0", parent);
  tmp->setFlat(true);
  tmp->setFixedSize(QSize(tmp->fontMetrics().width(" 0 "),
			  tmp->fontMetrics().height()));

  return tmp;
}

KCalcBitset::KCalcBitset(QWidget *parent)
   : QFrame(parent), mValue(0)
{
	setFrameStyle(QFrame::Panel | QFrame::Sunken);
	bitButtonGroup = new QButtonGroup(this);
	connect(bitButtonGroup, SIGNAL(buttonClicked (int)),
		SLOT(slotToggleBit(int)));

	QGridLayout *layout = new QGridLayout(this);
	layout->setMargin(0);
	//layout->setSpacing(0);

	int bitCounter = 63;
	for (int countRows=0; countRows<3; countRows+=2)
	{
		for (int countCols=0; countCols<8; countCols++)
		{
			// Labels on column 0, 4, 7
			if(countCols == 0 || countCols == 4 || countCols == 7)
			{
				QLabel *label = new QLabel(this);
				int bn = bitCounter;
				if(countCols == 7)
				{
					label->setAlignment(Qt::AlignRight);
					bn-=3;
				}
				label->setText(QString::number(bn));
				{
				  QPalette palette;
				  palette.setColor(label->foregroundRole(), QColor(0,0,255));
				  label->setPalette(palette);
				}
				layout->addWidget(label, countRows+1,
						  countCols);
			}

			QWidget *group4Bits = new QWidget(this);
			QHBoxLayout *groupLayout = new QHBoxLayout(this);
			for( int b=0; b<4; b++ )
			{

				QPushButton *tmpBitButton = createBitButton(group4Bits);
				groupLayout->addWidget(tmpBitButton);
				bitButtonGroup->addButton(tmpBitButton, bitCounter);
				bitCounter--;
			}
			groupLayout->setSpacing(0);
			group4Bits->setLayout(groupLayout);
			layout->addWidget(group4Bits, countRows, countCols);
#if 0
			if(countCols != 7 )
			{
				// Add a "spacer"
				QLabel *sp = new QLabel(this);
				sp->setMinimumWidth(5);
			}
#endif
		}
	}

}


void KCalcBitset::setValue(unsigned long long value)
{
	mValue = value;
	for(int i=0; i<64; i++) {
		bitButtonGroup->button(i)->setText(value&1?"1":"0");
		value>>=1;
	}
}

unsigned long long KCalcBitset::getValue()
{
	return mValue;
}

void KCalcBitset::slotToggleBit( int bit )
{
  unsigned long long nv = getValue()^(1LL<<bit);
  setValue(nv);
  emit valueChanged(mValue);
}

