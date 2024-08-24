/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knumber_operators.h"
#include "knumber.h"
#include "knumber_base.h"
#include <config-kcalc.h>

KNumber operator+(const KNumber &lhs, const KNumber &rhs)
{
    KNumber x(lhs);
    x += rhs;
    return x;
}

KNumber operator-(const KNumber &lhs, const KNumber &rhs)
{
    KNumber x(lhs);
    x -= rhs;
    return x;
}

KNumber operator*(const KNumber &lhs, const KNumber &rhs)
{
    KNumber x(lhs);
    x *= rhs;
    return x;
}

KNumber operator/(const KNumber &lhs, const KNumber &rhs)
{
    KNumber x(lhs);
    x /= rhs;
    return x;
}

KNumber operator%(const KNumber &lhs, const KNumber &rhs)
{
    KNumber x(lhs);
    x %= rhs;
    return x;
}

KNumber operator&(const KNumber &lhs, const KNumber &rhs)
{
    KNumber x(lhs);
    x &= rhs;
    return x;
}

KNumber operator|(const KNumber &lhs, const KNumber &rhs)
{
    KNumber x(lhs);
    x |= rhs;
    return x;
}

KNumber operator^(const KNumber &lhs, const KNumber &rhs)
{
    KNumber x(lhs);
    x ^= rhs;
    return x;
}

KNumber operator>>(const KNumber &lhs, const KNumber &rhs)
{
    KNumber x(lhs);
    x >>= rhs;
    return x;
}

KNumber operator<<(const KNumber &lhs, const KNumber &rhs)
{
    KNumber x(lhs);
    x <<= rhs;
    return x;
}

KNumber abs(const KNumber &x)
{
    return x.abs();
}

KNumber cbrt(const KNumber &x)
{
    return x.cbrt();
}

KNumber sqrt(const KNumber &x)
{
    return x.sqrt();
}

KNumber sin(const KNumber &x)
{
    return x.sin();
}

KNumber cos(const KNumber &x)
{
    return x.cos();
}

KNumber tan(const KNumber &x)
{
    return x.tan();
}

KNumber asin(const KNumber &x)
{
    return x.asin();
}

KNumber acos(const KNumber &x)
{
    return x.acos();
}

KNumber tgamma(const KNumber &x)
{
    return x.tgamma();
}

KNumber atan(const KNumber &x)
{
    return x.atan();
}

KNumber pow(const KNumber &x, const KNumber &y)
{
    return x.pow(y);
}

KNumber factorial(const KNumber &x)
{
    return x.factorial();
}

KNumber log2(const KNumber &x)
{
    return x.log2();
}

KNumber floor(const KNumber &x)
{
    return x.floor();
}

KNumber ceil(const KNumber &x)
{
    return x.ceil();
}

KNumber log10(const KNumber &x)
{
    return x.log10();
}

KNumber ln(const KNumber &x)
{
    return x.ln();
}

KNumber exp2(const KNumber &x)
{
    return x.exp2();
}

KNumber exp10(const KNumber &x)
{
    return x.exp10();
}

KNumber exp(const KNumber &x)
{
    return x.exp();
}

bool operator==(const KNumber &lhs, const KNumber &rhs)
{
    return lhs.m_value->compare(rhs.m_value) == 0;
}

bool operator!=(const KNumber &lhs, const KNumber &rhs)
{
    return lhs.m_value->compare(rhs.m_value) != 0;
}

bool operator>=(const KNumber &lhs, const KNumber &rhs)
{
    return lhs.m_value->compare(rhs.m_value) >= 0;
}

bool operator<=(const KNumber &lhs, const KNumber &rhs)
{
    return lhs.m_value->compare(rhs.m_value) <= 0;
}

bool operator>(const KNumber &lhs, const KNumber &rhs)
{
    return lhs.m_value->compare(rhs.m_value) > 0;
}

bool operator<(const KNumber &lhs, const KNumber &rhs)
{
    return lhs.m_value->compare(rhs.m_value) < 0;
}
