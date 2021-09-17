/*
SPDX-FileCopyrightText: 2001-2013 Evan Teran
    evan.teran@gmail.com

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "knumber_base.h"

class KNumber;

namespace detail
{
class knumber_integer : public knumber_base
{
    friend class ::KNumber;
    friend class knumber_error;
    friend class knumber_fraction;
    friend class knumber_float;

public:
    explicit knumber_integer(const QString &s);
    explicit knumber_integer(qint32 value);
    explicit knumber_integer(qint64 value);
    explicit knumber_integer(quint32 value);
    explicit knumber_integer(quint64 value);
    explicit knumber_integer(mpz_t mpz);
    ~knumber_integer() override;

public:
    knumber_base *clone() override;

public:
    QString toString(int precision) const override;
    quint64 toUint64() const override;
    qint64 toInt64() const override;

public:
    virtual bool is_even() const;
    virtual bool is_odd() const;
    bool is_integer() const override;
    bool is_zero() const override;
    int sign() const override;

public:
    knumber_base *add(knumber_base *rhs) override;
    knumber_base *sub(knumber_base *rhs) override;
    knumber_base *mul(knumber_base *rhs) override;
    knumber_base *div(knumber_base *rhs) override;
    knumber_base *mod(knumber_base *rhs) override;

public:
    knumber_base *bitwise_and(knumber_base *rhs) override;
    knumber_base *bitwise_xor(knumber_base *rhs) override;
    knumber_base *bitwise_or(knumber_base *rhs) override;
    knumber_base *bitwise_shift(knumber_base *rhs) override;

public:
    knumber_base *pow(knumber_base *rhs) override;
    knumber_base *neg() override;
    knumber_base *cmp() override;
    knumber_base *abs() override;
    knumber_base *sqrt() override;
    knumber_base *cbrt() override;
    knumber_base *factorial() override;
    knumber_base *reciprocal() override;

public:
    knumber_base *log2() override;
    knumber_base *log10() override;
    knumber_base *ln() override;
    knumber_base *exp2() override;
    knumber_base *floor() override;
    knumber_base *ceil() override;
    knumber_base *exp10() override;
    knumber_base *exp() override;
    knumber_base *bin(knumber_base *rhs) override;

public:
    knumber_base *sin() override;
    knumber_base *cos() override;
    knumber_base *tan() override;
    knumber_base *asin() override;
    knumber_base *acos() override;
    knumber_base *atan() override;
    knumber_base *sinh() override;
    knumber_base *cosh() override;
    knumber_base *tanh() override;
    knumber_base *asinh() override;
    knumber_base *acosh() override;
    knumber_base *atanh() override;
    knumber_base *tgamma() override;

public:
    int compare(knumber_base *rhs) override;

private:
    // conversion constructors
    explicit knumber_integer(const knumber_integer *value);
    explicit knumber_integer(const knumber_fraction *value);
    explicit knumber_integer(const knumber_float *value);
    explicit knumber_integer(const knumber_error *value);

private:
    mpz_t mpz_;
};

}

