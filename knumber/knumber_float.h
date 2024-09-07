/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "knumber_base.h"
#include <config-knumber.h>

#include <mpfr.h>

class KNumber;

namespace detail
{
class KNumberFloat : public KNumberBase
{
    friend class ::KNumber;
    friend class KNumberError;
    friend class KNumberInteger;
    friend class KNumberFraction;

private:
    static const mpfr_rnd_t rounding_mode;
    static const mpfr_prec_t precision;

public:
    explicit KNumberFloat(const QString &s);
    explicit KNumberFloat(double value);
#ifdef HAVE_LONG_DOUBLE
    explicit KNumberFloat(long double value);
#endif

    explicit KNumberFloat(mpfr_t mpfr);
    ~KNumberFloat() override;

private:
    // conversion constructors
    explicit KNumberFloat(const KNumberInteger *value);
    explicit KNumberFloat(const KNumberFraction *value);
    explicit KNumberFloat(const KNumberFloat *value);
    explicit KNumberFloat(const KNumberError *value);

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
    KNumberBase *floor() override;
    KNumberBase *ceil() override;
    KNumberBase *exp2() override;
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

public:
    KNumberBase *bitwiseAnd(KNumberBase *rhs) override;
    KNumberBase *bitwiseXor(KNumberBase *rhs) override;
    KNumberBase *bitwiseOr(KNumberBase *rhs) override;
    KNumberBase *bitwiseShift(KNumberBase *rhs) override;

public:
    KNumberBase *clone() override;

private:
    KNumberBase *ensureIsValid(mpfr_ptr mpfr);

    template<int F(mpfr_ptr rop, mpfr_srcptr op)>
    KNumberBase *execute_mpfr_func();

    template<int F(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd)>
    KNumberBase *execute_mpfr_func();

    template<int F(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd)>
    KNumberBase *execute_mpfr_func(mpfr_srcptr op);

    mpfr_ptr new_mpfr();

private:
    mpfr_t m_mpfr;
};

}
