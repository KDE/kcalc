/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
// clang-format off
#include "config-knumber.h"
// clang-format on
#include "knumber.h"
#include "knumber_base.h"
#include "knumber_error.h"
#include "knumber_float.h"
#include "knumber_fraction.h"
#include "knumber_integer.h"
#include <QRegularExpression>
#include <QStringList>
#include <cmath>

QString KNumber::GroupSeparator = QStringLiteral(",");
QString KNumber::DecimalSeparator = QStringLiteral(".");

const KNumber KNumber::Zero(QStringLiteral("0"));
const KNumber KNumber::One(QStringLiteral("1"));
const KNumber KNumber::NegOne(QStringLiteral("-1"));
const KNumber KNumber::OneHundred(QStringLiteral("100"));
const KNumber KNumber::OneThousand(QStringLiteral("1000"));
const KNumber KNumber::PosInfinity(QStringLiteral("inf"));
const KNumber KNumber::NegInfinity(QStringLiteral("-inf"));
const KNumber KNumber::NaN(QStringLiteral("nan"));

namespace
{
namespace impl
{
//------------------------------------------------------------------------------
// Name: increment
//------------------------------------------------------------------------------
void increment(QString &str, int position)
{
    for (int i = position; i >= 0; i--) {
        const char last_char = str[i].toLatin1();
        switch (last_char) {
        case '0':
            str[i] = QLatin1Char('1');
            break;
        case '1':
            str[i] = QLatin1Char('2');
            break;
        case '2':
            str[i] = QLatin1Char('3');
            break;
        case '3':
            str[i] = QLatin1Char('4');
            break;
        case '4':
            str[i] = QLatin1Char('5');
            break;
        case '5':
            str[i] = QLatin1Char('6');
            break;
        case '6':
            str[i] = QLatin1Char('7');
            break;
        case '7':
            str[i] = QLatin1Char('8');
            break;
        case '8':
            str[i] = QLatin1Char('9');
            break;
        case '9':
            str[i] = QLatin1Char('0');
            if (i == 0) {
                str.prepend(QLatin1Char('1'));
            }
            continue;
        default:
            continue;
        }
        break;
    }
}

//------------------------------------------------------------------------------
// Name: round
//------------------------------------------------------------------------------
void round(QString &str, int precision, const QString &decimalSeparator)
{
    int decimalSymbolPos = str.indexOf(decimalSeparator);
    if (decimalSymbolPos == -1) {
        if (precision == 0) {
            return;
        } else {
            str.append(decimalSeparator);
            str.append(QString().fill(QLatin1Char('0'), precision));
            return;
        }
    } else {
        int desiredLength = decimalSymbolPos + precision + 1;
        int extraZeroNeeded = desiredLength - str.length();

        if (extraZeroNeeded == 0) {
            return;
        } else if (extraZeroNeeded > 0) {
            str.append(QString().fill(QLatin1Char('0'), extraZeroNeeded));
        } else {
            // decide whether to round up or down based on the digit after desired length
            if (str.at(desiredLength).toLatin1() >= '5') {
                increment(str, desiredLength - 1);
            }
            decimalSymbolPos = str.indexOf(decimalSeparator);
            str.truncate(decimalSymbolPos + precision + 1);
        }
    }
}
}
}

//------------------------------------------------------------------------------
// Name: round
//------------------------------------------------------------------------------
QString KNumber::round(const QString &s, int precision) const
{
    QString tmp = s;

    const QRegularExpression rx(QString(QLatin1String(R"(^[+-]?\d+(%1\d+)*(e[+-]?\d+)?$)")).arg(QRegularExpression::escape(DecimalSeparator)));

    if (precision < 0 || !rx.match(tmp).hasMatch()) {
        return s;
    }

    // Skip the sign (for now)
    const bool neg = (tmp[0] == QLatin1Char('-'));
    if (neg || tmp[0] == QLatin1Char('+')) {
        tmp.remove(0, 1);
    }

    // Split off exponential part (including 'e'-symbol)
    QString mantString = tmp.section(QLatin1Char('e'), 0, 0, QString::SectionCaseInsensitiveSeps);
    QString expString = tmp.section(QLatin1Char('e'), 1, 1, QString::SectionCaseInsensitiveSeps | QString::SectionIncludeLeadingSep);

    if (expString.length() == 1) {
        expString.clear();
    }

    impl::round(mantString, precision, KNumber::decimalSeparator());

    if (neg) {
        mantString.prepend(QLatin1Char('-'));
    }

    return mantString + expString;
}

