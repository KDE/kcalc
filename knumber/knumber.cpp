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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <math.h>

#include <config.h>

#include <qregexp.h>
#include <qstring.h>

#include "knumber.h"

KNumber const KNumber::Zero(0);
KNumber const KNumber::One(1);
KNumber const KNumber::MinusOne(-1);
KNumber const KNumber::Pi("3.14159265358979323846264338327950288");

bool KNumber::_float_output = false;
bool KNumber::_fraction_input = false;
bool KNumber::_splitoffinteger_output = false;

KNumber::KNumber(signed int num)
{
  _num = new _knuminteger(num);
}

KNumber::KNumber(double num)
{
  _num = new _knumfloat(num);
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

QString const KNumber::toQString(QString const & prec) const
{
  QString tmp_str;
  switch (type()) {
  case IntegerType:
    tmp_str = QString(_num->ascii(prec));
    if (QRegExp("^\\d+$").exactMatch(prec)) {
      int int_num = prec.toInt();
      if (int_num < tmp_str.length()) {
	bool tmp_bool = _fraction_input; // stupid work-around
	_fraction_input = false;
	tmp_str = (KNumber("1.0")*(*this)).toQString(prec);
	_fraction_input = tmp_bool;
      }
    }
    break;
  case FractionType:
    if (_float_output) {
      bool tmp_bool = _fraction_input; // stupid work-around
      _fraction_input = false;
      tmp_str = QString((KNumber("1.0")*(*this))._num->ascii());
      _fraction_input = tmp_bool;
    }
    else if(_splitoffinteger_output) {
      // split off integer part
      KNumber int_part = this->integerPart();
      if (int_part == Zero)
	tmp_str = QString(_num->ascii());
      else if (int_part < Zero)
	tmp_str = int_part.toQString() + " " + (int_part - *this)._num->ascii();
      else
	tmp_str = int_part.toQString() + " " + (*this - int_part)._num->ascii();
    }  else
      tmp_str = QString(_num->ascii());

    if (QRegExp("^\\d+$").exactMatch(prec)) {
      int int_num = prec.toInt();
      if (int_num < tmp_str.length()) {
	bool tmp_bool = _fraction_input; // stupid work-around
	_fraction_input = false;
	tmp_str = (KNumber("1.0")*(*this)).toQString(prec);
	_fraction_input = tmp_bool;
      }
    }
    break;
  default:
    tmp_str = QString(_num->ascii(prec));
  }

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

  if (exp == Zero)
    return One;
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

KNumber::operator long int(void) const
{
  return static_cast<long int>(*_num);
}

KNumber::operator double(void) const
{
  return static_cast<double>(*_num);
}

int const KNumber::compare(KNumber const & arg2) const
{
  return _num->compare(*arg2._num);
}
