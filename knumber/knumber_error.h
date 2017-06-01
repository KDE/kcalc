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

#ifndef KNUMBER_ERROR_H_
#define KNUMBER_ERROR_H_

#include "knumber_base.h"

class KNumber;

namespace detail {

class knumber_error : public knumber_base {
	friend class ::KNumber;
	friend class knumber_integer;
	friend class knumber_fraction;
	friend class knumber_float;

public:
	enum Error {
		ERROR_UNDEFINED,
		ERROR_POS_INFINITY,
		ERROR_NEG_INFINITY
	};

public:
	explicit knumber_error(const QString &s);
	explicit knumber_error(Error e);
	knumber_error();
	virtual ~knumber_error();

public:
	QString toString(int precision) const Q_DECL_OVERRIDE;
	quint64 toUint64() const Q_DECL_OVERRIDE;
	qint64 toInt64() const Q_DECL_OVERRIDE;

public:
	bool is_integer() const Q_DECL_OVERRIDE;
	bool is_zero() const Q_DECL_OVERRIDE;
	int sign() const Q_DECL_OVERRIDE;

public:
	knumber_base *add(knumber_base *rhs) Q_DECL_OVERRIDE;
	knumber_base *sub(knumber_base *rhs) Q_DECL_OVERRIDE;
	knumber_base *mul(knumber_base *rhs) Q_DECL_OVERRIDE;
	knumber_base *div(knumber_base *rhs) Q_DECL_OVERRIDE;
	knumber_base *mod(knumber_base *rhs) Q_DECL_OVERRIDE;

public:
	knumber_base *bitwise_and(knumber_base *rhs) Q_DECL_OVERRIDE;
	knumber_base *bitwise_xor(knumber_base *rhs) Q_DECL_OVERRIDE;
	knumber_base *bitwise_or(knumber_base *rhs) Q_DECL_OVERRIDE;
	knumber_base *bitwise_shift(knumber_base *rhs) Q_DECL_OVERRIDE;

public:
	knumber_base *pow(knumber_base *rhs) Q_DECL_OVERRIDE;
	knumber_base *neg() Q_DECL_OVERRIDE;
	knumber_base *cmp() Q_DECL_OVERRIDE;
	knumber_base *abs() Q_DECL_OVERRIDE;
	knumber_base *sqrt() Q_DECL_OVERRIDE;
	knumber_base *cbrt() Q_DECL_OVERRIDE;
	knumber_base *factorial() Q_DECL_OVERRIDE;
	knumber_base *reciprocal() Q_DECL_OVERRIDE;
	knumber_base *tgamma() Q_DECL_OVERRIDE;

public:
	knumber_base *log2() Q_DECL_OVERRIDE;
	knumber_base *log10() Q_DECL_OVERRIDE;
	knumber_base *ln() Q_DECL_OVERRIDE;
	knumber_base *exp2() Q_DECL_OVERRIDE;
	knumber_base *exp10() Q_DECL_OVERRIDE;
	knumber_base *floor() Q_DECL_OVERRIDE;
	knumber_base *ceil() Q_DECL_OVERRIDE;
	knumber_base *exp() Q_DECL_OVERRIDE;
	knumber_base *bin(knumber_base *rhs) Q_DECL_OVERRIDE;

public:
	knumber_base *sin() Q_DECL_OVERRIDE;
	knumber_base *cos() Q_DECL_OVERRIDE;
	knumber_base *tan() Q_DECL_OVERRIDE;
	knumber_base *asin() Q_DECL_OVERRIDE;
	knumber_base *acos() Q_DECL_OVERRIDE;
	knumber_base *atan() Q_DECL_OVERRIDE;
	knumber_base *sinh() Q_DECL_OVERRIDE;
	knumber_base *cosh() Q_DECL_OVERRIDE;
	knumber_base *tanh() Q_DECL_OVERRIDE;
	knumber_base *asinh() Q_DECL_OVERRIDE;
	knumber_base *acosh() Q_DECL_OVERRIDE;
	knumber_base *atanh() Q_DECL_OVERRIDE;

public:
	int compare(knumber_base *rhs) Q_DECL_OVERRIDE;

private:
	// conversion constructors
	explicit knumber_error(const knumber_integer *value);
	explicit knumber_error(const knumber_fraction *value);
	explicit knumber_error(const knumber_float *value);
	explicit knumber_error(const knumber_error *value);

public:
	knumber_base *clone() Q_DECL_OVERRIDE;

private:
	Error error_;
};

}

#endif
