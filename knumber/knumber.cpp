// -*- c-basic-offset: 2 -*-
/* This file is part of the KDE libraries
   Copyright (c) 2005 Klaus Niederkrueger <kniederk@math.uni-koeln.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <math.h>

#include <config.h>

#include <qregexp.h>
#include <qstring.h>

#include "knumber.h"

KNumber const KNumber::Zero(0);
KNumber const KNumber::One(1);
KNumber const KNumber::MinusOne(-1);
KNumber const KNumber::Pi("3.141592653589793238462643383279502884197169"
			  "39937510582097494459230781640628620899862803"
			  "4825342117068");
KNumber const KNumber::Euler("2.718281828459045235360287471352662497757"
			     "24709369995957496696762772407663035354759"
			     "4571382178525166427");
KNumber const KNumber::NotDefined("nan");

bool KNumber::_float_output = false;
bool KNumber::_fraction_input = false;
bool KNumber::_splitoffinteger_output = false;

KNumber::KNumber(signed int num)
{
  _num = new _knuminteger(num);
}

KNumber::KNumber(unsigned int num)
{
  _num = new _knuminteger(num);
}

KNumber::KNumber(signed long int num)
{
  _num = new _knuminteger(num);
}

KNumber::KNumber(unsigned long int num)
{
  _num = new _knuminteger(num);
}

KNumber::KNumber(unsigned long long int num)
{
  _num = new _knuminteger(num);
}

KNumber::KNumber(double num)
{
  if ( isinf(num) ) _num = new _knumerror( _knumber::Infinity );
  else if ( isnan(num) ) _num = new _knumerror( _knumber::UndefinedNumber );
  else _num = new _knumfloat(num);      

}

KNumber::KNumber(KNumber const & num)
{
  switch(num.type()) {
  case SpecialType:
    _num = new _knumerror(*(num._num));
    return;
  case IntegerType:
    _num = new _knuminteger(*(num._num));
    return;
  case  FractionType:
    _num = new _knumfraction(*(num._num));
    return;
  case FloatType:
    _num = new _knumfloat(*(num._num));
    return;
  };
}


KNumber::KNumber(QString const & num)
{
  if (QRegExp("^(inf|-inf|nan)$").exactMatch(num))
    _num = new _knumerror(num);
  else if (QRegExp("^[+-]?\\d+$").exactMatch(num))
    _num = new _knuminteger(num);
  else if (QRegExp("^[+-]?\\d+/\\d+$").exactMatch(num)) {
    _num = new _knumfraction(num);
    simplifyRational();
  }
  else if (QRegExp("^[+-]?\\d+(\\.\\d*)?(e[+-]?\\d+)?$").exactMatch(num))
    if (_fraction_input == true) {
      _num = new _knumfraction(num);
      simplifyRational();
    } else
      _num = new _knumfloat(num);
  else
    _num = new _knumerror("nan");
}

KNumber::NumType KNumber::type(void) const
{
  if(dynamic_cast<_knumerror *>(_num))
    return SpecialType;
  if(dynamic_cast<_knuminteger *>(_num))
    return IntegerType;
  if(dynamic_cast<_knumfraction *>(_num))
    return FractionType;
  if(dynamic_cast<_knumfloat *>(_num))
    return FloatType;

  return SpecialType;
}

// This method converts a fraction to an integer, whenever possible,
// i.e. 5/1 --> 5
// This method should be called, whenever such a inproper fraction can occur,
// e.g. when adding 4/3 + 2/3....
void KNumber::simplifyRational(void)
{
  if (type() != FractionType)
    return;
  
  _knumfraction *tmp_num = dynamic_cast<_knumfraction *>(_num);

  if (tmp_num->isInteger()) {
    _knumber *tmp_num2 = tmp_num->intPart();
    delete tmp_num;
    _num = tmp_num2;
  }

}


KNumber const & KNumber::operator=(KNumber const & num)
{
  if (this == & num) 
    return *this;

  delete _num;

  switch(num.type()) {
  case SpecialType:
    _num = new _knumerror();
    break;
  case IntegerType:
    _num = new _knuminteger();
    break;
  case FractionType:
    _num = new _knumfraction();
    break;
  case FloatType:
    _num = new _knumfloat();
    break;
  };
  
  _num->copy(*(num._num));

  return *this;
}

KNumber & KNumber::operator +=(KNumber const &arg)
{
  KNumber tmp_num = *this + arg;

  delete _num;

  switch(tmp_num.type()) {
  case SpecialType:
    _num = new _knumerror();
    break;
  case IntegerType:
    _num = new _knuminteger();
    break;
  case FractionType:
    _num = new _knumfraction();
    break;
  case FloatType:
    _num = new _knumfloat();
    break;
  };
  
  _num->copy(*(tmp_num._num));

  return *this;
}

KNumber & KNumber::operator -=(KNumber const &arg)
{
  KNumber tmp_num = *this - arg;

  delete _num;

  switch(tmp_num.type()) {
  case SpecialType:
    _num = new _knumerror();
    break;
  case IntegerType:
    _num = new _knuminteger();
    break;
  case FractionType:
    _num = new _knumfraction();
    break;
  case FloatType:
    _num = new _knumfloat();
    break;
  };
  
  _num->copy(*(tmp_num._num));

  return *this;
}

// increase the digit at 'position' by one
static void _inc_by_one(QString &str, int position)
{
  for (int i = position; i >= 0; i--)
    {
      char last_char = str[i].latin1();
      switch(last_char)
	{
	case '0':
	  str[i] = '1';
	  break;
	case '1':
	  str[i] = '2';
	  break;
	case '2':
	  str[i] = '3';
	  break;
	case '3':
	  str[i] = '4';
	  break;
	case '4':
	  str[i] = '5';
	  break;
	case '5':
	  str[i] = '6';
	  break;
	case '6':
	  str[i] = '7';
	  break;
	case '7':
	  str[i] = '8';
	  break;
	case '8':
	  str[i] = '9';
	  break;
	case '9':
	  str[i] = '0';
	  if (i == 0) str.prepend('1');
	  continue;
	case '.':
	  continue;
	}
      break;
    }
}

// Cut off if more digits in fractional part than 'precision'
static void _round(QString &str, int precision)
{
  int decimalSymbolPos = str.find('.');

  if (decimalSymbolPos == -1)
    if (precision == 0)  return;
    else if (precision > 0) // add dot if missing (and needed)
      {
	str.append('.');
	decimalSymbolPos = str.length() - 1;
      }

  // fill up with more than enough zeroes (in case fractional part too short)
  str.append(QString().fill('0', precision));

  // Now decide whether to round up or down
  char last_char = str[decimalSymbolPos + precision + 1].latin1();
  switch (last_char)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
      // nothing to do, rounding down
      break;
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      // rounding up
      _inc_by_one(str, decimalSymbolPos + precision);
      break;
    default:
      break;
    }

  decimalSymbolPos = str.find('.');
  str.truncate(decimalSymbolPos + precision + 1);

  // if precision == 0 delete also '.'
  if (precision == 0) str = str.section('.', 0, 0);
}

static QString roundNumber(const QString &numStr, int precision)
{
  QString tmpString = numStr;
  if (precision < 0  ||
      ! QRegExp("^[+-]?\\d+(\\.\\d+)*(e[+-]?\\d+)?$").exactMatch(tmpString))
    return numStr;


  // Skip the sign (for now)
  bool neg = (tmpString[0] == '-');
  if (neg  ||  tmpString[0] == '+') tmpString.remove(0, 1);


  // Split off exponential part (including 'e'-symbol)
  QString mantString = tmpString.section('e', 0, 0,
					 QString::SectionCaseInsensitiveSeps);
  QString expString = tmpString.section('e', 1, 1,
					QString::SectionCaseInsensitiveSeps |
					QString::SectionIncludeLeadingSep);
  if (expString.length() == 1) expString = QString();


  _round(mantString, precision);

  if(neg) mantString.prepend('-');

  return mantString +  expString;
}


QString const KNumber::toQString(int width, int prec) const
{
  QString tmp_str;

  if (*this == Zero) // important to avoid infinite loops below
    return "0";
  switch (type()) {
  case IntegerType:
    if (width > 0) { //result needs to be cut-off
      bool tmp_bool = _fraction_input; // stupid work-around
      _fraction_input = false;
      tmp_str = (KNumber("1.0")*(*this)).toQString(width, -1);
      _fraction_input = tmp_bool;
    } else
      tmp_str = QString(_num->ascii());
    break;
  case FractionType:
    if (_float_output) {
      bool tmp_bool = _fraction_input; // stupid work-around
      _fraction_input = false;
      tmp_str = (KNumber("1.0")*(*this)).toQString(width, -1);
      _fraction_input = tmp_bool;
    } else { // _float_output == false
      if(_splitoffinteger_output) {
	// split off integer part
	KNumber int_part = this->integerPart();
	if (int_part == Zero)
	  tmp_str = QString(_num->ascii());
	else if (int_part < Zero)
	  tmp_str = int_part.toQString() + " " + (int_part - *this)._num->ascii();
	else
	  tmp_str = int_part.toQString() + " " + (*this - int_part)._num->ascii();
      } else
	tmp_str = QString(_num->ascii());

      if (width > 0  &&  tmp_str.length() > width) {
	//result needs to be cut-off
	bool tmp_bool = _fraction_input; // stupid work-around
	_fraction_input = false;
	tmp_str = (KNumber("1.0")*(*this)).toQString(width, -1);
	_fraction_input = tmp_bool;
      }
    }

    break;
  case FloatType:
    if (width > 0)
      tmp_str = QString(_num->ascii(width));
    else
      // rough estimate for  maximal decimal precision (10^3 = 2^10)
      tmp_str = QString(_num->ascii(3*mpf_get_default_prec()/10));
    break;
  default:
    return QString(_num->ascii());
  }
  
  if (prec >= 0)
    return roundNumber(tmp_str, prec);
  else
    return tmp_str;
}

void KNumber::setDefaultFloatOutput(bool flag)
{
  _float_output = flag;
}

void KNumber::setDefaultFractionalInput(bool flag)
{
  _fraction_input = flag;
}

void KNumber::setSplitoffIntegerForFractionOutput(bool flag)
{
  _splitoffinteger_output = flag;
}

void KNumber::setDefaultFloatPrecision(unsigned int prec)
{
  // Need to transform decimal digits into binary digits
  unsigned long int bin_prec = static_cast<unsigned long int>
    (double(prec) * M_LN10 / M_LN2 + 1);

  mpf_set_default_prec(bin_prec);
}

KNumber const KNumber::abs(void) const
{
  KNumber tmp_num;
  delete tmp_num._num;

  tmp_num._num = _num->abs();

  return tmp_num;
}

KNumber const KNumber::cbrt(void) const
{
  KNumber tmp_num;
  delete tmp_num._num;

  tmp_num._num = _num->cbrt();

  return tmp_num;
}

KNumber const KNumber::sqrt(void) const
{
  KNumber tmp_num;
  delete tmp_num._num;

  tmp_num._num = _num->sqrt();

  return tmp_num;
}

KNumber const KNumber::integerPart(void) const
{
  KNumber tmp_num;
  delete tmp_num._num;
  tmp_num._num = _num->intPart();

  return tmp_num;
}

KNumber const KNumber::power(KNumber const &exp) const
{
  if (*this == Zero) {
    if(exp == Zero)
      return KNumber("nan"); // 0^0 not defined
    else if (exp < Zero)
      return KNumber("inf");
    else
      return KNumber(0);
  }

  if (exp == Zero) {
    if (*this != Zero)
      return One;
    else
      return KNumber("nan");
  }
  else if (exp < Zero) {
    KNumber tmp_num;
    KNumber tmp_num2 = -exp;
    delete tmp_num._num;
    tmp_num._num = _num->power(*(tmp_num2._num));

    return One/tmp_num;
  }
  else {
    KNumber tmp_num;
    delete tmp_num._num;
    tmp_num._num = _num->power(*(exp._num));

    return tmp_num;
  }

}

KNumber const KNumber::operator-(void) const
{
  KNumber tmp_num;
  delete tmp_num._num;

  tmp_num._num = _num->change_sign();

  return tmp_num;
}

KNumber const KNumber::operator+(KNumber const & arg2) const
{
  KNumber tmp_num;
  delete tmp_num._num;

  tmp_num._num = _num->add(*arg2._num);

  tmp_num.simplifyRational();

  return tmp_num;
}

KNumber const KNumber::operator-(KNumber const & arg2) const
{
  return *this + (-arg2);
}

KNumber const KNumber::operator*(KNumber const & arg2) const
{
  KNumber tmp_num;
  delete tmp_num._num;

  tmp_num._num = _num->multiply(*arg2._num);

  tmp_num.simplifyRational();

  return tmp_num;
}

KNumber const KNumber::operator/(KNumber const & arg2) const
{
  KNumber tmp_num;
  delete tmp_num._num;

  tmp_num._num = _num->divide(*arg2._num);

  tmp_num.simplifyRational();

  return tmp_num;
}


KNumber const KNumber::operator%(KNumber const & arg2) const
{
  if (type() != IntegerType  ||  arg2.type() != IntegerType)
    return Zero;

  KNumber tmp_num;
  delete tmp_num._num;

  _knuminteger const *tmp_arg1 = dynamic_cast<_knuminteger const *>(_num);
  _knuminteger const *tmp_arg2 = dynamic_cast<_knuminteger const *>(arg2._num);

  tmp_num._num = tmp_arg1->mod(*tmp_arg2);

  return tmp_num;
}

KNumber const KNumber::operator&(KNumber const & arg2) const
{
  if (type() != IntegerType  ||  arg2.type() != IntegerType)
    return Zero;

  KNumber tmp_num;
  delete tmp_num._num;

  _knuminteger const *tmp_arg1 = dynamic_cast<_knuminteger const *>(_num);
  _knuminteger const *tmp_arg2 = dynamic_cast<_knuminteger const *>(arg2._num);

  tmp_num._num = tmp_arg1->intAnd(*tmp_arg2);

  return tmp_num;

}

KNumber const KNumber::operator|(KNumber const & arg2) const
{
  if (type() != IntegerType  ||  arg2.type() != IntegerType)
    return Zero;

  KNumber tmp_num;
  delete tmp_num._num;

  _knuminteger const *tmp_arg1 = dynamic_cast<_knuminteger const *>(_num);
  _knuminteger const *tmp_arg2 = dynamic_cast<_knuminteger const *>(arg2._num);

  tmp_num._num = tmp_arg1->intOr(*tmp_arg2);

  return tmp_num;
}


KNumber const KNumber::operator<<(KNumber const & arg2) const
{
  if (type() != IntegerType  ||  arg2.type() != IntegerType)
    return KNumber("nan");

  _knuminteger const *tmp_arg1 = dynamic_cast<_knuminteger const *>(_num);
  _knuminteger const *tmp_arg2 = dynamic_cast<_knuminteger const *>(arg2._num);

  KNumber tmp_num;
  delete tmp_num._num;
  tmp_num._num = tmp_arg1->shift(*tmp_arg2);

  return tmp_num;
}

KNumber const KNumber::operator>>(KNumber const & arg2) const
{
  if (type() != IntegerType  ||  arg2.type() != IntegerType)
    return KNumber("nan");

  KNumber tmp_num = -arg2;

  _knuminteger const *tmp_arg1 = dynamic_cast<_knuminteger const *>(_num);
  _knuminteger const *tmp_arg2 = dynamic_cast<_knuminteger const *>(tmp_num._num);

  KNumber tmp_num2;
  delete tmp_num2._num;
  tmp_num2._num = tmp_arg1->shift(*tmp_arg2);

  return tmp_num2;
}



KNumber::operator bool(void) const
{
  if (*this == Zero)
    return false;
  return true;
}

KNumber::operator signed long int(void) const
{
  return static_cast<signed long int>(*_num);
}

KNumber::operator unsigned long int(void) const
{
  return static_cast<unsigned long int>(*_num);
}

KNumber::operator unsigned long long int(void) const
{
#if SIZEOF_UNSIGNED_LONG == 8
  return static_cast<unsigned long int>(*this);
#elif SIZEOF_UNSIGNED_LONG == 4
  KNumber tmp_num1 = this->abs().integerPart();
  unsigned long long int tmp_num2 =  static_cast<unsigned long int>(tmp_num1) +
    (static_cast<unsigned long long int>(
					 static_cast<unsigned long int>(tmp_num1 >> KNumber("32"))) << 32) ;
  
#warning the cast operator from KNumber to unsigned long long int is probably buggy, when a sign is involved
  if (*this > KNumber(0))
    return tmp_num2;
  else
    return static_cast<unsigned long long int> (- static_cast<signed long long int>(tmp_num2));
#else
#error "SIZEOF_UNSIGNED_LONG is a unhandled case"
#endif  
}

KNumber::operator double(void) const
{
  return static_cast<double>(*_num);
}

int const KNumber::compare(KNumber const & arg2) const
{
  return _num->compare(*arg2._num);
}
