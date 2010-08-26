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

    virtual void copy(knumber const & num) = 0;

    virtual NumType type(void) const = 0;

    virtual QString const ascii(int prec = -1) const = 0;

    virtual knumber * abs(void) const = 0;
    virtual knumber * intPart(void) const = 0;
    virtual int sign(void) const = 0;
    virtual knumber * sqrt(void) const = 0;
    virtual knumber * cbrt(void) const = 0;
    virtual knumber * factorial(void) const = 0;
    virtual knumber * change_sign(void) const = 0;
    virtual knumber * reciprocal(void) const = 0;
    virtual knumber * add(knumber const & arg2) const = 0;
    virtual knumber * multiply(knumber const & arg2) const = 0;
    knumber * divide(knumber const & arg2) const;

    virtual knumber * power(knumber const & exponent) const = 0;

    virtual int compare(knumber const &arg2) const = 0;

    virtual operator long int(void) const = 0;
    virtual operator unsigned long int(void) const = 0;
    virtual operator long long int(void) const = 0;
    virtual operator unsigned long long int(void) const = 0;
    virtual operator double(void) const = 0;
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

    virtual void copy(knumber const & num) {
        error_ = dynamic_cast<knumerror const &>(num).error_;
    }

    virtual NumType type(void) const {
        return SpecialType;
    }

    virtual QString const ascii(int prec = -1) const;

    virtual knumber * abs(void) const;
    virtual knumber * intPart(void) const;
    virtual int sign(void) const;
    virtual knumber * cbrt(void) const;
    virtual knumber * sqrt(void) const;
    virtual knumber * factorial(void) const;
    virtual knumber * change_sign(void) const;
    virtual knumber * reciprocal(void) const;
    virtual knumber * add(knumber const & arg2) const;
    virtual knumber * multiply(knumber const & arg2) const;

    virtual knumber * power(knumber const & exponent) const;

    virtual int compare(knumber const &arg2) const;

    virtual operator long int(void) const;
    virtual operator unsigned long int(void) const;
    virtual operator long long int(void) const;
    virtual operator unsigned long long int(void) const;
    virtual operator double(void) const;

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

    virtual void copy(knumber const & num) {
        mpz_set(mpz_, dynamic_cast<knuminteger const &>(num).mpz_);
    }

    virtual NumType type(void) const {
        return IntegerType;
    }

    virtual QString const ascii(int prec = -1) const;

    virtual knumber * abs(void) const;
    virtual knumber * intPart(void) const;
    virtual int sign(void) const;
    virtual knumber * cbrt(void) const;
    virtual knumber * sqrt(void) const;
    virtual knumber * factorial(void) const;
    virtual knumber * change_sign(void) const;
    virtual knumber * reciprocal(void) const;
    virtual knumber * add(knumber const & arg2) const;
    virtual knumber * multiply(knumber const & arg2) const;

    virtual int compare(knumber const &arg2) const;

    virtual knumber * power(knumber const & exponent) const;

    virtual operator long int (void) const;
    virtual operator unsigned long int (void) const;
    virtual operator long long int(void) const;
    virtual operator unsigned long long int(void) const;
    virtual operator double(void) const;

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

    virtual void copy(knumber const & num) {
        mpq_set(mpq_, dynamic_cast<knumfraction const &>(num).mpq_);
    }

    virtual NumType type(void) const {
        return FractionType;
    }

    virtual QString const ascii(int prec = -1) const;

    bool isInteger(void) const;

    virtual knumber * abs(void) const;
    virtual knumber * intPart(void) const;
    virtual int sign(void) const;
    virtual knumber * cbrt(void) const;
    virtual knumber * sqrt(void) const;
    virtual knumber * factorial(void) const;
    virtual knumber * change_sign(void) const;
    virtual knumber * reciprocal(void) const;
    virtual knumber * add(knumber const & arg2) const;
    virtual knumber * multiply(knumber const & arg2) const;

    virtual knumber * power(knumber const & exponent) const;

    virtual int compare(knumber const &arg2) const;

    virtual operator long int (void) const;
    virtual operator unsigned long int (void) const;
    virtual operator long long int(void) const;
    virtual operator unsigned long long int(void) const;
    virtual operator double(void) const;

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

    virtual void copy(knumber const & num) {
        mpf_set(mpf_, dynamic_cast<knumfloat const &>(num).mpf_);
    }

    virtual NumType type(void) const {
        return FloatType;
    }

    virtual QString const ascii(int prec = -1) const;

    virtual knumber * abs(void) const;
    virtual knumber * intPart(void) const;
    virtual int sign(void) const;
    virtual knumber * cbrt(void) const;
    virtual knumber * sqrt(void) const;
    virtual knumber * factorial(void) const;
    virtual knumber * change_sign(void) const;
    virtual knumber * reciprocal(void) const;
    virtual knumber * add(knumber const & arg2) const;
    virtual knumber * multiply(knumber const & arg2) const;
    virtual knumber * divide(knumber const & arg2) const;

    virtual knumber * power(knumber const & exponent) const;

    virtual int compare(knumber const &arg2) const;

    virtual operator long int (void) const;
    virtual operator unsigned long int (void) const;
    virtual operator long long int(void) const;
    virtual operator unsigned long long int(void) const;
    virtual operator double(void) const;

private:
    mpf_t mpf_;

    friend class knuminteger;
    friend class knumfraction;
};
}

#endif // KNUMBER_PRIV_H_
