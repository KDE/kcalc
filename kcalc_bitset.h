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
#ifndef _KCALC_BITSET_H_
#define _KCALC_BITSET_H_

#include <QFrame>
#include <QAbstractButton>
#include <QButtonGroup>

class KCalcBitset : public QFrame
{
	Q_OBJECT
public:
	KCalcBitset( QWidget *parent=0 );
	unsigned long long getValue();

public slots:
	void setValue( unsigned long long value );
	void slotToggleBit( int bit );

signals:
	void valueChanged( unsigned long long value );

private:
	QButtonGroup *bitButtonGroup;
	unsigned long long mValue;
};

class BitButton : public QAbstractButton
{
	Q_OBJECT
public:
	explicit BitButton( QWidget *parent=0 );
	bool isOn() const;
	void setOn(bool value);

protected:
	void paintEvent(QPaintEvent *event);

private:
	bool on;
};

#endif
