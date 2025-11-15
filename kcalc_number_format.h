/*
    SPDX-FileCopyrightText: 2025 Marlon Pohl <admin@neo8.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once
#include <QString>

enum KCalcNumberBase {
    NbBinary = 2,
    NbOctal = 8,
    NbDecimal = 10,
    NbHex = 16,
};

class KCalcNumberFormatter
{
public:
    static QString formatNumber(const QString &string, KCalcNumberBase base);
    static QString formatDecimalNumber(QString string, bool explicitGrouping);
    static QString groupDigits(const QString &displayString, uint numDigits);

private:
    KCalcNumberFormatter() = default;
    ~KCalcNumberFormatter() = default;
};