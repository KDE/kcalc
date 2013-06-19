/*
Copyright (C) 2001 - 2013 Evan Teran
                          evan.teran@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config-kcalc.h>
#include "knumber.h"
#include "knumber_base.h"
#include "knumber_error.h"
#include "knumber_float.h"
#include "knumber_fraction.h"
#include "knumber_integer.h"
#include <QDebug>
#include <QRegExp>
#include <QStringList>
#include <cmath>

QString KNumber::GroupSeparator   = QLatin1String(",");
QString KNumber::DecimalSeparator = QLatin1String(".");

const KNumber KNumber::Zero(QLatin1String("0"));
const KNumber KNumber::One(QLatin1String("1"));
const KNumber KNumber::NegOne(QLatin1String("-1"));
const KNumber KNumber::PosInfinity(QLatin1String("inf"));
const KNumber KNumber::NegInfinity(QLatin1String("-inf"));
const KNumber KNumber::NaN(QLatin1String("nan"));

namespace {
namespace impl {

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void increment(QString &str, int position) {

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
        case '.':
            continue;
        }
        break;
    }
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void round(QString &str, int precision) {

	// Cut off if more digits in fractional part than 'precision'

    int decimalSymbolPos = str.indexOf(QLatin1Char('.'));

    if (decimalSymbolPos == -1) {
        if (precision == 0) {
            return;
        } else if (precision > 0) {   // add dot if missing (and needed)
            str.append(QLatin1Char('.'));
            decimalSymbolPos = str.length() - 1;
        }
    }

    // fill up with more than enough zeroes (in case fractional part too short)
    str.append(QString().fill(QLatin1Char('0'), precision));

    // Now decide whether to round up or down
    const char last_char = str[decimalSymbolPos + precision + 1].toLatin1();
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

    decimalSymbolPos = str.indexOf(QLatin1Char('.'));
    str.truncate(decimalSymbolPos + precision + 1);

    // if precision == 0 delete also '.'
    if (precision == 0) {
        str = str.section(QLatin1Char('.'), 0, 0);
    }
}
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
QString round(const QString &s, int precision) {

	QString tmp = s;
    if (precision < 0 || !QRegExp(QLatin1String("^[+-]?\\d+(\\.\\d+)*(e[+-]?\\d+)?$")).exactMatch(tmp)) {
        return s;
    }

    // Skip the sign (for now)
    const bool neg = (tmp[0] == QLatin1Char('-'));
    if (neg || tmp[0] == QLatin1Char('+')) {
        tmp.remove(0, 1);
    }

    // Split off exponential part (including 'e'-symbol)
    QString mantString = tmp.section(QLatin1Char('e'), 0, 0, QString::SectionCaseInsensitiveSeps);
    QString expString  = tmp.section(QLatin1Char('e'), 1, 1, QString::SectionCaseInsensitiveSeps | QString::SectionIncludeLeadingSep);

    if (expString.length() == 1) {
        expString.clear();
    }

    impl::round(mantString, precision);

    if (neg) {
        mantString.prepend(QLatin1Char('-'));
    }

    return mantString + expString;
}
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void KNumber::setGroupSeparator(const QString &ch) {
	GroupSeparator = ch;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void KNumber::setDecimalSeparator(const QString &ch) {
	DecimalSeparator = ch;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
QString KNumber::groupSeparator() {
	return GroupSeparator;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
QString KNumber::decimalSeparator() {
	return DecimalSeparator;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void KNumber::setDefaultFloatPrecision(int precision) {
    // Need to transform decimal digits into binary digits
    const unsigned long int bin_prec = static_cast<unsigned long int>(double(precision) * M_LN10 / M_LN2 + 1);
    mpf_set_default_prec(bin_prec);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void KNumber::setSplitoffIntegerForFractionOutput(bool x) {
	detail::knumber_fraction::set_split_off_integer_for_fraction_output(x);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void KNumber::setDefaultFractionalInput(bool x) {
	detail::knumber_fraction::set_default_fractional_input(x);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void KNumber::setDefaultFloatOutput(bool x) {
	detail::knumber_fraction::set_default_fractional_output(!x);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::Pi() {

	// TODO: after 4.10 release:
	//       create a new constructor which works just like the normal QString
	//       accepting constructor, but allows us to specify separator
	//       characters, this will allow things to be done slightly more
	// efficiently
	QString s(QLatin1String("3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117068"));
	s.replace('.', DecimalSeparator);
	return KNumber(s);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::Euler() {

	// TODO: after 4.10 release:
	//       create a new constructor which works just like the normal QString
	//       accepting constructor, but allows us to specify separator
	//       characters, this will allow things to be done slightly more
	// efficiently
	QString s(QLatin1String("2.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274"));
	s.replace('.', DecimalSeparator);
	return KNumber(s);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber::KNumber() : value_(new detail::knumber_integer(0)) {
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber::KNumber(const QString &s) : value_(0) {

	const QRegExp special_regex(QLatin1String("^(inf|-inf|nan)$"));
	const QRegExp integer_regex(QLatin1String("^[+-]?\\d+$"));
	const QRegExp fraction_regex(QLatin1String("^[+-]?\\d+/\\d+$"));
	const QRegExp float_regex(QString(QLatin1String("^([+-]?\\d*)(%1\\d*)?(e([+-]?\\d+))?$")).arg(QRegExp::escape(DecimalSeparator)));

	if (special_regex.exactMatch(s)) {
		value_ = new detail::knumber_error(s);
	} else if (integer_regex.exactMatch(s)) {
		value_ = new detail::knumber_integer(s);
	} else if (fraction_regex.exactMatch(s)) {
		value_ = new detail::knumber_fraction(s);
		simplify();
	} else if (float_regex.exactMatch(s)) {

		if(detail::knumber_fraction::default_fractional_input) {

			const QStringList list = float_regex.capturedTexts();
			if(list.size() == 5) {

				const QString ipart = list[1];
				const QString fpart = list[2];
				const QString epart = list[3];
				const int e_val     = list[4].toInt();

				QString num = ipart + fpart.mid(1);
				QString den = QLatin1String("1") + QString(fpart.size() - 1, QLatin1Char('0'));

				if(e_val < 0) {
					den = den + QString(::abs(e_val), QLatin1Char('0'));
				} else if(e_val > 0) {
					num = num + QString(::abs(e_val), QLatin1Char('0'));
				}

				value_ = new detail::knumber_fraction(QString(QLatin1String("%1/%2")).arg(num, den));
				simplify();
				return;
			}
		}

		// we need to normalize the decimal searator to US style because that's
		// the only type that the GMP function accept
		QString new_s = s;
		new_s.replace(DecimalSeparator, QLatin1String("."));

		value_ = new detail::knumber_float(new_s);
		simplify();
	} else {
		value_ = new detail::knumber_error(detail::knumber_error::ERROR_UNDEFINED);
	}
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber::KNumber(qint32 value) : value_(new detail::knumber_integer(value)) {
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber::KNumber(qint64 value) : value_(new detail::knumber_integer(value)) {
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber::KNumber(quint32 value) : value_(new detail::knumber_integer(value)) {
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber::KNumber(quint64 value) : value_(new detail::knumber_integer(value)) {
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber::KNumber(qint64 num, quint64 den) : value_(new detail::knumber_fraction(num, den)) {
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber::KNumber(quint64 num, quint64 den) : value_(new detail::knumber_fraction(num, den)) {
}

#ifdef HAVE_LONG_DOUBLE
//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber::KNumber(long double value) : value_(new detail::knumber_float(value)) {
	simplify();
}
#endif

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber::KNumber(double value) : value_(new detail::knumber_float(value)) {
	simplify();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber::KNumber(const KNumber &other) : value_(0) {
	if(&other != this) {
		value_ = other.value_->clone();
	}
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber::~KNumber() {
	delete value_;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber::Type KNumber::type() const {

	if(dynamic_cast<detail::knumber_integer *>(value_)) {
		return TYPE_INTEGER;
	} else if(dynamic_cast<detail::knumber_float *>(value_)) {
		return TYPE_FLOAT;
	} else if(dynamic_cast<detail::knumber_fraction *>(value_)) {
		return TYPE_FRACTION;
	} else if(dynamic_cast<detail::knumber_error *>(value_)) {
		return TYPE_ERROR;
	} else {
		Q_ASSERT(0);
		return TYPE_ERROR;
	}
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber &KNumber::operator=(const KNumber &rhs) {
	KNumber(rhs).swap(*this);
	return *this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void KNumber::swap(KNumber &other) {
	qSwap(value_, other.value_);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::integerPart() const {

	KNumber x(*this);

	if(detail::knumber_integer *const p = dynamic_cast<detail::knumber_integer *>(value_)) {
		// NO-OP
		Q_UNUSED(p);
	} else if(detail::knumber_float *const p = dynamic_cast<detail::knumber_float *>(value_)) {
		detail::knumber_base *v = new detail::knumber_integer(p);
		qSwap(v, x.value_);
		delete v;
	} else if(detail::knumber_fraction *const p = dynamic_cast<detail::knumber_fraction *>(value_)) {
		detail::knumber_base *v = new detail::knumber_integer(p);
		qSwap(v, x.value_);
		delete v;
	} else if(detail::knumber_error *const p = dynamic_cast<detail::knumber_error *>(value_)) {
		// NO-OP
		Q_UNUSED(p);
	} else {
		Q_ASSERT(0);
	}

	return x;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void KNumber::simplify() {

	if(value_->is_integer()) {

		if(detail::knumber_integer *const p = dynamic_cast<detail::knumber_integer *>(value_)) {
			// NO-OP
			Q_UNUSED(p);
		} else if(detail::knumber_float *const p = dynamic_cast<detail::knumber_float *>(value_)) {
			detail::knumber_base *v = new detail::knumber_integer(p);
			qSwap(v, value_);
			delete v;
		} else if(detail::knumber_fraction *const p = dynamic_cast<detail::knumber_fraction *>(value_)) {
			detail::knumber_base *v = new detail::knumber_integer(p);
			qSwap(v, value_);
			delete v;
		} else if(detail::knumber_error *const p = dynamic_cast<detail::knumber_error *>(value_)) {
			// NO-OP
			Q_UNUSED(p);
		} else {
			Q_ASSERT(0);
		}
	}
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber &KNumber::operator+=(const KNumber &rhs) {
	value_ = value_->add(rhs.value_);
	simplify();
	return *this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber &KNumber::operator-=(const KNumber &rhs) {
	value_ = value_->sub(rhs.value_);
	simplify();
	return *this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber &KNumber::operator*=(const KNumber &rhs) {
	value_ = value_->mul(rhs.value_);
	simplify();
	return *this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber &KNumber::operator/=(const KNumber &rhs) {
	value_ = value_->div(rhs.value_);
	simplify();
	return *this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber &KNumber::operator%=(const KNumber &rhs) {
	value_ = value_->mod(rhs.value_);
	simplify();
	return *this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber &KNumber::operator&=(const KNumber &rhs) {
	value_ = value_->bitwise_and(rhs.value_);
	return *this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber &KNumber::operator|=(const KNumber &rhs) {
	value_ = value_->bitwise_or(rhs.value_);
	return *this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber &KNumber::operator^=(const KNumber &rhs) {
	value_ = value_->bitwise_xor(rhs.value_);
	return *this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber &KNumber::operator<<=(const KNumber &rhs) {
	value_ = value_->bitwise_shift(rhs.value_);
	return *this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber &KNumber::operator>>=(const KNumber &rhs) {
	const KNumber rhs_neg(-rhs);
	value_ = value_->bitwise_shift(rhs_neg.value_);
	return *this;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::operator-() const {
	KNumber x(*this);
	x.value_ = x.value_->neg();
	return x;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::operator~() const {
	KNumber x(*this);
	x.value_ = x.value_->cmp();
	return x;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
QString KNumber::toQString(int width, int precision) const {

	if(value_->is_zero()) {
		return QLatin1String("0");
	}

	QString s;

	if(detail::knumber_integer *const p = dynamic_cast<detail::knumber_integer *>(value_)) {
		if(width > 0) {
			s = detail::knumber_float(p).toString(width);
		} else {
			s = value_->toString(width);
		}
	} else if(detail::knumber_float *const p = dynamic_cast<detail::knumber_float *>(value_)) {
		if(width > 0) {
			s = value_->toString(width);
		} else {
			s = value_->toString(3 * mpf_get_default_prec() / 10);
		}
	} else if(detail::knumber_fraction *const p = dynamic_cast<detail::knumber_fraction *>(value_)) {
		s = value_->toString(width);
	} else {
		return value_->toString(width);
	}

	// now do some rounding to make sure things are displayed reasonably
    if (precision >= 0) {
        return round(s, precision);
    } else {
        return s;
    }
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
quint64 KNumber::toUint64() const {
	return value_->toUint64();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
qint64 KNumber::toInt64() const {
	return value_->toInt64();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::abs() const {
	KNumber z(*this);
	z.value_ = z.value_->abs();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::cbrt() const {
	KNumber z(*this);
	z.value_ = z.value_->cbrt();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::sqrt() const {
	KNumber z(*this);
	z.value_ = z.value_->sqrt();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::pow(const KNumber &x) const {

	// if the LHS is a special then we can use this function
	// no matter what, cause the result is a special too
	if(!dynamic_cast<detail::knumber_error *>(value_)) {
		// number much bigger than this tend to crash GMP with
		// an abort
		if(x > KNumber(QLatin1String("1000000000"))) {
			return PosInfinity;
		}
	}

	KNumber z(*this);
	z.value_ = z.value_->pow(x.value_);
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::sin() const {
	KNumber z(*this);
	z.value_ = z.value_->sin();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::cos() const {
	KNumber z(*this);
	z.value_ = z.value_->cos();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::tan() const {
	KNumber z(*this);
	z.value_ = z.value_->tan();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::asin() const {
	KNumber z(*this);
	z.value_ = z.value_->asin();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::acos() const {
	KNumber z(*this);
	z.value_ = z.value_->acos();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::atan() const {
	KNumber z(*this);
	z.value_ = z.value_->atan();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::sinh() const {
	KNumber z(*this);
	z.value_ = z.value_->sinh();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::cosh() const {
	KNumber z(*this);
	z.value_ = z.value_->cosh();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::tanh() const {
	KNumber z(*this);
	z.value_ = z.value_->tanh();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::asinh() const {
	KNumber z(*this);
	z.value_ = z.value_->asinh();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::acosh() const {
	KNumber z(*this);
	z.value_ = z.value_->acosh();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::atanh() const {
	KNumber z(*this);
	z.value_ = z.value_->atanh();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::factorial() const {
	KNumber z(*this);

	// number much bigger than this tend to crash GMP with
	// an abort
	if(z > KNumber(QLatin1String("10000000000"))) {
		return PosInfinity;
	}

	z.value_ = z.value_->factorial();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::log2() const {
	KNumber z(*this);
	z.value_ = z.value_->log2();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::log10() const {
	KNumber z(*this);
	z.value_ = z.value_->log10();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::ln() const {
	KNumber z(*this);
	z.value_ = z.value_->ln();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::exp2() const {
	KNumber z(*this);
	z.value_ = z.value_->exp2();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::exp10() const {
	KNumber z(*this);
	z.value_ = z.value_->exp10();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::exp() const {
	KNumber z(*this);
	z.value_ = z.value_->exp();
	z.simplify();
	return z;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
KNumber KNumber::bin(const KNumber &x) const {
	KNumber z(*this);
	z.value_ = z.value_->bin(x.value_);
	z.simplify();
	return z;
}
