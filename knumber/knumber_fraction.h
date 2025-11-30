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
class KNumberFraction : public KNumberBase
{
    friend class ::KNumber;
    friend class KNumberError;
    friend class KNumberInteger;
    friend class KNumberFloat;
    friend class KNumberComplex;

public:
    static bool defaultFractionalInput;
    static bool defaultFractionalOutput;
    static bool splitOffIntegerForFractionOutput;

    static void setDefaultFractionalInput(bool value);
    static void setDefaultFractionalOutput(bool value);
    static void setSplitOffIntegerForFractionOutput(bool value);

    explicit KNumberFraction(const QString &s);
    KNumberFraction(qint64 num, quint64 den);
    KNumberFraction(quint64 num, quint64 den);
    explicit KNumberFraction(mpq_t mpq);
    ~KNumberFraction() override;

    KNumberBase *clone() override;

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

    int compare(KNumberBase *rhs) override;

private:
    KNumberInteger *numerator() const;
    KNumberInteger *denominator() const;

    // conversion constructors
    explicit KNumberFraction(const KNumberInteger *value);
    explicit KNumberFraction(const KNumberFraction *value);
#if 0
	// TODO: this is omitted because there is no good way to
	// implement it
    // truncate?
	KNumberFraction(const KNumberFloat *value);
    KNumberFraction(const KNumberComplex *value);
#endif
    explicit KNumberFraction(const KNumberError *value);

    mpq_t m_mpq;
};

}
