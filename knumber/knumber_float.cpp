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
#include <QScopedArrayPointer>
#include <QDebug>
#include <cmath>

// NOTE: these assume IEEE floats..
#ifndef HAVE_FUNC_ISINF
#define isinf(x) ((x) != 0.0 && (x) + (x) == (x))
#endif

#ifndef HAVE_FUNC_ISNAN
#define isnan(x) ((x) != (x))
#endif

namespace detail {

#ifdef KNUMBER_USE_MPFR
const mpfr_rnd_t  knumber_float::rounding_mode = MPFR_RNDN;
const mpfr_prec_t knumber_float::precision     = 1024;
#endif

template <double F(double)>
knumber_base *knumber_float::execute_libc_func(double x) {
	const double r = F(x);
	if(isnan(r)) {
		knumber_error *e = new knumber_error(knumber_error::ERROR_UNDEFINED);
		delete this;
		return e;
	} else if(isinf(r)) {
		knumber_error *e = new knumber_error(knumber_error::ERROR_POS_INFINITY);
		delete this;
		return e;
	} else {
		mpf_set_d(mpf_, r);
		return this;
	}
}

template <double F(double, double)>
knumber_base *knumber_float::execute_libc_func(double x, double y) {
	const double r = F(x, y);
	if(isnan(r)) {
		knumber_error *e = new knumber_error(knumber_error::ERROR_UNDEFINED);
		delete this;
		return e;
	} else if(isinf(r)) {
		knumber_error *e = new knumber_error(knumber_error::ERROR_POS_INFINITY);
		delete this;
		return e;
	} else {
		mpf_set_d(mpf_, r);
		return this;
	}
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_float::knumber_float(const QString &s) {

	mpf_init(mpf_);
	mpf_set_str(mpf_, s.toAscii(), 10);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_float::knumber_float(double value) {

	Q_ASSERT(!isinf(value));
	Q_ASSERT(!isnan(value));

	mpf_init_set_d(mpf_, value);
}

#ifdef HAVE_LONG_DOUBLE
//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_float::knumber_float(long double value) {

	Q_ASSERT(!isinf(value));
	Q_ASSERT(!isnan(value));

	mpf_init_set_d(mpf_, value);
}
#endif

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_float::knumber_float(mpf_t mpf) {

	mpf_init(mpf_);
	mpf_set(mpf_, mpf);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_float::knumber_float(const knumber_float *value) {

	mpf_init_set(mpf_, value->mpf_);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_float::knumber_float(const knumber_integer *value) {

	mpf_init(mpf_);
	mpf_set_z(mpf_, value->mpz_);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_float::knumber_float(const knumber_fraction *value) {

	mpf_init(mpf_);
	mpf_set_q(mpf_, value->mpq_);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::clone() {

	return new knumber_float(this);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_float::~knumber_float() {

	mpf_clear(mpf_);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::add(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		knumber_float f(p);
		return add(&f);
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		mpf_add(mpf_, mpf_, p->mpf_);
		return this;
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		knumber_float f(p);
		return add(&f);
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		knumber_error *e = new knumber_error(p);
		delete this;
		return e;
	}

	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::sub(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		knumber_float f(p);
		return sub(&f);
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		mpf_sub(mpf_, mpf_, p->mpf_);
		return this;
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		knumber_float f(p);
		return sub(&f);
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		knumber_error *e = new knumber_error(p);
		delete this;
		return e->neg();
	}

	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::mul(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		knumber_float f(p);
		return mul(&f);
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		mpf_mul(mpf_, mpf_, p->mpf_);
		return this;
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		knumber_float f(p);
		return mul(&f);
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		if(is_zero()) {
			delete this;
			return new knumber_error(knumber_error::ERROR_UNDEFINED);
		}

		if(sign() < 0) {
			delete this;
			knumber_error *e = new knumber_error(p);
			return e->neg();
		} else {
			delete this;
			return new knumber_error(p);
		}
	}

	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::div(knumber_base *rhs) {

	if(rhs->is_zero()) {
		if(sign() < 0) {
			delete this;
			return new knumber_error(knumber_error::ERROR_NEG_INFINITY);
		} else {
			delete this;
			return new knumber_error(knumber_error::ERROR_POS_INFINITY);
		}
	}

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		knumber_float f(p);
		return div(&f);
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		mpf_div(mpf_, mpf_, p->mpf_);
		return this;
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		knumber_float f(p);
		return div(&f);
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		if(p->sign() > 0 || p->sign() < 0) {
			delete this;
			return new knumber_integer(0);
		}

		delete this;
		return new knumber_error(p);
	}

	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::mod(knumber_base *rhs) {

	Q_UNUSED(rhs);

	if(rhs->is_zero()) {
		delete this;
		return new knumber_error(knumber_error::ERROR_UNDEFINED);
	}

	delete this;
	return new knumber_integer(0);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::bitwise_and(knumber_base *rhs) {

	Q_UNUSED(rhs);
	delete this;
	return new knumber_integer(0);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::bitwise_xor(knumber_base *rhs) {

	Q_UNUSED(rhs);
	delete this;
	return new knumber_integer(0);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::bitwise_or(knumber_base *rhs) {

	Q_UNUSED(rhs);
	delete this;
	return new knumber_integer(0);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::bitwise_shift(knumber_base *rhs) {

	Q_UNUSED(rhs);
	delete this;
	return new knumber_integer(0);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::neg() {

	mpf_neg(mpf_, mpf_);
	return this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::cmp() {

	delete this;
	return new knumber_error(knumber_error::ERROR_UNDEFINED);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::abs() {

	mpf_abs(mpf_, mpf_);
	return this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::sqrt() {

	if(sign() < 0) {
		delete this;
		return new knumber_error(knumber_error::ERROR_UNDEFINED);
	}

#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_sqrt(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
#else
	mpf_sqrt(mpf_, mpf_);
#endif
	return this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::cbrt() {

#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_cbrt(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
#else
	const double x = mpf_get_d(mpf_);
	if(isinf(x)) {
		delete this;
		return new knumber_error(knumber_error::ERROR_POS_INFINITY);
	} else {
#ifdef Q_CC_MSVC
		return execute_libc_func< ::pow>(x, 1.0 / 3.0);
#else
		return execute_libc_func< ::cbrt>(x);
#endif
	}
#endif
	return this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::factorial() {

	if(sign() < 0) {
		delete this;
		return new knumber_error(knumber_error::ERROR_UNDEFINED);
	}

	knumber_integer *i = new knumber_integer(this);
	delete this;
	return i->factorial();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::sin() {

#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_sin(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
	return this;
#else
	const double x = mpf_get_d(mpf_);
	if(isinf(x)) {
		delete this;
		return new knumber_error(knumber_error::ERROR_POS_INFINITY);
	} else {
		return execute_libc_func< ::sin>(x);
	}
#endif

}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::cos() {

#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_cos(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
	return this;
#else
	const double x = mpf_get_d(mpf_);
	if(isinf(x)) {
		delete this;
		return new knumber_error(knumber_error::ERROR_POS_INFINITY);
	} else {
		return execute_libc_func< ::cos>(x);
	}
#endif
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::tan() {

#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_tan(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
	return this;
#else
	const double x = mpf_get_d(mpf_);
	if(isinf(x)) {
		delete this;
		return new knumber_error(knumber_error::ERROR_POS_INFINITY);
	} else {
		return execute_libc_func< ::tan>(x);
	}
#endif
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::asin() {
	if(mpf_cmp_d(mpf_, 1.0) > 0 || mpf_cmp_d(mpf_, -1.0) < 0) {
		delete this;
		return new knumber_error(knumber_error::ERROR_UNDEFINED);
	}

#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_asin(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
	return this;
#else
	const double x = mpf_get_d(mpf_);
	if(isinf(x)) {
		delete this;
		return new knumber_error(knumber_error::ERROR_POS_INFINITY);
	} else {
		return execute_libc_func< ::asin>(x);
	}
#endif
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------s
knumber_base *knumber_float::acos() {
	if(mpf_cmp_d(mpf_, 1.0) > 0 || mpf_cmp_d(mpf_, -1.0) < 0) {
		delete this;
		return new knumber_error(knumber_error::ERROR_UNDEFINED);
	}

#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_acos(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
	return this;
#else
	const double x = mpf_get_d(mpf_);
	if(isinf(x)) {
		delete this;
		return new knumber_error(knumber_error::ERROR_POS_INFINITY);
	} else {
		return execute_libc_func< ::acos>(x);
	}
#endif
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::atan() {

#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_atan(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
	return this;
#else
	const double x = mpf_get_d(mpf_);
	if(isinf(x)) {
		delete this;
		return new knumber_error(knumber_error::ERROR_POS_INFINITY);
	} else {
		return execute_libc_func< ::atan>(x);
	}
#endif
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::sinh() {
#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_sinh(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
	return this;
#else
	const double x = mpf_get_d(mpf_);
	return execute_libc_func< ::sinh>(x);
#endif

}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::cosh() {
#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_cosh(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
	return this;
#else
	const double x = mpf_get_d(mpf_);
	return execute_libc_func< ::cosh>(x);
#endif
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::tanh() {
#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_tanh(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
	return this;
#else
	const double x = mpf_get_d(mpf_);
	return execute_libc_func< ::tanh>(x);
#endif
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::asinh() {
#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_asinh(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
	return this;
#else
	const double x = mpf_get_d(mpf_);
	return execute_libc_func< ::asinh>(x);
#endif
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::acosh() {
#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_acosh(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
	return this;
#else
	const double x = mpf_get_d(mpf_);
	return execute_libc_func< ::acosh>(x);
#endif
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::atanh() {
#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_atanh(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
	return this;
#else
	const double x = mpf_get_d(mpf_);
	return execute_libc_func< ::atanh>(x);
#endif
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::pow(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		mpf_pow_ui(mpf_, mpf_, mpz_get_ui(p->mpz_));

		if(p->sign() < 0) {
			return reciprocal();
		} else {
			return this;
		}
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		return execute_libc_func< ::pow>(mpf_get_d(mpf_), mpf_get_d(p->mpf_));
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		knumber_float f(p);
		return execute_libc_func< ::pow>(mpf_get_d(mpf_), mpf_get_d(f.mpf_));
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		if(p->sign() > 0) {
			knumber_error *e = new knumber_error(knumber_error::ERROR_POS_INFINITY);
			delete this;
			return e;
		} else if(p->sign() < 0) {
			knumber_integer *n = new knumber_integer(0);
			delete this;
			return n;
		} else {
			knumber_error *e = new knumber_error(knumber_error::ERROR_UNDEFINED);
			delete this;
			return e;
		}
	}

	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
int knumber_float::compare(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		knumber_float f(p);
		return compare(&f);
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		return mpf_cmp(mpf_, p->mpf_);
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		knumber_float f(p);
		return compare(&f);
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		// NOTE: any number compared to NaN/Inf/-Inf always compares less
		//       at the moment
		return -1;
	}

	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
QString knumber_float::toString(int precision) const {

	size_t size;
	if (precision > 0) {
		size = gmp_snprintf(NULL, 0, "%.*Fg", precision, mpf_) + 1;
	} else {
		size = gmp_snprintf(NULL, 0, "%.Fg", mpf_) + 1;
	}

	QScopedArrayPointer<char> buf(new char[size]);

	if (precision > 0) {
		gmp_snprintf(&buf[0], size, "%.*Fg", precision, mpf_);
	} else {
		gmp_snprintf(&buf[0], size, "%.Fg", mpf_);
	}

	return QLatin1String(&buf[0]);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
bool knumber_float::is_integer() const {

	return mpf_integer_p(mpf_) != 0;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
bool knumber_float::is_zero() const {

	return mpf_sgn(mpf_) == 0;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
int knumber_float::sign() const {

	return mpf_sgn(mpf_);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::reciprocal() {

	mpf_t mpf;
	mpf_init_set_d(mpf, 1.0);
	mpf_div(mpf_, mpf, mpf_);
	return this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::log2() {
#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_log2(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
	return this;
#else
	const double x = mpf_get_d(mpf_);
	if(isinf(x)) {
		delete this;
		return new knumber_error(knumber_error::ERROR_POS_INFINITY);
	} else {
		return execute_libc_func< ::log2>(x);
	}
#endif
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::log10() {
#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_log10(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
	return this;
#else
	const double x = mpf_get_d(mpf_);
	if(isinf(x)) {
		delete this;
		return new knumber_error(knumber_error::ERROR_POS_INFINITY);
	} else {
		return execute_libc_func< ::log10>(x);
	}
#endif
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::ln() {
#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_log(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
	return this;
#else
	const double x = mpf_get_d(mpf_);
	if(isinf(x)) {
		delete this;
		return new knumber_error(knumber_error::ERROR_POS_INFINITY);
	} else {
		return execute_libc_func< ::log>(x);
	}
#endif
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::exp2() {
#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_exp2(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
	return this;
#else
	const double x = mpf_get_d(mpf_);
	if(isinf(x)) {
		delete this;
		return new knumber_error(knumber_error::ERROR_POS_INFINITY);
	} else {
		return execute_libc_func< ::exp2>(x);
	}
#endif
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::exp10() {
#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_exp10(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
	return this;
#else
	const double x = mpf_get_d(mpf_);
	if(isinf(x)) {
		delete this;
		return new knumber_error(knumber_error::ERROR_POS_INFINITY);
	} else {
		return execute_libc_func< ::exp10>(x);
	}
#endif
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::exp() {
#ifdef KNUMBER_USE_MPFR
	mpfr_t mpfr;
	mpfr_init_set_f(mpfr, mpf_, rounding_mode);
	mpfr_exp(mpfr, mpfr, rounding_mode);
	mpfr_get_f(mpf_, mpfr, rounding_mode);
	mpfr_clear(mpfr);
	return this;
#else
	const double x = mpf_get_d(mpf_);
	if(isinf(x)) {
		delete this;
		return new knumber_error(knumber_error::ERROR_POS_INFINITY);
	} else {
		return execute_libc_func< ::exp>(x);
	}
#endif
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
quint64 knumber_float::toUint64() const {
	return knumber_integer(this).toUint64();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
qint64 knumber_float::toInt64() const {
	return knumber_integer(this).toInt64();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::bin(knumber_base *rhs) {
	Q_UNUSED(rhs);
	delete this;
	return new knumber_error(knumber_error::ERROR_UNDEFINED);
}

}