//------------------------------------------------------------------------------
// Name: setGroupSeparator
//------------------------------------------------------------------------------
void KNumber::setGroupSeparator(const QString &ch)
{
    GroupSeparator = ch;
}

//------------------------------------------------------------------------------
// Name: setDecimalSeparator
//------------------------------------------------------------------------------
void KNumber::setDecimalSeparator(const QString &ch)
{
    DecimalSeparator = ch;
}

//------------------------------------------------------------------------------
// Name: groupSeparator
//------------------------------------------------------------------------------
QString KNumber::groupSeparator()
{
    return GroupSeparator;
}

//------------------------------------------------------------------------------
// Name: decimalSeparator
//------------------------------------------------------------------------------
const QString KNumber::decimalSeparator()
{
    return DecimalSeparator;
}

//------------------------------------------------------------------------------
// Name: setDefaultFloatPrecision
//------------------------------------------------------------------------------
void KNumber::setDefaultFloatPrecision(int precision)
{
    // Need to transform decimal digits into binary digits
    const auto bin_prec = static_cast<unsigned long int>(::ceil(precision * M_LN10 / M_LN2) + 1);
    mpfr_set_default_prec(static_cast<mpfr_prec_t>(bin_prec));
}

//------------------------------------------------------------------------------
// Name: setSplitoffIntegerForFractionOutput
//------------------------------------------------------------------------------
void KNumber::setSplitoffIntegerForFractionOutput(bool x)
{
    detail::KNumberFraction::setSplitOffIntegerForFractionOutput(x);
}

//------------------------------------------------------------------------------
// Name: setDefaultFractionalInput
//------------------------------------------------------------------------------
void KNumber::setDefaultFractionalInput(bool x)
{
    detail::KNumberFraction::setDefaultFractionalInput(x);
}

//------------------------------------------------------------------------------
// Name: setDefaultFloatOutput
//------------------------------------------------------------------------------
void KNumber::setDefaultFloatOutput(bool x)
{
    detail::KNumberFraction::setDefaultFractionalOutput(!x);
}

//------------------------------------------------------------------------------
// Name: Pi
//------------------------------------------------------------------------------
KNumber KNumber::Pi()
{
    // TODO: after 4.10 release:
    //       create a new constructor which works just like the normal QString
    //       accepting constructor, but allows us to specify separator
    //       characters, this will allow things to be done slightly more
    // efficiently
    QString s(QStringLiteral("3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117068"));
    s.replace(QLatin1Char('.'), DecimalSeparator);
    return KNumber(s);
}

//------------------------------------------------------------------------------
// Name: Euler
//------------------------------------------------------------------------------
KNumber KNumber::Euler()
{
    // TODO: after 4.10 release:
    //       create a new constructor which works just like the normal QString
    //       accepting constructor, but allows us to specify separator
    //       characters, this will allow things to be done slightly more
    // efficiently
    QString s(QStringLiteral("2.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274"));
    s.replace(QLatin1Char('.'), DecimalSeparator);
    return KNumber(s);
}

//------------------------------------------------------------------------------
// Name: KNumber
//------------------------------------------------------------------------------
KNumber::KNumber()
    : m_value(new detail::KNumberInteger(0))
{
}

