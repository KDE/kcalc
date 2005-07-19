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


#include <qregexp.h>
#include <qstring.h>

#include "knumber.h"

KNumber const KNumber::ZeroInteger(0);
KNumber const KNumber::ZeroFloat("0.0");
KNumber const KNumber::OneInteger(1);
KNumber const KNumber::OneFloat("1.0");
KNumber const KNumber::MinusOneInteger(-1);
KNumber const KNumber::MinusOneFloat("-1.0");

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
  if (QRegExp("^[+-]?\\d+$").exactMatch(num))
    _num = new _knuminteger(num);
  else if (QRegExp("^[+-]?\\d+/\\d+$").exactMatch(num))
    _num = new _knumfraction(num);
  else if (QRegExp("^[+-]?\\d+(\\.\\d*)?(e[+-]?\\d+)?$").exactMatch(num))
    _num = new _knumfloat(num);
}

KNumber const & KNumber::operator=(KNumber const & num)
{
  if (this == & num) 
    return *this;

  delete _num;

  switch(num.type()) {
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

QString const KNumber::toQString(void) const
{
  return QString(_num->ascii());
}

KNumber const KNumber::abs(void) const
{
  KNumber tmp_num;
  delete tmp_num._num;

  tmp_num._num = _num->abs();

  return tmp_num;
}

KNumber const KNumber::operator -(void) const
{
  KNumber tmp_num;
  delete tmp_num._num;

  tmp_num._num = _num->change_sign();

  return tmp_num;
}

KNumber const KNumber::add(KNumber const & arg2) const
{
  KNumber tmp_num;
  delete tmp_num._num;

  tmp_num._num = _num->add(*arg2._num);

  return tmp_num;
}

KNumber const KNumber::multiply(KNumber const & arg2) const
{
  KNumber tmp_num;
  delete tmp_num._num;

  tmp_num._num = _num->multiply(*arg2._num);

  return tmp_num;
}

KNumber const KNumber::divide(KNumber const & arg2) const
{
  KNumber tmp_num;
  delete tmp_num._num;

  tmp_num._num = _num->divide(*arg2._num);

  return tmp_num;
}


int const KNumber::compare(KNumber const & arg2) const
{
#warning this is a stub
  return 0;
}

