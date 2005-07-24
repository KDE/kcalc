// -*- c-basic-offset: 2 -*-
/* This file is part of the KDE libraries
    Copyright (C) 2005 Klaus Niederkrueger <kniederk@math.uni-koeln.de>

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
#ifndef _KNUMBER_H
#define _KNUMBER_H

#include <gmp.h>
#include <qstring.h>

#include <kdelibs_export.h>

#include "knumber_priv.h"

/**
  *
  * KNumber provides access to arbitrary precision numbers from within
  * KDE.
  *
  * KNumber is base on the GMP (GNU Multiprecision) library and
  * provides transparent support to integer, fractional and floating
  * point number. It also includes methods for converting the numbers
  * to QStrings for output, and to read QStrings to obtain a KNumber.
  *
  * @author Klaus Niederkrueger <kniederk@math.uni-koeln.de>
  * @short class that provides arbitrary prcision numbers
  */
class KDE_EXPORT KNumber
{
 public:
  static KNumber const ZeroInteger;
  static KNumber const OneInteger;
  static KNumber const MinusOneInteger;

  static KNumber const ZeroFloat;
  static KNumber const OneFloat;
  static KNumber const MinusOneFloat;

  enum NumType {IntegerType, FractionType, FloatType};
  
  KNumber(signed int num = 1);

  KNumber(double num);

  KNumber(KNumber const & num);
  
  KNumber(QString const & num);
  
  ~KNumber()
  {
    delete _num;
  }
  
  KNumber const & operator=(KNumber const & num);

  NumType type(void) const
  {
    if(dynamic_cast<_knuminteger *>(_num))
      return IntegerType;
    if(dynamic_cast<_knumfraction *>(_num))
      return FractionType;
    if(dynamic_cast<_knumfloat *>(_num))
      return FloatType;
  }

  QString const toQString(void) const;
  
  KNumber const operator -(void) const;
  KNumber const abs(void) const;
  KNumber const sqrt(void) const;
  KNumber const integerPart(void) const;

  KNumber const add(KNumber const & arg2) const;
  KNumber const multiply(KNumber const & arg2) const;
  KNumber const divide(KNumber const & arg2) const;
  KNumber const mod(KNumber const & arg2) const;

  int const compare(KNumber const & arg2) const;
  

  KNumber & operator +=(KNumber const &arg);
  KNumber & operator -=(KNumber const &arg);


  //KNumber const toFloat(void) const;
  
  
  
  
 private:
  _knumber *_num;

};


KNumber const operator+(KNumber const & arg1, KNumber const & arg2)
{
  return arg1.add(arg2);
}

KNumber const operator-(KNumber const & arg1, KNumber const & arg2)
{
  return arg1 + (-arg2);
}
KNumber const operator*(KNumber const & arg1, KNumber const & arg2)
{
  return arg1.multiply(arg2);
}

KNumber const operator/(KNumber const & arg1, KNumber const & arg2)
{
  return arg1.divide(arg2);
}

KNumber const operator%(KNumber const & arg1, KNumber const & arg2)
{
  return arg1.mod(arg2);
}

bool const operator==(KNumber const & arg1, KNumber const & arg2)
{
  return (arg1.compare(arg2) == 0);
}  

bool const operator>(KNumber const & arg1, KNumber const & arg2)
{
  return (arg1.compare(arg2) > 0);
}  

bool const operator<(KNumber const & arg1, KNumber const & arg2)
{
  return (arg1.compare(arg2) < 0);
}  

bool const operator>=(KNumber const & arg1, KNumber const & arg2)
{
  return (arg1.compare(arg2) >= 0);
}  

bool const operator<=(KNumber const & arg1, KNumber const & arg2)
{
  return (arg1.compare(arg2) <= 0);
}  

#warning below all stubs
KNumber const operator&(KNumber const & arg1, KNumber const & arg2)
{
  return arg1.divide(arg2);
}  

KNumber const operator|(KNumber const & arg1, KNumber const & arg2)
{
  return arg1.divide(arg2);
}  

KNumber const operator<<(KNumber const & arg1, KNumber const & arg2)
{
  return arg1.divide(arg2);
}  

KNumber const operator>>(KNumber const & arg1, KNumber const & arg2)
{
  return arg1.divide(arg2);
}  


#endif // _KNUMBER_H