//------------------------------------------------------------------------------
// Name: KNumber
//------------------------------------------------------------------------------
KNumber::KNumber(const QString &s)
    : m_value(nullptr)
{
    static const QRegularExpression specialRegex(QLatin1String("^(inf|-inf|nan)$"));
    // const QRegularExpression integerRegex(QLatin1String("^[+-]?\\d+$"));
    static const QRegularExpression integerRegex(QLatin1String("^[+-]?\\[1-9]d+$"));
    static const QRegularExpression binaryIntegerRegex(QLatin1String("^0b[0-1]{1,64}$"));
    static const QRegularExpression octalIntegerRegex(QLatin1String("^0[0-7]{1,21}$"));
    static const QRegularExpression hexIntegerRegex(QLatin1String("^0x[0-9A-Fa-f]{1,16}$"));
    static const QRegularExpression fractionRegex(QLatin1String("^[+-]?\\d+/\\d+$"));
    const QRegularExpression floatRegex(QString(QLatin1String(R"(^([+-]?\d*)(%1\d*)?([e|E]([+-]?\d+))?$)")).arg(QRegularExpression::escape(DecimalSeparator)));

    if (specialRegex.match(s).hasMatch()) {
        m_value = new detail::KNumberError(s);
    } else if (integerRegex.match(s).hasMatch()) {
        m_value = new detail::KNumberInteger(s);
    } else if (fractionRegex.match(s).hasMatch()) {
        m_value = new detail::KNumberFraction(s);
        simplify();
    } else if (hexIntegerRegex.match(s).hasMatch() || octalIntegerRegex.match(s).hasMatch() || binaryIntegerRegex.match(s).hasMatch()) {
        m_value = new detail::KNumberInteger(s.toULongLong(nullptr, 0));
    } else if (const auto match = floatRegex.match(s); match.hasMatch()) {
        if (detail::KNumberFraction::defaultFractionalInput) {
            const QString ipart = match.captured(1);
            const QString fpart = match.captured(2);
            const int e_val = match.captured(4).toInt();

            QString num = ipart + fpart.mid(1);
            QString den = QLatin1String("1") + QString(fpart.size() - 1, QLatin1Char('0'));

            if (e_val < 0) {
                den = den + QString(::abs(e_val), QLatin1Char('0'));
            } else if (e_val > 0) {
                num = num + QString(::abs(e_val), QLatin1Char('0'));
            }

            m_value = new detail::KNumberFraction(QStringLiteral("%1/%2").arg(num, den));
            simplify();
            return;
        }

        // we need to normalize the decimal separator to US style because that's
        // the only type that the GMP function accept
        QString new_s = s;
        new_s.replace(DecimalSeparator, QLatin1String("."));

        m_value = new detail::KNumberFloat(new_s);
        simplify();
    } else {
        m_value = new detail::KNumberError(detail::KNumberError::Undefined);
    }
}

//------------------------------------------------------------------------------
// Name: KNumber
//------------------------------------------------------------------------------
KNumber::KNumber(qint32 value)
    : m_value(new detail::KNumberInteger(value))
{
}

//------------------------------------------------------------------------------
// Name: KNumber
//------------------------------------------------------------------------------
KNumber::KNumber(qint64 value)
    : m_value(new detail::KNumberInteger(value))
{
}

//------------------------------------------------------------------------------
// Name: KNumber
//------------------------------------------------------------------------------
KNumber::KNumber(quint32 value)
    : m_value(new detail::KNumberInteger(value))
{
}

//------------------------------------------------------------------------------
// Name: KNumber
//------------------------------------------------------------------------------
KNumber::KNumber(quint64 value)
    : m_value(new detail::KNumberInteger(value))
{
}

//------------------------------------------------------------------------------
// Name: KNumber
//------------------------------------------------------------------------------
KNumber::KNumber(qint64 num, quint64 den)
    : m_value(new detail::KNumberFraction(num, den))
{
}

//------------------------------------------------------------------------------
// Name: KNumber
//------------------------------------------------------------------------------
KNumber::KNumber(quint64 num, quint64 den)
    : m_value(new detail::KNumberFraction(num, den))
{
}

#ifdef HAVE_LONG_DOUBLE
//------------------------------------------------------------------------------
// Name: KNumber
//------------------------------------------------------------------------------
KNumber::KNumber(long double value)
    : m_value(new detail::KNumberFloat(value))
{
    simplify();
}
#endif

//------------------------------------------------------------------------------
// Name: KNumber
//------------------------------------------------------------------------------
KNumber::KNumber(double value)
    : m_value(new detail::KNumberFloat(value))
{
    simplify();
}

//------------------------------------------------------------------------------
// Name: KNumber
//------------------------------------------------------------------------------
KNumber::KNumber(const KNumber &other)
    : m_value(nullptr)
{
    if (&other != this) {
        m_value = other.m_value->clone();
    }
}

//------------------------------------------------------------------------------
// Name: ~KNumber
//------------------------------------------------------------------------------
KNumber::~KNumber()
{
    delete m_value;
}

//------------------------------------------------------------------------------
// Name: type
//------------------------------------------------------------------------------
KNumber::Type KNumber::type() const
{
    if (dynamic_cast<detail::KNumberInteger *>(m_value)) {
        return TypeInteger;
    } else if (dynamic_cast<detail::KNumberFloat *>(m_value)) {
        return TypeFloat;
    } else if (dynamic_cast<detail::KNumberFraction *>(m_value)) {
        return TypeFraction;
    } else if (dynamic_cast<detail::KNumberError *>(m_value)) {
        return TypeError;
    } else {
        Q_ASSERT(0);
        return TypeError;
    }
}

