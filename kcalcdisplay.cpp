/*
    $Id$

    KCalc, a scientific calculator for the X window system using the
    Qt widget libraries, available at no cost at http://www.troll.no
    
    Copyright (C) 1996 Bernd Johannes Wuebben   
                       wuebben@math.cornell.edu

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

#include <errno.h>
#include <stdlib.h>
#include <ctype.h>

#include <qclipboard.h>
#include <qpainter.h>
#include <qregexp.h>

#include <kglobal.h>
#include <klocale.h>
#include <knotifyclient.h>
#include "kcalc_settings.h"
#include "kcalcdisplay.h"
#include "kcalcdisplay.moc"


KCalcDisplay::KCalcDisplay(QWidget *parent, const char *name)
  :QLabel(parent,name), _beep(false), _groupdigits(false), _button(0), _lit(false),
   _num_base(NB_DECIMAL), _precision(9),
   _fixed_precision(-1), _display_amount(0),
   selection_timer(new QTimer)
{
	setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
	setAlignment(AlignRight | AlignVCenter);
	setFocus();
	setFocusPolicy(QWidget::StrongFocus);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed, false);

	connect(this, SIGNAL(clicked()), this, SLOT(slotDisplaySelected()));

	connect(selection_timer, SIGNAL(timeout()),
		this, SLOT(slotSelectionTimedOut()));

	sendEvent(EventReset);
}

KCalcDisplay::~KCalcDisplay()
{
	delete selection_timer;
}

bool KCalcDisplay::sendEvent(Event const event)
{
	switch(event)
	{
	case EventReset:
		_display_amount = 0;
		_str_int = "0";
		_str_int_exp = QString::null;

		_eestate = false;
		_period = false;
		_neg_sign = false;

		updateDisplay();

		return true;
	case EventClear:
		return sendEvent(EventReset);
	case EventChangeSign:
		return changeSign();
	case EventError:
		updateDisplay();

		return true;
	default:
		return false;
	}
}


void KCalcDisplay::slotCut(void)
{
	slotCopy();
	sendEvent(EventReset);
}

void KCalcDisplay::slotCopy(void)
{
	QString txt = QLabel::text();
	if (_num_base == NB_HEX)
		txt.prepend( "0x" );
	(QApplication::clipboard())->setText(txt, QClipboard::Clipboard);
	(QApplication::clipboard())->setText(txt, QClipboard::Selection);
}

void KCalcDisplay::slotPaste(bool bClipboard)
{
	QString tmp_str = (QApplication::clipboard())->text(bClipboard ? QClipboard::Clipboard : QClipboard::Selection);

	if (tmp_str.isNull())
	{
		if (_beep)  KNotifyClient::beep();
		return;
	}

	NumBase tmp_num_base = _num_base;

	tmp_str = tmp_str.stripWhiteSpace();

	if (tmp_str.startsWith("0x", false))
	  tmp_num_base = NB_HEX;

	if (tmp_num_base != NB_DECIMAL)
	{
		bool was_ok;
		unsigned long long int tmp_result = tmp_str.toULongLong(& was_ok, tmp_num_base);

		if (!was_ok)
		{
			setAmount(KNumber::NotDefined);
			if(_beep) KNotifyClient::beep();
			return ;
		}
	  
		setAmount(KNumber(tmp_result));
	} 
	else // _num_base == NB_DECIMAL && ! tmp_str.startsWith("0x", false)
	{
		setAmount(KNumber(tmp_str));
		if (_beep  &&  _display_amount == KNumber::NotDefined)
			KNotifyClient::beep();
	}
}

void KCalcDisplay::slotDisplaySelected(void)
{
	if(_button == LeftButton) {
		if(_lit) {
			slotCopy();
			selection_timer->start(100);
		} else {
			selection_timer->stop();
		}

		invertColors();
	} else {
		slotPaste(false); // Selection
	}
}

void KCalcDisplay::slotSelectionTimedOut(void)
{
	_lit = false;
	invertColors();
	selection_timer->stop();
}

void KCalcDisplay::invertColors()
{
	QColor tmp_col(paletteBackgroundColor());
	setPaletteBackgroundColor(paletteForegroundColor());
	setPaletteForegroundColor(tmp_col);
}



void KCalcDisplay::mousePressEvent(QMouseEvent *e)
{
	if(e->button() == LeftButton) {
		_lit = !_lit;
		_button = LeftButton;
	} else {
		_button = MidButton;
	}
	
	emit clicked();
}

void KCalcDisplay::setPrecision(int precision)
{
	_precision = precision;
}

void KCalcDisplay::setFixedPrecision(int precision)
{
	if (_fixed_precision > _precision)
		_fixed_precision = -1;
	else
		_fixed_precision = precision;
}

void KCalcDisplay::setBeep(bool flag)
{
	_beep = flag;
}

void KCalcDisplay::setGroupDigits(bool flag)
{
	_groupdigits = flag;
}

KNumber const & KCalcDisplay::getAmount(void) const
{
	return _display_amount;
}


bool KCalcDisplay::setAmount(KNumber const & new_amount)
{
	QString display_str;

	_str_int = "0";
	_str_int_exp = QString::null;
	_period = false;
	_neg_sign = false;
	_eestate = false;

	if (_num_base != NB_DECIMAL  && new_amount.type() != KNumber::SpecialType)
	{
		_display_amount = new_amount.integerPart();
		unsigned long long int tmp_workaround = static_cast<unsigned long long int>(_display_amount);

		display_str = QString::number(tmp_workaround, _num_base).upper();
	}
	else // _num_base == NB_DECIMAL || new_amount.type() ==
	     // KNumber::SpecialType
	{
		_display_amount = new_amount;
	
		display_str = _display_amount.toQString(KCalcSettings::precision(), _fixed_precision);
#if 0
		else if (_display_amount > 1.0e+16)
			display_str = QCString().sprintf(PRINT_LONG_BIG, _precision + 1, _display_amount);
		else
			display_str = QCString().sprintf(PRINT_LONG_BIG, _precision, _display_amount);
#endif
	}

	setText(display_str);
	return true;
	
}

void KCalcDisplay::setText(QString const &string)
{
	QString localizedString = string;

	// If we aren't in decimal mode, we don't need to modify the string
	if (_num_base == NB_DECIMAL  &&  _groupdigits)
	  // when input ends with "." (because uncomplete), the
	  // formatNumber-method does not work; fix by hand by
	  // truncating, formatting and appending again
	  if (string.endsWith(".")) {
	    localizedString.truncate(localizedString.length() - 1);
	    localizedString = KGlobal::locale()->formatNumber(localizedString, false, 0); // Note: rounding happened already above!
	    localizedString.append(KGlobal::locale()->decimalSymbol());
	  } else
	    localizedString = KGlobal::locale()->formatNumber(string, false, 0); // Note: rounding happened already above!

	QLabel::setText(localizedString);
	emit changedText(localizedString);
}

QString KCalcDisplay::text() const
{
	if (_num_base != NB_DECIMAL)
		return QLabel::text();
	QString display_str = _display_amount.toQString(KCalcSettings::precision());

	return display_str;
	//	return QCString().sprintf(PRINT_LONG_BIG, 40, _display_amount);
}

/* change representation of display to new base (i.e. binary, decimal,
   octal, hexadecimal). The amount being displayed is changed to this
   base, but for now this amount can not be modified anymore (like
   being set with "setAmount"). Return value is the new base. */
