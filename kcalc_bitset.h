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

#ifndef KCALC_BITSET_H_
#define KCALC_BITSET_H_

#include <QFrame>

class QButtonGroup;
class QAbstractButton;
class KCalcBitset : public QFrame {
	Q_OBJECT
	
public:
	explicit KCalcBitset(QWidget *parent = nullptr);
    Q_REQUIRED_RESULT quint64 getValue() const;

public Q_SLOTS:
	void setValue(quint64 value);
	void slotToggleBit(QAbstractButton *button);

Q_SIGNALS:
	void valueChanged(quint64 value);

private:
	QButtonGroup * const bit_button_group_;
	quint64       value_;
};

#endif
