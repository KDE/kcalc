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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <qregexp.h>
#include <qstring.h>

#include "knumber_priv.h"

_knumerror::_knumerror(_knumber const & num)
{
  switch(num.type()) {
  case SpecialType:
    _error = dynamic_cast<_knumerror const &>(num)._error;
    break;
  case IntegerType:
  case FractionType:
  case FloatType:
    // What should I do here?
    break;
  }
}



_knuminteger::_knuminteger(unsigned long long int num)
{
  mpz_init(_mpz);
#if SIZEOF_UNSIGNED_LONG == 8
  mpz_init_set_ui(_mpz, static_cast<unsigned long int>(num));
#elif SIZEOF_UNSIGNED_LONG == 4
  mpz_set_ui(_mpz, static_cast<unsigned long int>(num >> 32));
  mpz_mul_2exp(_mpz, _mpz, 32);
  mpz_add_ui(_mpz, _mpz, static_cast<unsigned long int>(num));
#else
#error "SIZEOF_UNSIGNED_LONG is a unhandled case"
#endif 
}


_knuminteger::_knuminteger(_knumber const & num)
{
  mpz_init(_mpz);

  switch(num.type()) {
  case IntegerType:
    mpz_set(_mpz, dynamic_cast<_knuminteger const &>(num)._mpz);
    break;
  case FractionType:
  case FloatType:
  case SpecialType:
    // What should I do here?
    break;
  }
}

_knumfraction::_knumfraction(_knumber const & num)
{
  mpq_init(_mpq);
  
  switch(num.type()) {
  case IntegerType:
    mpq_set_z(_mpq, dynamic_cast<_knuminteger const &>(num)._mpz);
    break;
  case FractionType:
    mpq_set(_mpq, dynamic_cast<_knumfraction const &>(num)._mpq);
    break;
  case FloatType:
  case SpecialType:
    // What should I do here?
    break;
  }
}

_knumfloat::_knumfloat(_knumber const & num)
{
  mpf_init(_mpf);
  
  switch(num.type()) {
  case IntegerType:
    mpf_set_z(_mpf, dynamic_cast<_knuminteger const &>(num)._mpz);
    break;
  case FractionType:
    mpf_set_q(_mpf, dynamic_cast<_knumfraction const &>(num)._mpq);
    break;
  case FloatType:
    mpf_set(_mpf, dynamic_cast<_knumfloat const &>(num)._mpf);
    break;
  case SpecialType:
    // What should I do here?
    break;
  }
}



_knumerror::_knumerror(QString const & num)
{
  if (num == "nan")
    _error = UndefinedNumber;
  else if (num == "inf")
    _error = Infinity;
  else if (num == "-inf")
    _error = MinusInfinity;
}

_knuminteger::_knuminteger(QString const & num)
{
  mpz_init(_mpz);
  mpz_set_str(_mpz, num.ascii(), 10);
}

_knumfraction::_knumfraction(QString const & num)
{
  mpq_init(_mpq);
  if (QRegExp("^[+-]?\\d+(\\.\\d*)?(e[+-]?\\d+)?$").exactMatch(num)) {
    // my hand-made conversion is terrible
    // first me convert the mantissa
    unsigned long int digits_after_dot = ((num.section( '.', 1, 1)).section('e', 0, 0)).length();
    QString tmp_num = num.section('e', 0, 0).remove('.');
    mpq_set_str(_mpq, tmp_num.ascii(), 10);
    mpz_t tmp_int;
    mpz_init(tmp_int);
    mpz_ui_pow_ui (tmp_int, 10, digits_after_dot);
    mpz_mul(mpq_denref(_mpq), mpq_denref(_mpq), tmp_int);
    // now we take care of the exponent
    if (! (tmp_num = num.section('e', 1, 1)).isEmpty()) {
      long int tmp_exp = tmp_num.toLong();
      if (tmp_exp > 0) {
	mpz_ui_pow_ui (tmp_int, 10,
		       static_cast<unsigned long int>(tmp_exp));
	mpz_mul(mpq_numref(_mpq), mpq_numref(_mpq), tmp_int);
      } else {
	mpz_ui_pow_ui (tmp_int, 10,
		       static_cast<unsigned long int>(-tmp_exp));
	mpz_mul(mpq_denref(_mpq), mpq_denref(_mpq), tmp_int);
      }
    }
    mpz_clear(tmp_int);
  } else
    mpq_set_str(_mpq, num.ascii(), 10);
  mpq_canonicalize(_mpq);
}

