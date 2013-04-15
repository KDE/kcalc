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

//------------------------------------------------------------------------------
// Name: knumber_integer
//------------------------------------------------------------------------------
knumber_integer::knumber_integer(const QString &s) {
    mpz_init(mpz_);
    mpz_set_str(mpz_, s.toAscii(), 10);
}

//------------------------------------------------------------------------------
// Name: knumber_integer
//------------------------------------------------------------------------------
knumber_integer::knumber_integer(qint32 value) {
	mpz_init_set_si(mpz_, static_cast<signed long int>(value));
}

//------------------------------------------------------------------------------
// Name: knumber_integer
//------------------------------------------------------------------------------
knumber_integer::knumber_integer(qint64 value) {
    mpz_init(mpz_);
#if SIZEOF_SIGNED_LONG == 8
    mpz_set_si(mpz_, static_cast<signed long int>(value));
#elif SIZEOF_SIGNED_LONG == 4
    mpz_set_si(mpz_, static_cast<signed long int>(value >> 32));
    mpz_mul_2exp(mpz_, mpz_, 32);
    mpz_add_ui(mpz_, mpz_, static_cast<signed long int>(value));
#else
#error "SIZEOF_SIGNED_LONG is a unhandled case"
#endif
}

//------------------------------------------------------------------------------
// Name: knumber_integer
//------------------------------------------------------------------------------
knumber_integer::knumber_integer(quint32 value) {
	mpz_init_set_ui(mpz_, static_cast<unsigned long int>(value));
}

//------------------------------------------------------------------------------
// Name: knumber_integer
//------------------------------------------------------------------------------
knumber_integer::knumber_integer(quint64 value) {
    mpz_init(mpz_);
#if SIZEOF_UNSIGNED_LONG == 8
    mpz_set_ui(mpz_, static_cast<unsigned long int>(value));
#elif SIZEOF_UNSIGNED_LONG == 4
    mpz_set_ui(mpz_, static_cast<unsigned long int>(value >> 32));
    mpz_mul_2exp(mpz_, mpz_, 32);
    mpz_add_ui(mpz_, mpz_, static_cast<unsigned long int>(value));
#else
#error "SIZEOF_UNSIGNED_LONG is a unhandled case"
#endif
}

//------------------------------------------------------------------------------
// Name: knumber_integer
//------------------------------------------------------------------------------
knumber_integer::knumber_integer(mpz_t mpz) {
	mpz_init_set(mpz_, mpz);
}

//------------------------------------------------------------------------------
// Name: knumber_integer
//------------------------------------------------------------------------------
knumber_integer::knumber_integer(const knumber_integer *value) {
	mpz_init_set(mpz_, value->mpz_);
}

//------------------------------------------------------------------------------
// Name: knumber_integer
//------------------------------------------------------------------------------
knumber_integer::knumber_integer(const knumber_float *value) {
	mpz_init(mpz_);
	mpz_set_f(mpz_, value->mpf_);
}

//------------------------------------------------------------------------------
// Name: knumber_integer
//------------------------------------------------------------------------------
knumber_integer::knumber_integer(const knumber_fraction *value) {
	mpz_init(mpz_);
	mpz_set_q(mpz_, value->mpq_);
}

//------------------------------------------------------------------------------
// Name: clone
//------------------------------------------------------------------------------
knumber_base *knumber_integer::clone() {
	return new knumber_integer(this);
}

//------------------------------------------------------------------------------
// Name: ~knumber_integer
//------------------------------------------------------------------------------
knumber_integer::~knumber_integer() {
	mpz_clear(mpz_);
}

//------------------------------------------------------------------------------
// Name: add
//------------------------------------------------------------------------------
knumber_base *knumber_integer::add(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		mpz_add(mpz_, mpz_, p->mpz_);
		return this;
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		knumber_float *const f = new knumber_float(this);
		delete this;
		return f->add(p);
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		knumber_fraction *const q = new knumber_fraction(this);
		delete this;
		return q->add(p);
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		delete this;
		return p->clone();
	}

	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name: sub
//------------------------------------------------------------------------------
knumber_base *knumber_integer::sub(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		mpz_sub(mpz_, mpz_, p->mpz_);
		return this;
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		knumber_float *f = new knumber_float(this);
		delete this;
		return f->sub(p);
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		knumber_fraction *q = new knumber_fraction(this);
		delete this;
		return q->sub(p);
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		knumber_base *e = p->clone();
		delete this;
		return e->neg();
	}

	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name: mul
