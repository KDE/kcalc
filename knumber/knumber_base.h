/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QString>

namespace detail
{
class KNumberError;
class KNumberInteger;
class KNumberFraction;
class KNumberFloat;

class KNumberBase
{
public:
    virtual ~KNumberBase() = default;

public:
    virtual KNumberBase *clone() = 0;

public:
    virtual QString toString(int precision) const = 0;
    virtual quint64 toUint64() const = 0;
    virtual qint64 toInt64() const = 0;

public:
    virtual bool isInteger() const = 0;
    virtual bool isZero() const = 0;
    virtual int sign() const = 0;

public:
    // basic math
    virtual KNumberBase *add(KNumberBase *rhs) = 0;
    virtual KNumberBase *sub(KNumberBase *rhs) = 0;
    virtual KNumberBase *mul(KNumberBase *rhs) = 0;
    virtual KNumberBase *div(KNumberBase *rhs) = 0;
    virtual KNumberBase *mod(KNumberBase *rhs) = 0;

public:
    // logical operators
    virtual KNumberBase *bitwiseAnd(KNumberBase *rhs) = 0;
    virtual KNumberBase *bitwiseXor(KNumberBase *rhs) = 0;
    virtual KNumberBase *bitwiseOr(KNumberBase *rhs) = 0;
    virtual KNumberBase *bitwiseShift(KNumberBase *rhs) = 0;

public:
    // algebraic functions
    virtual KNumberBase *pow(KNumberBase *rhs) = 0;
    virtual KNumberBase *neg() = 0;
    virtual KNumberBase *cmp() = 0;
    virtual KNumberBase *abs() = 0;
    virtual KNumberBase *sqrt() = 0;
    virtual KNumberBase *cbrt() = 0;
    virtual KNumberBase *factorial() = 0;
    virtual KNumberBase *reciprocal() = 0;

public:
    // special functions
    virtual KNumberBase *log2() = 0;
    virtual KNumberBase *log10() = 0;
    virtual KNumberBase *ln() = 0;
    virtual KNumberBase *exp2() = 0;
    virtual KNumberBase *exp10() = 0;
    virtual KNumberBase *floor() = 0;
    virtual KNumberBase *ceil() = 0;
    virtual KNumberBase *exp() = 0;
    virtual KNumberBase *bin(KNumberBase *rhs) = 0;

public:
    // trig functions
    virtual KNumberBase *sin() = 0;
    virtual KNumberBase *cos() = 0;
    virtual KNumberBase *tan() = 0;
    virtual KNumberBase *asin() = 0;
    virtual KNumberBase *acos() = 0;
    virtual KNumberBase *atan() = 0;
    virtual KNumberBase *sinh() = 0;
    virtual KNumberBase *cosh() = 0;
    virtual KNumberBase *tanh() = 0;
    virtual KNumberBase *asinh() = 0;
    virtual KNumberBase *acosh() = 0;
    virtual KNumberBase *atanh() = 0;
    virtual KNumberBase *tgamma() = 0;

public:
    // comparison
    virtual int compare(KNumberBase *rhs) = 0;
};

}
