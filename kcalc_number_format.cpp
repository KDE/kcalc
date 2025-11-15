/*
    SPDX-FileCopyrightText: 2025 Marlon Pohl <admin@neo8.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_number_format.h"

#include "kcalc_settings.h"

// KCalcNumberFormatter
QString KCalcNumberFormatter::formatNumber(const QString &string, const KCalcNumberBase base)
{
    QString formattedNumber = string;
    const bool groupDigits_ = KCalcSettings::groupDigits();
    const bool special = (string.contains(QLatin1String("nan")) || string.contains(QLatin1String("inf")));

    // The decimal mode needs special treatment for two reasons, because: a) it uses KGlobal::locale() to get a localized
    // format and b) it has possible numbers after the decimal place. Neither applies to Binary, Hexadecimal or Octal.

    if ((groupDigits_ || base == NbDecimal) && !special) {
        switch (base) {
        case NbDecimal:
            formattedNumber = formatDecimalNumber(string, groupDigits_);
            break;
        case NbBinary:
            formattedNumber = groupDigits(string, KCalcSettings::binaryGrouping());
            break;
        case NbOctal:
            formattedNumber = groupDigits(string, KCalcSettings::octalGrouping());
            break;
        case NbHex:
            formattedNumber = groupDigits(string, KCalcSettings::hexadecimalGrouping());
            break;
        }
    }

    return formattedNumber;
}

QString KCalcNumberFormatter::formatDecimalNumber(QString string, bool explicitGrouping)
{
    QLocale locale;

    string.replace(QLatin1Char('.'), locale.decimalPoint());

    if (explicitGrouping && !(locale.numberOptions() & QLocale::OmitGroupSeparator)) {
        // find position after last digit
        int pos = string.indexOf(locale.decimalPoint());
        if (pos < 0) {
            // do not group digits after the exponent part
            const int expPos = string.indexOf(QLatin1Char('e'));
            if (expPos > 0) {
                pos = expPos;
            } else {
                pos = string.length();
            }
        }

        // find first digit to not group leading spaces or signs
        int firstDigitPos = 0;
        for (int i = 0, total = string.length(); i < total; ++i) {
            if (string.at(i).isDigit()) {
                firstDigitPos = i;
                break;
            }
        }

        const auto groupSeparator = locale.groupSeparator();
        const int groupSize = 3;

        string.reserve(string.length() + (pos - 1) / groupSize);
        while ((pos -= groupSize) > firstDigitPos) {
            string.insert(pos, groupSeparator);
        }
    }

    string.replace(QLatin1Char('-'), locale.negativeSign());
    string.replace(QLatin1Char('+'), locale.positiveSign());

    // Digits in unicode is encoded in contiguous range and with the digit zero as the first.
    // To convert digits to other locales,
    // just add the digit value and the leading zero's code point.
    // ref: Unicode15 chapter 4.6 Numeric Value https://www.unicode.org/versions/Unicode15.0.0/ch04.pdf

    // QLocale switched return type of many functions from QChar to QString,
    // because UTF-16 may need surrogate pairs to represent these fields.
    // We only need digits, thus we only need the first QChar with Qt>=6.

    auto zero = locale.zeroDigit().at(0).unicode();

    for (auto &i : string) {
        if (i.isDigit()) {
            i = QChar(zero + i.digitValue());
        }
    }

    return string;
}

QString KCalcNumberFormatter::groupDigits(const QString &displayString, const uint numDigits)
{
    QString tmpDisplayString;
    const int stringLength = displayString.length();

    for (int i = stringLength; i > 0; i--) {
        if (i % numDigits == 0 && i != stringLength) {
            tmpDisplayString = tmpDisplayString + QLatin1Char(' ');
        }

        tmpDisplayString = tmpDisplayString + displayString[stringLength - i];
    }

    return tmpDisplayString;
}