_knumfloat::_knumfloat(QString const & num)
{
  mpf_init(_mpf);
  mpf_set_str(_mpf, num.ascii(), 10);
}

_knuminteger const & _knuminteger::operator = (_knuminteger const & num)
{
  if (this == &num)
    return *this;

  mpz_set(_mpz, num._mpz);
  return *this;
}

QString const _knumerror::ascii(int prec) const
{
  static_cast<void>(prec);

  switch(_error) {
  case UndefinedNumber:
    return QString("nan");
  case Infinity:
    return QString("inf");
  case MinusInfinity:
    return QString("-inf");
  default:
    return QString::null;
  }
}

QString const _knuminteger::ascii(int prec) const
{
  static_cast<void>(prec);
  char *tmp_ptr;

  gmp_asprintf(&tmp_ptr, "%Zd", _mpz);
  QString ret_str = tmp_ptr;

  free(tmp_ptr);
  return ret_str;
}

QString const _knumfraction::ascii(int prec) const
{
  static_cast<void>(prec);
  char *tmp_ptr = mpq_get_str(0, 10, _mpq);
  QString ret_str = tmp_ptr;

  free(tmp_ptr);

  return ret_str;
}

QString const _knumfloat::ascii(int prec) const
{
  QString ret_str;
  char *tmp_ptr;
  if (prec > 0)
    gmp_asprintf(&tmp_ptr, ("%." + QString().setNum(prec) + "Fg").ascii(), _mpf);
  else
    gmp_asprintf(&tmp_ptr, "%Fg", _mpf);

  ret_str = tmp_ptr;

  free(tmp_ptr);

  return ret_str;
}


bool _knumfraction::isInteger(void) const
{
  if (mpz_cmp_ui(mpq_denref(_mpq), 1) == 0)
    return true;
  else
    return false;
}


_knumber * _knumerror::abs(void) const
{
  _knumerror * tmp_num = new _knumerror(*this);

  if(_error == MinusInfinity) tmp_num->_error = Infinity;

  return tmp_num;
}

_knumber * _knuminteger::abs(void) const
{
  _knuminteger * tmp_num = new _knuminteger();

  mpz_abs(tmp_num->_mpz, _mpz);

  return tmp_num;
}

_knumber * _knumfraction::abs(void) const
{
  _knumfraction * tmp_num = new _knumfraction();
  
  mpq_abs(tmp_num->_mpq, _mpq);
  
  return tmp_num;
}

_knumber * _knumfloat::abs(void) const
{
  _knumfloat * tmp_num = new _knumfloat();
  
  mpf_abs(tmp_num->_mpf, _mpf);
  
  return tmp_num;
}



_knumber * _knumerror::intPart(void) const
{
  return new _knumerror(*this);
}

_knumber * _knuminteger::intPart(void) const
{
  _knuminteger *tmp_num = new _knuminteger();
  mpz_set(tmp_num->_mpz, _mpz);
  return tmp_num;
}

_knumber * _knumfraction::intPart(void) const
{
  _knuminteger *tmp_num = new _knuminteger();

  mpz_set_q(tmp_num->_mpz, _mpq);
  
  return tmp_num;
}

_knumber * _knumfloat::intPart(void) const
{
  _knuminteger *tmp_num = new _knuminteger();

  mpz_set_f(tmp_num->_mpz, _mpf);
  
  return tmp_num;
}




int _knumerror::sign(void) const
{
  switch(_error) {
  case Infinity:
    return 1;
  case MinusInfinity:
    return -1;
  default:
    return 0;
  }
}

int _knuminteger::sign(void) const
{
  return mpz_sgn(_mpz);
}

int _knumfraction::sign(void) const
{
  return mpq_sgn(_mpq);
}

int _knumfloat::sign(void) const
{
  return mpf_sgn(_mpf);
}



#warning _cbrt for now this is a stupid work around
static void _cbrt(mpf_t &num)
{
  double tmp_num = cbrt(mpf_get_d(num));
  mpf_init_set_d(num, tmp_num);
}


