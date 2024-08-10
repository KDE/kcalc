/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knumber_complex.h"
#include "knumber_cleanup_string.h"
#include "knumber_error.h"
#include "knumber_float.h"
#include "knumber_fraction.h"
#include "knumber_integer.h"

#include <mpc.h>
#include <mpfr.h>

#include <QDebug>
#include <QScopedArrayPointer>

namespace detail
{
const mpc_rnd_t KNumberComplex::rounding_mode = MPC_RNDNN;
const mpfr_prec_t KNumberComplex::precision = 1024;

KNumberBase *KNumberComplex::ensureIsValid()
{
    if (mpfr_number_p(mpc_realref(m_mpc)) && mpfr_number_p(mpc_imagref(m_mpc))) {
        return this;
    } else {
        delete this;
        return new KNumberError(KNumberError::Undefined);
    }
}

template<int F(mpc_ptr rop, mpc_srcptr op)>
KNumberBase *KNumberComplex::execute_mpc_func()
{
    F(m_mpc, m_mpc);
    return ensureIsValid();
}

template<int F(mpc_ptr rop, mpc_srcptr op, mpc_rnd_t rnd)>
KNumberBase *KNumberComplex::execute_mpc_func()
{
    F(m_mpc, m_mpc, rounding_mode);
    return ensureIsValid();
}

template<int F(mpc_ptr rop, mpc_srcptr op1, mpc_srcptr op2, mpc_rnd_t rnd)>
KNumberBase *KNumberComplex::execute_mpc_func(mpc_srcptr op)
{
    F(m_mpc, m_mpc, op, rounding_mode);
    return ensureIsValid();
}

KNumberComplex::KNumberComplex(const QString &s)
{
    mpc_set_str(new_mpc(), s.toLatin1().constData(), 10, rounding_mode);
}

KNumberComplex::KNumberComplex(const QString &mod, const QString &arg)
{
    new_mpc();
    mpfr_t argFr;
    mpfr_init(argFr);
    mpfr_set_str(argFr, arg.toLatin1().constData(), 10, KNumberFloat::rounding_mode);

    mpfr_t modFr;
    mpfr_init(modFr);
    mpfr_set_str(modFr, mod.toLatin1().constData(), 10, KNumberFloat::rounding_mode);

    mpfr_sin_cos(mpc_imagref(m_mpc), mpc_realref(m_mpc), argFr, KNumberFloat::rounding_mode);
    mpc_mul_fr(m_mpc, m_mpc, modFr, rounding_mode);

    mpfr_clear(argFr);
    mpfr_clear(modFr);
}

KNumberComplex::KNumberComplex(double re)
{
    Q_ASSERT(!std::isinf(re));
    Q_ASSERT(!std::isnan(re));

    mpc_set_d(new_mpc(), re, rounding_mode);
}

KNumberComplex::KNumberComplex(double re, double img)
{
    Q_ASSERT(!std::isinf(re));
    Q_ASSERT(!std::isnan(re));
    Q_ASSERT(!std::isinf(img));
    Q_ASSERT(!std::isnan(img));

    mpc_set_d_d(new_mpc(), re, img, rounding_mode);
}

#ifdef HAVE_LONG_DOUBLE

KNumberComplex::KNumberComplex(long double re)
{
    Q_ASSERT(!std::isinf(re));
    Q_ASSERT(!std::isnan(re));

    mpc_set_ld(new_mpc(), re, rounding_mode);
}

KNumberComplex::KNumberComplex(long double re, long double img)
{
    Q_ASSERT(!std::isinf(re));
    Q_ASSERT(!std::isnan(re));
    Q_ASSERT(!std::isinf(img));
    Q_ASSERT(!std::isnan(img));

    mpc_set_ld_ld(new_mpc(), re, img, rounding_mode);
}
#endif

KNumberComplex::KNumberComplex(mpc_t mpc)
{
    mpc_set(new_mpc(), mpc, rounding_mode);
}

KNumberComplex::KNumberComplex(const KNumberComplex *value)
{
    mpc_set(new_mpc(), value->m_mpc, rounding_mode);
}

KNumberComplex::KNumberComplex(const KNumberInteger *value)
{
    mpc_set_z(new_mpc(), value->m_mpz, rounding_mode);
}

KNumberComplex::KNumberComplex(const KNumberFraction *value)
{
    mpc_set_q(new_mpc(), value->m_mpq, rounding_mode);
}

KNumberComplex::KNumberComplex(const KNumberFloat *value)
{
    mpc_set_fr(new_mpc(), value->m_mpfr, rounding_mode);
}

KNumberBase *KNumberComplex::clone()
{
    return new KNumberComplex(this);
}

mpc_ptr KNumberComplex::new_mpc()
{
    mpc_init2(m_mpc, precision);
    return m_mpc;
}

KNumberComplex::~KNumberComplex()
{
    mpc_clear(m_mpc);
}

KNumberBase *KNumberComplex::realPart()
{
    auto real = new KNumberFloat(mpc_realref(m_mpc));
    delete this;
    return real;
}

KNumberBase *KNumberComplex::imaginaryPart()
{
    auto img = new KNumberFloat(mpc_imagref(m_mpc));
    delete this;
    return img;
}

KNumberBase *KNumberComplex::add(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        KNumberComplex c(p);
        return add(&c);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        KNumberComplex c(p);
        return add(&c);
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        KNumberComplex c(p);
        return add(&c);
    } else if (auto const p = dynamic_cast<KNumberComplex *>(rhs)) {
        mpc_add(m_mpc, m_mpc, p->m_mpc, rounding_mode);
        return this;
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        auto e = new KNumberError(p);
        delete this;
        return e;
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberComplex::sub(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        KNumberComplex c(p);
        return sub(&c);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        KNumberComplex c(p);
        return sub(&c);
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        KNumberComplex c(p);
        return sub(&c);
    } else if (auto const p = dynamic_cast<KNumberComplex *>(rhs)) {
        mpc_sub(m_mpc, m_mpc, p->m_mpc, rounding_mode);
        return this;
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        auto e = new KNumberError(p);
        delete this;
        return e->neg();
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberComplex::mul(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        KNumberComplex c(p);
        return mul(&c);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        KNumberComplex c(p);
        return mul(&c);
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        KNumberComplex c(p);
        return mul(&c);
    } else if (auto const p = dynamic_cast<KNumberComplex *>(rhs)) {
        mpc_mul(m_mpc, m_mpc, p->m_mpc, rounding_mode);
        return this;
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

KNumberBase *KNumberComplex::div(KNumberBase *rhs)
{
    if (rhs->isZero()) {
        if (sign() < 0) {
            delete this;
            return new KNumberError(KNumberError::PositiveInfinity);
        } else {
            delete this;
            return new KNumberError(KNumberError::PositiveInfinity);
        }
    }

    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        KNumberComplex c(p);
        return div(&c);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        KNumberComplex c(p);
        return div(&c);
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        KNumberComplex c(p);
        return div(&c);
    } else if (auto const p = dynamic_cast<KNumberComplex *>(rhs)) {
        mpc_div(m_mpc, m_mpc, p->m_mpc, rounding_mode);
        return this;
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

KNumberBase *KNumberComplex::mod(KNumberBase *rhs)
{
    Q_UNUSED(rhs);

    if (rhs->isZero()) {
        delete this;
        return new KNumberError(KNumberError::Undefined);
    }

    delete this;
    return new KNumberInteger(0);
}

KNumberBase *KNumberComplex::bitwiseAnd(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    return new KNumberInteger(0);
}

KNumberBase *KNumberComplex::bitwiseXor(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    return new KNumberInteger(0);
}

KNumberBase *KNumberComplex::bitwiseOr(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    return new KNumberInteger(0);
}

KNumberBase *KNumberComplex::bitwiseShift(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    return new KNumberError(KNumberError::Undefined);
}

KNumberBase *KNumberComplex::neg()
{
    mpc_neg(m_mpc, m_mpc, rounding_mode);
    return this;
}

KNumberBase *KNumberComplex::cmp()
{
    delete this;
    return new KNumberError(KNumberError::Undefined);
}

KNumberBase *KNumberComplex::abs()
{
    mpfr_t abs;
    mpfr_init(abs);
    mpc_abs(abs, m_mpc, KNumberFloat::rounding_mode);

    delete this;
    return new KNumberFloat(abs);
}

KNumberBase *KNumberComplex::sqrt()
{
    mpc_sqrt(m_mpc, m_mpc, rounding_mode);

    return this;
}

KNumberBase *KNumberComplex::cbrt()
{
    mpq_t oneThird;
    mpq_init(oneThird);
    mpq_set_ui(oneThird, 1, 3);

    mpfr_t oneThirdF;
    mpfr_init_set_q(oneThirdF, oneThird, KNumberFloat::rounding_mode);

    mpc_pow_fr(m_mpc, m_mpc, oneThirdF, rounding_mode);

    mpq_clear(oneThird);
    mpfr_clear(oneThirdF);

    return this;
}

KNumberBase *KNumberComplex::factorial()
{
    if (sign() < 0) {
        delete this;
        return new KNumberError(KNumberError::Undefined);
    }

    auto i = new KNumberInteger(this);
    delete this;
    return i->factorial();
}

KNumberBase *KNumberComplex::sin()
{
    return execute_mpc_func<::mpc_sin>();
}

KNumberBase *KNumberComplex::floor()
{
    delete this;
    return new KNumberError(KNumberError::Undefined);
}

KNumberBase *KNumberComplex::ceil()
{
    delete this;
    return new KNumberError(KNumberError::Undefined);
}

KNumberBase *KNumberComplex::cos()
{
    return execute_mpc_func<::mpc_cos>();
}

KNumberBase *KNumberComplex::tan()
{
    return execute_mpc_func<::mpc_tan>();
}

KNumberBase *KNumberComplex::asin()
{
    return execute_mpc_func<::mpc_asin>();
}

KNumberBase *KNumberComplex::acos()
{
    return execute_mpc_func<::mpc_acos>();
}

KNumberBase *KNumberComplex::atan()
{
    return execute_mpc_func<::mpc_atan>();
}

KNumberBase *KNumberComplex::sinh()
{
    return execute_mpc_func<::mpc_sinh>();
}

KNumberBase *KNumberComplex::cosh()
{
    return execute_mpc_func<::mpc_cosh>();
}

KNumberBase *KNumberComplex::tanh()
{
    return execute_mpc_func<::mpc_tanh>();
}

KNumberBase *KNumberComplex::tgamma()
{
    // TODO: MPC does not provide gamma, returning undefined for now
    // NON-Junior Job, go and implement gamma for mpc
    // return execute_mpc_func<::mpfr_gamma>();
    delete this;
    return new KNumberError(KNumberError::Undefined);
}

KNumberBase *KNumberComplex::asinh()
{
    return execute_mpc_func<::mpc_asinh>();
}

KNumberBase *KNumberComplex::acosh()
{
    return execute_mpc_func<::mpc_acosh>();
}

KNumberBase *KNumberComplex::atanh()
{
    return execute_mpc_func<::mpc_atanh>();
}

KNumberBase *KNumberComplex::pow(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        mpc_pow_z(m_mpc, m_mpc, p->m_mpz, rounding_mode);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        mpc_pow_fr(m_mpc, m_mpc, p->m_mpfr, rounding_mode);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        KNumberFloat f(p);
        mpc_pow_fr(m_mpc, m_mpc, f.m_mpfr, rounding_mode);
        return this;
    } else if (auto const p = dynamic_cast<KNumberComplex *>(rhs)) {
        return execute_mpc_func<::mpc_pow>(p->m_mpc);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        mpfr_t norm;
        mpfr_init(norm);
        mpc_norm(norm, m_mpc, KNumberFloat::rounding_mode);
        int inUnitCircle = mpfr_cmpabs_ui(norm, 1);
        mpfr_clear(norm);

        if (inUnitCircle < 0) {
            if (p->m_error == KNumberError::PositiveInfinity) {
                delete this;
                return new KNumberInteger(0);
            } else if (p->m_error == KNumberError::NegativeInfinity) {
                auto e = new KNumberError(KNumberError::ComplexInfinity);
                delete this;
                return e;
            } else {
                auto e = new KNumberError(KNumberError::Undefined);
                delete this;
                return e;
            }
        } else if (inUnitCircle > 0) {
            if (p->m_error == KNumberError::PositiveInfinity) {
                auto e = new KNumberError(KNumberError::ComplexInfinity);
                delete this;
                return e;
            } else if (p->m_error == KNumberError::NegativeInfinity) {
                delete this;
                return new KNumberInteger(0);
            } else {
                auto e = new KNumberError(KNumberError::Undefined);
                delete this;
                return e;
            }
        } else {
            auto e = new KNumberError(KNumberError::Undefined);
            delete this;
            return e;
        }
    }

    Q_ASSERT(0);
    delete this;
    return nullptr;
}

int KNumberComplex::compare(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        KNumberComplex c(p);
        return compare(&c);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        KNumberComplex c(p);
        return compare(&c);
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        KNumberComplex c(p);
        return compare(&c);
    } else if (auto const p = dynamic_cast<KNumberComplex *>(rhs)) {
        return mpc_cmp(m_mpc, p->m_mpc);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        // NOTE: any number compared to NaN/Inf/-Inf always compares less
        //       at the moment
        return -1;
    }

    Q_ASSERT(0);
    return 0;
}

QString KNumberComplex::toString(int precision) const
{
    size_t n = static_cast<size_t>(precision);

    char *buf = mpc_get_str(10, n, m_mpc, rounding_mode);
    QScopedPointer<char, KNumberCleanupString<mpc_free_str>> array(buf);

    return QLatin1String(&buf[0]);
}

bool KNumberComplex::isReal() const
{
    return mpfr_zero_p(mpc_imagref(m_mpc));
}

bool KNumberComplex::isInteger() const
{
    return mpfr_integer_p(mpc_realref(m_mpc)) && mpfr_zero_p(mpc_imagref(m_mpc));
}

bool KNumberComplex::isZero() const
{
    return mpfr_zero_p(mpc_realref(m_mpc)) && mpfr_zero_p(mpc_imagref(m_mpc));
}

int KNumberComplex::sign() const
{
    // TODO: what to return here?
    return -1;
}

KNumberBase *KNumberComplex::reciprocal()
{
    mpc_t unit;
    mpc_init2(unit, precision);
    mpc_set_d(unit, 1.0, rounding_mode);
    mpc_div(m_mpc, unit, m_mpc, rounding_mode);
    mpc_clear(unit);
    return this;
}

KNumberBase *KNumberComplex::log2()
{
    // TODO: MPC does not provide log2, it can be implemented
    //  by doing a division using the loge
    // return execute_mpc_func<::mpc_log2>();
    if (isZero()) {
        delete this;
        return new KNumberError(KNumberError::NegativeInfinity);
    } else {
        delete this;
        return new KNumberError(KNumberError::Undefined);
    }
}

KNumberBase *KNumberComplex::log10()
{
    if (isZero()) {
        delete this;
        return new KNumberError(KNumberError::NegativeInfinity);
    } else {
        return execute_mpc_func<::mpc_log10>();
    }
}

KNumberBase *KNumberComplex::ln()
{
    if (isZero()) {
        delete this;
        return new KNumberError(KNumberError::NegativeInfinity);
    } else {
        return execute_mpc_func<::mpc_log>();
    }
}

KNumberBase *KNumberComplex::exp2()
{
    // TODO: MPC does not provide mpc_exp2, it can be implemented
    //  as exp(x*ln2)
    delete this;
    return new KNumberError(KNumberError::Undefined);
}

KNumberBase *KNumberComplex::exp10()
{
    mpfr_t ln10;
    mpfr_init(ln10);
    mpfr_log_ui(ln10, 10, KNumberFloat::rounding_mode);
    mpc_mul_fr(m_mpc, m_mpc, ln10, rounding_mode);
    mpc_exp(m_mpc, m_mpc, rounding_mode);
    mpfr_clear(ln10);

    return this;
}

KNumberBase *KNumberComplex::exp()
{
    return execute_mpc_func<::mpc_exp>();
}

KNumberBase *KNumberComplex::arg()
{
    mpfr_t arg;
    mpfr_init(arg);
    mpc_arg(arg, m_mpc, KNumberFloat::rounding_mode);
    auto a = new KNumberFloat(arg);
    mpfr_clear(arg);
    delete this;
    return a;
}

KNumberBase *KNumberComplex::conj()
{
    mpc_conj(m_mpc, m_mpc, rounding_mode);
    return this;
}

quint64 KNumberComplex::toUint64() const
{
    return KNumberInteger(this).toUint64();
}

qint64 KNumberComplex::toInt64() const
{
    return KNumberInteger(this).toInt64();
}

KNumberBase *KNumberComplex::bin(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    return new KNumberError(KNumberError::Undefined);
}

}
