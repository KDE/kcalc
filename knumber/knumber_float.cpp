/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knumber_float.h"
#include "knumber_error.h"
#include "knumber_fraction.h"
#include "knumber_integer.h"
#include <QScopedArrayPointer>
#include <cmath>

#ifdef _MSC_VER
double log2(double x)
{
    return log(x) / log(2);
}
double exp2(double x)
{
    return exp(x * log(2));
}
double exp10(double x)
{
    return exp(x * log(10));
}
#endif

// NOTE: these assume IEEE floats.
#ifndef isinf
#define isinf(x) ((x) != 0.0 && (x) + (x) == (x))
#endif

#ifndef isnan
#define isnan(x) ((x) != (x))
#endif

namespace detail
{
const mpfr_rnd_t knumber_float::rounding_mode = MPFR_RNDN;
const mpfr_prec_t knumber_float::precision = 1024;

knumber_base *knumber_float::ensureIsValid(mpfr_ptr mpfr)
{
    if (mpfr_nan_p(mpfr)) {
        auto e = new knumber_error(knumber_error::ERROR_UNDEFINED);
        delete this;
        return e;
    } else if (mpfr_inf_p(mpfr)) {
        auto e = new knumber_error(knumber_error::ERROR_POS_INFINITY);
        delete this;
        return e;
    } else {
        return this;
    }
}

template<int F(mpfr_ptr rop, mpfr_srcptr op)>
knumber_base *knumber_float::execute_mpfr_func()
{
    F(mpfr_, mpfr_);
    return ensureIsValid(mpfr_);
}

template<int F(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd)>
knumber_base *knumber_float::execute_mpfr_func()
{
    F(mpfr_, mpfr_, rounding_mode);
    return ensureIsValid(mpfr_);
}

template<int F(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd)>
knumber_base *knumber_float::execute_mpfr_func(mpfr_srcptr op)
{
    F(mpfr_, mpfr_, op, rounding_mode);
    return ensureIsValid(mpfr_);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_float::knumber_float(const QString &s)
{
    mpfr_set_str(new_mpfr(), s.toLatin1().constData(), 10, rounding_mode);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_float::knumber_float(double value)
{
    Q_ASSERT(!isinf(value));
    Q_ASSERT(!isnan(value));

    mpfr_set_d(new_mpfr(), value, rounding_mode);
}

#ifdef HAVE_LONG_DOUBLE
//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_float::knumber_float(long double value)
{
    Q_ASSERT(!isinf(value));
    Q_ASSERT(!isnan(value));

    mpfr_set_ld(new_mpfr(), value, rounding_mode);
}
#endif

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_float::knumber_float(mpfr_t mpfr)
{
    mpfr_set(new_mpfr(), mpfr, rounding_mode);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_float::knumber_float(const knumber_float *value)
{
    mpfr_set(new_mpfr(), value->mpfr_, rounding_mode);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_float::knumber_float(const knumber_integer *value)
{
    mpfr_set_z(new_mpfr(), value->mpz_, rounding_mode);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_float::knumber_float(const knumber_fraction *value)
{
    mpfr_set_q(new_mpfr(), value->mpq_, rounding_mode);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::clone()
{
    return new knumber_float(this);
}

mpfr_ptr knumber_float::new_mpfr()
{
    mpfr_init(mpfr_);
    return mpfr_;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_float::~knumber_float()
{
    mpfr_clear(mpfr_);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::add(knumber_base *rhs)
{
    if (auto const p = dynamic_cast<knumber_integer *>(rhs)) {
        knumber_float f(p);
        return add(&f);
    } else if (auto const p = dynamic_cast<knumber_float *>(rhs)) {
        mpfr_add(mpfr_, mpfr_, p->mpfr_, rounding_mode);
        return this;
    } else if (auto const p = dynamic_cast<knumber_fraction *>(rhs)) {
        knumber_float f(p);
        return add(&f);
    } else if (auto const p = dynamic_cast<knumber_error *>(rhs)) {
        auto e = new knumber_error(p);
        delete this;
        return e;
    }

    Q_ASSERT(0);
    return nullptr;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::sub(knumber_base *rhs)
{
    if (auto const p = dynamic_cast<knumber_integer *>(rhs)) {
        knumber_float f(p);
        return sub(&f);
    } else if (auto const p = dynamic_cast<knumber_float *>(rhs)) {
        mpfr_sub(mpfr_, mpfr_, p->mpfr_, rounding_mode);
        return this;
    } else if (auto const p = dynamic_cast<knumber_fraction *>(rhs)) {
        knumber_float f(p);
        return sub(&f);
    } else if (auto const p = dynamic_cast<knumber_error *>(rhs)) {
        auto e = new knumber_error(p);
        delete this;
        return e->neg();
    }

    Q_ASSERT(0);
    return nullptr;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::mul(knumber_base *rhs)
{
    if (auto const p = dynamic_cast<knumber_integer *>(rhs)) {
        knumber_float f(p);
        return mul(&f);
    } else if (auto const p = dynamic_cast<knumber_float *>(rhs)) {
        mpfr_mul(mpfr_, mpfr_, p->mpfr_, rounding_mode);
        return this;
    } else if (auto const p = dynamic_cast<knumber_fraction *>(rhs)) {
        knumber_float f(p);
        return mul(&f);
    } else if (auto const p = dynamic_cast<knumber_error *>(rhs)) {
        if (is_zero()) {
            delete this;
            return new knumber_error(knumber_error::ERROR_UNDEFINED);
        }

        if (sign() < 0) {
            delete this;
            auto e = new knumber_error(p);
            return e->neg();
        } else {
            delete this;
            return new knumber_error(p);
        }
    }

    Q_ASSERT(0);
    return nullptr;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::div(knumber_base *rhs)
{
    if (rhs->is_zero()) {
        if (sign() < 0) {
            delete this;
            return new knumber_error(knumber_error::ERROR_NEG_INFINITY);
        } else {
            delete this;
            return new knumber_error(knumber_error::ERROR_POS_INFINITY);
        }
    }

    if (auto const p = dynamic_cast<knumber_integer *>(rhs)) {
        knumber_float f(p);
        return div(&f);
    } else if (auto const p = dynamic_cast<knumber_float *>(rhs)) {
        mpfr_div(mpfr_, mpfr_, p->mpfr_, rounding_mode);
        return this;
    } else if (auto const p = dynamic_cast<knumber_fraction *>(rhs)) {
        knumber_float f(p);
        return div(&f);
    } else if (auto const p = dynamic_cast<knumber_error *>(rhs)) {
        if (p->sign() > 0 || p->sign() < 0) {
            delete this;
            return new knumber_integer(0);
        }

        delete this;
        return new knumber_error(p);
    }

    Q_ASSERT(0);
    return nullptr;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::mod(knumber_base *rhs)
{
    Q_UNUSED(rhs);

    if (rhs->is_zero()) {
        delete this;
        return new knumber_error(knumber_error::ERROR_UNDEFINED);
    }

    delete this;
    return new knumber_integer(0);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::bitwise_and(knumber_base *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    return new knumber_integer(0);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::bitwise_xor(knumber_base *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    return new knumber_integer(0);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::bitwise_or(knumber_base *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    return new knumber_integer(0);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::bitwise_shift(knumber_base *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    // NOTE: we don't support bitwise operations with non-integer operands
    return new knumber_error(knumber_error::ERROR_UNDEFINED);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::neg()
{
    mpfr_neg(mpfr_, mpfr_, rounding_mode);
    return this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::cmp()
{
    delete this;
    return new knumber_error(knumber_error::ERROR_UNDEFINED);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::abs()
{
    mpfr_abs(mpfr_, mpfr_, rounding_mode);
    return this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::sqrt()
{
    if (sign() < 0) {
        delete this;
        return new knumber_error(knumber_error::ERROR_UNDEFINED);
    }

    mpfr_sqrt(mpfr_, mpfr_, rounding_mode);
    return this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::cbrt()
{
    return execute_mpfr_func<::mpfr_cbrt>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::factorial()
{
    if (sign() < 0) {
        delete this;
        return new knumber_error(knumber_error::ERROR_UNDEFINED);
    }

    auto i = new knumber_integer(this);
    delete this;
    return i->factorial();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::sin()
{
    return execute_mpfr_func<::mpfr_sin>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::floor()
{
    return execute_mpfr_func<::mpfr_floor>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::ceil()
{
    return execute_mpfr_func<::mpfr_ceil>();
}
//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::cos()
{
    return execute_mpfr_func<::mpfr_cos>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::tan()
{
    return execute_mpfr_func<::mpfr_tan>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::asin()
{
    if (mpfr_cmp_d(mpfr_, 1.0) > 0 || mpfr_cmp_d(mpfr_, -1.0) < 0) {
        delete this;
        return new knumber_error(knumber_error::ERROR_UNDEFINED);
    }

    return execute_mpfr_func<::mpfr_asin>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------s
knumber_base *knumber_float::acos()
{
    if (mpfr_cmp_d(mpfr_, 1.0) > 0 || mpfr_cmp_d(mpfr_, -1.0) < 0) {
        delete this;
        return new knumber_error(knumber_error::ERROR_UNDEFINED);
    }

    return execute_mpfr_func<::mpfr_acos>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::atan()
{
    return execute_mpfr_func<::mpfr_atan>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::sinh()
{
    return execute_mpfr_func<::mpfr_sinh>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::cosh()
{
    return execute_mpfr_func<::mpfr_cosh>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::tanh()
{
    return execute_mpfr_func<::mpfr_tanh>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::tgamma()
{
    return execute_mpfr_func<::mpfr_gamma>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::asinh()
{
    return execute_mpfr_func<::mpfr_asinh>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::acosh()
{
    return execute_mpfr_func<::mpfr_acosh>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::atanh()
{
    return execute_mpfr_func<::mpfr_atanh>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::pow(knumber_base *rhs)
{
    if (auto const p = dynamic_cast<knumber_integer *>(rhs)) {
        mpfr_pow_ui(mpfr_, mpfr_, mpz_get_ui(p->mpz_), rounding_mode);

        if (p->sign() < 0) {
            return reciprocal();
        } else {
            return this;
        }
    } else if (auto const p = dynamic_cast<knumber_float *>(rhs)) {
        return execute_mpfr_func<::mpfr_pow>(p->mpfr_);
    } else if (auto const p = dynamic_cast<knumber_fraction *>(rhs)) {
        knumber_float f(p);
        return execute_mpfr_func<::mpfr_pow>(f.mpfr_);
    } else if (auto const p = dynamic_cast<knumber_error *>(rhs)) {
        if (p->sign() > 0) {
            auto e = new knumber_error(knumber_error::ERROR_POS_INFINITY);
            delete this;
            return e;
        } else if (p->sign() < 0) {
            auto n = new knumber_integer(0);
            delete this;
            return n;
        } else {
            auto e = new knumber_error(knumber_error::ERROR_UNDEFINED);
            delete this;
            return e;
        }
    }

    Q_ASSERT(0);
    return nullptr;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
int knumber_float::compare(knumber_base *rhs)
{
    if (auto const p = dynamic_cast<knumber_integer *>(rhs)) {
        knumber_float f(p);
        return compare(&f);
    } else if (auto const p = dynamic_cast<knumber_float *>(rhs)) {
        return mpfr_cmp(mpfr_, p->mpfr_);
    } else if (auto const p = dynamic_cast<knumber_fraction *>(rhs)) {
        knumber_float f(p);
        return compare(&f);
    } else if (auto const p = dynamic_cast<knumber_error *>(rhs)) {
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
QString knumber_float::toString(int precision) const
{
    size_t size;

    if (precision > 0) {
        size = static_cast<size_t>(mpfr_snprintf(nullptr, 0, "%.*Rg", precision, mpfr_) + 1);
    } else {
        size = static_cast<size_t>(mpfr_snprintf(nullptr, 0, "%.Rg", mpfr_) + 1);
    }

    QScopedArrayPointer<char> buf(new char[size]);

    if (precision > 0) {
        mpfr_snprintf(&buf[0], size, "%.*Rg", precision, mpfr_);
    } else {
        mpfr_snprintf(&buf[0], size, "%.Rg", mpfr_);
    }

    return QLatin1String(&buf[0]);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
bool knumber_float::is_integer() const
{
    return mpfr_integer_p(mpfr_) != 0;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
bool knumber_float::is_zero() const
{
    return mpfr_zero_p(mpfr_);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
int knumber_float::sign() const
{
    return mpfr_sgn(mpfr_);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::reciprocal()
{
    mpfr_t mpfr;
    mpfr_init_set_d(mpfr, 1.0, rounding_mode);
    mpfr_div(mpfr_, mpfr, mpfr_, rounding_mode);
    mpfr_clear(mpfr);
    return this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::log2()
{
    return execute_mpfr_func<::mpfr_log2>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::log10()
{
    return execute_mpfr_func<::mpfr_log10>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::ln()
{
    return execute_mpfr_func<::mpfr_log>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::exp2()
{
    return execute_mpfr_func<::mpfr_exp2>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::exp10()
{
    return execute_mpfr_func<::mpfr_exp10>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::exp()
{
    return execute_mpfr_func<::mpfr_exp>();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
quint64 knumber_float::toUint64() const
{
    return knumber_integer(this).toUint64();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
qint64 knumber_float::toInt64() const
{
    return knumber_integer(this).toInt64();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
knumber_base *knumber_float::bin(knumber_base *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    return new knumber_error(knumber_error::ERROR_UNDEFINED);
}

}
