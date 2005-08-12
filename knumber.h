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

#include <kdelibs_export.h>

#include "knumber_priv.h"

class QString;

/**
  *
  * @short Class that provides arbitrary precision numbers
  *
  * KNumber provides access to arbitrary precision numbers from within
  * KDE.
  *
  * KNumber is based on the GMP (GNU Multiprecision) library and
  * provides transparent support to integer, fractional and floating
  * point number. It contains rudimentary error handling, and also
  * includes methods for converting the numbers to QStrings for
  * output, and to read QStrings to obtain a KNumber.
  *
  * The different types of numbers that can be represented by objects
  * of this class will be described below:
  *
  * @li @p NumType::SpecialType - This type represents an error that
  * has occured, e.g. trying to divide 1 by 0 gives an object that
  * represents infinity.
  *
  * @li @p NumType::IntegerType - The number is an integer. It can be
  * arbitrarily large (restricted by the memory of the system).
  *
  * @li @p NumType::FractionType - A fraction is a number of the form
  * denominator divided by nominator, where both denominator and
  * nominator are integers of arbitrary size.
  *
  * @li @p NumType::FloatType - The number is of floating point
  * type. These numbers are usually rounded, so that they do not
  * represent precise values.
  *
  *
  * @author Klaus Niederkrueger <kniederk@math.uni-koeln.de>
  */
class KDE_EXPORT KNumber
{
 public:
  static KNumber const Zero;
  static KNumber const One;
  static KNumber const MinusOne;
  static KNumber const Pi;

  /**
   * KNumber tries to provide transparent access to the following type
   * of numbers:
   *
   * @li @p NumType::SpecialType - Some type of error has occured,
   * further inspection with @p KNumber::ErrorType
   *
   * @li @p NumType::IntegerType - the number is an integer
   *
   * @li @p NumType::FractionType - the number is a fraction
   *
   * @li @p NumType::FloatType - the number is of floating point type
   *
   */
  enum NumType {SpecialType, IntegerType, FractionType, FloatType};
  /**
   * A KNumber that represents an error, i.e. that is of type @p
   * NumType::SpecialType can further distinguished:
   *
   * @li @p ErrorType::UndefinedNumber - This is e.g. the result of
   * taking the square root of a negative number or computing
   * \f$ \infty - \infty \f$.
   *
   * @li @p ErrorType::Infinity - Such a number can be e.g. obtained
   * by dividing 1 by 0. Some further calculations are still allowed,
   * e.g. \f$ \infty + 5 \f$ still gives \f$\infty\f$.
   *
   * @li @p ErrorType::MinusInfinity - MinusInfinity behaves similarly
   * to infinity above. It can be obtained by changing the sign of
   * infinity.
   *
   */
  enum ErrorType {UndefinedNumber, Infinity, MinusInfinity};
  
  KNumber(signed int num = 1);

  KNumber(double num);

  KNumber(KNumber const & num);
  
  KNumber(QString const & num);
  
  ~KNumber()
  {
    delete _num;
  }
  
  KNumber const & operator=(KNumber const & num);

  NumType type(void) const;

  QString const toQString(int prec = -1) const;
  
  /**
   * Compute the absoulte value, i.e. @p x.abs() returns the value
   *
   *  \f[ \left\{\begin{array}{cl} x, & x \ge 0 \\ -x, & x <
   *  0\end{array}\right.\f]
   * This method works for \f$ x = \infty \f$ and \f$ x = -\infty \f$.
   */
  KNumber const abs(void) const;
  KNumber const sqrt(void) const;
  KNumber const cbrt(void) const;
  KNumber const integerPart(void) const;

  KNumber const operator+(KNumber const & arg2) const;
  KNumber const operator -(void) const;
  KNumber const operator-(KNumber const & arg2) const;
  KNumber const operator*(KNumber const & arg2) const;
  KNumber const operator/(KNumber const & arg2) const;
  KNumber const operator%(KNumber const & arg2) const;

  KNumber const operator&(KNumber const & arg2) const;
  KNumber const operator|(KNumber const & arg2) const;
  KNumber const operator<<(KNumber const & arg2) const;
  KNumber const operator>>(KNumber const & arg2) const;

  operator bool(void) const;

  bool const operator==(KNumber const & arg2) const
  { return (compare(arg2) == 0); }

  bool const operator!=(KNumber const & arg2) const
  { return (compare(arg2) != 0); }

  bool const operator>(KNumber const & arg2) const
  { return (compare(arg2) > 0); }

  bool const operator<(KNumber const & arg2) const
  { return (compare(arg2) < 0); }

  bool const operator>=(KNumber const & arg2) const
  { return (compare(arg2) >= 0); }

  bool const operator<=(KNumber const & arg2) const
  { return (compare(arg2) <= 0); }

  KNumber & operator +=(KNumber const &arg);
  KNumber & operator -=(KNumber const &arg);


  //KNumber const toFloat(void) const;
  
  
  
  
 private:
  void simplifyRational(void);
  int const compare(KNumber const & arg2) const;
  
  _knumber *_num;
  
};



#endif // _KNUMBER_H