//------------------------------------------------------------------------------
knumber_base *knumber_integer::mul(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		mpz_mul(mpz_, mpz_, p->mpz_);
		return this;
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		knumber_float *f = new knumber_float(this);
		delete this;
		return f->mul(p);
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		knumber_fraction *q = new knumber_fraction(this);
		delete this;
		return q->mul(p);
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {

		if(is_zero()) {
			delete this;
			knumber_error *e = new knumber_error(knumber_error::ERROR_UNDEFINED);
			return e->neg();
		}

		if(sign() < 0) {
			delete this;
			knumber_base *e = p->clone();
			return e->neg();
		} else {
			delete this;
			return p->clone();
		}
	}

	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name: div
//------------------------------------------------------------------------------
knumber_base *knumber_integer::div(knumber_base *rhs) {

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
		knumber_fraction *q = new knumber_fraction(this);
		delete this;
		return q->div(p);
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		knumber_float *f = new knumber_float(this);
		delete this;
		return f->div(p);
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		knumber_fraction *q = new knumber_fraction(this);
		delete this;
		return q->div(p);
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {

		if(p->sign() > 0) {
			delete this;
			return new knumber_integer(0);
		} else if(p->sign() < 0) {
			delete this;
			return new knumber_integer(0);
		}

		delete this;
		return p->clone();
	}

	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name: mod
//------------------------------------------------------------------------------
knumber_base *knumber_integer::mod(knumber_base *rhs) {

	if(rhs->is_zero()) {
		delete this;
		return new knumber_error(knumber_error::ERROR_UNDEFINED);
	}

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		mpz_mod(mpz_, mpz_, p->mpz_);
		return this;
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		knumber_float *f = new knumber_float(this);
		delete this;
		return f->mod(p);
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		knumber_fraction *q = new knumber_fraction(this);
		delete this;
		return q->mod(p);
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		delete this;
		return p->clone();
	}

	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name: bitwise_and
//------------------------------------------------------------------------------
knumber_base *knumber_integer::bitwise_and(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		mpz_and(mpz_, mpz_, p->mpz_);
		return this;
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		knumber_float *f = new knumber_float(this);
		delete this;
		return f->bitwise_and(p);
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		knumber_fraction *f = new knumber_fraction(this);
		delete this;
		return f->bitwise_and(p);
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		delete this;
		return p->clone();
	}

	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name: bitwise_xor
//------------------------------------------------------------------------------
knumber_base *knumber_integer::bitwise_xor(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		mpz_xor(mpz_, mpz_, p->mpz_);
		return this;
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		knumber_float *f = new knumber_float(this);
		delete this;
		return f->bitwise_xor(p);
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		knumber_fraction *f = new knumber_fraction(this);
		delete this;
		return f->bitwise_xor(p);
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		delete this;
		return p->clone();
	}

	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name: bitwise_or
//------------------------------------------------------------------------------
knumber_base *knumber_integer::bitwise_or(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		mpz_ior(mpz_, mpz_, p->mpz_);
		return this;
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		knumber_float *f = new knumber_float(this);
		delete this;
		return f->bitwise_or(p);
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		knumber_fraction *f = new knumber_fraction(this);
		delete this;
		return f->bitwise_or(p);
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		delete this;
		return p->clone();
	}

	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name: bitwise_shift
//------------------------------------------------------------------------------
knumber_base *knumber_integer::bitwise_shift(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {

		const signed long int bit_count = mpz_get_si(p->mpz_);

		// TODO: left shift with high bit set is broken in
		//       non decimal modes :-/, always displays 0
		//       interestingly, the bit is not "lost"
		//       we simply don't have a mechanism to display
		//       values in HEX/DEC/OCT mode which are greater than
		//       64-bits

		if(bit_count > 0) {
			// left shift
			mpz_mul_2exp(mpz_, mpz_, bit_count);
		} else if(bit_count < 0) {
        	// right shift
        	if(mpz_sgn(mpz_) < 0) {
            	mpz_fdiv_q_2exp(mpz_, mpz_, -bit_count);
        	} else {
            	mpz_tdiv_q_2exp(mpz_, mpz_, -bit_count);
        	}
		}
		return this;
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		Q_UNUSED(p);
		knumber_error *e = new knumber_error(knumber_error::ERROR_UNDEFINED);
		delete this;
		return e;
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		Q_UNUSED(p);
		knumber_error *e = new knumber_error(knumber_error::ERROR_UNDEFINED);
		delete this;
		return e;
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		Q_UNUSED(p);
		knumber_error *e = new knumber_error(knumber_error::ERROR_UNDEFINED);
		delete this;
		return e;
	}

	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name: neg
//------------------------------------------------------------------------------
knumber_base *knumber_integer::neg() {

	mpz_neg(mpz_, mpz_);
	return this;
}

//------------------------------------------------------------------------------
// Name: cmp
//------------------------------------------------------------------------------
knumber_base *knumber_integer::cmp() {

#if 0
	// unfortunately this breaks things pretty badly
	// for non-decimal modes :-(
	mpz_com(mpz_, mpz_);
#else
	mpz_swap(mpz_, knumber_integer(~toUint64()).mpz_);
#endif
	return this;
}

//------------------------------------------------------------------------------
// Name: abs
//------------------------------------------------------------------------------
knumber_base *knumber_integer::abs() {

	mpz_abs(mpz_, mpz_);
	return this;
}

//------------------------------------------------------------------------------
// Name: sqrt
//------------------------------------------------------------------------------
knumber_base *knumber_integer::sqrt() {

	if(sign() < 0) {
		delete this;
		return new knumber_error(knumber_error::ERROR_UNDEFINED);
	}

	if(mpz_perfect_square_p(mpz_)) {
		mpz_sqrt(mpz_, mpz_);
		return this;
	} else {
		knumber_float *f = new knumber_float(this);
		delete this;
		return f->sqrt();
	}
}

//------------------------------------------------------------------------------
// Name: cbrt
//------------------------------------------------------------------------------
knumber_base *knumber_integer::cbrt() {

	mpz_t x;
	mpz_init_set(x, mpz_);
	if(mpz_root(x, x, 3)) {
		mpz_swap(mpz_, x);
		mpz_clear(x);
		return this;
	}

	mpz_clear(x);
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->cbrt();
}

//------------------------------------------------------------------------------
// Name: pow
//------------------------------------------------------------------------------
knumber_base *knumber_integer::pow(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {

		if(is_zero() && p->is_even() && p->sign() < 0) {
			delete this;
			return new knumber_error(knumber_error::ERROR_POS_INFINITY);
		}

		mpz_pow_ui(mpz_, mpz_, mpz_get_ui(p->mpz_));

		if(p->sign() < 0) {
			return reciprocal();
		} else {
			return this;
		}
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		knumber_float *f = new knumber_float(this);
		delete this;
		return f->pow(p);
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		knumber_fraction *f = new knumber_fraction(this);
		delete this;
		return f->pow(p);
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		if(p->sign() > 0) {
			knumber_error *e = new knumber_error(knumber_error::ERROR_POS_INFINITY);
			delete this;
			return e;
		} else if(p->sign() < 0) {
			mpz_init_set_si(mpz_, 0);
			return this;
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
// Name: sin
//------------------------------------------------------------------------------
knumber_base *knumber_integer::sin() {

	knumber_float *f = new knumber_float(this);
	delete this;
	return f->sin();
}

//------------------------------------------------------------------------------
// Name: cos
//------------------------------------------------------------------------------
knumber_base *knumber_integer::cos() {

	knumber_float *f = new knumber_float(this);
	delete this;
	return f->cos();
}

//------------------------------------------------------------------------------
// Name: tan
//------------------------------------------------------------------------------
knumber_base *knumber_integer::tan() {

	knumber_float *f = new knumber_float(this);
	delete this;
	return f->tan();
}

//------------------------------------------------------------------------------
// Name: asin
//------------------------------------------------------------------------------
knumber_base *knumber_integer::asin() {

	knumber_float *f = new knumber_float(this);
	delete this;
	return f->asin();
}

//------------------------------------------------------------------------------
// Name: acos
//------------------------------------------------------------------------------
knumber_base *knumber_integer::acos() {

	knumber_float *f = new knumber_float(this);
	delete this;
	return f->acos();
}

//------------------------------------------------------------------------------
// Name: atan
//------------------------------------------------------------------------------
knumber_base *knumber_integer::atan() {

	knumber_float *f = new knumber_float(this);
	delete this;
	return f->atan();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_integer::sinh() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->sinh();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_integer::cosh() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->cosh();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_integer::tanh() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->tanh();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_integer::asinh() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->asinh();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_integer::acosh() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->acosh();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_integer::atanh() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->atanh();
}

//------------------------------------------------------------------------------
// Name: factorial
//------------------------------------------------------------------------------
knumber_base *knumber_integer::factorial() {

	if(sign() < 0) {
		delete this;
		return new knumber_error(knumber_error::ERROR_UNDEFINED);
	}

	mpz_fac_ui(mpz_, mpz_get_ui(mpz_));
	return this;
}

//------------------------------------------------------------------------------
// Name: compare
//------------------------------------------------------------------------------
int knumber_integer::compare(knumber_base *rhs) {

	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		return mpz_cmp(mpz_, p->mpz_);
	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		return knumber_float(this).compare(p);
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		return knumber_fraction(this).compare(p);
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		// NOTE: any number compared to NaN/Inf/-Inf always compares less
		//       at the moment
		return -1;
	}

	Q_ASSERT(0);
	return 0;
}

//------------------------------------------------------------------------------
// Name: toString
//------------------------------------------------------------------------------
QString knumber_integer::toString(int precision) const {

	Q_UNUSED(precision);

    const size_t size = gmp_snprintf(NULL, 0, "%Zd", mpz_) + 1;
	QScopedArrayPointer<char> buf(new char[size]);
    gmp_snprintf(&buf[0], size, "%Zd", mpz_);
    return QLatin1String(&buf[0]);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
quint64 knumber_integer::toUint64() const {
    // libgmp doesn't have unsigned long long conversion
    // so convert to string and then to unsigned long long
    const QString tmpstring = toString(-1);

    bool ok;
    quint64 value;

    if (sign() < 0) {
        const qint64 signedvalue = tmpstring.toLongLong(&ok, 10);
        value = static_cast<quint64>(signedvalue);
    } else {
        value = tmpstring.toULongLong(&ok, 10);
    }

    if (!ok) {
        // TODO: what to do if error?
        value = 0;
    }
    return value;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
qint64 knumber_integer::toInt64() const {
	// libgmp doesn't have long long conversion
	// so convert to string and then to long long
	const QString tmpstring = toString(-1);

	bool ok;
	qint64 value = tmpstring.toLongLong(&ok, 10);

	if (!ok) {
		// TODO: what to do if error?
		value = 0;
	}

	return value;
}

//------------------------------------------------------------------------------
// Name: is_integer
//------------------------------------------------------------------------------
bool knumber_integer::is_integer() const {
	return true;
}

//------------------------------------------------------------------------------
// Name: is_zero
//------------------------------------------------------------------------------
bool knumber_integer::is_zero() const {
	return mpz_sgn(mpz_) == 0;
}

//------------------------------------------------------------------------------
// Name: sign
//------------------------------------------------------------------------------
int knumber_integer::sign() const {
	return mpz_sgn(mpz_);
}

//------------------------------------------------------------------------------
// Name: is_even
//------------------------------------------------------------------------------
bool knumber_integer::is_even() const {
	return mpz_even_p(mpz_);
}

//------------------------------------------------------------------------------
// Name: is_odd
//------------------------------------------------------------------------------
bool knumber_integer::is_odd() const {
	return mpz_odd_p(mpz_);
}

//------------------------------------------------------------------------------
// Name: reciprocal
//------------------------------------------------------------------------------
knumber_base *knumber_integer::reciprocal() {
	knumber_fraction *q = new knumber_fraction(this);
	delete this;
	return q->reciprocal();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_integer::log2() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->log2();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_integer::log10() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->log10();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_integer::ln() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->ln();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_integer::exp2() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->exp2();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_integer::exp10() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->exp10();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_integer::exp() {
	knumber_float *f = new knumber_float(this);
	delete this;
	return f->exp();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_integer::bin(knumber_base *rhs) {


	if(knumber_integer *const p = dynamic_cast<knumber_integer *>(rhs)) {
		mpz_bin_ui(mpz_, mpz_, mpz_get_ui(p->mpz_));
		return this;

	} else if(knumber_float *const p = dynamic_cast<knumber_float *>(rhs)) {
		delete this;
		return new knumber_error(knumber_error::ERROR_UNDEFINED);
	} else if(knumber_fraction *const p = dynamic_cast<knumber_fraction *>(rhs)) {
		delete this;
		return new knumber_error(knumber_error::ERROR_UNDEFINED);
	} else if(knumber_error *const p = dynamic_cast<knumber_error *>(rhs)) {
		delete this;
		return new knumber_error(knumber_error::ERROR_UNDEFINED);
	}

	Q_ASSERT(0);
	return 0;
}

}
