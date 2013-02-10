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

#ifndef KNUMBER_BASE_H_
#define KNUMBER_BASE_H_

#include <gmp.h>

#ifdef KNUMBER_USE_MPFR
#include <mpfr.h>
#endif

#include <QtGlobal>
#include <QString>

namespace detail {

class knumber_error;
class knumber_integer;
class knumber_fraction;
class knumber_float;

class knumber_base {
public:
	virtual ~knumber_base() { }
	
public:
	virtual knumber_base *clone() = 0;

public:
	virtual QString toString(int precision) const = 0;
	virtual quint64 toUint64() const = 0;
	virtual qint64 toInt64() const = 0;
	
public:
	virtual bool is_integer() const = 0;
	virtual bool is_zero() const = 0;
	virtual int sign() const = 0;

public:
	// basic math
	virtual knumber_base *add(knumber_base *rhs) = 0;
	virtual knumber_base *sub(knumber_base *rhs) = 0;
	virtual knumber_base *mul(knumber_base *rhs) = 0;
	virtual knumber_base *div(knumber_base *rhs) = 0;
	virtual knumber_base *mod(knumber_base *rhs) = 0;
	
public:
	// logical operators
	virtual knumber_base *bitwise_and(knumber_base *rhs) = 0;
	virtual knumber_base *bitwise_xor(knumber_base *rhs) = 0;
	virtual knumber_base *bitwise_or(knumber_base *rhs) = 0;
	virtual knumber_base *bitwise_shift(knumber_base *rhs) = 0;
	
public:
	// algebraic functions
	virtual knumber_base *pow(knumber_base *rhs) = 0;
	virtual knumber_base *neg() = 0;
	virtual knumber_base *cmp() = 0;
	virtual knumber_base *abs() = 0;
	virtual knumber_base *sqrt() = 0;
	virtual knumber_base *cbrt() = 0;
	virtual knumber_base *factorial() = 0;
	virtual knumber_base *reciprocal() = 0;
	
public:
	// special functions
	virtual knumber_base *log2() = 0;
	virtual knumber_base *log10() = 0;
	virtual knumber_base *ln() = 0;
	virtual knumber_base *exp2() = 0;
	virtual knumber_base *exp10() = 0;
	virtual knumber_base *exp() = 0;
	virtual knumber_base *bin(knumber_base *rhs) = 0;
	
public:
	// trig functions
	virtual knumber_base *sin() = 0;
	virtual knumber_base *cos() = 0;
	virtual knumber_base *tan() = 0;
	virtual knumber_base *asin() = 0;
	virtual knumber_base *acos() = 0;
	virtual knumber_base *atan() = 0;
	virtual knumber_base *sinh() = 0;
	virtual knumber_base *cosh() = 0;
	virtual knumber_base *tanh() = 0;
	virtual knumber_base *asinh() = 0;
	virtual knumber_base *acosh() = 0;
	virtual knumber_base *atanh() = 0;
	
public:
	// comparison
	virtual int compare(knumber_base *rhs) = 0;
};

}

#endif
