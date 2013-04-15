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

namespace detail {

bool knumber_fraction::default_fractional_input              = false;
bool knumber_fraction::default_fractional_output             = true;
bool knumber_fraction::split_off_integer_for_fraction_output = false;

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void knumber_fraction::set_default_fractional_input(bool value) {
	default_fractional_input = value;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void knumber_fraction::set_default_fractional_output(bool value) {
	default_fractional_output = value;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void knumber_fraction::set_split_off_integer_for_fraction_output(bool value) {
	split_off_integer_for_fraction_output = value;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_fraction::knumber_fraction(const QString &s) {
	mpq_init(mpq_);
	mpq_set_str(mpq_, s.toAscii(), 10);
	mpq_canonicalize(mpq_);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_fraction::knumber_fraction(qint64 num, quint64 den) {
	mpq_init(mpq_);
	mpq_set_si(mpq_, num, den);
	mpq_canonicalize(mpq_);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_fraction::knumber_fraction(quint64 num, quint64 den) {
	mpq_init(mpq_);
	mpq_set_ui(mpq_, num, den);
	mpq_canonicalize(mpq_);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_fraction::knumber_fraction(mpq_t mpq) {
	mpq_init(mpq_);
	mpq_set(mpq_, mpq);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_fraction::knumber_fraction(const knumber_fraction *value) {
	mpq_init(mpq_);
	mpq_set(mpq_, value->mpq_);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_fraction::knumber_fraction(const knumber_integer *value) {
	mpq_init(mpq_);
	mpq_set_z(mpq_, value->mpz_);
}

#if 0
//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_fraction::knumber_fraction(const knumber_float *value) {
	mpq_init(mpq_);
	mpq_set_f(mpq_, value->mpf_);
}
#endif

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::clone() {
	return new knumber_fraction(this);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_fraction::~knumber_fraction() {
	mpq_clear(mpq_);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
bool knumber_fraction::is_integer() const {
	return (mpz_cmp_ui(mpq_denref(mpq_), 1) == 0);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::add(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		knumber_fraction q(p);
		mpq_add(mpq_, mpq_, q.mpq_);
		return this;
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		knumber_float *f = new knumber_float(this);
		delete this;
		return f->add(p);
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		mpq_add(mpq_, mpq_, p->mpq_);
		return this;
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
knumber_base *knumber_fraction::sub(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		knumber_fraction q(p);
		mpq_sub(mpq_, mpq_, q.mpq_);
		return this;
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		knumber_float *f = new knumber_float(this);
		delete this;
		return f->sub(p);
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		mpq_sub(mpq_, mpq_, p->mpq_);
		return this;
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
knumber_base *knumber_fraction::mul(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		knumber_fraction q(p);
		mpq_mul(mpq_, mpq_, q.mpq_);
		return this;
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		knumber_float *q = new knumber_float(this);
		delete this;
		return q->mul(p);
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		mpq_mul(mpq_, mpq_, p->mpq_);
		return this;
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		if(is_zero()) {
			delete this;
			knumber_error *e = new knumber_error(knumber_error::ERROR_UNDEFINED);
			return e;
		}

		if(sign() < 0) {
			delete this;
			knumber_error *e = new knumber_error(p);
			return e->neg();
		} else {
			delete this;
			knumber_error *e = new knumber_error(p);
			return e;
		}
	}

	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::div(knumber_base *rhs) {

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
		knumber_fraction f(p);
		return div(&f);
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		knumber_float *f = new knumber_float(this);
		delete this;
		return f->div(p);
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		mpq_div(mpq_, mpq_, p->mpq_);
		return this;
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {

		if(p->sign() > 0) {
			delete this;
			return new knumber_integer(0);
		} else if(p->sign() < 0) {
			delete this;
			return new knumber_integer(0);
		}

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
knumber_base *knumber_fraction::mod(knumber_base *rhs) {

	if(rhs->is_zero()) {
		delete this;
		return new knumber_error(knumber_error::ERROR_UNDEFINED);
	}

	// NOTE: we don't support modulus operations with non-integer operands
	mpq_set_d(mpq_, 0);
	return this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::bitwise_and(knumber_base *rhs) {

	Q_UNUSED(rhs);
	delete this;
	// NOTE: we don't support bitwise operations with non-integer operands
	return new knumber_integer(0);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::bitwise_xor(knumber_base *rhs) {

	Q_UNUSED(rhs);
	delete this;
	// NOTE: we don't support bitwise operations with non-integer operands
	return new knumber_integer(0);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::bitwise_or(knumber_base *rhs) {

	Q_UNUSED(rhs);
	delete this;
	// NOTE: we don't support bitwise operations with non-integer operands
	return new knumber_integer(0);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::bitwise_shift(knumber_base *rhs) {

	Q_UNUSED(rhs);
	delete this;
	// NOTE: we don't support bitwise operations with non-integer operands
	return new knumber_integer(0);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::neg() {
	mpq_neg(mpq_, mpq_);
	return this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::abs() {
	mpq_abs(mpq_, mpq_);
	return this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::cmp() {

	delete this;
	return new knumber_error(knumber_error::ERROR_UNDEFINED);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::sqrt() {

	if(sign() < 0) {
		delete this;
		return new knumber_error(knumber_error::ERROR_UNDEFINED);
	}

	if(mpz_perfect_square_p(mpq_numref(mpq_)) && mpz_perfect_square_p(mpq_denref(mpq_))) {
		mpz_t num;
		mpz_t den;
		mpz_init(num);
		mpz_init(den);
		mpq_get_num(num, mpq_);
		mpq_get_den(den, mpq_);
		mpz_sqrt(num, num);
		mpz_sqrt(den, den);
		mpq_set_num(mpq_, num);
		mpq_set_den(mpq_, den);
		mpq_canonicalize(mpq_);
		mpz_clear(num);
		mpz_clear(den);
		return this;
	} else {
		knumber_float *f = new knumber_float(this);
		delete this;
		return f->sqrt();
	}
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::cbrt() {

	// TODO: figure out how to properly use mpq_numref/mpq_denref here
	mpz_t num;
	mpz_t den;

	mpz_init(num);
	mpz_init(den);

	mpq_get_num(num, mpq_);
	mpq_get_den(den, mpq_);

	if(mpz_root(num, num, 3) && mpz_root(den, den, 3)) {
		mpq_set_num(mpq_, num);
		mpq_set_den(mpq_, den);
		mpq_canonicalize(mpq_);
		mpz_clear(num);
		mpz_clear(den);
		return this;
	} else {
		mpz_clear(num);
		mpz_clear(den);
		knumber_float *f = new knumber_float(this);
		delete this;
		return f->cbrt();
	}
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::factorial() {

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
knumber_base *knumber_fraction::pow(knumber_base *rhs) {

	// TODO: figure out how to properly use mpq_numref/mpq_denref here
	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {

		mpz_t num;
		mpz_t den;

		mpz_init(num);
		mpz_init(den);

		mpq_get_num(num, mpq_);
		mpq_get_den(den, mpq_);

		mpz_pow_ui(num, num, mpz_get_ui(p->mpz_));
		mpz_pow_ui(den, den, mpz_get_ui(p->mpz_));
		mpq_set_num(mpq_, num);
		mpq_set_den(mpq_, den);
		mpq_canonicalize(mpq_);
		mpz_clear(num);
		mpz_clear(den);

		if(p->sign() < 0) {
			return reciprocal();
		} else {
			return this;
		}
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		Q_UNUSED(p);
		knumber_float *f = new knumber_float(this);
		delete this;
		return f->pow(rhs);
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {

		// ok, so if any part of the number is > 1,000,000, then we risk
		// the pow function overflowing... so we'll just convert to float to be safe
		// TODO: at some point, we should figure out exactly what the threashold is
		//       and if there is a better way to determine if the pow function will
		//       overflow.
		if(mpz_cmpabs_ui(mpq_numref(mpq_), 1000000) > 0 || mpz_cmpabs_ui(mpq_denref(mpq_), 1000000) > 0 || mpz_cmpabs_ui(mpq_numref(p->mpq_), 1000000) > 0 || mpz_cmpabs_ui(mpq_denref(p->mpq_), 1000000) > 0) {
			knumber_float *f = new knumber_float(this);
			delete this;
			return f->pow(rhs);
		}

		mpz_t lhs_num;
		mpz_t lhs_den;
		mpz_t rhs_num;
		mpz_t rhs_den;

		mpz_init(lhs_num);
		mpz_init(lhs_den);
		mpz_init(rhs_num);
		mpz_init(rhs_den);

		mpq_get_num(lhs_num, mpq_);
		mpq_get_den(lhs_den, mpq_);
		mpq_get_num(rhs_num, p->mpq_);
		mpq_get_den(rhs_den, p->mpq_);

		mpz_pow_ui(lhs_num, lhs_num, mpz_get_ui(rhs_num));
		mpz_pow_ui(lhs_den, lhs_den, mpz_get_ui(rhs_num));

		if(mpz_sgn(lhs_num) < 0 && mpz_even_p(rhs_den)) {
			mpz_clear(lhs_num);
			mpz_clear(lhs_den);
			mpz_clear(rhs_num);
			mpz_clear(rhs_den);
			delete this;
			return new knumber_error(knumber_error::ERROR_UNDEFINED);
		}

		if(mpz_sgn(lhs_den) < 0 && mpz_even_p(rhs_den)) {
			mpz_clear(lhs_num);
			mpz_clear(lhs_den);
			mpz_clear(rhs_num);
			mpz_clear(rhs_den);
			delete this;
			return new knumber_error(knumber_error::ERROR_UNDEFINED);
		}

		const int n1 = mpz_root(lhs_num, lhs_num, mpz_get_ui(rhs_den));
		const int n2 = mpz_root(lhs_den, lhs_den, mpz_get_ui(rhs_den));

		if(n1 && n2) {

			mpq_set_num(mpq_, lhs_num);
			mpq_set_den(mpq_, lhs_den);
			mpq_canonicalize(mpq_);
			mpz_clear(lhs_num);
			mpz_clear(lhs_den);
			mpz_clear(rhs_num);
			mpz_clear(rhs_den);

			if(p->sign() < 0) {
				return reciprocal();
			} else {
				return this;
			}
		} else {
			mpz_clear(lhs_num);
			mpz_clear(lhs_den);
			mpz_clear(rhs_num);
			mpz_clear(rhs_den);
			knumber_float *f = new knumber_float(this);
			delete this;

			return f->pow(rhs);
		}

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
knumber_base *knumber_fraction::sin() {

	knumber_float *f = new knumber_float(this);
	delete this;
	return f->sin();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::cos() {

	knumber_float *f = new knumber_float(this);
	delete this;
	return f->cos();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::tan() {

	knumber_float *f = new knumber_float(this);
	delete this;
	return f->tan();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::asin() {

	knumber_float *f = new knumber_float(this);
	delete this;
	return f->asin();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::acos() {

	knumber_float *f = new knumber_float(this);
	delete this;
	return f->acos();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::atan() {

	knumber_float *f = new knumber_float(this);
	delete this;
	return f->atan();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::sinh() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->sinh();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::cosh() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->cosh();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::tanh() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->tanh();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::asinh() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->asinh();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::acosh() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->acosh();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::atanh() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->atanh();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
int knumber_fraction::compare(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		knumber_fraction f(p);
		return mpq_cmp(mpq_, f.mpq_);
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		knumber_float f(this);
		return f.compare(p);
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		return mpq_cmp(mpq_, p->mpq_);
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
QString knumber_fraction::toString(int precision) const {


	if(knumber_fraction::default_fractional_output) {

		// TODO: figure out how to properly use mpq_numref/mpq_denref here

		knumber_integer integer_part(this);
		if(split_off_integer_for_fraction_output && !integer_part.is_zero()) {

			mpz_t num;
			mpz_init(num);
			mpq_get_num(num, mpq_);

			knumber_integer integer_part_1(this);

			mpz_mul(integer_part.mpz_, integer_part.mpz_, mpq_denref(mpq_));
			mpz_sub(num, num, integer_part.mpz_);

			if(mpz_sgn(num) < 0) {
				mpz_neg(num, num);
			}

    		const size_t size = gmp_snprintf(NULL, 0, "%Zd %Zd/%Zd", integer_part_1.mpz_, num, mpq_denref(mpq_)) + 1;
			QScopedArrayPointer<char> buf(new char[size]);
    		gmp_snprintf(&buf[0], size, "%Zd %Zd/%Zd", integer_part_1.mpz_, num, mpq_denref(mpq_));

			mpz_clear(num);

    		return QLatin1String(&buf[0]);
		} else {

			mpz_t num;
			mpz_init(num);
			mpq_get_num(num, mpq_);

    		const size_t size = gmp_snprintf(NULL, 0, "%Zd/%Zd", num, mpq_denref(mpq_)) + 1;
			QScopedArrayPointer<char> buf(new char[size]);
    		gmp_snprintf(&buf[0], size, "%Zd/%Zd", num, mpq_denref(mpq_));

			mpz_clear(num);

    		return QLatin1String(&buf[0]);
		}
	} else {
		return knumber_float(this).toString(precision);
	}
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
bool knumber_fraction::is_zero() const {
	return mpq_sgn(mpq_) == 0;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
int knumber_fraction::sign() const {
	return mpq_sgn(mpq_);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::reciprocal() {

	mpq_inv(mpq_, mpq_);
	return this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_integer *knumber_fraction::numerator() const {

	mpz_t num;
	mpz_init(num);
	mpq_get_num(num, mpq_);
	knumber_integer *n = new knumber_integer(num);
	mpz_clear(num);
	return n;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_integer *knumber_fraction::denominator() const {

	mpz_t den;
	mpz_init(den);
	mpq_get_den(den, mpq_);
	knumber_integer *n = new knumber_integer(den);
	mpz_clear(den);
	return n;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::log2() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->log2();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::log10() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->log10();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::ln() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->ln();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::exp2() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->exp2();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::exp10() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->exp10();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::exp() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->exp();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
quint64 knumber_fraction::toUint64() const {
	return knumber_integer(this).toUint64();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
qint64 knumber_fraction::toInt64() const {
	return knumber_integer(this).toInt64();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_fraction::bin(knumber_base *rhs) {
	Q_UNUSED(rhs);
	delete this;
	return new knumber_error(knumber_error::ERROR_UNDEFINED);

}

}
