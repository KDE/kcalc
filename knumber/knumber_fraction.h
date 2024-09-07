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

public:
    static bool defaultFractionalInput;
    static bool defaultFractionalOutput;
    static bool splitOffIntegerForFractionOutput;

public:
    static void setDefaultFractionalInput(bool value);
    static void setDefaultFractionalOutput(bool value);
    static void setSplitOffIntegerForFractionOutput(bool value);

public:
    explicit KNumberFraction(const QString &s);
    KNumberFraction(qint64 num, quint64 den);
    KNumberFraction(quint64 num, quint64 den);
    explicit KNumberFraction(mpq_t mpq);
    ~KNumberFraction() override;

public:
    KNumberBase *clone() override;

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
    KNumberBase *floor() override;
    KNumberBase *ceil() override;
    KNumberBase *exp10() override;
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
    KNumberInteger *numerator() const;
    KNumberInteger *denominator() const;

private:
    // conversion constructors
    explicit KNumberFraction(const KNumberInteger *value);
    explicit KNumberFraction(const KNumberFraction *value);
#if 0
	// TODO: this is omitted because there is no good way to
	// implement it
	KNumberFraction(const knumber_float *value);
#endif
    explicit KNumberFraction(const KNumberError *value);

private:
    mpq_t m_mpq;
};

}