//------------------------------------------------------------------------------
// Name: operator=
//------------------------------------------------------------------------------
KNumber &KNumber::operator=(const KNumber &rhs)
{
    KNumber(rhs).swap(*this);
    return *this;
}

//------------------------------------------------------------------------------
// Name: swap
//------------------------------------------------------------------------------
void KNumber::swap(KNumber &other)
{
    qSwap(m_value, other.m_value);
}

//------------------------------------------------------------------------------
// Name: integerPart
//------------------------------------------------------------------------------
KNumber KNumber::integerPart() const
{
    KNumber x(*this);

    if (auto const p = dynamic_cast<detail::KNumberInteger *>(m_value)) {
        // NO-OP
        Q_UNUSED(p);
    } else if (auto const p = dynamic_cast<detail::KNumberFloat *>(m_value)) {
        detail::KNumberBase *v = new detail::KNumberInteger(p);
        qSwap(v, x.m_value);
        delete v;
    } else if (auto const p = dynamic_cast<detail::KNumberFraction *>(m_value)) {
        detail::KNumberBase *v = new detail::KNumberInteger(p);
        qSwap(v, x.m_value);
        delete v;
    } else if (auto const p = dynamic_cast<detail::KNumberError *>(m_value)) {
        // NO-OP
        Q_UNUSED(p);
    } else {
        Q_ASSERT(0);
    }

    return x;
}

//------------------------------------------------------------------------------
// Name: simplify
//------------------------------------------------------------------------------
void KNumber::simplify()
{
    if (m_value->isInteger()) {
        if (auto const p = dynamic_cast<detail::KNumberInteger *>(m_value)) {
            // NO-OP
            Q_UNUSED(p);
        } else if (auto const p = dynamic_cast<detail::KNumberFloat *>(m_value)) {
            detail::KNumberBase *v = new detail::KNumberInteger(p);
            qSwap(v, m_value);
            delete v;
        } else if (auto const p = dynamic_cast<detail::KNumberFraction *>(m_value)) {
            detail::KNumberBase *v = new detail::KNumberInteger(p);
            qSwap(v, m_value);
            delete v;
        } else if (auto const p = dynamic_cast<detail::KNumberError *>(m_value)) {
            // NO-OP
            Q_UNUSED(p);
        } else {
            Q_ASSERT(0);
        }
    }
}

//------------------------------------------------------------------------------
// Name: operator+=
//------------------------------------------------------------------------------
KNumber &KNumber::operator+=(const KNumber &rhs)
{
    m_value = m_value->add(rhs.m_value);
    simplify();
    return *this;
}

//------------------------------------------------------------------------------
// Name: operator-=
//------------------------------------------------------------------------------
KNumber &KNumber::operator-=(const KNumber &rhs)
{
    m_value = m_value->sub(rhs.m_value);
    simplify();
    return *this;
}

//------------------------------------------------------------------------------
// Name: operator*=
//------------------------------------------------------------------------------
KNumber &KNumber::operator*=(const KNumber &rhs)
{
    m_value = m_value->mul(rhs.m_value);
    simplify();
    return *this;
}

//------------------------------------------------------------------------------
// Name: operator/=
//------------------------------------------------------------------------------
KNumber &KNumber::operator/=(const KNumber &rhs)
{
    // Fix for bug #330577, x/0  is undefined, not infinity
    // Also indirectly fixes bug #329897, tan(90) is undefined, not infinity
    if (rhs == Zero) {
        *this = NaN;
        return *this;
    }

    m_value = m_value->div(rhs.m_value);
    simplify();
    return *this;
}

//------------------------------------------------------------------------------
// Name: operator%=
//------------------------------------------------------------------------------
KNumber &KNumber::operator%=(const KNumber &rhs)
{
    m_value = m_value->mod(rhs.m_value);
    simplify();
    return *this;
}

//------------------------------------------------------------------------------
// Name: operator&=
//------------------------------------------------------------------------------
KNumber &KNumber::operator&=(const KNumber &rhs)
{
    m_value = m_value->bitwiseAnd(rhs.m_value);
    return *this;
}

