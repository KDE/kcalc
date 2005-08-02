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
#ifndef _KNUMBER_PRIV_H
#define _KNUMBER_PRIV_H

class QString;

#include <gmp.h>

class _knumber
{
 public:
  enum NumType {IntegerType, FractionType, FloatType};

  _knumber() {}

  virtual ~_knumber() {};

  virtual void copy(_knumber const & num) = 0;

  virtual NumType type(void) const = 0;

  virtual QString const ascii(void) const = 0;

  virtual _knumber * abs(void) const = 0;
  virtual _knumber * intPart(void) const = 0;
  virtual _knumber * sqrt(void) const = 0;
  virtual _knumber * change_sign(void) const = 0;
  virtual _knumber * reciprocal(void) const = 0;
  virtual _knumber * add(_knumber const & arg2) const = 0;
  virtual _knumber * multiply(_knumber const & arg2) const = 0;
  virtual _knumber * divide(_knumber const & arg2) const = 0;

};

class _knuminteger : public _knumber
{
 public:
  _knuminteger(signed int num = 0)
  {
    mpz_init_set_si(_mpz, num);
  }
  
  _knuminteger(signed long int num)
  {
    mpz_init_set_si(_mpz, num);
  }

  _knuminteger(_knumber const & num);

  _knuminteger(const QString & num);

  virtual ~_knuminteger()
  {
    mpz_clear(_mpz);
  }

  _knuminteger const & operator = (_knuminteger const & num);

  virtual void copy(_knumber const & num)
  {
    mpz_set(_mpz, dynamic_cast<_knuminteger const &>(num)._mpz);
  }

  virtual NumType type(void) const {return IntegerType;}

  virtual QString const ascii(void) const;

  virtual _knumber * abs(void) const;
  virtual _knumber * intPart(void) const;
  virtual _knumber * sqrt(void) const;
  virtual _knumber * change_sign(void) const;
  virtual _knumber * reciprocal(void) const;
  virtual _knumber * add(_knumber const & arg2) const;
  virtual _knumber * multiply(_knumber const & arg2) const;
  virtual _knumber * divide(_knumber const & arg2) const;

  _knuminteger * intAnd(_knuminteger const &arg2) const;
  _knuminteger * intOr(_knuminteger const &arg2) const;
  _knuminteger * mod(_knuminteger const &arg2) const;
  _knuminteger * leftShift(_knuminteger const &arg2) const;
  _knuminteger * rightShift(_knuminteger const &arg2) const;
  
 private:
  mpz_t _mpz;

  friend class _knumfraction;
  friend class _knumfloat;
};



class _knumfraction : public _knumber
{
 public:
  
  _knumfraction(signed long int nom = 0, signed long int denom = 1)
  {
    mpq_init(_mpq);
    mpq_set_si(_mpq, nom, denom);
    mpq_canonicalize(_mpq);
  }

  _knumfraction(_knumber const & num);
  
  _knumfraction(QString const & num);

  virtual ~_knumfraction()
  {
    mpq_clear(_mpq);
  }
  
  virtual void copy(_knumber const & num)
  {
    mpq_set(_mpq, dynamic_cast<_knumfraction const &>(num)._mpq);
  }

  virtual NumType type(void) const {return FractionType;}

  virtual QString const ascii(void) const;

  virtual _knumber * abs(void) const;
  virtual _knumber * intPart(void) const;
  virtual _knumber * sqrt(void) const;
  virtual _knumber * change_sign(void) const;
  virtual _knumber * reciprocal(void) const;
  virtual _knumber * add(_knumber const & arg2) const;
  virtual _knumber * multiply(_knumber const & arg2) const;
  virtual _knumber * divide(_knumber const & arg2) const;
  
 private:
  mpq_t _mpq;

  friend class _knuminteger;
  friend class _knumfloat;
};

class _knumfloat : public _knumber
{
 public:
  _knumfloat(double num = 1.0)
  {
    mpf_init(_mpf);
    mpf_set_d(_mpf, num);
  }
  
  _knumfloat(_knumber const & num);

  _knumfloat(QString const & num);
  
  virtual ~_knumfloat()
  {
    mpf_clear(_mpf);
  }

  virtual void copy(_knumber const & num)
  {
    mpf_set(_mpf, dynamic_cast<_knumfloat const &>(num)._mpf);
  }

  virtual NumType type(void) const {return FloatType;}

  virtual QString const ascii(void) const;

  virtual _knumber * abs(void) const;
  virtual _knumber * intPart(void) const;
  virtual _knumber * sqrt(void) const;
  virtual _knumber * change_sign(void) const;
  virtual _knumber * reciprocal(void) const;
  virtual _knumber * add(_knumber const & arg2) const;
  virtual _knumber * multiply(_knumber const & arg2) const;
  virtual _knumber * divide(_knumber const & arg2) const;

 private:
    mpf_t _mpf;

  friend class _knuminteger;
  friend class _knumfraction;
};


#endif //  _KNUMBER_PRIV_H
