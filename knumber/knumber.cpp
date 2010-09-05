// -*- indent-tabs-mode: nil -*-
/* This file is part of the KDE libraries
   Copyright (c) 2005 Klaus Niederkrueger <kniederk@math.uni-koeln.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "knumber.h"
#include "knumber_priv.h"
#include "kdemacros.h"
#include "kglobal.h"

#include <cmath>
#include <cstdio>

#include <config-kcalc.h>

#include <QRegExp>

using namespace std;

const KNumber KNumber::Zero(0);
const KNumber KNumber::One(1);
const KNumber KNumber::MinusOne(-1);
const KNumber KNumber::NotDefined("nan");
bool KNumber::FloatOutput = false;
bool KNumber::FractionInput = false;
bool KNumber::SplitOffIntegerOutput = false;

#ifndef HAVE_FUNC_ISINF

#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif

#define isinf(x) (!finite(x) && x == x)
#endif

// this constructor is different from the rest
// firstly it is explicit so that the compiler doesn't
// get confused if you pass a 0 constant
// secondly it takes a pointer to the knumber implementation
// class and TAKES OWNERSHIP of it.
// It's purpose it enable us to avoid code which creates
// a KNumber just to do a "delete tmp.num_;"
KNumber::KNumber(detail::knumber *num) : num_(num)
{
	Q_ASSERT(num != 0);
}

KNumber::KNumber(qint32 num) : num_(new detail::knuminteger(num))
{
}

KNumber::KNumber(quint32 num) :  num_(new detail::knuminteger(num))
{
}

KNumber::KNumber(qint64 num) : num_(new detail::knuminteger(num))
{
}

KNumber::KNumber(quint64 num) : num_(new detail::knuminteger(num))
{
}

KNumber::KNumber(double num)
{
    if (isinf(num)) num_ = new detail::knumerror(detail::knumber::Infinity);
    else if (isnan(num)) num_ = new detail::knumerror(detail::knumber::UndefinedNumber);
    else num_ = new detail::knumfloat(num);

}

void KNumber::swap(KNumber &other)
{
	qSwap(num_, other.num_);
}

KNumber::KNumber(const KNumber &num)
{
    switch (num.type()) {
    case SpecialType:
        num_ = new detail::knumerror(*(num.num_));
        return;
    case IntegerType:
        num_ = new detail::knuminteger(*(num.num_));
        return;
    case  FractionType:
        num_ = new detail::knumfraction(*(num.num_));
        return;
    case FloatType:
        num_ = new detail::knumfloat(*(num.num_));
        return;
    }
}

KNumber::KNumber(const QString &num)
{
    if (QRegExp("^(inf|-inf|nan)$").exactMatch(num))
        num_ = new detail::knumerror(num);
    else if (QRegExp("^[+-]?\\d+$").exactMatch(num))
        num_ = new detail::knuminteger(num);
    else if (QRegExp("^[+-]?\\d+/\\d+$").exactMatch(num)) {
        num_ = new detail::knumfraction(num);
        simplifyRational();
    } else if (QRegExp("^[+-]?\\d+(\\.\\d*)?(e[+-]?\\d+)?$").exactMatch(num))
        if (FractionInput == true) {
            num_ = new detail::knumfraction(num);
            simplifyRational();
        } else
            num_ = new detail::knumfloat(num);
    else
        num_ = new detail::knumerror("nan");
}

KNumber::~KNumber()
{
    delete num_;
}

KNumber::NumType KNumber::type() const
{
	// TODO: wouldn't it be more efficient to just
	// use "return num_->type();"
    if (dynamic_cast<detail::knumerror *>(num_))
        return SpecialType;
    if (dynamic_cast<detail::knuminteger *>(num_))
        return IntegerType;
    if (dynamic_cast<detail::knumfraction *>(num_))
        return FractionType;
    if (dynamic_cast<detail::knumfloat *>(num_))
        return FloatType;

    return SpecialType;
}

// This method converts a fraction to an integer, whenever possible,
// i.e. 5/1 --> 5
// This method should be called, whenever such a inproper fraction can occur,
// e.g. when adding 4/3 + 2/3....
void KNumber::simplifyRational()
{
    if (type() != FractionType)
        return;

    detail::knumfraction *tmp_num = dynamic_cast<detail::knumfraction *>(num_);

    if (tmp_num->isInteger()) {
        KNumber(tmp_num->intPart()).swap(*this);
    }
}


KNumber & KNumber::operator=(const KNumber &num)
{
    if (this != & num) {
        KNumber(num).swap(*this);
	}
    return *this;
}

KNumber & KNumber::operator +=(const KNumber &arg)
{
    KNumber(*this + arg).swap(*this);
    return *this;
}

KNumber & KNumber::operator -=(const KNumber &arg)
{
    KNumber(*this - arg).swap(*this);
    return *this;
}

namespace {
// increase the digit at 'position' by one
void increment(QString &str, int position)
{
    for (int i = position; i >= 0; i--) {
        char last_char = str[i].toLatin1();
        switch (last_char) {
        case '0':
            str[i] = '1';
            break;
        case '1':
            str[i] = '2';
            break;
        case '2':
            str[i] = '3';
            break;
        case '3':
            str[i] = '4';
            break;
        case '4':
            str[i] = '5';
            break;
        case '5':
            str[i] = '6';
            break;
        case '6':
            str[i] = '7';
            break;
        case '7':
            str[i] = '8';
            break;
        case '8':
            str[i] = '9';
            break;
        case '9':
            str[i] = '0';
            if (i == 0) str.prepend('1');
            continue;
        case '.':
            continue;
        }
        break;
    }
}

namespace impl {
// Cut off if more digits in fractional part than 'precision'
void round(QString &str, int precision)
{
    int decimalSymbolPos = str.indexOf('.');

    if (decimalSymbolPos == -1) {
        if (precision == 0) {
            return;
        } else if (precision > 0) {   // add dot if missing (and needed)
            str.append('.');
            decimalSymbolPos = str.length() - 1;
        }
    }

    // fill up with more than enough zeroes (in case fractional part too short)
    str.append(QString().fill('0', precision));

    // Now decide whether to round up or down
    char last_char = str[decimalSymbolPos + precision + 1].toLatin1();
    switch (last_char) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
        // nothing to do, rounding down
        break;
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        // rounding up
        increment(str, decimalSymbolPos + precision);
        break;
    default:
        break;
    }

    decimalSymbolPos = str.indexOf('.');
    str.truncate(decimalSymbolPos + precision + 1);

    // if precision == 0 delete also '.'
    if (precision == 0) str = str.section('.', 0, 0);
}
}

QString round(const QString &numStr, int precision)
{
    QString tmpString = numStr;
    if (precision < 0  ||
            ! QRegExp("^[+-]?\\d+(\\.\\d+)*(e[+-]?\\d+)?$").exactMatch(tmpString))
        return numStr;


    // Skip the sign (for now)
    bool neg = (tmpString[0] == '-');
    if (neg  ||  tmpString[0] == '+') tmpString.remove(0, 1);


    // Split off exponential part (including 'e'-symbol)
    QString mantString = tmpString.section('e', 0, 0,
                                           QString::SectionCaseInsensitiveSeps);
    QString expString = tmpString.section('e', 1, 1,
                                          QString::SectionCaseInsensitiveSeps |
                                          QString::SectionIncludeLeadingSep);
    if (expString.length() == 1) expString.clear();


    impl::round(mantString, precision);

    if (neg) mantString.prepend('-');

    return mantString +  expString;
}

}




QString KNumber::toQString(int width, int prec) const
{
    QString tmp_str;

    if (*this == Zero)   // important to avoid infinite loops below
        return "0";
    switch (type()) {
    case IntegerType:
        if (width > 0) {   //result needs to be cut-off
            bool tmp_bool = FractionInput; // stupid work-around
            FractionInput = false;
            tmp_str = (KNumber(1.0) * (*this)).toQString(width, -1);
            FractionInput = tmp_bool;
        } else
            tmp_str = QString(num_->ascii());
        break;
    case FractionType:
        if (FloatOutput) {
            bool tmp_bool = FractionInput; // stupid work-around
            FractionInput = false;
            tmp_str = (KNumber(1.0) * (*this)).toQString(width, -1);
            FractionInput = tmp_bool;
        } else { // FloatOutput == false
            if (SplitOffIntegerOutput) {
                // split off integer part
                KNumber int_part = this->integerPart();
                if (int_part == Zero)
                    tmp_str = QString(num_->ascii());
                else if (int_part < Zero)
                    tmp_str = int_part.toQString() + ' ' + (int_part - *this).num_->ascii();
                else
                    tmp_str = int_part.toQString() + ' ' + (*this - int_part).num_->ascii();
            } else
                tmp_str = QString(num_->ascii());

            if (width > 0 && tmp_str.length() > width) {
                //result needs to be cut-off
                bool tmp_bool = FractionInput; // stupid work-around
                FractionInput = false;
                tmp_str = (KNumber(1.0) * (*this)).toQString(width, -1);
                FractionInput = tmp_bool;
            }
        }

        break;
    case FloatType:
        if (width > 0)
            tmp_str = QString(num_->ascii(width));
        else
            // rough estimate for  maximal decimal precision (10^3 = 2^10)
            tmp_str = QString(num_->ascii(3 * mpf_get_default_prec() / 10));
        break;
    default:
        return QString(num_->ascii());
    }

    if (prec >= 0)
        return round(tmp_str, prec);
    else
        return tmp_str;
}

void KNumber::setDefaultFloatOutput(bool flag)
{
    FloatOutput = flag;
}

void KNumber::setDefaultFractionalInput(bool flag)
{
    FractionInput = flag;
}

void KNumber::setSplitoffIntegerForFractionOutput(bool flag)
{
    SplitOffIntegerOutput = flag;
}

void KNumber::setDefaultFloatPrecision(unsigned int prec)
{
    // Need to transform decimal digits into binary digits
    unsigned long int bin_prec = static_cast<unsigned long int>
                                 (double(prec) * M_LN10 / M_LN2 + 1);

    mpf_set_default_prec(bin_prec);
}

KNumber KNumber::abs() const
{
    return KNumber(num_->abs());
}

KNumber KNumber::cbrt() const
{
    return KNumber(num_->cbrt());
}

KNumber KNumber::sqrt() const
{
    return KNumber(num_->sqrt());
}

KNumber KNumber::factorial() const
{
    return KNumber(num_->factorial());
}

KNumber KNumber::integerPart() const
{
    return KNumber(num_->intPart());
}

KNumber KNumber::power(const KNumber &exp) const
{
    if (*this == Zero) {
        if (exp == Zero)
            return KNumber("nan");   // 0^0 not defined
        else if (exp < Zero)
            return KNumber("inf");
        else
            return Zero;
    }

    if (exp == Zero) {
        if (*this != Zero)
            return One;
        else
            return KNumber("nan");
    } else if (exp < Zero) {
        KNumber tmp_num2 = -exp;
        KNumber tmp_num(num_->power(*(tmp_num2.num_)));
        return One / tmp_num;
    } else {
        return KNumber(num_->power(*(exp.num_)));
    }

}

KNumber KNumber::operator-() const
{
	return KNumber(num_->change_sign());
}

KNumber KNumber::operator+(const KNumber &arg2) const
{
    KNumber tmp_num(num_->add(*arg2.num_));
    tmp_num.simplifyRational();
    return tmp_num;
}

KNumber KNumber::operator-(const KNumber &arg2) const
{
    return *this + (-arg2);
}

KNumber KNumber::operator*(const KNumber &arg2) const
{
    KNumber tmp_num(num_->multiply(*arg2.num_));
    tmp_num.simplifyRational();
    return tmp_num;
}

KNumber KNumber::operator/(const KNumber &arg2) const
{
    KNumber tmp_num(num_->divide(*arg2.num_));
    tmp_num.simplifyRational();
    return tmp_num;
}


KNumber KNumber::operator%(const KNumber &arg2) const
{
    if (type() != IntegerType  ||  arg2.type() != IntegerType)
        return Zero;

    const detail::knuminteger *tmp_arg1 = dynamic_cast<const detail::knuminteger *>(num_);
    const detail::knuminteger *tmp_arg2 = dynamic_cast<const detail::knuminteger *>(arg2.num_);

    return KNumber(tmp_arg1->mod(*tmp_arg2));
}

KNumber KNumber::operator&(const KNumber &arg2) const
{
    if (type() != IntegerType  ||  arg2.type() != IntegerType)
        return Zero;

    const detail::knuminteger *tmp_arg1 = dynamic_cast<const detail::knuminteger *>(num_);
    const detail::knuminteger *tmp_arg2 = dynamic_cast<const detail::knuminteger *>(arg2.num_);

    return KNumber(tmp_arg1->intAnd(*tmp_arg2));

}

KNumber KNumber::operator|(const KNumber &arg2) const
{
    if (type() != IntegerType  ||  arg2.type() != IntegerType)
        return Zero;

    const detail::knuminteger *tmp_arg1 = dynamic_cast<const detail::knuminteger *>(num_);
    const detail::knuminteger *tmp_arg2 = dynamic_cast<const detail::knuminteger *>(arg2.num_);

    return KNumber(tmp_arg1->intOr(*tmp_arg2));
}


KNumber KNumber::operator<<(const KNumber &arg2) const
{
    if (type() != IntegerType  ||  arg2.type() != IntegerType)
        return KNumber("nan");

    const detail::knuminteger *tmp_arg1 = dynamic_cast<const detail::knuminteger *>(num_);
    const detail::knuminteger *tmp_arg2 = dynamic_cast<const detail::knuminteger *>(arg2.num_);

    return KNumber(tmp_arg1->shift(*tmp_arg2));
}

KNumber KNumber::operator>>(const KNumber &arg2) const
{
    if (type() != IntegerType  ||  arg2.type() != IntegerType)
        return KNumber("nan");

    KNumber tmp_num = -arg2;

    const detail::knuminteger *tmp_arg1 = dynamic_cast<const detail::knuminteger *>(num_);
    const detail::knuminteger *tmp_arg2 = dynamic_cast<const detail::knuminteger *>(tmp_num.num_);

    return KNumber(tmp_arg1->shift(*tmp_arg2));
}



KNumber::operator bool() const
{
    if (*this == Zero)
        return false;
    return true;
}

KNumber::operator qint32() const
{
    return static_cast<long int>(*num_);
}

KNumber::operator quint32() const
{
    return static_cast<unsigned long int>(*num_);
}

KNumber::operator qint64() const
{
    return static_cast<long long int>(*num_);
}

KNumber::operator quint64() const
{
    return static_cast<unsigned long long int>(*num_);
}

KNumber::operator double() const
{
    return static_cast<double>(*num_);
}

int KNumber::compare(const KNumber &arg2) const
{
    return num_->compare(*arg2.num_);
}

K_GLOBAL_STATIC_WITH_ARGS(KNumber, g_Pi, (
                              "3.141592653589793238462643383279502884197169"
                              "39937510582097494459230781640628620899862803"
                              "4825342117068"))
KNumber KNumber::Pi()
{
    return *g_Pi;
}

K_GLOBAL_STATIC_WITH_ARGS(KNumber, g_Euler, (
                              "2.718281828459045235360287471352662497757"
                              "24709369995957496696762772407663035354759"
                              "4571382178525166427"))
KNumber KNumber::Euler()
{
    return *g_Euler;
}