//------------------------------------------------------------------------------
// Name: operator|=
//------------------------------------------------------------------------------
KNumber &KNumber::operator|=(const KNumber &rhs)
{
    m_value = m_value->bitwiseOr(rhs.m_value);
    return *this;
}

//------------------------------------------------------------------------------
// Name: operator^=
//------------------------------------------------------------------------------
KNumber &KNumber::operator^=(const KNumber &rhs)
{
    m_value = m_value->bitwiseXor(rhs.m_value);
    return *this;
}

//------------------------------------------------------------------------------
// Name: operator<<
//------------------------------------------------------------------------------
KNumber &KNumber::operator<<=(const KNumber &rhs)
{
    m_value = m_value->bitwiseShift(rhs.m_value);
    return *this;
}

//------------------------------------------------------------------------------
// Name: operator>>=
//------------------------------------------------------------------------------
KNumber &KNumber::operator>>=(const KNumber &rhs)
{
    const KNumber rhs_neg(-rhs);
    m_value = m_value->bitwiseShift(rhs_neg.m_value);
    return *this;
}

//------------------------------------------------------------------------------
// Name: operator-
//------------------------------------------------------------------------------
KNumber KNumber::operator-() const
{
    KNumber x(*this);
    x.m_value = x.m_value->neg();
    return x;
}

//------------------------------------------------------------------------------
// Name: operator~
//------------------------------------------------------------------------------
KNumber KNumber::operator~() const
{
    KNumber x(*this);
    x.m_value = x.m_value->cmp();
    return x;
}

//------------------------------------------------------------------------------
// Name: toQString
//------------------------------------------------------------------------------
QString KNumber::toQString(int width, int precision) const
{
    if (m_value->isZero()) {
        return QStringLiteral("0");
    }

    QString s;

    if (auto const p = dynamic_cast<detail::KNumberInteger *>(m_value)) {
        if (width > 0) {
            s = detail::KNumberFloat(p).toString(width);
        } else {
            s = m_value->toString(width);
        }
    } else if (dynamic_cast<detail::KNumberFloat *>(m_value)) {
        if (width > 0) {
            s = m_value->toString(width);
        } else {
            s = m_value->toString(3 * mpf_get_default_prec() / 10);
        }
        localizeDecimalSeparator(s);
    } else if (dynamic_cast<detail::KNumberFraction *>(m_value)) {
        s = m_value->toString(width);
        localizeDecimalSeparator(s);
    } else {
        return m_value->toString(width);
    }

    // now do some rounding to make sure things are displayed reasonably
    if (precision >= 0) {
        return round(s, precision);
    } else {
        return s;
    }
}

//------------------------------------------------------------------------------
// Name: localizeDecimalSeparator
// Desc: replaces the internal '.' as decimal separator
//       with the local decimal separator of the user's machine
//------------------------------------------------------------------------------
void KNumber::localizeDecimalSeparator(QString &knumber)
{
    knumber.replace(QLatin1Char('.'), KNumber::DecimalSeparator);
}

//------------------------------------------------------------------------------
// Name: toUint64
//------------------------------------------------------------------------------
quint64 KNumber::toUint64() const
{
    return m_value->toUint64();
}

//------------------------------------------------------------------------------
// Name: toInt64
//------------------------------------------------------------------------------
qint64 KNumber::toInt64() const
{
    return m_value->toInt64();
}

