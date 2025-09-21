/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knumber_fraction.h"
#include "knumber_complex.h"
#include "knumber_error.h"
#include "knumber_float.h"
#include "knumber_integer.h"
#include <QScopedArrayPointer>
#include <config-knumber.h>

#include <mpfr.h>

namespace detail
{
bool KNumberFraction::defaultFractionalInput = false;
bool KNumberFraction::defaultFractionalOutput = true;
bool KNumberFraction::splitOffIntegerForFractionOutput = false;

void KNumberFraction::setDefaultFractionalInput(bool value)
{
    defaultFractionalInput = value;
}

void KNumberFraction::setDefaultFractionalOutput(bool value)
{
    defaultFractionalOutput = value;
}

void KNumberFraction::setSplitOffIntegerForFractionOutput(bool value)
{
    splitOffIntegerForFractionOutput = value;
}

KNumberFraction::KNumberFraction(const QString &s)
{
    mpq_init(m_mpq);
    mpq_set_str(m_mpq, s.toLatin1().constData(), 10);
    mpq_canonicalize(m_mpq);
}

KNumberFraction::KNumberFraction(qint64 num, quint64 den)
{
    mpq_init(m_mpq);
    mpq_set_si(m_mpq, num, den);
    mpq_canonicalize(m_mpq);
}

KNumberFraction::KNumberFraction(quint64 num, quint64 den)
{
    mpq_init(m_mpq);
    mpq_set_ui(m_mpq, num, den);
    mpq_canonicalize(m_mpq);
}

KNumberFraction::KNumberFraction(mpq_t mpq)
{
    mpq_init(m_mpq);
    mpq_set(m_mpq, mpq);
}

KNumberFraction::KNumberFraction(const KNumberFraction *value)
{
    mpq_init(m_mpq);
    mpq_set(m_mpq, value->m_mpq);
}

KNumberFraction::KNumberFraction(const KNumberInteger *value)
{
    mpq_init(m_mpq);
    mpq_set_z(m_mpq, value->m_mpz);
}

#if 0

KNumberFraction::KNumberFraction(const KNumberFloat *value) {
	mpq_init(m_mpq);
	mpq_set_f(m_mpq, value->mpf_);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumberFraction::KNumberFraction(const KNumberComplex *value) {
	mpq_init(m_mpq);
	mpq_set_c(m_mpq, value->mpf_);
}
#endif

KNumberBase *KNumberFraction::clone()
{
    return new KNumberFraction(this);
}

KNumberFraction::~KNumberFraction()
{
    mpq_clear(m_mpq);
}

bool KNumberFraction::isReal() const
{
    return true;
}

bool KNumberFraction::isInteger() const
{
    return (mpz_cmp_ui(mpq_denref(m_mpq), 1) == 0);
}

KNumberBase *KNumberFraction::add(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        KNumberFraction q(p);
        mpq_add(m_mpq, m_mpq, q.m_mpq);
        return this;
    } else if (auto const p = dynamic_cast<KNumberComplex *>(rhs)) {
        auto f = new KNumberComplex(this);
        delete this;
        return f->add(p);
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        auto f = new KNumberFloat(this);
        delete this;
        return f->add(p);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        mpq_add(m_mpq, m_mpq, p->m_mpq);
        return this;
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        auto e = new KNumberError(p);
        delete this;
        return e;
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberFraction::sub(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        KNumberFraction q(p);
        mpq_sub(m_mpq, m_mpq, q.m_mpq);
        return this;
    } else if (auto const p = dynamic_cast<KNumberComplex *>(rhs)) {
        auto f = new KNumberComplex(this);
        delete this;
        return f->sub(p);
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        auto f = new KNumberFloat(this);
        delete this;
        return f->sub(p);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        mpq_sub(m_mpq, m_mpq, p->m_mpq);
        return this;
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        auto e = new KNumberError(p);
        delete this;
        return e->neg();
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberFraction::mul(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        KNumberFraction q(p);
        mpq_mul(m_mpq, m_mpq, q.m_mpq);
        return this;
    } else if (auto const p = dynamic_cast<KNumberComplex *>(rhs)) {
        auto q = new KNumberComplex(this);
        delete this;
        return q->mul(p);
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        auto q = new KNumberFloat(this);
        delete this;
        return q->mul(p);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        mpq_mul(m_mpq, m_mpq, p->m_mpq);
        return this;
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        if (isZero()) {
            delete this;
            auto e = new KNumberError(KNumberError::Undefined);
            return e;
        }

        if (sign() < 0) {
            delete this;
            auto e = new KNumberError(p);
            return e->neg();
        } else {
            delete this;
            auto e = new KNumberError(p);
            return e;
        }
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberFraction::div(KNumberBase *rhs)
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
        KNumberFraction f(p);
        return div(&f);
    } else if (auto const p = dynamic_cast<KNumberComplex *>(rhs)) {
        auto f = new KNumberComplex(this);
        delete this;
        return f->div(p);
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        auto f = new KNumberFloat(this);
        delete this;
        return f->div(p);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        mpq_div(m_mpq, m_mpq, p->m_mpq);
        return this;
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        if (p->sign() > 0) {
            delete this;
            return new KNumberInteger(0);
        } else if (p->sign() < 0) {
            delete this;
            return new KNumberInteger(0);
        }

        auto e = new KNumberError(p);
        delete this;
        return e;
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberFraction::mod(KNumberBase *rhs)
{
    if (rhs->isZero()) {
        delete this;
        return new KNumberError(KNumberError::Undefined);
    }

    // NOTE: we don't support modulus operations with non-integer operands
    mpq_set_d(m_mpq, 0);
    return this;
}

KNumberBase *KNumberFraction::bitwiseAnd(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    // NOTE: we don't support bitwise operations with non-integer operands
    return new KNumberInteger(0);
}

KNumberBase *KNumberFraction::bitwiseXor(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    // NOTE: we don't support bitwise operations with non-integer operands
    return new KNumberInteger(0);
}

KNumberBase *KNumberFraction::bitwiseOr(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    // NOTE: we don't support bitwise operations with non-integer operands
    return new KNumberInteger(0);
}

KNumberBase *KNumberFraction::bitwiseShift(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    // NOTE: we don't support bitwise operations with non-integer operands
    return new KNumberError(KNumberError::Undefined);
}

KNumberBase *KNumberFraction::neg()
{
    mpq_neg(m_mpq, m_mpq);
    return this;
}

KNumberBase *KNumberFraction::abs()
{
    mpq_abs(m_mpq, m_mpq);
    return this;
}

KNumberBase *KNumberFraction::cmp()
{
    delete this;
    return new KNumberError(KNumberError::Undefined);
}

KNumberBase *KNumberFraction::sqrt()
{
    if (sign() < 0) {
        auto f = new KNumberComplex(this);
        delete this;
        return f->sqrt();
    }

    if (mpz_perfect_square_p(mpq_numref(m_mpq)) && mpz_perfect_square_p(mpq_denref(m_mpq))) {
        mpz_t num;
        mpz_t den;
        mpz_init(num);
        mpz_init(den);
        mpq_get_num(num, m_mpq);
        mpq_get_den(den, m_mpq);
        mpz_sqrt(num, num);
        mpz_sqrt(den, den);
        mpq_set_num(m_mpq, num);
        mpq_set_den(m_mpq, den);
        mpq_canonicalize(m_mpq);
        mpz_clear(num);
        mpz_clear(den);
        return this;
    } else {
        auto f = new KNumberFloat(this);
        delete this;
        return f->sqrt();
    }
}

KNumberBase *KNumberFraction::cbrt()
{
    // TODO: figure out how to properly use mpq_numref/mpq_denref here
    mpz_t num;
    mpz_t den;

    mpz_init(num);
    mpz_init(den);

    mpq_get_num(num, m_mpq);
    mpq_get_den(den, m_mpq);

    if (mpz_root(num, num, 3) && mpz_root(den, den, 3)) {
        mpq_set_num(m_mpq, num);
        mpq_set_den(m_mpq, den);
        mpq_canonicalize(m_mpq);
        mpz_clear(num);
        mpz_clear(den);
        return this;
    } else {
        mpz_clear(num);
        mpz_clear(den);
        auto f = new KNumberFloat(this);
        delete this;
        return f->cbrt();
    }
}

KNumberBase *KNumberFraction::factorial()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->factorial();
}

KNumberBase *KNumberFraction::pow(KNumberBase *rhs)
{
    // TODO: figure out how to properly use mpq_numref/mpq_denref here
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        mpz_t num;
        mpz_t den;

        mpz_init(num);
        mpz_init(den);

        mpq_get_num(num, m_mpq);
        mpq_get_den(den, m_mpq);

        mpz_pow_ui(num, num, mpz_get_ui(p->m_mpz));
        mpz_pow_ui(den, den, mpz_get_ui(p->m_mpz));
        mpq_set_num(m_mpq, num);
        mpq_set_den(m_mpq, den);
        mpq_canonicalize(m_mpq);
        mpz_clear(num);
        mpz_clear(den);

        if (p->sign() < 0) {
            return reciprocal();
        } else {
            return this;
        }
    } else if (auto const p = dynamic_cast<KNumberComplex *>(rhs)) {
        Q_UNUSED(p);
        auto f = new KNumberComplex(this);
        delete this;
        return f->pow(rhs);
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        Q_UNUSED(p);
        auto f = new KNumberFloat(this);
        delete this;
        return f->pow(rhs);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        // ok, so if any part of the number is > 1,000,000, then we risk
        // the pow function overflowing... so we'll just convert to float to be safe
        // TODO: at some point, we should figure out exactly what the threshold is
        //       and if there is a better way to determine if the pow function will
        //       overflow.
        if (mpz_cmpabs_ui(mpq_numref(m_mpq), 1000000) > 0 || mpz_cmpabs_ui(mpq_denref(m_mpq), 1000000) > 0 || mpz_cmpabs_ui(mpq_numref(p->m_mpq), 1000000) > 0
            || mpz_cmpabs_ui(mpq_denref(p->m_mpq), 1000000) > 0) {
            auto f = new KNumberFloat(this);
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

        mpq_get_num(lhs_num, m_mpq);
        mpq_get_den(lhs_den, m_mpq);
        mpq_get_num(rhs_num, p->m_mpq);
        mpq_get_den(rhs_den, p->m_mpq);

        mpz_pow_ui(lhs_num, lhs_num, mpz_get_ui(rhs_num));
        mpz_pow_ui(lhs_den, lhs_den, mpz_get_ui(rhs_num));

        if (mpz_sgn(lhs_num) < 0 && mpz_even_p(rhs_den)) {
            mpz_clear(lhs_num);
            mpz_clear(lhs_den);
            mpz_clear(rhs_num);
            mpz_clear(rhs_den);
            auto c = new KNumberComplex(this);
            delete this;
            return c->pow(p);
        }

        if (mpz_sgn(lhs_den) < 0 && mpz_even_p(rhs_den)) {
            mpz_clear(lhs_num);
            mpz_clear(lhs_den);
            mpz_clear(rhs_num);
            mpz_clear(rhs_den);
            auto c = new KNumberComplex(this);
            delete this;
            return c->pow(p);
        }

        const int n1 = mpz_root(lhs_num, lhs_num, mpz_get_ui(rhs_den));
        const int n2 = mpz_root(lhs_den, lhs_den, mpz_get_ui(rhs_den));

        if (n1 && n2) {
            mpq_set_num(m_mpq, lhs_num);
            mpq_set_den(m_mpq, lhs_den);
            mpq_canonicalize(m_mpq);
            mpz_clear(lhs_num);
            mpz_clear(lhs_den);
            mpz_clear(rhs_num);
            mpz_clear(rhs_den);

            if (p->sign() < 0) {
                return reciprocal();
            } else {
                return this;
            }
        } else {
            mpz_clear(lhs_num);
            mpz_clear(lhs_den);
            mpz_clear(rhs_num);
            mpz_clear(rhs_den);
            auto f = new KNumberFloat(this);
            delete this;

            return f->pow(rhs);
        }

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

KNumberBase *KNumberFraction::sin()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->sin();
}

KNumberBase *KNumberFraction::floor()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->floor();
}

KNumberBase *KNumberFraction::ceil()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->ceil();
}

KNumberBase *KNumberFraction::cos()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->cos();
}

KNumberBase *KNumberFraction::tgamma()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->tgamma();
}

KNumberBase *KNumberFraction::tan()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->tan();
}

