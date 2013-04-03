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

#include <config-kcalc.h>
#include "knumber_integer.h"
#include "knumber_float.h"
#include "knumber_fraction.h"
#include "knumber_error.h"
#include <cmath> // for M_PI
#include <QDebug>

namespace detail {


//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_error::knumber_error(Error e) : error_(e) {
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_error::knumber_error(const QString &s) {

    if (s == QLatin1String("nan"))       error_ = ERROR_UNDEFINED;
    else if (s == QLatin1String("inf"))  error_ = ERROR_POS_INFINITY;
    else if (s == QLatin1String("-inf")) error_ = ERROR_NEG_INFINITY;
    else                                 error_ = ERROR_UNDEFINED;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_error::knumber_error() : error_(ERROR_UNDEFINED) {

}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_error::~knumber_error() {

}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_error::knumber_error(const knumber_integer *) : error_(ERROR_UNDEFINED) {
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_error::knumber_error(const knumber_fraction *) : error_(ERROR_UNDEFINED) {
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_error::knumber_error(const knumber_float *) : error_(ERROR_UNDEFINED) {
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_error::knumber_error(const knumber_error *value) : error_(value->error_) {
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
QString knumber_error::toString(int precision) const {

	Q_UNUSED(precision);

	switch(error_) {
	case ERROR_POS_INFINITY:
		return QLatin1String("inf");
	case ERROR_NEG_INFINITY:
		return QLatin1String("-inf");
	case ERROR_UNDEFINED:
	default:
		return QLatin1String("nan");
	}
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::add(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		Q_UNUSED(p);
		return this;
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		Q_UNUSED(p);
		return this;
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		Q_UNUSED(p);
		return this;
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		if(error_ == ERROR_POS_INFINITY && p->error_ == ERROR_NEG_INFINITY) {
			error_ = ERROR_UNDEFINED;
		} else if(error_ == ERROR_NEG_INFINITY && p->error_ == ERROR_POS_INFINITY) {
			error_ = ERROR_UNDEFINED;
		} else if(p->error_ == ERROR_UNDEFINED) {
			error_ = ERROR_UNDEFINED;
		}
		return this;
	}
	
	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::sub(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		Q_UNUSED(p);
		return this;
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		Q_UNUSED(p);
		return this;
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		Q_UNUSED(p);
		return this;
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		if(error_ == ERROR_POS_INFINITY && p->error_ == ERROR_POS_INFINITY) {
			error_ = ERROR_UNDEFINED;
		} else if(error_ == ERROR_NEG_INFINITY && p->error_ == ERROR_NEG_INFINITY) {
			error_ = ERROR_UNDEFINED;
		} else if(p->error_ == ERROR_UNDEFINED) {
			error_ = ERROR_UNDEFINED;
		}
		return this;
	}
	
	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::mul(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		if(p->is_zero()) {
			error_ = ERROR_UNDEFINED;
		}
		return this;
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		if(p->is_zero()) {
			error_ = ERROR_UNDEFINED;
		}
		return this;
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		if(p->is_zero()) {
			error_ = ERROR_UNDEFINED;
		}
		return this;
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		if(error_ == ERROR_POS_INFINITY && p->error_ == ERROR_NEG_INFINITY) {
			error_ = ERROR_NEG_INFINITY;
		} else if(error_ == ERROR_NEG_INFINITY && p->error_ == ERROR_POS_INFINITY) {
			error_ = ERROR_NEG_INFINITY;
		} else if(error_ == ERROR_NEG_INFINITY && p->error_ == ERROR_NEG_INFINITY) {
			error_ = ERROR_POS_INFINITY;
		} else if(p->error_ == ERROR_UNDEFINED) {
			error_ = ERROR_UNDEFINED;
		}
		return this;
	}
	
	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::div(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		Q_UNUSED(p);
		return this;
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		Q_UNUSED(p);
		return this;
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		Q_UNUSED(p);
		return this;
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		Q_UNUSED(p);
		error_ = ERROR_UNDEFINED;
		return this;
	}
	
	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::mod(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		Q_UNUSED(p);
		return this;
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		Q_UNUSED(p);
		return this;
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		Q_UNUSED(p);
		return this;
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		Q_UNUSED(p);
		error_ = ERROR_UNDEFINED;
		return this;
	}
	
	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::pow(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		Q_UNUSED(p);
		return this;
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		Q_UNUSED(p);
		return this;
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		Q_UNUSED(p);
		return this;
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		
		switch(error_) {
		case ERROR_POS_INFINITY:
			if(p->sign() > 0) {
				return this;
			} else if(p->sign() < 0) {
				knumber_integer *n = new knumber_integer(0);
				delete this;
				return n;
			} else {
				error_ = ERROR_UNDEFINED;
				return this;
			}
			break;
		case ERROR_NEG_INFINITY:
			if(p->sign() > 0) {
				error_ = ERROR_POS_INFINITY;
				return this;
			} else if(p->sign() < 0) {
				knumber_integer *n = new knumber_integer(0);
				delete this;
				return n;
			} else {
				error_ = ERROR_UNDEFINED;
				return this;
			}
			break;
		case ERROR_UNDEFINED:
			return this;
		}
	}
	
	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::neg() {

	switch(error_) {
	case ERROR_POS_INFINITY:
		error_ = ERROR_NEG_INFINITY;
		break;
	case ERROR_NEG_INFINITY:
		error_ = ERROR_POS_INFINITY;
		break;
	case ERROR_UNDEFINED:
	default:
		break;
	}
	
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::cmp() {

	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::abs() {

	switch(error_) {
	case ERROR_NEG_INFINITY:
		error_ = ERROR_POS_INFINITY;
		break;
	case ERROR_POS_INFINITY:
	case ERROR_UNDEFINED:
	default:
		break;
	}
	
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::sqrt() {

	switch(error_) {
	case ERROR_NEG_INFINITY:
		error_ = ERROR_UNDEFINED;
		break;
	case ERROR_POS_INFINITY:
	case ERROR_UNDEFINED:
	default:
		break;
	}
	
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::cbrt() {

	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::factorial() {
	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::sin() {

	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::cos() {

	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::tan() {

	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::asin() {

	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::acos() {

	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::atan() {
	
	switch(error_) {
    case ERROR_POS_INFINITY:
		delete this;
		return new knumber_float(M_PI / 2.0);
    case ERROR_NEG_INFINITY:
		delete this;
		return new knumber_float(-M_PI / 2.0);
	case ERROR_UNDEFINED:
	default:
		return this;
	}
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::sinh() {

	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::cosh() {

	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::tanh() {

	if(sign() > 0) {
		delete this;
		return new knumber_integer(1);
	} else if(sign() < 0) {
		delete this;
		return new knumber_integer(-1);
	} else {
		return this;
	}
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::asinh() {

	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::acosh() {
	
	if(sign() < 0) {
		error_ = ERROR_UNDEFINED;
	}
	
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::atanh() {

	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
int knumber_error::compare(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		if(sign() > 0) {
			return 1;
		} else {
			return -1;
		}
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		if(sign() > 0) {
			return 1;
		} else {
			return -1;
		}
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		if(sign() > 0) {
			return 1;
		} else {
			return -1;
		}
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		return sign() == p->sign();
	}
	
	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::clone() {
	return new knumber_error(this);
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::bitwise_and(knumber_base *rhs) {
	Q_UNUSED(rhs);
	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::bitwise_xor(knumber_base *rhs) {
	Q_UNUSED(rhs);
	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::bitwise_or(knumber_base *rhs) {
	Q_UNUSED(rhs);
	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::bitwise_shift(knumber_base *rhs) {
	Q_UNUSED(rhs);
	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
bool knumber_error::is_integer() const {

	return false;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
bool knumber_error::is_zero() const {

	return false;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
int knumber_error::sign() const {

	switch(error_) {
	case ERROR_POS_INFINITY:
		return +1;
	case ERROR_NEG_INFINITY:
		return -1;
	case ERROR_UNDEFINED:
	default:
		return 0;
	}
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::reciprocal() {

	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::log2() {
	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::log10() {
	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::ln() {
	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::exp2() {
	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::exp10() {
	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::exp() {
	error_ = ERROR_UNDEFINED;
	return this;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
quint64 knumber_error::toUint64() const {
	return 0;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
qint64 knumber_error::toInt64() const {
	return 0;
}

//------------------------------------------------------------------------------
// Name: 
//------------------------------------------------------------------------------
knumber_base *knumber_error::bin(knumber_base *rhs) {
	Q_UNUSED(rhs);
	error_ = ERROR_UNDEFINED;
	return this;
}

}