int KCalcDisplay::setBase(NumBase new_base)
{
	CALCAMNT tmp_val = static_cast<unsigned long long int>(getAmount());

	switch(new_base)
	{
	case NB_HEX:
		_num_base	= NB_HEX;
		_period 	= false;
		break;
	case NB_DECIMAL:
		_num_base	= NB_DECIMAL;
		break;
	case NB_OCTAL:
		_num_base	= NB_OCTAL;
		_period 	= false;
		break;
	case NB_BINARY:
		_num_base	= NB_BINARY;
		_period 	= false;
		break;
	default: // we shouldn't ever end up here
		_num_base	= NB_DECIMAL;
	}

	setAmount(static_cast<unsigned long long int>(tmp_val));
	
	return _num_base;
}

void KCalcDisplay::setStatusText(uint i, const QString& text)
{
	if (i < NUM_STATUS_TEXT)
		_str_status[i] = text;
	update();
}

bool KCalcDisplay::updateDisplay(void)
{
	// Put sign in front.
	QString tmp_string;
	if(_neg_sign == true)
		tmp_string = "-" + _str_int;
	else
		tmp_string = _str_int;

	switch(_num_base)
	{
	case NB_BINARY:
		Q_ASSERT(_period == false  && _eestate == false);
		setText(tmp_string);
		_display_amount = static_cast<unsigned long long int>(STRTOUL(_str_int.latin1(), 0, 2));
		if (_neg_sign)
			_display_amount = -_display_amount;
		//str_size = cvb(_str_int, boh_work, DSP_SIZE);
		break;
	  
	case NB_OCTAL:
		Q_ASSERT(_period == false  && _eestate == false);
		setText(tmp_string);
		_display_amount = static_cast<unsigned long long int>(STRTOUL(_str_int.latin1(), 0, 8));
		if (_neg_sign)
			_display_amount = -_display_amount;
		break;
		
	case NB_HEX:
		Q_ASSERT(_period == false  && _eestate == false);
		setText(tmp_string);
		_display_amount = static_cast<unsigned long long int>(STRTOUL(_str_int.latin1(), 0, 16));
		if (_neg_sign)
			_display_amount = -_display_amount;
		break;
	  
	case NB_DECIMAL:
		if(_eestate == false)
		{
			setText(tmp_string);
			_display_amount = tmp_string;
		}
		else
		{
			if(_str_int_exp.isNull())
			{
				// add 'e0' to display but not to conversion
				_display_amount = tmp_string;
				setText(tmp_string + "e0");
			}
			else
			{
				tmp_string +=  'e' + _str_int_exp;
				setText(tmp_string);
				_display_amount = tmp_string;
			}
		}
		break;
	  
	default:
	  return false;
	}

	return true;
}