_knumber * _knumerror::cbrt(void) const
{
  // infty ^3 = infty;  -infty^3 = -infty
  _knumerror *tmp_num = new _knumerror(*this);
  
  return tmp_num;
}

_knumber * _knuminteger::cbrt(void) const
{
  _knuminteger * tmp_num = new _knuminteger();
  
  if(mpz_root(tmp_num->_mpz, _mpz, 3))
    return tmp_num; // root is perfect
  
  delete tmp_num; // root was not perfect, result will be float
  
  _knumfloat * tmp_num2 = new _knumfloat();
  mpf_set_z(tmp_num2->_mpf, _mpz);

  _cbrt(tmp_num2->_mpf);
    
  return tmp_num2;
}

_knumber * _knumfraction::cbrt(void) const
{
  _knumfraction * tmp_num = new _knumfraction();
  if (mpz_root(mpq_numref(tmp_num->_mpq), mpq_numref(_mpq), 3)
      &&  mpz_root(mpq_denref(tmp_num->_mpq), mpq_denref(_mpq), 3))
    return tmp_num; // root is perfect

  delete tmp_num; // root was not perfect, result will be float

  _knumfloat * tmp_num2 = new _knumfloat();
  mpf_set_q(tmp_num2->_mpf, _mpq);

  _cbrt(tmp_num2->_mpf);
    
  return tmp_num2;
}

_knumber * _knumfloat::cbrt(void) const
{
  _knumfloat * tmp_num = new _knumfloat(*this);

  _cbrt(tmp_num->_mpf);
  
  return tmp_num;
}




_knumber * _knumerror::sqrt(void) const
{
  _knumerror *tmp_num = new _knumerror(*this);
  
  if(_error == MinusInfinity) tmp_num->_error = UndefinedNumber;

  return tmp_num;
}

_knumber * _knuminteger::sqrt(void) const
{
  if (mpz_sgn(_mpz) < 0) {
    _knumerror *tmp_num = new _knumerror(UndefinedNumber);
    return tmp_num;
  }
  if (mpz_perfect_square_p(_mpz)) {
    _knuminteger * tmp_num = new _knuminteger();
    
    mpz_sqrt(tmp_num->_mpz, _mpz);

    return tmp_num;
  } else {
    _knumfloat * tmp_num = new _knumfloat();
    mpf_set_z(tmp_num->_mpf, _mpz);
    mpf_sqrt(tmp_num->_mpf, tmp_num->_mpf);
    
    return tmp_num;
  }
}

_knumber * _knumfraction::sqrt(void) const
{
  if (mpq_sgn(_mpq) < 0) {
    _knumerror *tmp_num = new _knumerror(UndefinedNumber);
    return tmp_num;
  }
  if (mpz_perfect_square_p(mpq_numref(_mpq))
      &&  mpz_perfect_square_p(mpq_denref(_mpq))) {
    _knumfraction * tmp_num = new _knumfraction();
    mpq_set(tmp_num->_mpq, _mpq);
    mpz_sqrt(mpq_numref(tmp_num->_mpq), mpq_numref(tmp_num->_mpq));
    mpz_sqrt(mpq_denref(tmp_num->_mpq), mpq_denref(tmp_num->_mpq));

    return tmp_num;
  } else {
    _knumfloat * tmp_num = new _knumfloat();
    mpf_set_q(tmp_num->_mpf, _mpq);
    mpf_sqrt(tmp_num->_mpf, tmp_num->_mpf);
    
    return tmp_num;
  }

  _knumfraction * tmp_num = new _knumfraction();
  
  return tmp_num;
}

_knumber * _knumfloat::sqrt(void) const
{
  if (mpf_sgn(_mpf) < 0) {
    _knumerror *tmp_num = new _knumerror(UndefinedNumber);
    return tmp_num;
  }
  _knumfloat * tmp_num = new _knumfloat();
  
  mpf_sqrt(tmp_num->_mpf, _mpf);
  
  return tmp_num;
}



_knumber * _knumerror::change_sign(void) const
{
  _knumerror * tmp_num = new _knumerror();

  if(_error == Infinity) tmp_num->_error = MinusInfinity;
  if(_error == MinusInfinity) tmp_num->_error = Infinity;

  return tmp_num;
}

_knumber * _knuminteger::change_sign(void) const
{
  _knuminteger * tmp_num = new _knuminteger();

  mpz_neg(tmp_num->_mpz, _mpz);

  return tmp_num;
}

