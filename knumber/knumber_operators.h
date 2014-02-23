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

#ifndef KNUMBER_OPERATORS_H_
#define KNUMBER_OPERATORS_H_

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

#endif
