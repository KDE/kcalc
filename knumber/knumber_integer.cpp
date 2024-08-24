/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knumber_integer.h"
#include "knumber_error.h"
#include "knumber_float.h"
#include "knumber_fraction.h"
#include <QScopedArrayPointer>
#include <config-kcalc.h>

namespace detail
{

KNumberInteger::KNumberInteger(const QString &s)
{
    mpz_init(m_mpz);
    mpz_set_str(m_mpz, s.toLatin1().constData(), 10);
}

KNumberInteger::KNumberInteger(qint32 value)
{
    mpz_init_set_si(m_mpz, static_cast<signed long int>(value));
}

KNumberInteger::KNumberInteger(qint64 value)
{
    mpz_init(m_mpz);
#if SIZEOF_SIGNED_LONG == 8
    mpz_set_si(m_mpz, static_cast<signed long int>(value));
#elif SIZEOF_SIGNED_LONG == 4
    mpz_set_si(m_mpz, static_cast<signed long int>(value >> 32));
    mpz_mul_2exp(m_mpz, m_mpz, 32);
    mpz_add_ui(m_mpz, m_mpz, static_cast<signed long int>(value));
#else
#error "SIZEOF_SIGNED_LONG is a unhandled case"
#endif
}

KNumberInteger::KNumberInteger(quint32 value)
{
    mpz_init_set_ui(m_mpz, static_cast<unsigned long int>(value));
}

KNumberInteger::KNumberInteger(quint64 value)
{
    mpz_init(m_mpz);
#if SIZEOF_UNSIGNED_LONG == 8
    mpz_set_ui(m_mpz, static_cast<unsigned long int>(value));
#elif SIZEOF_UNSIGNED_LONG == 4
    mpz_set_ui(m_mpz, static_cast<unsigned long int>(value >> 32));
    mpz_mul_2exp(m_mpz, m_mpz, 32);
    mpz_add_ui(m_mpz, m_mpz, static_cast<unsigned long int>(value));
#else
#error "SIZEOF_UNSIGNED_LONG is a unhandled case"
#endif
}

KNumberInteger::KNumberInteger(mpz_t mpz)
{
    mpz_init_set(m_mpz, mpz);
}

KNumberInteger::KNumberInteger(const KNumberInteger *value)
{
    mpz_init_set(m_mpz, value->m_mpz);
}

KNumberInteger::KNumberInteger(const KNumberFloat *value)
{
    mpz_init(m_mpz);

    mpf_t mpf;
    mpf_init(mpf);

    mpfr_get_f(mpf, value->m_mpfr, KNumberFloat::rounding_mode);
    mpz_set_f(m_mpz, mpf);

    mpf_clear(mpf);
}

KNumberInteger::KNumberInteger(const KNumberFraction *value)
{
    mpz_init(m_mpz);
    mpz_set_q(m_mpz, value->m_mpq);
}

KNumberBase *KNumberInteger::clone()
{
    return new KNumberInteger(this);
}

KNumberInteger::~KNumberInteger()
{
    mpz_clear(m_mpz);
}

KNumberBase *KNumberInteger::add(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        mpz_add(m_mpz, m_mpz, p->m_mpz);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        auto const f = new KNumberFloat(this);
        delete this;
        return f->add(p);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        auto const q = new KNumberFraction(this);
        delete this;
        return q->add(p);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        delete this;
        return p->clone();
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberInteger::sub(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        mpz_sub(m_mpz, m_mpz, p->m_mpz);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        auto f = new KNumberFloat(this);
        delete this;
        return f->sub(p);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        auto q = new KNumberFraction(this);
        delete this;
        return q->sub(p);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        KNumberBase *e = p->clone();
        delete this;
        return e->neg();
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberInteger::mul(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        mpz_mul(m_mpz, m_mpz, p->m_mpz);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        auto f = new KNumberFloat(this);
        delete this;
        return f->mul(p);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        auto q = new KNumberFraction(this);
        delete this;
        return q->mul(p);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        if (isZero()) {
            delete this;
            auto e = new KNumberError(KNumberError::Undefined);
            return e->neg();
        }

        if (sign() < 0) {
            delete this;
            KNumberBase *e = p->clone();
            return e->neg();
        } else {
            delete this;
            return p->clone();
        }
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberInteger::div(KNumberBase *rhs)
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
        auto q = new KNumberFraction(this);
        delete this;
        return q->div(p);
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        auto f = new KNumberFloat(this);
        delete this;
        return f->div(p);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        auto q = new KNumberFraction(this);
        delete this;
        return q->div(p);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        if (p->sign() > 0) {
            delete this;
            return new KNumberInteger(0);
        } else if (p->sign() < 0) {
            delete this;
            return new KNumberInteger(0);
        }

        delete this;
        return p->clone();
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberInteger::mod(KNumberBase *rhs)
{
    if (rhs->isZero()) {
        delete this;
        return new KNumberError(KNumberError::Undefined);
    }

    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        mpz_mod(m_mpz, m_mpz, p->m_mpz);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        auto f = new KNumberFloat(this);
        delete this;
        return f->mod(p);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        auto q = new KNumberFraction(this);
        delete this;
        return q->mod(p);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        delete this;
        return p->clone();
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberInteger::bitwiseAnd(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        mpz_and(m_mpz, m_mpz, p->m_mpz);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        auto f = new KNumberFloat(this);
        delete this;
        return f->bitwiseAnd(p);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        auto f = new KNumberFraction(this);
        delete this;
        return f->bitwiseAnd(p);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        delete this;
        return p->clone();
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberInteger::bitwiseXor(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        mpz_xor(m_mpz, m_mpz, p->m_mpz);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        auto f = new KNumberFloat(this);
        delete this;
        return f->bitwiseXor(p);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        auto f = new KNumberFraction(this);
        delete this;
        return f->bitwiseXor(p);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        delete this;
        return p->clone();
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberInteger::bitwiseOr(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        mpz_ior(m_mpz, m_mpz, p->m_mpz);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        auto f = new KNumberFloat(this);
        delete this;
        return f->bitwiseOr(p);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        auto f = new KNumberFraction(this);
        delete this;
        return f->bitwiseOr(p);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        delete this;
        return p->clone();
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberInteger::bitwiseShift(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        const signed long int bit_count = mpz_get_si(p->m_mpz);

        // TODO: left shift with high bit set is broken in
        //       non decimal modes :-/, always displays 0
        //       interestingly, the bit is not "lost"
        //       we simply don't have a mechanism to display
        //       values in HEX/DEC/OCT mode which are greater than
        //       64-bits

        if (bit_count > 0) {
            // left shift
            mpz_mul_2exp(m_mpz, m_mpz, bit_count);
        } else if (bit_count < 0) {
            // right shift
            if (mpz_sgn(m_mpz) < 0) {
                mpz_fdiv_q_2exp(m_mpz, m_mpz, -bit_count);
            } else {
                mpz_tdiv_q_2exp(m_mpz, m_mpz, -bit_count);
            }
        }
        return this;
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        Q_UNUSED(p);
        auto e = new KNumberError(KNumberError::Undefined);
        delete this;
        return e;
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        Q_UNUSED(p);
        auto e = new KNumberError(KNumberError::Undefined);
        delete this;
        return e;
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        Q_UNUSED(p);
        auto e = new KNumberError(KNumberError::Undefined);
        delete this;
        return e;
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberInteger::neg()
{
    mpz_neg(m_mpz, m_mpz);
    return this;
}

KNumberBase *KNumberInteger::cmp()
{
#if 0
	// unfortunately this breaks things pretty badly
	// for non-decimal modes :-(
	mpz_com(mpz_, mpz_);
#else
    mpz_swap(m_mpz, KNumberInteger(~toUint64()).m_mpz);
#endif
    return this;
}

KNumberBase *KNumberInteger::abs()
{
    mpz_abs(m_mpz, m_mpz);
    return this;
}

KNumberBase *KNumberInteger::sqrt()
{
    if (sign() < 0) {
        delete this;
        return new KNumberError(KNumberError::Undefined);
    }

    if (mpz_perfect_square_p(m_mpz)) {
        mpz_sqrt(m_mpz, m_mpz);
        return this;
    } else {
        auto f = new KNumberFloat(this);
        delete this;
        return f->sqrt();
    }
}

KNumberBase *KNumberInteger::cbrt()
{
    mpz_t x;
    mpz_init_set(x, m_mpz);
    if (mpz_root(x, x, 3)) {
        mpz_swap(m_mpz, x);
        mpz_clear(x);
        return this;
    }

    mpz_clear(x);
    auto f = new KNumberFloat(this);
    delete this;
    return f->cbrt();
}

KNumberBase *KNumberInteger::pow(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        if (isZero() && p->isEven() && p->sign() < 0) {
            delete this;
            return new KNumberError(KNumberError::PositiveInfinity);
        }

        mpz_pow_ui(m_mpz, m_mpz, mpz_get_ui(p->m_mpz));

        if (p->sign() < 0) {
            return reciprocal();
        } else {
            return this;
        }
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        auto f = new KNumberFloat(this);
        delete this;
        return f->pow(p);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        auto f = new KNumberFraction(this);
        delete this;
        return f->pow(p);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        if (p->sign() > 0) {
            auto e = new KNumberError(KNumberError::PositiveInfinity);
            delete this;
            return e;
        } else if (p->sign() < 0) {
            mpz_init_set_si(m_mpz, 0);
            return this;
        } else {
            auto e = new KNumberError(KNumberError::Undefined);
            delete this;
            return e;
        }
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberInteger::sin()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->sin();
}

KNumberBase *KNumberInteger::cos()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->cos();
}

KNumberBase *KNumberInteger::tan()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->tan();
}

KNumberBase *KNumberInteger::asin()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->asin();
}

KNumberBase *KNumberInteger::acos()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->acos();
}

KNumberBase *KNumberInteger::atan()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->atan();
}

KNumberBase *KNumberInteger::tgamma()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->tgamma();
}

KNumberBase *KNumberInteger::sinh()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->sinh();
}

KNumberBase *KNumberInteger::cosh()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->cosh();
}

KNumberBase *KNumberInteger::tanh()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->tanh();
}