//------------------------------------------------------------------------------
// Name: abs
//------------------------------------------------------------------------------
KNumber KNumber::abs() const
{
    KNumber z(*this);
    z.m_value = z.m_value->abs();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: cbrt
//------------------------------------------------------------------------------
KNumber KNumber::cbrt() const
{
    KNumber z(*this);
    z.m_value = z.m_value->cbrt();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: sqrt
//------------------------------------------------------------------------------
KNumber KNumber::sqrt() const
{
    KNumber z(*this);
    z.m_value = z.m_value->sqrt();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: pow
//------------------------------------------------------------------------------
KNumber KNumber::pow(const KNumber &x) const
{
    // Fix for bug #330711, pow(0, -x) was causing crashes
    // Fix for bug #330597, pow(0,0) was 1 now it is NaN
    // Thanks to Raushan Kumar for identifying the issue and submitting
    // patches
    if (*this == Zero && x <= Zero) {
        return NaN;
    }

    // if the LHS is a special then we can use this function
    // no matter what, cause the result is a special too
    if (!dynamic_cast<detail::KNumberError *>(m_value)) {
        // number much bigger than this tend to crash GMP with
        // an abort
        if (x > KNumber(QStringLiteral("1000000000"))) {
            return PosInfinity;
        }
    }

    KNumber z(*this);
    z.m_value = z.m_value->pow(x.m_value);
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: sin
//------------------------------------------------------------------------------
KNumber KNumber::sin() const
{
    KNumber z(*this);
    z.m_value = z.m_value->sin();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: cos
//------------------------------------------------------------------------------
KNumber KNumber::cos() const
{
    KNumber z(*this);
    z.m_value = z.m_value->cos();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: tan
//------------------------------------------------------------------------------
KNumber KNumber::tan() const
{
    KNumber z(*this);
    z.m_value = z.m_value->tan();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: tgamma
//------------------------------------------------------------------------------
KNumber KNumber::tgamma() const
{
    KNumber z(*this);
    if (z > KNumber(QStringLiteral("10000000000"))) {
        return PosInfinity;
    }
    z.m_value = z.m_value->tgamma();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: asin
//------------------------------------------------------------------------------
KNumber KNumber::asin() const
{
    KNumber z(*this);
    z.m_value = z.m_value->asin();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: acos
//------------------------------------------------------------------------------
KNumber KNumber::acos() const
{
    KNumber z(*this);
    z.m_value = z.m_value->acos();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: atan
//------------------------------------------------------------------------------
KNumber KNumber::atan() const
{
    KNumber z(*this);
    z.m_value = z.m_value->atan();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: sinh
//------------------------------------------------------------------------------
KNumber KNumber::sinh() const
{
    KNumber z(*this);
    z.m_value = z.m_value->sinh();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: cosh
//------------------------------------------------------------------------------
KNumber KNumber::cosh() const
{
    KNumber z(*this);
    z.m_value = z.m_value->cosh();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: tanh
//------------------------------------------------------------------------------
KNumber KNumber::tanh() const
{
    KNumber z(*this);
    z.m_value = z.m_value->tanh();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: asinh
//------------------------------------------------------------------------------
KNumber KNumber::asinh() const
{
    KNumber z(*this);
    z.m_value = z.m_value->asinh();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: acosh
//------------------------------------------------------------------------------
KNumber KNumber::acosh() const
{
    KNumber z(*this);
    z.m_value = z.m_value->acosh();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: atanh
//------------------------------------------------------------------------------
KNumber KNumber::atanh() const
{
    KNumber z(*this);
    z.m_value = z.m_value->atanh();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: factorial
//------------------------------------------------------------------------------
KNumber KNumber::factorial() const
{
    KNumber z(*this);

    // number much bigger than this tend to crash GMP with
    // an abort
    if (z > KNumber(QStringLiteral("10000000000"))) {
        return PosInfinity;
    }

    z.m_value = z.m_value->factorial();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: log2
//------------------------------------------------------------------------------
KNumber KNumber::log2() const
{
    KNumber z(*this);
    z.m_value = z.m_value->log2();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: log10
//------------------------------------------------------------------------------
KNumber KNumber::log10() const
{
    KNumber z(*this);
    z.m_value = z.m_value->log10();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: ln
//------------------------------------------------------------------------------
KNumber KNumber::ln() const
{
    KNumber z(*this);
    z.m_value = z.m_value->ln();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: floor
//------------------------------------------------------------------------------
KNumber KNumber::floor() const
{
    KNumber z(*this);
    z.m_value = z.m_value->floor();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: ceil
//------------------------------------------------------------------------------
KNumber KNumber::ceil() const
{
    KNumber z(*this);
    z.m_value = z.m_value->ceil();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: exp2
//------------------------------------------------------------------------------
KNumber KNumber::exp2() const
{
    KNumber z(*this);
    z.m_value = z.m_value->exp2();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: exp10
//------------------------------------------------------------------------------
KNumber KNumber::exp10() const
{
    KNumber z(*this);
    z.m_value = z.m_value->exp10();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: exp
//------------------------------------------------------------------------------
KNumber KNumber::exp() const
{
    KNumber z(*this);
    z.m_value = z.m_value->exp();
    z.simplify();
    return z;
}

//------------------------------------------------------------------------------
// Name: bin
//------------------------------------------------------------------------------
KNumber KNumber::bin(const KNumber &x) const
{
    KNumber z(*this);
    z.m_value = z.m_value->bin(x.m_value);
    z.simplify();
    return z;
}
