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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>

#include <qclipboard.h>

#include <kglobal.h>
#include <klocale.h>
#include <knotifyclient.h>
#include "kcalcdisplay.h"
#include "kcalcdisplay.moc"

// We can't use QString::toDouble because it uses DOUBLE and use LONG DOUBLE!!
static CALCAMNT toDouble(const QString &s, bool &ok)
{
	char *ptr = 0;
	errno = 0;
	CALCAMNT result = (CALCAMNT) STRTOD(s.latin1(),&ptr);

	// find first non-space character for check below
	while (ptr != 0 && *ptr != '\0' && isspace(*ptr)) {
	// *ptr == 0 is also caught by isspace(*ptr), but you never know
		ptr++;
	}
	// input contains more than a number or another error
	ok = (errno == 0) && (ptr != 0) && (*ptr == 0);
	return result;
}

KCalcDisplay::KCalcDisplay(QWidget *parent, const char *name)
  :QLabel(parent,name), _button(0), _lit(false),
   _num_base(NB_DECIMAL), _input_limit(0),
   _display_size(DEC_SIZE), _precision(9),
   _fixed_precision(-1), selection_timer(new QTimer)
{
	setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
	setAlignment(AlignRight | AlignVCenter);
	setFocus();
	setFocusPolicy(QWidget::StrongFocus);

	connect(this, SIGNAL(clicked()), this, SLOT(slotDisplaySelected()));

	connect(selection_timer, SIGNAL(timeout()),
		this, SLOT(slotSelectionTimedOut()));

	Reset();
}

KCalcDisplay::~KCalcDisplay()
{
	delete selection_timer;
}

void KCalcDisplay::slotCut(void)
{
	if(_error  &&  _beep)
	{
		KNotifyClient::beep();
		return;
	}

	slotCopy();
	Reset();
}

void KCalcDisplay::slotCopy(void)
{
	if(_error  &&  _beep)
	{
		KNotifyClient::beep();
		return;
	}

	QString txt = text();
	if (_num_base == NB_HEX)
		txt.prepend( "0x" );
	(QApplication::clipboard())->setText(txt, QClipboard::Clipboard);
	(QApplication::clipboard())->setText(txt, QClipboard::Selection);
}