void KCalcDisplay::newCharacter(char const new_char)
{
	// test if character is valid
	switch(new_char)
	{
	case 'e':
		// EE can be set only once and in decimal mode
		if (_num_base != NB_DECIMAL  ||
		    _eestate == true)
		{
			if(_beep) KNotifyClient::beep();
			return;
		}
		_eestate = true;
		break;

	case '.':
		// Period can be set only once and only in decimal
		// mode, also not in EE-mode
		if (_num_base != NB_DECIMAL  ||
		    _period == true  ||
		    _eestate == true)
		{
			if(_beep) KNotifyClient::beep();
			return;
		}
		_period = true;
		break;

	case 'F':
	case 'E':
	case 'D':
	case 'C':
	case 'B':
	case 'A':
		if (_num_base == NB_DECIMAL)
		{
			if(_beep) KNotifyClient::beep();
			return;
		}
		// no break
	case '9':
	case '8':
		if (_num_base == NB_OCTAL)
		{
			if(_beep) KNotifyClient::beep();
			return;
		}
		// no break
	case '7':
	case '6':
	case '5':
	case '4':
	case '3':
	case '2':
		if (_num_base == NB_BINARY)
		{
			if(_beep) KNotifyClient::beep();
			return;
		}
		// no break
	case '1':
	case '0':
		break;

	default:
		if(_beep) KNotifyClient::beep();
		return;
	}

	// change exponent or mantissa
	if (_eestate)
	{
	  // ignore ',' before 'e'. turn e.g. '123.e' into '123e'
	  if (new_char == 'e'  && _str_int.endsWith( "." ))
	    {
			_str_int.truncate(_str_int.length() - 1);
			_period = false;
	    }

	  // 'e' only starts ee_mode, leaves strings unchanged
	  if (new_char != 'e'  &&
	      // do not add '0' if at start of exp
	      !(_str_int_exp.isNull() && new_char == '0'))
			_str_int_exp.append(new_char);
	}
	else
	{
		// handle first character
		if (_str_int == "0")
		{
			switch(new_char)
			{
			case '.':
				// display "0." not just "."
				_str_int.append(new_char);
				break;
			case 'e':
				// display "0e" not just "e"
				// "0e" does not make sense either, but...
				_str_int.append(new_char);
				break;
			default:
				// no leading '0's
				_str_int[0] = new_char;
			}
		}
		else
			_str_int.append(new_char);
	}

	updateDisplay();
}

void KCalcDisplay::deleteLastDigit(void)
{
	// Only partially implemented !!
	if (_eestate)
	{
		if(_str_int_exp.isNull())
		{
			_eestate = false;
		}
		else
		{
			int length = _str_int_exp.length();
			if(length > 1)
			{
				_str_int_exp.truncate(length-1);
			}
			else
			{
				_str_int_exp = (char *)0;
			}
		}
	}
	else
	{
		int length = _str_int.length();
		if(length > 1)
		{
			if (_str_int[length-1] == '.')
				_period = false;
			_str_int.truncate(length-1);
		}
		else
		{
			Q_ASSERT(_period == false);
			_str_int[0] = '0';
		}
	}

	updateDisplay();
}

// change Sign of display. Problem: Only possible here, when in input
// mode. Otherwise return 'false' so that the kcalc_core can handle
// things.
bool KCalcDisplay::changeSign(void)
{
	//stupid way, to see if in input_mode or display_mode
	if (_str_int == "0") return false;

	if(_eestate)
	{
		if(!_str_int_exp.isNull())
		{
			if (_str_int_exp[0] != '-')
				_str_int_exp.prepend('-');
			else
				_str_int_exp.remove('-');
		}
	}
	else
	{
		_neg_sign = ! _neg_sign;
	}
	
	updateDisplay();

	return true;
}

void KCalcDisplay::drawContents(QPainter *p)
{
	QLabel::drawContents(p);

	// draw the status texts using half of the normal
	// font size but not smaller than 7pt
	QFont f(font());
	f.setPointSize(QMAX((f.pointSize() / 2), 7));
	p->setFont(f);
	QFontMetrics fm(f);
	uint w = fm.width("_____");
	uint h = fm.height();

	for (uint i = 0; i < NUM_STATUS_TEXT; i++)
	{
		p->drawText(5 + i * w, h, _str_status[i]);
	}
}

// Return the QLabel's normal size hint vertically expanded
// by half the font height to make room for the status texts
QSize KCalcDisplay::sizeHint() const
{
	QFont f(font());
	f.setPointSize(QMAX((f.pointSize() / 2), 7));
	QFontMetrics fm(f);
	return QLabel::sizeHint() + QSize(0, fm.height());
}