_knumber * _knumfraction::change_sign(void) const
{
  _knumfraction * tmp_num = new _knumfraction();
  
  mpq_neg(tmp_num->_mpq, _mpq);
  
  return tmp_num;
}

_knumber *_knumfloat::change_sign(void) const
{
  _knumfloat * tmp_num = new _knumfloat();
  
  mpf_neg(tmp_num->_mpf, _mpf);
  
  return tmp_num;
}


_knumber * _knumerror::reciprocal(void) const
{
  switch(_error) {
  case  Infinity:
  case  MinusInfinity:
    return new _knuminteger(0);
  case  UndefinedNumber:
  default:
    return new _knumerror(UndefinedNumber);
  }
}

_knumber * _knuminteger::reciprocal(void) const
{
  if(mpz_cmp_si(_mpz, 0) == 0) return new _knumerror(Infinity);

  _knumfraction * tmp_num = new _knumfraction(*this);

  mpq_inv(tmp_num->_mpq, tmp_num->_mpq);

  return tmp_num;
}

_knumber * _knumfraction::reciprocal() const
{
  if(mpq_cmp_si(_mpq, 0, 1) == 0) return new _knumerror(Infinity);

  _knumfraction * tmp_num = new _knumfraction();
  
  mpq_inv(tmp_num->_mpq, _mpq);
  
  return tmp_num;
}

_knumber *_knumfloat::reciprocal(void) const
{
  if(mpf_cmp_si(_mpf, 0) == 0) return new _knumerror(Infinity);

  _knumfloat * tmp_num = new _knumfloat();
  
  mpf_div(tmp_num->_mpf, _knumfloat("1.0")._mpf, _mpf);
  
  return tmp_num;
}



_knumber * _knumerror::add(_knumber const & arg2) const
{
  if (arg2.type() != SpecialType)
    return new _knumerror(_error);

  _knumerror const & tmp_arg2 = dynamic_cast<_knumerror const &>(arg2);
  
  if (_error == UndefinedNumber  
      || tmp_arg2._error == UndefinedNumber
      || (_error == Infinity && tmp_arg2._error == MinusInfinity)
      || (_error == MinusInfinity && tmp_arg2._error == Infinity)
      )
    return new _knumerror(UndefinedNumber);      

  return new _knumerror(_error);
}

_knumber * _knuminteger::add(_knumber const & arg2) const
{
  if (arg2.type() != IntegerType)
    return arg2.add(*this);
  
  _knuminteger * tmp_num = new _knuminteger();

  mpz_add(tmp_num->_mpz, _mpz,
	  dynamic_cast<_knuminteger const &>(arg2)._mpz);

  return tmp_num;
}

_knumber * _knumfraction::add(_knumber const & arg2) const
{
  if (arg2.type() == IntegerType) {
    // need to cast arg2 to fraction
    _knumfraction tmp_num(arg2);
    return tmp_num.add(*this);
  }

  
  if (arg2.type() == FloatType  ||  arg2.type() == SpecialType)
    return arg2.add(*this);
  
  _knumfraction * tmp_num = new _knumfraction();
  
  mpq_add(tmp_num->_mpq, _mpq,
	  dynamic_cast<_knumfraction const &>(arg2)._mpq);
  
  return tmp_num;
}

_knumber *_knumfloat::add(_knumber const & arg2) const
{
  if (arg2.type() == SpecialType)
    return arg2.add(*this);

  if (arg2.type() != FloatType) {
    // need to cast arg2 to float
    _knumfloat tmp_num(arg2);
    return tmp_num.add(*this);
  }
  
  _knumfloat * tmp_num = new _knumfloat();
  
  mpf_add(tmp_num->_mpf, _mpf,
	  dynamic_cast<_knumfloat const &>(arg2)._mpf);
  
  return tmp_num;
}