KNumberBase *KNumberFraction::asin()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->asin();
}

KNumberBase *KNumberFraction::acos()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->acos();
}

KNumberBase *KNumberFraction::atan()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->atan();
}

KNumberBase *KNumberFraction::sinh()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->sinh();
}

KNumberBase *KNumberFraction::cosh()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->cosh();
}

KNumberBase *KNumberFraction::tanh()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->tanh();
}

KNumberBase *KNumberFraction::asinh()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->asinh();
}

KNumberBase *KNumberFraction::acosh()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->acosh();
}

KNumberBase *KNumberFraction::atanh()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->atanh();
}

int KNumberFraction::compare(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        KNumberFraction f(p);
        return mpq_cmp(m_mpq, f.m_mpq);
    } else if (auto const p = dynamic_cast<KNumberComplex *>(rhs)) {
        KNumberComplex f(this);
        return f.compare(p);
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        KNumberFloat f(this);
        return f.compare(p);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        return mpq_cmp(m_mpq, p->m_mpq);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        // NOTE: any number compared to NaN/Inf/-Inf always compares less
        //       at the moment
        return -1;
    }

    Q_ASSERT(0);
    return 0;
}

QString KNumberFraction::toString(int precision) const
{
    if (KNumberFraction::defaultFractionalOutput) {
        // TODO: figure out how to properly use mpq_numref/mpq_denref here

        KNumberInteger integer_part(this);
        if (splitOffIntegerForFractionOutput && !integer_part.isZero()) {
            mpz_t num;
            mpz_init(num);
            mpq_get_num(num, m_mpq);

            KNumberInteger integer_part_1(this);

            mpz_mul(integer_part.m_mpz, integer_part.m_mpz, mpq_denref(m_mpq));
            mpz_sub(num, num, integer_part.m_mpz);

            if (mpz_sgn(num) < 0) {
                mpz_neg(num, num);
            }

            const size_t size = gmp_snprintf(nullptr, 0, "%Zd %Zd/%Zd", integer_part_1.m_mpz, num, mpq_denref(m_mpq)) + 1;
            QScopedArrayPointer<char> buf(new char[size]);
            gmp_snprintf(&buf[0], size, "%Zd %Zd/%Zd", integer_part_1.m_mpz, num, mpq_denref(m_mpq));

            mpz_clear(num);

            return QLatin1String(&buf[0]);
        } else {
            mpz_t num;
            mpz_init(num);
            mpq_get_num(num, m_mpq);

            const size_t size = gmp_snprintf(nullptr, 0, "%Zd/%Zd", num, mpq_denref(m_mpq)) + 1;
            QScopedArrayPointer<char> buf(new char[size]);
            gmp_snprintf(&buf[0], size, "%Zd/%Zd", num, mpq_denref(m_mpq));

            mpz_clear(num);

            return QLatin1String(&buf[0]);
        }
    } else {
        return KNumberFloat(this).toString(precision);
    }
}

