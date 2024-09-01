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

namespace detail
{
const mpfr_rnd_t KNumberFloat::rounding_mode = MPFR_RNDN;
const mpfr_prec_t KNumberFloat::precision = 1024;

KNumberBase *KNumberFloat::ensureIsValid(mpfr_ptr mpfr)
{
    if (mpfr_nan_p(mpfr)) {
        auto e = new KNumberError(KNumberError::Undefined);
        delete this;
        return e;
    } else if (mpfr_inf_p(mpfr)) {
        auto inifnityType = mpfr_signbit(mpfr) ? KNumberError::NegativeInfinity : KNumberError::PositiveInfinity;
        auto e = new KNumberError(inifnityType);
        delete this;
        return e;
    } else {
        return this;
    }
}

template<int F(mpfr_ptr rop, mpfr_srcptr op)>
KNumberBase *KNumberFloat::execute_mpfr_func()
{
    F(m_mpfr, m_mpfr);
    return ensureIsValid(m_mpfr);
}

template<int F(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd)>
KNumberBase *KNumberFloat::execute_mpfr_func()
{
    F(m_mpfr, m_mpfr, rounding_mode);
    return ensureIsValid(m_mpfr);
}

template<int F(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd)>
KNumberBase *KNumberFloat::execute_mpfr_func(mpfr_srcptr op)
{
    F(m_mpfr, m_mpfr, op, rounding_mode);
    return ensureIsValid(m_mpfr);
}

KNumberFloat::KNumberFloat(const QString &s)
{
    mpfr_set_str(new_mpfr(), s.toLatin1().constData(), 10, rounding_mode);
}

KNumberFloat::KNumberFloat(double value)
{
    Q_ASSERT(!std::isinf(value));
    Q_ASSERT(!std::isnan(value));

    mpfr_set_d(new_mpfr(), value, rounding_mode);
}

#ifdef HAVE_LONG_DOUBLE

KNumberFloat::KNumberFloat(long double value)
{
    Q_ASSERT(!std::isinf(value));
    Q_ASSERT(!std::isnan(value));

    mpfr_set_ld(new_mpfr(), value, rounding_mode);
}
#endif

KNumberFloat::KNumberFloat(mpfr_t mpfr)
{
    mpfr_set(new_mpfr(), mpfr, rounding_mode);
}

KNumberFloat::KNumberFloat(const KNumberFloat *value)
{
    mpfr_set(new_mpfr(), value->m_mpfr, rounding_mode);
}

KNumberFloat::KNumberFloat(const KNumberInteger *value)
{
    mpfr_set_z(new_mpfr(), value->m_mpz, rounding_mode);
}

KNumberFloat::KNumberFloat(const KNumberFraction *value)
{
    mpfr_set_q(new_mpfr(), value->m_mpq, rounding_mode);
}

KNumberBase *KNumberFloat::clone()
{
    return new KNumberFloat(this);
}

mpfr_ptr KNumberFloat::new_mpfr()
{
    mpfr_init(m_mpfr);
    return m_mpfr;
}

KNumberFloat::~KNumberFloat()
{
    mpfr_clear(m_mpfr);
}

KNumberBase *KNumberFloat::add(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        KNumberFloat f(p);
        return add(&f);
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        mpfr_add(m_mpfr, m_mpfr, p->m_mpfr, rounding_mode);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        KNumberFloat f(p);
        return add(&f);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        auto e = new KNumberError(p);
        delete this;
        return e;
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberFloat::sub(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        KNumberFloat f(p);
        return sub(&f);
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        mpfr_sub(m_mpfr, m_mpfr, p->m_mpfr, rounding_mode);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        KNumberFloat f(p);
        return sub(&f);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        auto e = new KNumberError(p);
        delete this;
        return e->neg();
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberFloat::mul(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        KNumberFloat f(p);
        return mul(&f);
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        mpfr_mul(m_mpfr, m_mpfr, p->m_mpfr, rounding_mode);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        KNumberFloat f(p);
        return mul(&f);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        if (isZero()) {
            delete this;
            return new KNumberError(KNumberError::Undefined);
        }

        if (sign() < 0) {
            delete this;
            auto e = new KNumberError(p);
            return e->neg();
        } else {
            delete this;
            return new KNumberError(p);
        }
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberFloat::div(KNumberBase *rhs)
{
    if (rhs->isZero()) {
        if (sign() < 0) {
            delete this;
            return new KNumberError(KNumberError::NegativeInfinity);
        } else {
            delete this;
            return new KNumberError(KNumberError::PositiveInfinity);
        }
    }

    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        KNumberFloat f(p);
        return div(&f);
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        mpfr_div(m_mpfr, m_mpfr, p->m_mpfr, rounding_mode);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        KNumberFloat f(p);
        return div(&f);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        if (p->sign() > 0 || p->sign() < 0) {
            delete this;
            return new KNumberInteger(0);
        }

        delete this;
        return new KNumberError(p);
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberFloat::mod(KNumberBase *rhs)
{
    Q_UNUSED(rhs);

    if (rhs->isZero()) {
        delete this;
        return new KNumberError(KNumberError::Undefined);
    }

    delete this;
    return new KNumberInteger(0);
}

KNumberBase *KNumberFloat::bitwiseAnd(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    return new KNumberInteger(0);
}

KNumberBase *KNumberFloat::bitwiseXor(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    return new KNumberInteger(0);
}

KNumberBase *KNumberFloat::bitwiseOr(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    return new KNumberInteger(0);
}

KNumberBase *KNumberFloat::bitwiseShift(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    // NOTE: we don't support bitwise operations with non-integer operands
    return new KNumberError(KNumberError::Undefined);
}

KNumberBase *KNumberFloat::neg()
{
    mpfr_neg(m_mpfr, m_mpfr, rounding_mode);
    return this;
}

KNumberBase *KNumberFloat::cmp()
{
    delete this;
    return new KNumberError(KNumberError::Undefined);
}

KNumberBase *KNumberFloat::abs()
{
    mpfr_abs(m_mpfr, m_mpfr, rounding_mode);
    return this;
}

KNumberBase *KNumberFloat::sqrt()
{
    if (sign() < 0) {
        delete this;
        return new KNumberError(KNumberError::Undefined);
    }

    mpfr_sqrt(m_mpfr, m_mpfr, rounding_mode);
    return this;
}

KNumberBase *KNumberFloat::cbrt()
{
    return execute_mpfr_func<::mpfr_cbrt>();
}

KNumberBase *KNumberFloat::factorial()
{
    if (sign() < 0) {
        delete this;
        return new KNumberError(KNumberError::Undefined);
    }

    auto i = new KNumberInteger(this);
    delete this;
    return i->factorial();
}

KNumberBase *KNumberFloat::sin()
{
    return execute_mpfr_func<::mpfr_sin>();
}

KNumberBase *KNumberFloat::floor()
{
    return execute_mpfr_func<::mpfr_floor>();
}

KNumberBase *KNumberFloat::ceil()
{
    return execute_mpfr_func<::mpfr_ceil>();
}

KNumberBase *KNumberFloat::cos()
{
    return execute_mpfr_func<::mpfr_cos>();
}

KNumberBase *KNumberFloat::tan()
{
    return execute_mpfr_func<::mpfr_tan>();
}

KNumberBase *KNumberFloat::asin()
{
    if (mpfr_cmp_d(m_mpfr, 1.0) > 0 || mpfr_cmp_d(m_mpfr, -1.0) < 0) {
        delete this;
        return new KNumberError(KNumberError::Undefined);
    }

    return execute_mpfr_func<::mpfr_asin>();
}

KNumberBase *KNumberFloat::acos()
{
    if (mpfr_cmp_d(m_mpfr, 1.0) > 0 || mpfr_cmp_d(m_mpfr, -1.0) < 0) {
        delete this;
        return new KNumberError(KNumberError::Undefined);
    }

    return execute_mpfr_func<::mpfr_acos>();
}

KNumberBase *KNumberFloat::atan()
{
    return execute_mpfr_func<::mpfr_atan>();
}

KNumberBase *KNumberFloat::sinh()
{
    return execute_mpfr_func<::mpfr_sinh>();
}

KNumberBase *KNumberFloat::cosh()
{
    return execute_mpfr_func<::mpfr_cosh>();
}

KNumberBase *KNumberFloat::tanh()
{
    return execute_mpfr_func<::mpfr_tanh>();
}

KNumberBase *KNumberFloat::tgamma()
{
    return execute_mpfr_func<::mpfr_gamma>();
}

KNumberBase *KNumberFloat::asinh()
{
    return execute_mpfr_func<::mpfr_asinh>();
}

KNumberBase *KNumberFloat::acosh()
{
    return execute_mpfr_func<::mpfr_acosh>();
}

KNumberBase *KNumberFloat::atanh()
{
    return execute_mpfr_func<::mpfr_atanh>();
}

KNumberBase *KNumberFloat::pow(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        mpfr_pow_ui(m_mpfr, m_mpfr, mpz_get_ui(p->m_mpz), rounding_mode);

        if (p->sign() < 0) {
            return reciprocal();
        } else {
            return this;
        }
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        return execute_mpfr_func<::mpfr_pow>(p->m_mpfr);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        KNumberFloat f(p);
        return execute_mpfr_func<::mpfr_pow>(f.m_mpfr);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        if (p->sign() > 0) {
            auto e = new KNumberError(KNumberError::PositiveInfinity);
            delete this;
            return e;
        } else if (p->sign() < 0) {
            auto n = new KNumberInteger(0);
            delete this;
            return n;
        } else {
            auto e = new KNumberError(KNumberError::Undefined);
            delete this;
            return e;
        }
    }

    Q_ASSERT(0);
    return nullptr;
}

int KNumberFloat::compare(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        KNumberFloat f(p);
        return compare(&f);
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        return mpfr_cmp(m_mpfr, p->m_mpfr);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        KNumberFloat f(p);
        return compare(&f);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        // NOTE: any number compared to NaN/Inf/-Inf always compares less
        //       at the moment
        return -1;
    }

    Q_ASSERT(0);
    return 0;
}

QString KNumberFloat::toString(int precision) const
{
    size_t size;

    if (precision > 0) {
        size = static_cast<size_t>(mpfr_snprintf(nullptr, 0, "%.*Rg", precision, m_mpfr) + 1);
    } else {
        size = static_cast<size_t>(mpfr_snprintf(nullptr, 0, "%.Rg", m_mpfr) + 1);
    }

    QScopedArrayPointer<char> buf(new char[size]);

    if (precision > 0) {
        mpfr_snprintf(&buf[0], size, "%.*Rg", precision, m_mpfr);
    } else {
        mpfr_snprintf(&buf[0], size, "%.Rg", m_mpfr);
    }

    return QLatin1String(&buf[0]);
}

bool KNumberFloat::isInteger() const
{
    return mpfr_integer_p(m_mpfr) != 0;
}

bool KNumberFloat::isZero() const
{
    return mpfr_zero_p(m_mpfr);
}

int KNumberFloat::sign() const
{
    return mpfr_sgn(m_mpfr);
}

KNumberBase *KNumberFloat::reciprocal()
{
    mpfr_t mpfr;
    mpfr_init_set_d(mpfr, 1.0, rounding_mode);
    mpfr_div(m_mpfr, mpfr, m_mpfr, rounding_mode);
    mpfr_clear(mpfr);
    return this;
}

KNumberBase *KNumberFloat::log2()
{
    return execute_mpfr_func<::mpfr_log2>();
}

KNumberBase *KNumberFloat::log10()
{
    return execute_mpfr_func<::mpfr_log10>();
}

KNumberBase *KNumberFloat::ln()
{
    return execute_mpfr_func<::mpfr_log>();
}

KNumberBase *KNumberFloat::exp2()
{
    return execute_mpfr_func<::mpfr_exp2>();
}

KNumberBase *KNumberFloat::exp10()
{
    return execute_mpfr_func<::mpfr_exp10>();
}

KNumberBase *KNumberFloat::exp()
{
    return execute_mpfr_func<::mpfr_exp>();
}

quint64 KNumberFloat::toUint64() const
{
    return KNumberInteger(this).toUint64();
}

qint64 KNumberFloat::toInt64() const
{
    return KNumberInteger(this).toInt64();
}

KNumberBase *KNumberFloat::bin(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    return new KNumberError(KNumberError::Undefined);
}

}