_knumber * _knumerror::multiply(_knumber const & arg2) const
{
  //improve this
  switch(arg2.type()) {
  case SpecialType:
    {
      _knumerror const & tmp_arg2 = dynamic_cast<_knumerror const &>(arg2);
      if (_error == UndefinedNumber || tmp_arg2._error == UndefinedNumber)
	return new _knumerror(UndefinedNumber);
      if ( this->sign() * arg2.sign() > 0)
	return new _knumerror(Infinity);
      else
	return new _knumerror(MinusInfinity);
    }
  case IntegerType:
  case FractionType:
  case FloatType:
    {
      int sign_arg2 = arg2.sign();
      if (_error == UndefinedNumber || sign_arg2 == 0)
	return new _knumerror(UndefinedNumber);
      if ( (_error == Infinity  &&  sign_arg2 > 0)  ||
	   (_error == MinusInfinity  &&  sign_arg2 < 0) )
	return new _knumerror(Infinity);

      return new _knumerror(MinusInfinity);
    }
  }

  return new _knumerror(_error);
}


_knumber * _knuminteger::multiply(_knumber const & arg2) const
{
  if (arg2.type() != IntegerType)
    return arg2.multiply(*this);
  
  _knuminteger * tmp_num = new _knuminteger();

  mpz_mul(tmp_num->_mpz, _mpz,
	  dynamic_cast<_knuminteger const &>(arg2)._mpz);

  return tmp_num;
}

_knumber * _knumfraction::multiply(_knumber const & arg2) const
{
  if (arg2.type() == IntegerType) {
    // need to cast arg2 to fraction
    _knumfraction tmp_num(arg2);
    return tmp_num.multiply(*this);
  }

  
  if (arg2.type() == FloatType  ||  arg2.type() == SpecialType)
    return arg2.multiply(*this);
  
  _knumfraction * tmp_num = new _knumfraction();
  
  mpq_mul(tmp_num->_mpq, _mpq,
	  dynamic_cast<_knumfraction const &>(arg2)._mpq);
  
  return tmp_num;
}

_knumber *_knumfloat::multiply(_knumber const & arg2) const
{
  if (arg2.type() == SpecialType)
    return arg2.multiply(*this);
  if (arg2.type() == IntegerType  &&
      mpz_cmp_si(dynamic_cast<_knuminteger const &>(arg2)._mpz,0) == 0)
    // if arg2 == 0 return integer 0!!
    return new _knuminteger(0);

  if (arg2.type() != FloatType) {
    // need to cast arg2 to float
    _knumfloat tmp_num(arg2);
    return tmp_num.multiply(*this);
  }
  
  _knumfloat * tmp_num = new _knumfloat();
  
  mpf_mul(tmp_num->_mpf, _mpf,
	  dynamic_cast<_knumfloat const &>(arg2)._mpf);
  
  return tmp_num;
}





_knumber * _knumber::divide(_knumber const & arg2) const
{
  _knumber * tmp_num = arg2.reciprocal();
  _knumber * rslt_num = this->multiply(*tmp_num);

  delete tmp_num;

  return rslt_num;
}

_knumber *_knumfloat::divide(_knumber const & arg2) const
{
  if(mpf_cmp_si(_mpf, 0) == 0) return new _knumerror(Infinity);

  // automatically casts arg2 to float
  _knumfloat * tmp_num = new _knumfloat(arg2);

  mpf_div(tmp_num->_mpf, _mpf, tmp_num->_mpf);
  
  return tmp_num;
}




_knumber * _knumerror::power(_knumber const & exponent) const
{
  static_cast<void>(exponent);
  return new _knumerror(UndefinedNumber);
}