KNumberBase *KNumberInteger::asinh()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->asinh();
}

KNumberBase *KNumberInteger::acosh()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->acosh();
}

KNumberBase *KNumberInteger::atanh()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->atanh();
}

KNumberBase *KNumberInteger::factorial()
{
    if (sign() < 0) {
        delete this;
        return new KNumberError(KNumberError::Undefined);
    }

    mpz_fac_ui(m_mpz, mpz_get_ui(m_mpz));
    return this;
}

int KNumberInteger::compare(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        return mpz_cmp(m_mpz, p->m_mpz);
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        return KNumberFloat(this).compare(p);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        return KNumberFraction(this).compare(p);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        // NOTE: any number compared to NaN/Inf/-Inf always compares less
        //       at the moment
        return -1;
    }

    Q_ASSERT(0);
    return 0;
}

QString KNumberInteger::toString(int precision) const
{
    Q_UNUSED(precision);

    const size_t size = gmp_snprintf(nullptr, 0, "%Zd", m_mpz) + 1;
    QScopedArrayPointer<char> buf(new char[size]);
    gmp_snprintf(&buf[0], size, "%Zd", m_mpz);
    return QLatin1String(&buf[0]);
}

quint64 KNumberInteger::toUint64() const
{
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

qint64 KNumberInteger::toInt64() const
{
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

bool KNumberInteger::isInteger() const
{
    return true;
}

bool KNumberInteger::isZero() const
{
    return mpz_sgn(m_mpz) == 0;
}

int KNumberInteger::sign() const
{
    return mpz_sgn(m_mpz);
}

bool KNumberInteger::isEven() const
{
    return mpz_even_p(m_mpz);
}

bool KNumberInteger::isOdd() const
{
    return mpz_odd_p(m_mpz);
}

KNumberBase *KNumberInteger::reciprocal()
{
    auto q = new KNumberFraction(this);
    delete this;
    return q->reciprocal();
}

KNumberBase *KNumberInteger::log2()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->log2();
}

KNumberBase *KNumberInteger::floor()
{
    // should have no effect on the value
    return this;
}

KNumberBase *KNumberInteger::ceil()
{
    // should have no effect on the value
    return this;
}

KNumberBase *KNumberInteger::log10()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->log10();
}

KNumberBase *KNumberInteger::ln()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->ln();
}

KNumberBase *KNumberInteger::exp2()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->exp2();
}

KNumberBase *KNumberInteger::exp10()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->exp10();
}

KNumberBase *KNumberInteger::exp()
{
    auto f = new KNumberFloat(this);
    delete this;
    return f->exp();
}

KNumberBase *KNumberInteger::bin(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        mpz_bin_ui(m_mpz, m_mpz, mpz_get_ui(p->m_mpz));
        return this;

    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        delete this;
        return new KNumberError(KNumberError::Undefined);
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        delete this;
        return new KNumberError(KNumberError::Undefined);
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        delete this;
        return new KNumberError(KNumberError::Undefined);
    }

    Q_ASSERT(0);
    return nullptr;
}

}
