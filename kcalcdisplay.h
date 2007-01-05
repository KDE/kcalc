/*

 KCalc 

 Copyright (C) Bernd Johannes Wuebben
               wuebben@math.cornell.edu
	       wuebben@kde.org

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */


#ifndef _D_KCALCDISPLAY_H_
#define _D_KCALCDISPLAY_H_

#include <stdlib.h>
#include <qlabel.h>
#include <qtimer.h>
#include "knumber.h"
#include "kcalctype.h"

#if defined HAVE_LONG_DOUBLE && defined HAVE_L_FUNCS
	#define PRINT_FLOAT     "%.*Lf"
	#define PRINT_LONG_BIG  "%.*Lg"
	#define PRINT_LONG      "%Lg"
#else
	#define PRINT_FLOAT     "%.*f"
	#define PRINT_LONG_BIG  "%.*g"
	#define PRINT_LONG      "%g"
#endif

#ifdef HAVE_LONG_LONG
	#define PRINT_OCTAL  "%llo"
	#define PRINT_HEX    "%llX"
#else
	#define PRINT_OCTAL  "%lo"
	#define PRINT_HEX    "%lX"
#endif

#define		NUM_STATUS_TEXT 4

/*
  This class provides a pocket calculator display.  The display has
  implicitely two major modes: One is for editing and one is purely
  for displaying.

  When one uses "setAmount", the given amount is displayed, and the
  amount which was possibly typed in before is lost. At the same time
  this new value can not be modified.
  
  On the other hand, "addNewChar" adds a new digit to the amount that
  is being typed in. If "setAmount" was used before, the display is
  cleared and a new input starts.

  TODO: Check overflows, number of digits and such...
*/

enum NumBase {
	NB_BINARY = 2,
	NB_OCTAL = 8,
	NB_DECIMAL = 10,
	NB_HEX = 16
};


class KCalcDisplay : public QLabel
{
Q_OBJECT

public:
	KCalcDisplay(QWidget *parent=0, const char *name=0);
	~KCalcDisplay();

protected:
	void  mousePressEvent ( QMouseEvent *);
	virtual void drawContents(QPainter *p);

public:
	enum Event {
	  EventReset, // resets display
	  EventClear, // if no _error reset display
	  EventError,
	  EventChangeSign
	};
	bool sendEvent(Event const event);
	void deleteLastDigit(void);
	KNumber const & getAmount(void) const;
	void newCharacter(char const new_char);
	bool setAmount(KNumber const & new_amount);
	int setBase(NumBase new_base);
	void setBeep(bool flag);
	void setGroupDigits(bool flag);
	void setFixedPrecision(int precision);
	void setPrecision(int precision);
	void setText(QString const &string);
	QString text() const;
	bool updateDisplay(void);
	void setStatusText(uint i, const QString& text);
	virtual QSize sizeHint() const;
private:
	bool _beep;
	bool _groupdigits;
	int  _button;
	bool _lit;
	NumBase _num_base;

	int _precision;
	int _fixed_precision; // "-1" = no fixed_precision

	KNumber _display_amount;
private:
	bool changeSign(void);
	void invertColors(void);

	// only used for input of new numbers
	bool _eestate;
	bool _period;
	bool _neg_sign;
	QString _str_int;
	QString _str_int_exp;
	QString _str_status[NUM_STATUS_TEXT];

	QTimer* selection_timer;

signals:
	void clicked(void);
	void changedText(QString const &);

public slots:
	void slotCut(void);
	void slotCopy(void);
	void slotPaste(bool bClipboard=true);

private slots:
	void slotSelectionTimedOut(void);
	void slotDisplaySelected(void);
};

#endif // _KCALCDISPLAY_H_