_knumber * _knuminteger::power(_knumber const & exponent) const
{
  if (exponent.type() == IntegerType) {

    mpz_t tmp_mpz;
    mpz_init_set(tmp_mpz,
		 dynamic_cast<_knuminteger const &>(exponent)._mpz);
    
    if (! mpz_fits_ulong_p(tmp_mpz)) { // conversion wouldn't work, so
				       // use floats
      mpz_clear(tmp_mpz);
      // need to cast everything to float
      _knumfloat tmp_num1(*this), tmp_num2(exponent);
      return tmp_num1.power(tmp_num2);
    }

    unsigned long int tmp_int = mpz_get_ui(tmp_mpz);
    mpz_clear(tmp_mpz);

    _knuminteger * tmp_num = new _knuminteger();
    mpz_pow_ui(tmp_num->_mpz, _mpz, tmp_int);
    return tmp_num;
  }
  if (exponent.type() == FractionType) {
    if (mpz_sgn(_mpz) < 0)
      return new _knumerror(UndefinedNumber);
    // GMP only supports few root functions, so we need to convert
    // into signed long int
    mpz_t tmp_mpz;
    mpz_init_set(tmp_mpz,
		 mpq_denref(dynamic_cast<_knumfraction const &>(exponent)._mpq));
    
    if (! mpz_fits_ulong_p(tmp_mpz)) { // conversion wouldn't work, so
				       // use floats
      mpz_clear(tmp_mpz);
      // need to cast everything to float
      _knumfloat tmp_num1(*this), tmp_num2(exponent);
      return tmp_num1.power(tmp_num2);
    }

    unsigned long int tmp_int = mpz_get_ui(tmp_mpz);
    mpz_clear(tmp_mpz);

    // first check if result will be an integer
    _knuminteger * tmp_num = new _knuminteger();
    int flag = mpz_root(tmp_num->_mpz, _mpz, tmp_int);
    if (flag == 0) { // result is not exact
      delete tmp_num;
      // need to cast everything to float
      _knumfloat tmp_num1(*this), tmp_num2(exponent);
      return tmp_num1.power(tmp_num2);
    }

    // result is exact

    mpz_init_set(tmp_mpz,
		 mpq_numref(dynamic_cast<_knumfraction const &>(exponent)._mpq));
    
    if (! mpz_fits_ulong_p(tmp_mpz)) { // conversion wouldn't work, so
				       // use floats
      mpz_clear(tmp_mpz);
      // need to cast everything to float
      _knumfloat tmp_num1(*this), tmp_num2(exponent);
      return tmp_num1.power(tmp_num2);
    }
    tmp_int = mpz_get_ui(tmp_mpz);
    mpz_clear(tmp_mpz);

    mpz_pow_ui(tmp_num->_mpz, tmp_num->_mpz, tmp_int);

    return tmp_num;
  }
  if (exponent.type() == FloatType) {
    // need to cast everything to float
    _knumfloat tmp_num(*this);
    return tmp_num.power(exponent);
  }

  return new _knumerror(Infinity);
}

_knumber * _knumfraction::power(_knumber const & exponent) const
{
  _knuminteger tmp_num = _knuminteger();

  mpz_set(tmp_num._mpz, mpq_numref(_mpq));
  _knumber *numer = tmp_num.power(exponent);

  mpz_set(tmp_num._mpz, mpq_denref(_mpq));
  _knumber *denom = tmp_num.power(exponent);

  _knumber *result = numer->divide(*denom);
  delete numer;
  delete denom;
  return result;
}

_knumber * _knumfloat::power(_knumber const & exponent) const
{
  return new _knumfloat(pow(static_cast<double>(*this),
			    static_cast<double>(exponent)));
}


int _knumerror::compare(_knumber const &arg2) const
{
  if (arg2.type() != SpecialType) {
    switch(_error) {
    case Infinity:
      return 1;
    case MinusInfinity:
      return -1;
    default:
      return 1; // Not really o.k., but what should I return
    }
  }

  switch(_error) {
  case Infinity:
    if (dynamic_cast<_knumerror const &>(arg2)._error == Infinity)
      // Infinity is larger than anything else, but itself
      return 0;
    return 1;
  case MinusInfinity:
    if (dynamic_cast<_knumerror const &>(arg2)._error == MinusInfinity)
      // MinusInfinity is smaller than anything else, but itself
      return 0;
    return -1;
  default:
    if (dynamic_cast<_knumerror const &>(arg2)._error == UndefinedNumber)
      // Undefined only equal to itself
      return 0;
    return -arg2.compare(*this);
  }
}

int _knuminteger::compare(_knumber const &arg2) const
{
  if (arg2.type() != IntegerType)
    return - arg2.compare(*this);

  return mpz_cmp(_mpz, dynamic_cast<_knuminteger const &>(arg2)._mpz);
}

int _knumfraction::compare(_knumber const &arg2) const
{
  if (arg2.type() != FractionType) {
    if (arg2.type() == IntegerType) {
      mpq_t tmp_frac;
      mpq_init(tmp_frac);
      mpq_set_z(tmp_frac,
		dynamic_cast<_knuminteger const &>(arg2)._mpz);
      int cmp_result =  mpq_cmp(_mpq, tmp_frac);
      mpq_clear(tmp_frac);
      return cmp_result;
    } else
      return - arg2.compare(*this);
  }

  return mpq_cmp(_mpq, dynamic_cast<_knumfraction const &>(arg2)._mpq);
}

