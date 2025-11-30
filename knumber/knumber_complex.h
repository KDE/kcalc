/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "knumber_base.h"
#include <config-knumber.h>

#include <mpc.h>

class KNumber;

namespace detail
{
class KNumberComplex : public KNumberBase
{
    friend class ::KNumber;
    friend class KNumberError;
    friend class KNumberInteger;
    friend class KNumberFraction;
    friend class KNumberFloat;

private:
    static const mpc_rnd_t rounding_mode;
    static const mpfr_prec_t precision;

public:
    explicit KNumberComplex(const QString &s);
    explicit KNumberComplex(const QString &mod, const QString &arg);
    explicit KNumberComplex(double re);
    explicit KNumberComplex(double re, double img);
#ifdef HAVE_LONG_DOUBLE
    explicit KNumberComplex(long double re);
    explicit KNumberComplex(long double re, long double img);
#endif

    explicit KNumberComplex(mpc_t mpc);
    ~KNumberComplex() override;

private:
    // conversion constructors
    explicit KNumberComplex(const KNumberInteger *value);
    explicit KNumberComplex(const KNumberFraction *value);
    explicit KNumberComplex(const KNumberFloat *value);
    explicit KNumberComplex(const KNumberComplex *value);
    explicit KNumberComplex(const KNumberError *value);

public:
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
    KNumberBase *floor() override;
    KNumberBase *ceil() override;
    KNumberBase *exp2() override;
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

    KNumberBase *bitwiseAnd(KNumberBase *rhs) override;
    KNumberBase *bitwiseXor(KNumberBase *rhs) override;
    KNumberBase *bitwiseOr(KNumberBase *rhs) override;
    KNumberBase *bitwiseShift(KNumberBase *rhs) override;

    KNumberBase *clone() override;

private:
    KNumberBase *ensureIsValid();

    template<int F(mpc_ptr rop, mpc_srcptr op)>
    KNumberBase *execute_mpc_func();

    template<int F(mpc_ptr rop, mpc_srcptr op, mpc_rnd_t rnd)>
    KNumberBase *execute_mpc_func();

    template<int F(mpc_ptr rop, mpc_srcptr op1, mpc_srcptr op2, mpc_rnd_t rnd)>
    KNumberBase *execute_mpc_func(mpc_srcptr op);

    mpc_ptr new_mpc();

    mpc_t m_mpc;
};

}
