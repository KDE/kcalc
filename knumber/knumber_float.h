/*
Copyright (C) 2001 - 2013 Evan Teran
                          evan.teran@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNUMBER_FLOAT_H_
#define KNUMBER_FLOAT_H_

#include "knumber_base.h"

class KNumber;

namespace detail {

class knumber_float : public knumber_base {
	friend class ::KNumber;
	friend class knumber_error;
	friend class knumber_integer;
	friend class knumber_fraction;
	
private:
#ifdef KNUMBER_USE_MPFR
	static const mpfr_rnd_t  rounding_mode;
	static const mpfr_prec_t precision;
#endif

public:
	explicit knumber_float(const QString &s);
	explicit knumber_float(double value);
#ifdef HAVE_LONG_DOUBLE
	explicit knumber_float(long double value);
#endif	
	
	explicit knumber_float(mpf_t mpf);
	virtual ~knumber_float();
	
private:
	// conversion constructors
	explicit knumber_float(const knumber_integer *value);
	explicit knumber_float(const knumber_fraction *value);
	explicit knumber_float(const knumber_float *value);
	explicit knumber_float(const knumber_error *value);

public:
	virtual QString toString(int precision) const;
	virtual quint64 toUint64() const;
	virtual qint64 toInt64() const;
	
public:
	virtual bool is_integer() const;
	virtual bool is_zero() const;
	virtual int sign() const;

public:
	virtual knumber_base *add(knumber_base *rhs);
	virtual knumber_base *sub(knumber_base *rhs);
	virtual knumber_base *mul(knumber_base *rhs);
	virtual knumber_base *div(knumber_base *rhs);
	virtual knumber_base *mod(knumber_base *rhs);
	
public:
	virtual knumber_base *pow(knumber_base *rhs);
	virtual knumber_base *neg();
	virtual knumber_base *cmp();
	virtual knumber_base *abs();
	virtual knumber_base *sqrt();
	virtual knumber_base *cbrt();
	virtual knumber_base *factorial();
	virtual knumber_base *reciprocal();
	
public:
	virtual knumber_base *log2();
	virtual knumber_base *log10();
	virtual knumber_base *ln();
	virtual knumber_base *exp2();
	virtual knumber_base *exp10();
	virtual knumber_base *exp();
	virtual knumber_base *bin(knumber_base *rhs);
	
public:
	virtual knumber_base *sin();
	virtual knumber_base *cos();
	virtual knumber_base *tan();
	virtual knumber_base *asin();
	virtual knumber_base *acos();
	virtual knumber_base *atan();
	virtual knumber_base *sinh();
	virtual knumber_base *cosh();
	virtual knumber_base *tanh();
	virtual knumber_base *asinh();
	virtual knumber_base *acosh();
	virtual knumber_base *atanh();
	
public:
	virtual int compare(knumber_base *rhs);
	
public:
	virtual knumber_base *bitwise_and(knumber_base *rhs);
	virtual knumber_base *bitwise_xor(knumber_base *rhs);
	virtual knumber_base *bitwise_or(knumber_base *rhs);
	virtual knumber_base *bitwise_shift(knumber_base *rhs);

public:
	virtual knumber_base *clone();
	
private:
	template <double F(double)>
	knumber_base *execute_libc_func(double x);

	template <double F(double, double)>
	knumber_base *execute_libc_func(double x, double y);
	
private:
	mpf_t mpf_;
};

}

#endif