int _knumfloat::compare(_knumber const &arg2) const
{
  if (arg2.type() != FloatType) {
    mpf_t tmp_float;
    if (arg2.type() == IntegerType) {
      mpf_init(tmp_float);
      mpf_set_z(tmp_float,
		dynamic_cast<_knuminteger const &>(arg2)._mpz);
    } else if (arg2.type() == FractionType) {
      mpf_init(tmp_float);
      mpf_set_q(tmp_float,
		dynamic_cast<_knumfraction const &>(arg2)._mpq);
    } else
      return - arg2.compare(*this);
    
    int cmp_result =  mpf_cmp(_mpf, tmp_float);
    mpf_clear(tmp_float);
    return cmp_result;
  }

  return mpf_cmp(_mpf, dynamic_cast<_knumfloat const &>(arg2)._mpf);
}



_knumerror::operator signed long int (void) const
{
  // what would be the correct return values here?
  if (_error == Infinity)
    return 0;
  if (_error == MinusInfinity)
    return 0;
  else // if (_error == UndefinedNumber)
    return 0;
}

_knumerror::operator unsigned long int (void) const
{
  // what would be the correct return values here?
  if (_error == Infinity)
    return 0;
  if (_error == MinusInfinity)
    return 0;
  else // if (_error == UndefinedNumber)
    return 0;
}


_knuminteger::operator signed long int (void) const
{
  return mpz_get_si(_mpz);
}

_knumfraction::operator signed long int (void) const
{
  return static_cast<signed long int>(mpq_get_d(_mpq));
}

_knumfloat::operator signed long int (void) const
{
  return mpf_get_si(_mpf);
}

_knuminteger::operator unsigned long int (void) const
{
  return mpz_get_ui(_mpz);
}

_knumfraction::operator unsigned long int (void) const
{
  return static_cast<unsigned long int>(mpq_get_d(_mpq));
}

_knumfloat::operator unsigned long int (void) const
{
  return mpf_get_ui(_mpf);
}



_knumerror::operator double (void) const
{
  if (_error == Infinity)
    return INFINITY;
  if (_error == MinusInfinity)
    return -INFINITY;
  else // if (_error == UndefinedNumber)
    return NAN;
}

_knuminteger::operator double (void) const
{
  return mpz_get_d(_mpz);
}

_knumfraction::operator double (void) const
{
  return mpq_get_d(_mpq);
}

_knumfloat::operator double (void) const
{
  return mpf_get_d(_mpf);
}




_knuminteger * _knuminteger::intAnd(_knuminteger const &arg2) const
{
  _knuminteger * tmp_num = new _knuminteger();

  mpz_and(tmp_num->_mpz, _mpz, arg2._mpz);
  
  return tmp_num;
}

_knuminteger * _knuminteger::intOr(_knuminteger const &arg2) const
{
  _knuminteger * tmp_num = new _knuminteger();

  mpz_ior(tmp_num->_mpz, _mpz, arg2._mpz);
  
  return tmp_num;
}

_knumber * _knuminteger::mod(_knuminteger const &arg2) const
{
  if(mpz_cmp_si(arg2._mpz, 0) == 0) return new _knumerror(UndefinedNumber);

  _knuminteger * tmp_num = new _knuminteger();

  mpz_mod(tmp_num->_mpz, _mpz, arg2._mpz);
  
  return tmp_num;
}

_knumber * _knuminteger::shift(_knuminteger const &arg2) const
{
  mpz_t tmp_mpz;

  mpz_init_set (tmp_mpz, arg2._mpz);

  if (! mpz_fits_slong_p(tmp_mpz)) {
    mpz_clear(tmp_mpz);
    return new _knumerror(UndefinedNumber);
  }
  
  signed long int tmp_arg2 = mpz_get_si(tmp_mpz);
  mpz_clear(tmp_mpz);


  _knuminteger * tmp_num = new _knuminteger();

  if (tmp_arg2 > 0)  // left shift
    mpz_mul_2exp(tmp_num->_mpz, _mpz, tmp_arg2);
  else  // right shift
    mpz_tdiv_q_2exp(tmp_num->_mpz, _mpz, -tmp_arg2);


  return tmp_num;
}

