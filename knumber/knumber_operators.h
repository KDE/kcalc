/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

class KNumber;

bool operator==(const KNumber &lhs, const KNumber &rhs);
bool operator!=(const KNumber &lhs, const KNumber &rhs);
bool operator>=(const KNumber &lhs, const KNumber &rhs);
bool operator<=(const KNumber &lhs, const KNumber &rhs);
bool operator>(const KNumber &lhs, const KNumber &rhs);
bool operator<(const KNumber &lhs, const KNumber &rhs);

KNumber operator+(const KNumber &lhs, const KNumber &rhs);
KNumber operator-(const KNumber &lhs, const KNumber &rhs);
KNumber operator*(const KNumber &lhs, const KNumber &rhs);
KNumber operator/(const KNumber &lhs, const KNumber &rhs);
KNumber operator%(const KNumber &lhs, const KNumber &rhs);

KNumber operator&(const KNumber &lhs, const KNumber &rhs);
KNumber operator|(const KNumber &lhs, const KNumber &rhs);
KNumber operator^(const KNumber &lhs, const KNumber &rhs);
KNumber operator>>(const KNumber &lhs, const KNumber &rhs);
KNumber operator<<(const KNumber &lhs, const KNumber &rhs);

KNumber abs(const KNumber &x);
KNumber cbrt(const KNumber &x);
KNumber sqrt(const KNumber &x);
KNumber pow(const KNumber &x, const KNumber &y);

KNumber sin(const KNumber &x);
KNumber cos(const KNumber &x);
KNumber tan(const KNumber &x);
KNumber asin(const KNumber &x);
KNumber tgamma(const KNumber &x);
KNumber acos(const KNumber &x);
KNumber atan(const KNumber &x);

KNumber factorial(const KNumber &x);

KNumber log2(const KNumber &x);
KNumber log10(const KNumber &x);
KNumber ceil(const KNumber &x);
KNumber floor(const KNumber &x);
KNumber ln(const KNumber &x);
KNumber exp2(const KNumber &x);
KNumber exp10(const KNumber &x);
KNumber exp(const KNumber &x);
