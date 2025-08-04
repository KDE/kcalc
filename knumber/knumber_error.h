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
    friend class KNumberComplex;

public:
    enum Error {
        Undefined,
        PositiveInfinity,
        NegativeInfinity,
        ComplexInfinity,
    };

    explicit KNumberError(const QString &s);
    explicit KNumberError(Error e);
    KNumberError();
    ~KNumberError() override;

    QString toString(int precision) const override;
    quint64 toUint64() const override;
    qint64 toInt64() const override;

    bool isReal() const override;
    bool isInteger() const override;
    bool isZero() const override;
    int sign() const override;

    KNumberBase *add(KNumberBase *rhs) override;
    KNumberBase *sub(KNumberBase *rhs) override;
    KNumberBase *mul(KNumberBase *rhs) override;
    KNumberBase *div(KNumberBase *rhs) override;
    KNumberBase *mod(KNumberBase *rhs) override;

    KNumberBase *bitwiseAnd(KNumberBase *rhs) override;
    KNumberBase *bitwiseXor(KNumberBase *rhs) override;
    KNumberBase *bitwiseOr(KNumberBase *rhs) override;
    KNumberBase *bitwiseShift(KNumberBase *rhs) override;

    KNumberBase *pow(KNumberBase *rhs) override;
    KNumberBase *neg() override;
    KNumberBase *cmp() override;
    KNumberBase *abs() override;
    KNumberBase *sqrt() override;
    KNumberBase *cbrt() override;
    KNumberBase *factorial() override;
    KNumberBase *reciprocal() override;
    KNumberBase *tgamma() override;

    KNumberBase *log2() override;
    KNumberBase *log10() override;
    KNumberBase *ln() override;
    KNumberBase *exp2() override;
    KNumberBase *exp10() override;
    KNumberBase *floor() override;
    KNumberBase *ceil() override;
    KNumberBase *exp() override;
    KNumberBase *realPart() override;
    KNumberBase *imaginaryPart() override;
    KNumberBase *arg() override;
    KNumberBase *conj() override;
    KNumberBase *bin(KNumberBase *rhs) override;

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

    int compare(KNumberBase *rhs) override;

private:
    // conversion constructors
    explicit KNumberError(const KNumberInteger *value);
    explicit KNumberError(const KNumberFraction *value);
    explicit KNumberError(const KNumberFloat *value);
    explicit KNumberError(const KNumberComplex *value);
    explicit KNumberError(const KNumberError *value);

public:
    KNumberBase *clone() override;

private:
    Error m_error;
};

}