bool KNumberFraction::isZero() const
{
    return mpq_sgn(m_mpq) == 0;
}

int KNumberFraction::sign() const
{
    return mpq_sgn(m_mpq);
}

KNumberBase *KNumberFraction::reciprocal()
{
    mpq_inv(m_mpq, m_mpq);
    return this;
}

KNumberInteger *KNumberFraction::numerator() const
{
    mpz_t num;
    mpz_init(num);
    mpq_get_num(num, m_mpq);
    auto n = new KNumberInteger(num);
    mpz_clear(num);
    return n;
}

KNumberInteger *KNumberFraction::denominator() const
{
    mpz_t den;
    mpz_init(den);
    mpq_get_den(den, m_mpq);
    auto n = new KNumberInteger(den);
    mpz_clear(den);
    return n;
}

KNumberBase *KNumberFraction::log2()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->log2();
}

KNumberBase *KNumberFraction::log10()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->log10();
}

KNumberBase *KNumberFraction::ln()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->ln();
}

KNumberBase *KNumberFraction::exp2()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->exp2();
}

KNumberBase *KNumberFraction::exp10()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->exp10();
}

KNumberBase *KNumberFraction::exp()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->exp();
}

KNumberBase *KNumberFraction::realPart()
{
    return this;
}

KNumberBase *KNumberFraction::imaginaryPart()
{
    auto z = new KNumberInteger(0);
    delete this;
    return z;
}

KNumberBase *KNumberFraction::arg()
{
    if (sign() >= 0) {
        auto z = new KNumberInteger(0);
        delete this;
        return z;
    } else {
        delete this;
        return new KNumberFloat(M_PI);
    }
}

KNumberBase *KNumberFraction::conj()
{
    return this;
}

quint64 KNumberFraction::toUint64() const
{
    return KNumberInteger(this).toUint64();
}

qint64 KNumberFraction::toInt64() const
{
    return KNumberInteger(this).toInt64();
}

KNumberBase *KNumberFraction::bin(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    delete this;
    return new KNumberError(KNumberError::Undefined);
}
}
