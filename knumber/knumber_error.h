/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "knumber_base.h"

class KNumber;

namespace detail
{
class KNumberError : public KNumberBase
{
    friend class ::KNumber;
    friend class KNumberInteger;
    friend class KNumberFraction;
    friend class KNumberFloat;

public:
    enum Error {
        Undefined,
        PositiveInfinity,
        NegativeInfinity,
    };

public:
    explicit KNumberError(const QString &s);
    explicit KNumberError(Error e);
    KNumberError();
    ~KNumberError() override;

public:
    QString toString(int precision) const override;
    quint64 toUint64() const override;
    qint64 toInt64() const override;

public:
    bool isInteger() const override;
    bool isZero() const override;
    int sign() const override;

public:
    KNumberBase *add(KNumberBase *rhs) override;
    KNumberBase *sub(KNumberBase *rhs) override;
    KNumberBase *mul(KNumberBase *rhs) override;
    KNumberBase *div(KNumberBase *rhs) override;
    KNumberBase *mod(KNumberBase *rhs) override;

public:
    KNumberBase *bitwiseAnd(KNumberBase *rhs) override;
    KNumberBase *bitwiseXor(KNumberBase *rhs) override;
    KNumberBase *bitwiseOr(KNumberBase *rhs) override;
    KNumberBase *bitwiseShift(KNumberBase *rhs) override;

public:
    KNumberBase *pow(KNumberBase *rhs) override;
    KNumberBase *neg() override;
    KNumberBase *cmp() override;
    KNumberBase *abs() override;
    KNumberBase *sqrt() override;
    KNumberBase *cbrt() override;
    KNumberBase *factorial() override;
    KNumberBase *reciprocal() override;
    KNumberBase *tgamma() override;

public:
    KNumberBase *log2() override;
    KNumberBase *log10() override;
    KNumberBase *ln() override;
    KNumberBase *exp2() override;
    KNumberBase *exp10() override;
    KNumberBase *floor() override;
    KNumberBase *ceil() override;
    KNumberBase *exp() override;
    KNumberBase *bin(KNumberBase *rhs) override;

public:
    KNumberBase *sin() override;
    KNumberBase *cos() override;
    KNumberBase *tan() override;
    KNumberBase *asin() override;
    KNumberBase *acos() override;
    KNumberBase *atan() override;
    KNumberBase *sinh() override;
    KNumberBase *cosh() override;
    KNumberBase *tanh() override;
    KNumberBase *asinh() override;
    KNumberBase *acosh() override;
    KNumberBase *atanh() override;

public:
    int compare(KNumberBase *rhs) override;

private:
    // conversion constructors
    explicit KNumberError(const KNumberInteger *value);
    explicit KNumberError(const KNumberFraction *value);
    explicit KNumberError(const KNumberFloat *value);
    explicit KNumberError(const KNumberError *value);

public:
    KNumberBase *clone() override;

private:
    Error m_error;
};

}
