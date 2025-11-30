/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "knumber_base.h"

#include <gmp.h>

class KNumber;

namespace detail
{
class KNumberInteger : public KNumberBase
{
    friend class ::KNumber;
    friend class KNumberError;
    friend class KNumberFraction;
    friend class KNumberFloat;
    friend class KNumberComplex;

public:
    explicit KNumberInteger(const QString &s);
    explicit KNumberInteger(qint32 value);
    explicit KNumberInteger(qint64 value);
    explicit KNumberInteger(quint32 value);
    explicit KNumberInteger(quint64 value);
    explicit KNumberInteger(mpz_t mpz);
    ~KNumberInteger() override;

    KNumberBase *clone() override;

    QString toString(int precision) const override;
    quint64 toUint64() const override;
    qint64 toInt64() const override;

    virtual bool isEven() const;
    virtual bool isOdd() const;
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

    KNumberBase *log2() override;
    KNumberBase *log10() override;
    KNumberBase *ln() override;
    KNumberBase *exp2() override;
    KNumberBase *floor() override;
    KNumberBase *ceil() override;
    KNumberBase *exp10() override;
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
    KNumberBase *tgamma() override;

    int compare(KNumberBase *rhs) override;

private:
    // conversion constructors
    explicit KNumberInteger(const KNumberInteger *value);
    explicit KNumberInteger(const KNumberFraction *value);
    explicit KNumberInteger(const KNumberFloat *value);
    explicit KNumberInteger(const KNumberComplex *value);
    explicit KNumberInteger(const KNumberError *value);

    mpz_t m_mpz;
};

}
