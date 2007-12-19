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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef _KNUMBER_H
#define _KNUMBER_H

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
  * has occurred, e.g. trying to divide 1 by 0 gives an object that
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
  static KNumber const Euler;
  static KNumber const NotDefined;

  /**
   * KNumber tries to provide transparent access to the following type
   * of numbers:
   *
   * @li @p NumType::SpecialType - Some type of error has occurred,
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

  KNumber(signed int num = 0);
  KNumber(unsigned int num);  
  KNumber(signed long int num);
  KNumber(unsigned long int num);
  KNumber(unsigned long long int num);

  KNumber(double num);

  KNumber(KNumber const & num);
  
  KNumber(QString const & num);
  
  ~KNumber()
  {
    delete _num;
  }
  
  KNumber const & operator=(KNumber const & num);

  /**
   * Returns the type of the number, as explained in @p KNumber::NumType.
   */
  NumType type(void) const;

  /**
   * Set whether the output of numbers (with KNumber::toQString)
   * should happen as floating point numbers or not. This method has
   * in fact only an effect on numbers of type @p
   * NumType::FractionType, which can be either displayed as fractions
   * or in decimal notation.
   *
   * The default behavior is not to display fractions in floating
   * point notation.
   */
  static void setDefaultFloatOutput(bool flag);

  /**
   * Set whether a number constructed from a QString should be
   * initialized as a fraction or as a float, e.g. "1.01" would be
   * treated as 101/100, if this flag is set to true.
   *
   * The default setting is false.
   */
  static void setDefaultFractionalInput(bool flag);

  /**
   * Set the default precision to be *at least* @p prec (decimal)
   * digits.  All subsequent initialized floats will use at least this
   * precision, but previously initialized variables are unaffected.
   */
  static void setDefaultFloatPrecision(unsigned int prec);

  /**
   * What a terrible method name!!  When displaying a fraction, the
   * default mode gives @p "nomin/denom". With this method one can
   * choose to display a fraction as @p "integer nomin/denom".
   *
   * Examples: Default representation mode is 47/17, but if @p flag is
   * @p true, then the result is 2 13/17.
   */
  static void setSplitoffIntegerForFractionOutput(bool flag);

  /**
   * Return a QString representing the KNumber.
   *
   * @param width This number specifies the maximal length of the
   * output, before the method switches to exponential notation and
   * does rounding.  For negative numbers, this option is ignored.
   *
   * @param prec This parameter controls the number of digits
   * following the decimal point.  For negative numbers, this option
   * is ignored.
   *
   */
  QString const toQString(int width = -1, int prec = -1) const;
  
  /**
   * Compute the absolute value, i.e. @p x.abs() returns the value
   *
   *  \f[ \left\{\begin{array}{cl} x, & x \ge 0 \\ -x, & x <
   *  0\end{array}\right.\f]
   * This method works for \f$ x = \infty \f$ and \f$ x = -\infty \f$.
   */
  KNumber const abs(void) const;

  /**
   * Compute the square root. If \f$ x < 0 \f$ (including \f$
   * x=-\infty \f$), then @p x.sqrt() returns @p
   * ErrorType::UndefinedNumber.
   * 
   * If @p x is an integer or a fraction, then @p x.sqrt() tries to
   * compute the exact square root. If the square root is not a
   * fraction, then a float with the default precision is returned.
   *
   * This method works for \f$ x = \infty \f$ giving \f$ \infty \f$.
   */
  KNumber const sqrt(void) const;

  /**
   * Compute the cube root. 
   * 
   * If @p x is an integer or a fraction, then @p x.cbrt() tries to
   * compute the exact cube root. If the cube root is not a fraction,
   * then a float is returned, but
   *
   * WARNING: A float cube root is computed as a standard @p double
   * that is later transformed back into a @p KNumber.
   *
   * This method works for \f$ x = \infty \f$ giving \f$ \infty \f$,
   * and for \f$ x = -\infty \f$ giving \f$ -\infty \f$.
   */
  KNumber const cbrt(void) const;

  /**
   * Truncates a @p KNumber to its integer type returning a number of
   * type @p NumType::IntegerType.
   * 
   * If \f$ x = \pm\infty \f$, integerPart leaves the value unchanged,
   * i.e. it returns \f$ \pm\infty \f$.
   */
  KNumber const integerPart(void) const;

  KNumber const power(KNumber const &exp) const;

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
  operator signed long int(void) const;
  operator unsigned long int(void) const;
  operator unsigned long long int(void) const;
  operator double(void) const;

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
  static bool _float_output;
  static bool _fraction_input;
  static bool _splitoffinteger_output;
};



#endif // _KNUMBER_H
