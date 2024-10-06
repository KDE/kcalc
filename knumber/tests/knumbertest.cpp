/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2003-2005 Klaus Niederkrueger <kniederk@math.uni-koeln.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knumber.h"
#include <QString>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace
{
const int precision = 12;

QString numtypeToString(KNumber::Type type)
{
    switch (type) {
    case KNumber::Type::TypeError:
        return QStringLiteral("Special");
    case KNumber::Type::TypeInteger:
        return QStringLiteral("Integer");
    case KNumber::Type::TypeFraction:
        return QStringLiteral("Fraction");
    case KNumber::Type::TypeFloat:
        return QStringLiteral("Float");
    default:
        Q_UNREACHABLE();
        return QLatin1String("Unknown:") + QString::number(type);
    }
}

void checkResult(const QString &string, const KNumber &result, const QString &desired_string, KNumber::Type desired)
{
    std::cout << "Testing result of: " << qPrintable(string) << " should give " << qPrintable(desired_string) << " and gives "
              << qPrintable(result.toQString(precision)) << " ...\n";

    std::cout << "The type of the result should be " << qPrintable(numtypeToString(desired)) << " and gives " << qPrintable(numtypeToString(result.type()))
              << " ... ";

    if (result.type() == desired && result.toQString(precision) == desired_string) {
        std::cout << "OK\n";
        return;
    }

    std::cout << "Failed\n";
    exit(1);
}

void checkTruth(const QString &string, bool computation, bool desired_result)
{
    std::cout << "Testing truth of: " << qPrintable(string) << " should be " << desired_result << " and is " << computation << " ... ";

    if (computation == desired_result) {
        std::cout << "OK\n";
        return;
    }

    std::cout << "Failed\n";
    exit(1);
}

void testingFloatPrecision()
{
    KNumber::setDefaultFloatPrecision(100);
    checkResult(QStringLiteral("Precision >= 100: (KNumber(1) + KNumber(\"1e-80\")) - KNumber(1)"),
                (KNumber(1) + KNumber(QStringLiteral("1e-80"))) - KNumber(1),
                QStringLiteral("1e-80"),
                KNumber::TypeFloat);
    checkResult(QStringLiteral("Precision >= 100: (KNumber(1) + KNumber(\"1e-980\")) - KNumber(1)"),
                (KNumber(1) + KNumber(QStringLiteral("1e-980"))) - KNumber(1),
                QStringLiteral("0"),
                KNumber::TypeInteger);

    KNumber::setDefaultFloatPrecision(1000);
    checkResult(QStringLiteral("Precision >= 1000: (KNumber(1) + KNumber(\"1e-980\")) - KNumber(1)"),
                (KNumber(1) + KNumber(QStringLiteral("1e-980"))) - KNumber(1),
                QStringLiteral("1e-980"),
                KNumber::TypeFloat);

    KNumber::setDefaultFloatPrecision(20);
    checkResult(QStringLiteral("Precision >= 20: sin(KNumber(30))"),
                sin(KNumber(30) * (KNumber::Pi() / KNumber(180))),
                QStringLiteral("0.5"),
                KNumber::TypeFloat);
}

void testingOutput()
{
    KNumber::setDefaultFloatOutput(false);
    checkResult(QStringLiteral("Fractional output: KNumber(\"1/4\")"), KNumber(QStringLiteral("1/4")), QStringLiteral("1/4"), KNumber::TypeFraction);

    KNumber::setDefaultFloatOutput(true);
    checkResult(QStringLiteral("Float: KNumber(\"1/4\")"), KNumber(QStringLiteral("1/4")), QStringLiteral("0.25"), KNumber::TypeFraction);

    KNumber::setDefaultFloatOutput(false);
    KNumber::setSplitoffIntegerForFractionOutput(true);
    checkResult(QStringLiteral("Fractional output: KNumber(\"1/4\")"), KNumber(QStringLiteral("1/4")), QStringLiteral("1/4"), KNumber::TypeFraction);
    checkResult(QStringLiteral("Fractional output: KNumber(\"-1/4\")"), KNumber(QStringLiteral("-1/4")), QStringLiteral("-1/4"), KNumber::TypeFraction);
    checkResult(QStringLiteral("Fractional output: KNumber(\"21/4\")"), KNumber(QStringLiteral("21/4")), QStringLiteral("5 1/4"), KNumber::TypeFraction);
    checkResult(QStringLiteral("Fractional output: KNumber(\"-21/4\")"), KNumber(QStringLiteral("-21/4")), QStringLiteral("-5 1/4"), KNumber::TypeFraction);

    KNumber::setSplitoffIntegerForFractionOutput(false);
    checkResult(QStringLiteral("Fractional output: KNumber(\"1/4\")"), KNumber(QStringLiteral("1/4")), QStringLiteral("1/4"), KNumber::TypeFraction);
    checkResult(QStringLiteral("Fractional output: KNumber(\"-1/4\")"), KNumber(QStringLiteral("-1/4")), QStringLiteral("-1/4"), KNumber::TypeFraction);
    checkResult(QStringLiteral("Fractional output: KNumber(\"21/4\")"), KNumber(QStringLiteral("21/4")), QStringLiteral("21/4"), KNumber::TypeFraction);
    checkResult(QStringLiteral("Fractional output: KNumber(\"-21/4\")"), KNumber(QStringLiteral("-21/4")), QStringLiteral("-21/4"), KNumber::TypeFraction);
}

void testingRound()
{
    std::cout << "\n\n";
    std::cout << "Testing rounding numbers to precision=2:\n";
    std::cout << "----------\n";

    checkTruth(QStringLiteral("KNumber(3)     -> 3.00"), KNumber(QStringLiteral("3")).toQString(-1, 2) == QStringLiteral("3.00"), true);
    checkTruth(QStringLiteral("KNumber(3.)    -> 3.00"), KNumber(QStringLiteral("3.")).toQString(-1, 2) == QStringLiteral("3.00"), true);
    checkTruth(QStringLiteral("KNumber(3.0)   -> 3.00"), KNumber(QStringLiteral("3.0")).toQString(-1, 2) == QStringLiteral("3.00"), true);
    checkTruth(QStringLiteral("KNumber(3.00)  -> 3.00"), KNumber(QStringLiteral("3.00")).toQString(-1, 2) == QStringLiteral("3.00"), true);
    checkTruth(QStringLiteral("KNumber(3.02)  -> 3.02"), KNumber(QStringLiteral("3.02")).toQString(-1, 2) == QStringLiteral("3.02"), true);
    checkTruth(QStringLiteral("KNumber(3.09)  -> 3.09"), KNumber(QStringLiteral("3.09")).toQString(-1, 2) == QStringLiteral("3.09"), true);
    checkTruth(QStringLiteral("KNumber(3.001) -> 3.00"), KNumber(QStringLiteral("3.001")).toQString(-1, 2) == QStringLiteral("3.00"), true);
    checkTruth(QStringLiteral("KNumber(3.004) -> 3.00"), KNumber(QStringLiteral("3.004")).toQString(-1, 2) == QStringLiteral("3.00"), true);
    checkTruth(QStringLiteral("KNumber(3.005) -> 3.01"), KNumber(QStringLiteral("3.005")).toQString(-1, 2) == QStringLiteral("3.01"), true);
    checkTruth(QStringLiteral("KNumber(3.006) -> 3.01"), KNumber(QStringLiteral("3.006")).toQString(-1, 2) == QStringLiteral("3.01"), true);
    checkTruth(QStringLiteral("KNumber(3.091) -> 3.09"), KNumber(QStringLiteral("3.091")).toQString(-1, 2) == QStringLiteral("3.09"), true);
    checkTruth(QStringLiteral("KNumber(3.099) -> 3.10"), KNumber(QStringLiteral("3.099")).toQString(-1, 2) == QStringLiteral("3.10"), true);
    checkTruth(QStringLiteral("KNumber(3.999) -> 4.00"), KNumber(QStringLiteral("3.999")).toQString(-1, 2) == QStringLiteral("4.00"), true);
    checkTruth(QStringLiteral("KNumber(3.9099)-> 3.91"), KNumber(QStringLiteral("3.9099")).toQString(-1, 2) == QStringLiteral("3.91"), true);
    checkTruth(QStringLiteral("KNumber(9.999) -> 10.00"), KNumber(QStringLiteral("9.999")).toQString(-1, 2) == QStringLiteral("10.00"), true);

    std::cout << "\n\n";
    std::cout << "Testing rounding numbers to precision=2 and localized decimal separator:\n";
    std::cout << "----------\n";
    KNumber::setDecimalSeparator(QStringLiteral(","));
    checkTruth(QStringLiteral("KNumber(3)     -> 3,00"), KNumber(QStringLiteral("3")).toQString(-1, 2) == QStringLiteral("3,00"), true);
    checkTruth(QStringLiteral("KNumber(3,)    -> 3,00"), KNumber(QStringLiteral("3,")).toQString(-1, 2) == QStringLiteral("3,00"), true);
    checkTruth(QStringLiteral("KNumber(3,0)   -> 3,00"), KNumber(QStringLiteral("3,0")).toQString(-1, 2) == QStringLiteral("3,00"), true);
    checkTruth(QStringLiteral("KNumber(3,00)  -> 3,00"), KNumber(QStringLiteral("3,00")).toQString(-1, 2) == QStringLiteral("3,00"), true);
    checkTruth(QStringLiteral("KNumber(3,02)  -> 3,02"), KNumber(QStringLiteral("3,02")).toQString(-1, 2) == QStringLiteral("3,02"), true);
    checkTruth(QStringLiteral("KNumber(3,09)  -> 3,09"), KNumber(QStringLiteral("3,09")).toQString(-1, 2) == QStringLiteral("3,09"), true);
    checkTruth(QStringLiteral("KNumber(3,001) -> 3,00"), KNumber(QStringLiteral("3,001")).toQString(-1, 2) == QStringLiteral("3,00"), true);
    checkTruth(QStringLiteral("KNumber(3,005) -> 3,01"), KNumber(QStringLiteral("3,005")).toQString(-1, 2) == QStringLiteral("3,01"), true);
    checkTruth(QStringLiteral("KNumber(3,091) -> 3,09"), KNumber(QStringLiteral("3,091")).toQString(-1, 2) == QStringLiteral("3,09"), true);
    checkTruth(QStringLiteral("KNumber(3,099) -> 3,10"), KNumber(QStringLiteral("3,099")).toQString(-1, 2) == QStringLiteral("3,10"), true);
    checkTruth(QStringLiteral("KNumber(3,999) -> 4,00"), KNumber(QStringLiteral("3,999")).toQString(-1, 2) == QStringLiteral("4,00"), true);
    checkTruth(QStringLiteral("KNumber(3,9099)-> 3,91"), KNumber(QStringLiteral("3,9099")).toQString(-1, 2) == QStringLiteral("3,91"), true);
    checkTruth(QStringLiteral("KNumber(9,999) -> 10,00"), KNumber(QStringLiteral("9,999")).toQString(-1, 2) == QStringLiteral("10,00"), true);
    KNumber::setDecimalSeparator(QStringLiteral("."));
}
}

int main()
{
    testingFloatPrecision();
    testingOutput();
    testingRound();
    std::cout << "SUCCESS" << std::endl;
}