void KCalcDisplay::slotPaste(bool bClipboard)
{
	QString tmp_str = (QApplication::clipboard())->text(bClipboard ? QClipboard::Clipboard : QClipboard::Selection);

	if (_error  ||  tmp_str.isNull())
	{
		if (_beep)  KNotifyClient::beep();
		return;
	}

	bool was_ok;
	CALCAMNT tmp_result;
	
	if (_num_base == NB_HEX  &&  ! tmp_str.startsWith("0x", false))
	  tmp_str.prepend( "0x" );
	
	if ( (_num_base == NB_OCTAL || _num_base == NB_BINARY) &&
	     !  tmp_str.startsWith("0x",false))
	  tmp_result = CALCAMNT(tmp_str.toLongLong(& was_ok, _num_base));
	else
	  tmp_result = toDouble(tmp_str, was_ok);
	
	if (!was_ok)
	{
		tmp_result = (CALCAMNT) (0);
		if(_beep) KNotifyClient::beep();		
	}

	setAmount(tmp_result);
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

CALCAMNT KCalcDisplay::getAmount(void) const
{
	return _display_amount;
}

bool KCalcDisplay::setAmount(CALCAMNT new_amount)
{
	if(_error)
		return false;

	QString display_str;

	_str_int = "0";
	_str_int_exp = (char *)0;
	_period = false;
	_neg_sign = false;
	_eestate = false;

	_display_amount = new_amount;

	if (_num_base != NB_DECIMAL)
	{
		CALCAMNT	tmp_round;
		KCALC_LONG	rounded_num = 0;

		MODF(_display_amount, &tmp_round);

		if (tmp_round < KCALC_LONG_MIN || tmp_round > KCALC_ULONG_MAX)
			_error = true;

		//
		// We may be in that never-never land where boh numbers
		// turn from positive to negative - if so then we do
		// just that, allowing boh negative numbers to be entered
		// as read (from dumps and the like!)
		//
		else if (tmp_round > KCALC_LONG_MAX)
		{
			_display_amount = KCALC_LONG_MIN + (tmp_round - KCALC_LONG_MAX - 1);
			rounded_num = (KCALC_LONG)_display_amount;
		}
		else
		{
			_display_amount = tmp_round;
			rounded_num = (KCALC_LONG)_display_amount;
		}

		display_str = QString::number(rounded_num, _num_base).upper();
		if (display_str.length() > DSP_SIZE) _error = true;
	}
	else // _num_base == NB_DECIMAL
	{
		// We need to use QCString here since it uses the systems native ::sprintf()
		// implementation which is more flexible than QString's.

		if (_fixed_precision != -1 && _display_amount <= 1.0e+16)
			display_str = QCString().sprintf(PRINT_FLOAT, _fixed_precision, _display_amount);
		else if (_display_amount > 1.0e+16)
			display_str = QCString().sprintf(PRINT_LONG_BIG, _precision + 1, _display_amount);
		else
			display_str = QCString().sprintf(PRINT_LONG_BIG, _precision, _display_amount);

		if (display_str.length() > DSP_SIZE) _error = true;
	}

	if(_error)
	{
		if(_beep) KNotifyClient::beep();
		setText(i18n("Error"));
		return false;
	}
	else
	{
		setText(display_str);
		return true;
	}
	
}

void KCalcDisplay::setText(QString const &string)
{
	QString localizedString = string;
	
	// If we aren't in decimal mode, we don't need to modify the string
	if (_num_base == NB_DECIMAL)
	{
		// Obtain decimal symbol and thousands separator
		QString decimalSymbol = KGlobal::locale()->decimalSymbol();
		QString thousandsSeparator = KGlobal::locale()->thousandsSeparator();
	
		// Replace dot with locale decimal separator
		localizedString.replace(QChar('.'),decimalSymbol);
	
		// Insert the thousand separators
		int i = localizedString.find(decimalSymbol) != -1? localizedString.find(decimalSymbol)-1: localizedString.length()-1;
		for(int j=1;i>0;i--,j++)
			if (j%3 == 0)
				localizedString.insert(i,thousandsSeparator);
	}

	QLabel::setText(localizedString);
	emit changedText(localizedString);
}

QString KCalcDisplay::text() const
{
	if (_num_base != NB_DECIMAL)
		return QLabel::text();
	
	QString unlocalizedString = QLabel::text();
	
	// Obtain decimal symbol and thousands separator
	QString decimalSymbol = KGlobal::locale()->decimalSymbol();
	QString thousandsSeparator = KGlobal::locale()->thousandsSeparator();

	// TODO: This may cause problems if the user
	// sets decimalSymbol == thousandsSeparator
	// Since text() is only used for prepending
	// other text in NB_HEX mode and measuring
	// the display lenght, this is not a big
	// problem. Anyway, we should find a 
	// better way of doing this.

	// Delete thousands separators	
	unlocalizedString.remove(thousandsSeparator);
	
	// Replace decimalSymbol with '.'
	unlocalizedString.replace(decimalSymbol,QChar('.'));
	
	return unlocalizedString;
}

/* change representation of display to new base (i.e. binary, decimal,
   octal, hexadecimal). The amount being displayed is changed to this
   base, but for now this amount can not be modified anymore (like
   being set with "setAmount"). Return value is the new base. */
int KCalcDisplay::setBase(NumBase new_base)
{
	CALCAMNT tmp_val = getAmount();

	switch(new_base)
	{
	case NB_HEX:
		_num_base	= NB_HEX;
		_display_size	= HEX_SIZE;
		_period 	= false;
		_input_limit		= sizeof(KCALC_LONG)*2;
		break;
	case NB_DECIMAL:
		_num_base	= NB_DECIMAL;
		_display_size	= DEC_SIZE;
		_input_limit		= 0;
		break;
	case NB_OCTAL:
		_num_base	= NB_OCTAL;
		_display_size	= OCT_SIZE;
		_period 	= false;
		_input_limit		= 11;
		break;
	case NB_BINARY:
		_num_base	= NB_BINARY;
		_display_size	= BIN_SIZE;
		_period 	= false;
		_input_limit		= 32;
		break;
	default: // we shouldn't ever end up here
		_num_base	= NB_DECIMAL;
		_display_size	= DEC_SIZE;
		_input_limit		= 0;
	}

	setAmount(tmp_val);
	
	return _num_base;
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
	bool tmp_flag;
	case NB_BINARY:
		Q_ASSERT(_period == false  && _eestate == false);
		Q_ASSERT(tmp_string.length() < DSP_SIZE);
		setText(tmp_string);
		_display_amount = STRTOUL(_str_int.latin1(), 0, 2);
		if (_neg_sign)
			_display_amount = -_display_amount;
		//str_size = cvb(_str_int, boh_work, DSP_SIZE);
		break;
	  
	case NB_OCTAL:
		Q_ASSERT(_period == false  && _eestate == false);
		Q_ASSERT(tmp_string.length() < DSP_SIZE);
		setText(tmp_string);
		_display_amount = STRTOUL(_str_int.latin1(), 0, 8);
		if (_neg_sign)
			_display_amount = -_display_amount;
		break;
		
	case NB_HEX:
		Q_ASSERT(_period == false  && _eestate == false);
		Q_ASSERT(tmp_string.length() < DSP_SIZE);
		setText(tmp_string);
		_display_amount = STRTOUL(_str_int.latin1(), 0, 16);
		if (_neg_sign)
			_display_amount = -_display_amount;
		break;
	  
	case NB_DECIMAL:
		if(_eestate == false)
		{
			Q_ASSERT(tmp_string.length() < DSP_SIZE);
			setText(tmp_string);
			_display_amount = toDouble(tmp_string, tmp_flag);
			Q_ASSERT(tmp_flag == true);
		}
		else
		{
			if(_str_int_exp.isNull())
			{
				// add 'e0' to display but not to conversion
				Q_ASSERT(tmp_string.length()+2 < DSP_SIZE);
				_display_amount = toDouble(tmp_string, tmp_flag);
				Q_ASSERT(tmp_flag == true);
				setText(tmp_string + "e0");
			}
			else
			{
				tmp_string +=  'e' + _str_int_exp;
				Q_ASSERT(tmp_string.length() < DSP_SIZE);
				setText(tmp_string);
				_display_amount = toDouble(tmp_string, tmp_flag);
				Q_ASSERT(tmp_flag == true);
			}
		}
		break;
	  
	default:
	  return false;
	}

	if(_error)
	{
		if(_beep) KNotifyClient::beep();
		setText(i18n("Error"));
		return false;
	}
	else
		return true;
}

void KCalcDisplay::newCharacter(char const new_char)
{
	// error or DISPLAY is already full
	if (_error || text().length() >= DSP_SIZE)
	{
		if(_beep) KNotifyClient::beep();
		return;
	}

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

bool KCalcDisplay::clearLastInput(void)
{
	if (_error)
	{
		return updateDisplay();
	}
	else
	{
		_period	= false;
		_eestate = false;
		_neg_sign = false;
		_str_int = "0";
		_str_int_exp = (char *)0;
		return updateDisplay();
	}
}

void KCalcDisplay::setError(bool error)
{
	_error = error;
	updateDisplay();
}

bool KCalcDisplay::getError(void) const
{
	return _error;
}

void KCalcDisplay::Reset(void)
{
	// This updates display through setError.
	// Should one explicitly call UpdateDisplay to be nicer?

	_display_amount = 0.0;
	_str_int = "0";
	_str_int_exp = (char *)0;

	_eestate = false;
	_period = false;
	_neg_sign = false;

	setError(false);
}
