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
#ifndef KNUMBER_PRIV_H_
#define KNUMBER_PRIV_H_


#include <cstdio>
#include <gmp.h>
#include <QString>

#ifndef Q_CC_MSVC
// work-around for pre-C99-libs
#ifndef INFINITY
#define INFINITY  HUGE_VAL
#endif
// this is really ugly
#ifndef NAN
#define NAN (atof("nan"))
#endif
#endif

namespace detail {

class knumber
{
public:
    enum NumType {SpecialType, IntegerType, FractionType, FloatType};
    enum ErrorType {UndefinedNumber, Infinity, MinusInfinity};

    knumber() {}

    virtual ~knumber() {}

    virtual NumType type() const = 0;

    virtual QString const ascii(int prec = -1) const = 0;

    virtual knumber * abs() const = 0;
    virtual knumber * intPart() const = 0;
    virtual int sign() const = 0;
    virtual knumber * sqrt() const = 0;
    virtual knumber * cbrt() const = 0;
    virtual knumber * factorial() const = 0;
    virtual knumber * change_sign() const = 0;
    virtual knumber * reciprocal() const = 0;
    virtual knumber * add(knumber const & arg2) const = 0;
    virtual knumber * multiply(knumber const & arg2) const = 0;
    virtual knumber * divide(knumber const & arg2) const;

    virtual knumber * power(knumber const & exponent) const = 0;

    virtual int compare(knumber const &arg2) const = 0;

    virtual operator long int() const = 0;
    virtual operator unsigned long int() const = 0;
    virtual operator long long int() const = 0;
    virtual operator unsigned long long int() const = 0;
    virtual operator double() const = 0;
};


class knumerror : public knumber
{
public:
    knumerror(ErrorType error = UndefinedNumber)
            : error_(error) { }

    knumerror(knumber const & num);

    knumerror(const QString & num);

    //virtual ~knumerror() { }

    knumerror const & operator = (knumerror const & num);

    virtual NumType type() const {
        return SpecialType;
    }

    virtual QString const ascii(int prec = -1) const;

    virtual knumber * abs() const;
    virtual knumber * intPart() const;
    virtual int sign() const;
    virtual knumber * cbrt() const;
    virtual knumber * sqrt() const;
    virtual knumber * factorial() const;
    virtual knumber * change_sign() const;
    virtual knumber * reciprocal() const;
    virtual knumber * add(knumber const & arg2) const;
    virtual knumber * multiply(knumber const & arg2) const;

    virtual knumber * power(knumber const & exponent) const;

    virtual int compare(knumber const &arg2) const;

    virtual operator long int() const;
    virtual operator unsigned long int() const;
    virtual operator long long int() const;
    virtual operator unsigned long long int() const;
    virtual operator double() const;

private:

    ErrorType error_;

    friend class knuminteger;
    friend class knumfraction;
    friend class knumfloat;
};



class knuminteger : public knumber
{
public:
    knuminteger(qint32 num = 0) {
        mpz_init_set_si(mpz_, static_cast<signed long int>(num));
    }

    knuminteger(quint32 num) {
        mpz_init_set_ui(mpz_, static_cast<unsigned long int>(num));
    }

    knuminteger(qint64 num);

    knuminteger(quint64 num);

    knuminteger(knumber const & num);

    knuminteger(const QString & num);

    virtual ~knuminteger() {
        mpz_clear(mpz_);
    }

    knuminteger const & operator = (knuminteger const & num);

    virtual NumType type() const {
        return IntegerType;
    }

    virtual QString const ascii(int prec = -1) const;

    virtual knumber * abs() const;
    virtual knumber * intPart() const;
    virtual int sign() const;
    virtual knumber * cbrt() const;
    virtual knumber * sqrt() const;
    virtual knumber * factorial() const;
    virtual knumber * change_sign() const;
    virtual knumber * reciprocal() const;
    virtual knumber * add(knumber const & arg2) const;
    virtual knumber * multiply(knumber const & arg2) const;

    virtual int compare(knumber const &arg2) const;

    virtual knumber * power(knumber const & exponent) const;

    virtual operator long int () const;
    virtual operator unsigned long int () const;
    virtual operator long long int() const;
    virtual operator unsigned long long int() const;
    virtual operator double() const;

    knuminteger * intAnd(knuminteger const &arg2) const;
    knuminteger * intOr(knuminteger const &arg2) const;
    knumber * mod(knuminteger const &arg2) const;
    knumber * shift(knuminteger const &arg2) const;

private:
    mpz_t mpz_;

    friend class knumfraction;
    friend class knumfloat;
};



class knumfraction : public knumber
{
public:

    explicit knumfraction(signed long int nom = 0, signed long int denom = 1) {
        mpq_init(mpq_);
        mpq_set_si(mpq_, nom, denom);
        mpq_canonicalize(mpq_);
    }

    knumfraction(knumber const & num);

    knumfraction(QString const & num);

    virtual ~knumfraction() {
        mpq_clear(mpq_);
    }

    virtual NumType type() const {
        return FractionType;
    }

    virtual QString const ascii(int prec = -1) const;

    bool isInteger() const;

    virtual knumber * abs() const;
    virtual knumber * intPart() const;
    virtual int sign() const;
    virtual knumber * cbrt() const;
    virtual knumber * sqrt() const;
    virtual knumber * factorial() const;
    virtual knumber * change_sign() const;
    virtual knumber * reciprocal() const;
    virtual knumber * add(knumber const & arg2) const;
    virtual knumber * multiply(knumber const & arg2) const;

    virtual knumber * power(knumber const & exponent) const;

    virtual int compare(knumber const &arg2) const;

    virtual operator long int () const;
    virtual operator unsigned long int () const;
    virtual operator long long int() const;
    virtual operator unsigned long long int() const;
    virtual operator double() const;

private:
    mpq_t mpq_;

    friend class knuminteger;
    friend class knumfloat;
};

class knumfloat : public knumber
{
public:
    knumfloat(double num = 1.0) {
        mpf_init(mpf_);
        mpf_set_d(mpf_, num);
    }

    knumfloat(knumber const & num);

    knumfloat(QString const & num);

    virtual ~knumfloat() {
        mpf_clear(mpf_);
    }

    virtual NumType type() const {
        return FloatType;
    }

    virtual QString const ascii(int prec = -1) const;

    virtual knumber * abs() const;
    virtual knumber * intPart() const;
    virtual int sign() const;
    virtual knumber * cbrt() const;
    virtual knumber * sqrt() const;
    virtual knumber * factorial() const;
    virtual knumber * change_sign() const;
    virtual knumber * reciprocal() const;
    virtual knumber * add(knumber const & arg2) const;
    virtual knumber * multiply(knumber const & arg2) const;
    virtual knumber * divide(knumber const & arg2) const;

    virtual knumber * power(knumber const & exponent) const;

    virtual int compare(knumber const &arg2) const;

    virtual operator long int () const;
    virtual operator unsigned long int () const;
    virtual operator long long int() const;
    virtual operator unsigned long long int() const;
    virtual operator double() const;

private:
    mpf_t mpf_;

    friend class knuminteger;
    friend class knumfraction;
};
}

#endif // KNUMBER_PRIV_H_
