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

#ifdef HAVE_GMP
# include "knumber_priv.h"

_knuminteger::_knuminteger(_knumber const & num)
  : _knumber(num)
{
  switch(num.type()) {
  case IntegerType:
    mpz_init_set(_mpz, dynamic_cast<_knuminteger const &>(num)._mpz);
    break;
  case FractionType:
  case FloatType:
    // What should I do here?
    break;
  }
}

_knumfraction::_knumfraction(_knumber const & num)
  : _knumber(num)
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
    // What should I do here?
    break;
  }
}

_knumfloat::_knumfloat(_knumber const & num)
  : _knumber(num)
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
  }
}

_knuminteger::_knuminteger(QString const & num)
{
  mpz_init(_mpz);
  mpz_set_str(_mpz, num.ascii(), 10);
}

_knumfraction::_knumfraction(QString const & num)
{
  mpq_init(_mpq);
  mpq_set_str(_mpq, num.ascii(), 10);
  mpq_canonicalize(_mpq);
}

_knumfloat::_knumfloat(QString const & num)
{
  mpf_init(_mpf);
  mpf_set_str(_mpf, num.ascii(), 10);
}



QString const _knuminteger::ascii(void) const
{
  char const *tmp_ptr = mpz_get_str(0, 10, _mpz);
  QString ret_str = tmp_ptr;
#warning ok to free with delete?
  delete tmp_ptr;

  return ret_str;
}

QString const _knumfraction::ascii(void) const
{
  char const *tmp_ptr = mpq_get_str(0, 10, _mpq);
  QString ret_str = tmp_ptr;
#warning ok to free with delete?
  delete tmp_ptr;

  return mpq_get_str(0, 10, _mpq);

  return ret_str;
}

QString const _knumfloat::ascii(void) const
{
#warning this is just a stub
  mp_exp_t tmp_exp;

  char const *tmp_ptr = mpf_get_str(0, &tmp_exp, 10, 8, _mpf);
  QString ret_str = tmp_ptr;
#warning ok to free with delete?
  delete tmp_ptr;

  if(ret_str[0] == '-') {
    if(tmp_exp >= ret_str.length()-1)
      return ret_str;
    if (tmp_exp > 0)
      ret_str.insert(tmp_exp + 1, ".");
    else
      ret_str.insert(1,"0.");
  }
  else {
    if(tmp_exp >= ret_str.length())
      return ret_str;
    if (tmp_exp > 0)
      ret_str.insert(tmp_exp, ".");
    else
      ret_str.insert(0,"0.");
  }

  return ret_str;


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



_knumber * _knuminteger::sqrt(void) const
{
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
  
#warning implement sqrt
  //  mpq_sqrt(tmp_num->_mpq, _mpq);
  
  return tmp_num;
}

_knumber * _knumfloat::sqrt(void) const
{
  _knumfloat * tmp_num = new _knumfloat();
  
  mpf_sqrt(tmp_num->_mpf, _mpf);
  
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



_knumber * _knuminteger::reciprocal(void) const
{
#warning test if dividing by zero
  _knumfraction * tmp_num = new _knumfraction(*this);

  mpq_inv(tmp_num->_mpq, tmp_num->_mpq);

  return tmp_num;
}

_knumber * _knumfraction::reciprocal() const
{
#warning test if dividing by zero
  _knumfraction * tmp_num = new _knumfraction();
  
  mpq_inv(tmp_num->_mpq, _mpq);
  
  return tmp_num;
}

_knumber *_knumfloat::reciprocal(void) const
{
#warning test if dividing by zero
  _knumfloat * tmp_num = new _knumfloat();
  
  mpf_div(tmp_num->_mpf, _knumfloat("1.0")._mpf, _mpf);
  
  return tmp_num;
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

  
  if (arg2.type() == FloatType)
    return arg2.add(*this);
  
  _knumfraction * tmp_num = new _knumfraction();
  
  mpq_add(tmp_num->_mpq, _mpq,
	  dynamic_cast<_knumfraction const &>(arg2)._mpq);
  
  return tmp_num;
}

_knumber *_knumfloat::add(_knumber const & arg2) const
{
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

  
  if (arg2.type() == FloatType)
    return arg2.multiply(*this);
  
  _knumfraction * tmp_num = new _knumfraction();
  
  mpq_mul(tmp_num->_mpq, _mpq,
	  dynamic_cast<_knumfraction const &>(arg2)._mpq);
  
  return tmp_num;
}

_knumber *_knumfloat::multiply(_knumber const & arg2) const
{
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





_knumber * _knuminteger::divide(_knumber const & arg2) const
{
  _knumber * tmp_num = arg2.reciprocal();
  _knumber * rslt_num = this->multiply(*tmp_num);

  delete tmp_num;

  return rslt_num;
}

_knumber * _knumfraction::divide(_knumber const & arg2) const
{
  _knumber * tmp_num = arg2.reciprocal();
  _knumber * rslt_num = this->multiply(*tmp_num);
  
  delete tmp_num;
  
  return rslt_num;
}

_knumber *_knumfloat::divide(_knumber const & arg2) const
{
#warning test if dividing by zero  
  // automatically casts arg2 to float
  _knumfloat * tmp_num = new _knumfloat(arg2);

  mpf_div(tmp_num->_mpf, _mpf, tmp_num->_mpf);
  
  return tmp_num;
}


_knuminteger * _knuminteger::mod(_knuminteger const &arg2) const
{
#warning test if dividing by zero  
  _knuminteger * tmp_num = new _knuminteger();

  mpz_mod(tmp_num->_mpz, _mpz, arg2._mpz);
  
  return tmp_num;
}

#else  // !HAVE_GMP
static void knumber_priv_dummy()
{
}

#endif // HAVE_GMP

