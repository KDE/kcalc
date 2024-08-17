/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knumber_error.h"
#include "knumber_float.h"
#include "knumber_fraction.h"
#include "knumber_integer.h"
#include <cmath> // for M_PI
#include <config-kcalc.h>

namespace detail
{

KNumberError::KNumberError(Error e)
    : m_error(e)
{
}

KNumberError::KNumberError(const QString &s)
{
    if (s == QLatin1String("nan"))
        m_error = Undefined;
    else if (s == QLatin1String("inf"))
        m_error = PositiveInfinity;
    else if (s == QLatin1String("-inf"))
        m_error = NegativeInfinity;
    else
        m_error = Undefined;
}

KNumberError::KNumberError()
    : m_error(Undefined)
{
}

KNumberError::~KNumberError() = default;

KNumberError::KNumberError(const KNumberInteger *)
    : m_error(Undefined)
{
}

KNumberError::KNumberError(const KNumberFraction *)
    : m_error(Undefined)
{
}

KNumberError::KNumberError(const KNumberFloat *)
    : m_error(Undefined)
{
}

KNumberError::KNumberError(const KNumberError *value)
    : m_error(value->m_error)
{
}

QString KNumberError::toString(int precision) const
{
    Q_UNUSED(precision);

    switch (m_error) {
    case PositiveInfinity:
        return QStringLiteral("inf");
    case NegativeInfinity:
        return QStringLiteral("-inf");
    case Undefined:
    default:
        return QStringLiteral("nan");
    }
}

KNumberBase *KNumberError::add(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        Q_UNUSED(p);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        Q_UNUSED(p);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        Q_UNUSED(p);
        return this;
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        if (m_error == PositiveInfinity && p->m_error == NegativeInfinity) {
            m_error = Undefined;
        } else if (m_error == NegativeInfinity && p->m_error == PositiveInfinity) {
            m_error = Undefined;
        } else if (p->m_error == Undefined) {
            m_error = Undefined;
        }
        return this;
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberError::sub(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        Q_UNUSED(p);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        Q_UNUSED(p);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        Q_UNUSED(p);
        return this;
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        if (m_error == PositiveInfinity && p->m_error == PositiveInfinity) {
            m_error = Undefined;
        } else if (m_error == NegativeInfinity && p->m_error == NegativeInfinity) {
            m_error = Undefined;
        } else if (p->m_error == Undefined) {
            m_error = Undefined;
        }
        return this;
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberError::mul(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        if (p->isZero()) {
            m_error = Undefined;
        }
        return this;
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        if (p->isZero()) {
            m_error = Undefined;
        }
        return this;
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        if (p->isZero()) {
            m_error = Undefined;
        }
        return this;
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        if (m_error == PositiveInfinity && p->m_error == NegativeInfinity) {
            m_error = NegativeInfinity;
        } else if (m_error == NegativeInfinity && p->m_error == PositiveInfinity) {
            m_error = NegativeInfinity;
        } else if (m_error == NegativeInfinity && p->m_error == NegativeInfinity) {
            m_error = PositiveInfinity;
        } else if (p->m_error == Undefined) {
            m_error = Undefined;
        }
        return this;
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberError::div(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        Q_UNUSED(p);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        Q_UNUSED(p);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        Q_UNUSED(p);
        return this;
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        Q_UNUSED(p);
        m_error = Undefined;
        return this;
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberError::mod(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        Q_UNUSED(p);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        Q_UNUSED(p);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        Q_UNUSED(p);
        return this;
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        Q_UNUSED(p);
        m_error = Undefined;
        return this;
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberError::pow(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        Q_UNUSED(p);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        Q_UNUSED(p);
        return this;
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        Q_UNUSED(p);
        return this;
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        switch (m_error) {
        case PositiveInfinity:
            if (p->sign() > 0) {
                return this;
            } else if (p->sign() < 0) {
                auto n = new KNumberInteger(0);
                delete this;
                return n;
            } else {
                m_error = Undefined;
                return this;
            }
            break;
        case NegativeInfinity:
            if (p->sign() > 0) {
                m_error = PositiveInfinity;
                return this;
            } else if (p->sign() < 0) {
                auto n = new KNumberInteger(0);
                delete this;
                return n;
            } else {
                m_error = Undefined;
                return this;
            }
            break;
        case Undefined:
            return this;
        }
    }

    Q_ASSERT(0);
    return nullptr;
}

KNumberBase *KNumberError::neg()
{
    switch (m_error) {
    case PositiveInfinity:
        m_error = NegativeInfinity;
        break;
    case NegativeInfinity:
        m_error = PositiveInfinity;
        break;
    case Undefined:
    default:
        break;
    }

    return this;
}

KNumberBase *KNumberError::cmp()
{
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::abs()
{
    switch (m_error) {
    case NegativeInfinity:
        m_error = PositiveInfinity;
        break;
    case PositiveInfinity:
    case Undefined:
    default:
        break;
    }

    return this;
}

KNumberBase *KNumberError::sqrt()
{
    switch (m_error) {
    case NegativeInfinity:
        m_error = Undefined;
        break;
    case PositiveInfinity:
    case Undefined:
    default:
        break;
    }

    return this;
}

KNumberBase *KNumberError::cbrt()
{
    return this;
}

KNumberBase *KNumberError::factorial()
{
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::sin()
{
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::cos()
{
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::tgamma()
{
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::tan()
{
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::asin()
{
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::acos()
{
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::atan()
{
    switch (m_error) {
    case PositiveInfinity:
        delete this;
        return new KNumberFloat(M_PI / 2.0);
    case NegativeInfinity:
        delete this;
        return new KNumberFloat(-M_PI / 2.0);
    case Undefined:
    default:
        return this;
    }
}

KNumberBase *KNumberError::sinh()
{
    return this;
}

KNumberBase *KNumberError::cosh()
{
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::tanh()
{
    if (sign() > 0) {
        delete this;
        return new KNumberInteger(1);
    } else if (sign() < 0) {
        delete this;
        return new KNumberInteger(-1);
    } else {
        return this;
    }
}

KNumberBase *KNumberError::asinh()
{
    return this;
}

KNumberBase *KNumberError::acosh()
{
    if (sign() < 0) {
        m_error = Undefined;
    }

    return this;
}

KNumberBase *KNumberError::atanh()
{
    m_error = Undefined;
    return this;
}

int KNumberError::compare(KNumberBase *rhs)
{
    if (auto const p = dynamic_cast<KNumberInteger *>(rhs)) {
        if (sign() > 0) {
            return 1;
        } else {
            return -1;
        }
    } else if (auto const p = dynamic_cast<KNumberFloat *>(rhs)) {
        if (sign() > 0) {
            return 1;
        } else {
            return -1;
        }
    } else if (auto const p = dynamic_cast<KNumberFraction *>(rhs)) {
        if (sign() > 0) {
            return 1;
        } else {
            return -1;
        }
    } else if (auto const p = dynamic_cast<KNumberError *>(rhs)) {
        // TODO: What to return when comparing to NaN?
        if (m_error == p->m_error) {
            return 0;
        } else if (m_error == PositiveInfinity || p->m_error == NegativeInfinity) {
            return 1;
        } else if (m_error == NegativeInfinity || p->m_error == PositiveInfinity) {
            return -1;
        } else {
            return -1;
        }
    }

    Q_ASSERT(0);
    return 0;
}

KNumberBase *KNumberError::clone()
{
    return new KNumberError(this);
}

KNumberBase *KNumberError::bitwiseAnd(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::bitwiseXor(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::bitwiseOr(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::bitwiseShift(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    m_error = Undefined;
    return this;
}

bool KNumberError::isInteger() const
{
    return false;
}

bool KNumberError::isZero() const
{
    return false;
}

int KNumberError::sign() const
{
    switch (m_error) {
    case PositiveInfinity:
        return +1;
    case NegativeInfinity:
        return -1;
    case Undefined:
    default:
        return 0;
    }
}

KNumberBase *KNumberError::reciprocal()
{
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::log2()
{
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::log10()
{
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::ln()
{
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::ceil()
{
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::floor()
{
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::exp2()
{
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::exp10()
{
    m_error = Undefined;
    return this;
}

KNumberBase *KNumberError::exp()
{
    m_error = Undefined;
    return this;
}

quint64 KNumberError::toUint64() const
{
    return 0;
}

qint64 KNumberError::toInt64() const
{
    return 0;
}

KNumberBase *KNumberError::bin(KNumberBase *rhs)
{
    Q_UNUSED(rhs);
    m_error = Undefined;
    return this;
}

}
