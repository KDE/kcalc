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

#ifndef KNUMBER_H_
#define KNUMBER_H_

#include "knumber_operators.h"
#include <QSharedPointer>
#include <QString>
#include <QtGlobal>

namespace detail {
class knumber_base;
}

class KNumber {
private:	
	friend bool operator==(const KNumber &lhs, const KNumber &rhs);
	friend bool operator!=(const KNumber &lhs, const KNumber &rhs);
	friend bool operator>=(const KNumber &lhs, const KNumber &rhs);
	friend bool operator<=(const KNumber &lhs, const KNumber &rhs);
	friend bool operator>(const KNumber &lhs, const KNumber &rhs);
	friend bool operator<(const KNumber &lhs, const KNumber &rhs);
	
public:
	enum Type {
		TYPE_ERROR,
		TYPE_INTEGER,
		TYPE_FLOAT,
		TYPE_FRACTION
	};
	
public:
	// useful constants
	static const KNumber Zero;
	static const KNumber One;
	static const KNumber NegOne;
	static const KNumber PosInfinity;
	static const KNumber NegInfinity;
	static const KNumber NaN;
	
public:
	static KNumber Pi();
	static KNumber Euler();
	
public:
	// construction/destruction
	KNumber();
	explicit KNumber(const QString &s);
	
	explicit KNumber(qint32 value);
	explicit KNumber(qint64 value);
	explicit KNumber(quint32 value);
	explicit KNumber(quint64 value);
	
	KNumber(qint64 num, quint64 den);
	KNumber(quint64 num, quint64 den);
	
#ifdef HAVE_LONG_DOUBLE
	explicit KNumber(long double value);
#endif
	explicit KNumber(double value);
	
	KNumber(const KNumber &other);
	~KNumber();

public:
	Type type() const;

public:
	// assignment
	KNumber &operator=(const KNumber &rhs);
	
public:
	// basic math operators
	KNumber &operator+=(const KNumber &rhs);
	KNumber &operator-=(const KNumber &rhs);
	KNumber &operator*=(const KNumber &rhs);
	KNumber &operator/=(const KNumber &rhs);
	KNumber &operator%=(const KNumber &rhs);

public:
	// bitwise operators
    KNumber &operator&=(const KNumber &rhs);
    KNumber &operator|=(const KNumber &rhs);
	KNumber &operator^=(const KNumber &rhs);
    KNumber &operator<<=(const KNumber &rhs);
    KNumber &operator>>=(const KNumber &rhs);

public:
	// neg/cmp
	KNumber operator-() const;
	KNumber operator~() const;
	
public:
	KNumber integerPart() const;

public:
	QString toQString(int width = -1, int precision = -1) const;
	quint64 toUint64() const;
	qint64 toInt64() const;
	
	
public:
	KNumber abs() const;
	KNumber cbrt() const;
	KNumber sqrt() const;
	KNumber pow(const KNumber &x) const;

	KNumber sin() const;
	KNumber cos() const;
	KNumber tan() const;
	KNumber asin() const;
	KNumber acos() const;
	KNumber atan() const;
	KNumber sinh() const;
	KNumber cosh() const;
	KNumber tanh() const;
	KNumber asinh() const;
	KNumber acosh() const;
	KNumber atanh() const;
	
	KNumber factorial() const;
	
	KNumber log2() const;
	KNumber log10() const;
	KNumber ln() const;
	KNumber exp2() const;
	KNumber exp10() const;
	KNumber exp() const;
	KNumber bin(const KNumber &x) const;
	
public:
	static void setDefaultFloatPrecision(int precision);
	static void setSplitoffIntegerForFractionOutput(bool x);
	static void setDefaultFractionalInput(bool x);
	static void setDefaultFloatOutput(bool x);
	static void setGroupSeparator(const QString &ch);
	static void setDecimalSeparator(const QString &ch);
	
	static QString groupSeparator();
	static QString decimalSeparator();

public:
	void swap(KNumber &other);
	
private:
	void simplify();

private:
	detail::knumber_base *value_;
	
private:
	static QString GroupSeparator;
	static QString DecimalSeparator;
};

#endif
