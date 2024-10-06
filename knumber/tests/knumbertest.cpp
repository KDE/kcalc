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

void checkType(const QString &string, KNumber::Type test_arg, KNumber::Type desired)
{
    std::cout << "Testing type of: " << qPrintable(string) << " should give " << qPrintable(numtypeToString(desired)) << " and gives "
              << qPrintable(numtypeToString(test_arg)) << " ...";

    if (test_arg == desired) {
        std::cout << "OK\n";
        return;
    }

    std::cout << "Failed\n";
    exit(1);
}

void testingCompare()
{
    std::cout << "\n\n";
    std::cout << "Testing Compare:\n";
    std::cout << "----------------\n";

    checkTruth(QStringLiteral("KNumber(5) == KNumber(2)"), KNumber(5) == KNumber(2), false);
    checkTruth(QStringLiteral("KNumber(5) > KNumber(2)"), KNumber(5) > KNumber(2), true);
    checkTruth(QStringLiteral("KNumber(5) < KNumber(2)"), KNumber(5) < KNumber(2), false);
    checkTruth(QStringLiteral("KNumber(5) < KNumber(0)"), KNumber(5) < KNumber(0), false);
    checkTruth(QStringLiteral("KNumber(-5) < KNumber(0)"), KNumber(-5) < KNumber(0), true);
    checkTruth(QStringLiteral("KNumber(5) >= KNumber(2)"), KNumber(5) >= KNumber(2), true);
    checkTruth(QStringLiteral("KNumber(5) <= KNumber(2)"), KNumber(5) <= KNumber(2), false);
    checkTruth(QStringLiteral("KNumber(5) != KNumber(2)"), KNumber(5) != KNumber(2), true);

    checkTruth(QStringLiteral("KNumber(2) == KNumber(2)"), KNumber(2) == KNumber(2), true);
    checkTruth(QStringLiteral("KNumber(2) > KNumber(2)"), KNumber(2) > KNumber(2), false);
    checkTruth(QStringLiteral("KNumber(2) < KNumber(2)"), KNumber(2) < KNumber(2), false);
    checkTruth(QStringLiteral("KNumber(2) >= KNumber(2)"), KNumber(2) >= KNumber(2), true);
    checkTruth(QStringLiteral("KNumber(2) <= KNumber(2)"), KNumber(2) <= KNumber(2), true);
    checkTruth(QStringLiteral("KNumber(2) != KNumber(2)"), KNumber(2) != KNumber(2), false);

    checkTruth(QStringLiteral("KNumber(5) == KNumber(\"1/2\")"), KNumber(5) == KNumber(QStringLiteral("1/2")), false);
    checkTruth(QStringLiteral("KNumber(5) > KNumber(\"1/2\")"), KNumber(5) > KNumber(QStringLiteral("1/2")), true);
    checkTruth(QStringLiteral("KNumber(5) < KNumber(\"1/2\")"), KNumber(5) < KNumber(QStringLiteral("1/2")), false);
    checkTruth(QStringLiteral("KNumber(5) >= KNumber(\"1/2\")"), KNumber(5) >= KNumber(QStringLiteral("1/2")), true);
    checkTruth(QStringLiteral("KNumber(5) <= KNumber(\"1/2\")"), KNumber(5) <= KNumber(QStringLiteral("1/2")), false);
    checkTruth(QStringLiteral("KNumber(5) != KNumber(\"1/2\")"), KNumber(5) != KNumber(QStringLiteral("1/2")), true);

    checkTruth(QStringLiteral("KNumber(\"1/2\") == KNumber(\"1/2\")"), KNumber(QStringLiteral("1/2")) == KNumber(QStringLiteral("1/2")), true);
    checkTruth(QStringLiteral("KNumber(\"1/2\") > KNumber(\"1/2\")"), KNumber(QStringLiteral("1/2")) > KNumber(QStringLiteral("1/2")), false);
    checkTruth(QStringLiteral("KNumber(\"1/2\") < KNumber(\"1/2\")"), KNumber(QStringLiteral("1/2")) < KNumber(QStringLiteral("1/2")), false);
    checkTruth(QStringLiteral("KNumber(\"1/2\") >= KNumber(\"1/2\")"), KNumber(QStringLiteral("1/2")) >= KNumber(QStringLiteral("1/2")), true);
    checkTruth(QStringLiteral("KNumber(\"1/2\") <= KNumber(\"1/2\")"), KNumber(QStringLiteral("1/2")) <= KNumber(QStringLiteral("1/2")), true);
    checkTruth(QStringLiteral("KNumber(\"1/2\") != KNumber(\"1/2\")"), KNumber(QStringLiteral("1/2")) != KNumber(QStringLiteral("1/2")), false);

    checkTruth(QStringLiteral("KNumber(\"3/2\") == KNumber(\"1/2\")"), KNumber(QStringLiteral("3/2")) == KNumber(QStringLiteral("1/2")), false);
    checkTruth(QStringLiteral("KNumber(\"3/2\") > KNumber(\"1/2\")"), KNumber(QStringLiteral("3/2")) > KNumber(QStringLiteral("1/2")), true);
    checkTruth(QStringLiteral("KNumber(\"3/2\") < KNumber(\"1/2\")"), KNumber(QStringLiteral("3/2")) < KNumber(QStringLiteral("1/2")), false);
    checkTruth(QStringLiteral("KNumber(\"3/2\") >= KNumber(\"1/2\")"), KNumber(QStringLiteral("3/2")) >= KNumber(QStringLiteral("1/2")), true);
    checkTruth(QStringLiteral("KNumber(\"3/2\") <= KNumber(\"1/2\")"), KNumber(QStringLiteral("3/2")) <= KNumber(QStringLiteral("1/2")), false);
    checkTruth(QStringLiteral("KNumber(\"3/2\") != KNumber(\"1/2\")"), KNumber(QStringLiteral("3/2")) != KNumber(QStringLiteral("1/2")), true);

    checkTruth(QStringLiteral("KNumber(3.2) != KNumber(3)"), KNumber(3.2) != KNumber(3), true);
    checkTruth(QStringLiteral("KNumber(3.2) > KNumber(3)"), KNumber(3.2) > KNumber(3), true);
    checkTruth(QStringLiteral("KNumber(3.2) < KNumber(3)"), KNumber(3.2) < KNumber(3), false);

    checkTruth(QStringLiteral("KNumber(3.2) < KNumber(\"3/5\")"), KNumber(3.2) < KNumber(QStringLiteral("3/5")), false);

    checkTruth(QStringLiteral("KNumber(-inf) == KNumber::NegInfinity"), KNumber(QStringLiteral("-inf")) == KNumber::NegInfinity, true);
    checkTruth(QStringLiteral("KNumber(inf) == KNumber::PosInfinity"), KNumber(QStringLiteral("inf")) == KNumber::PosInfinity, true);
    checkTruth(QStringLiteral("KNumber(inf) == KNumber(inf)"), KNumber(QStringLiteral("inf")) == KNumber(QStringLiteral("inf")), true);
    checkTruth(QStringLiteral("KNumber(-inf) == KNumber(-inf)"), KNumber(QStringLiteral("-inf")) == KNumber(QStringLiteral("-inf")), true);
    checkTruth(QStringLiteral("KNumber(inf) > KNumber(5)"), KNumber(QStringLiteral("inf")) > KNumber(5), true);

    checkTruth(QStringLiteral("KNumber(-inf) < KNumber(5)"), KNumber(QStringLiteral("-inf")) < KNumber(5), true);
    checkTruth(QStringLiteral("KNumber(nan) != KNumber(inf)"), KNumber(QStringLiteral("nan")) != KNumber(QStringLiteral("inf")), true);
    checkTruth(QStringLiteral("KNumber(-inf) < KNumber(inf)"), KNumber(QStringLiteral("-inf")) < KNumber(QStringLiteral("inf")), true);
    checkTruth(QStringLiteral("KNumber(nan) != KNumber(-inf)"), KNumber(QStringLiteral("nan")) != KNumber(QStringLiteral("-inf")), true);
    checkTruth(QStringLiteral("KNumber(inf) != KNumber(-inf)"), KNumber(QStringLiteral("inf")) != KNumber(QStringLiteral("-inf")), true);
    checkTruth(QStringLiteral("KNumber(-inf) > KNumber(inf)"), KNumber(QStringLiteral("-inf")) > KNumber(QStringLiteral("inf")), false);
    checkTruth(QStringLiteral("KNumber(inf) > KNumber(-inf)"), KNumber(QStringLiteral("inf")) > KNumber(QStringLiteral("-inf")), true);
}

void testingSpecial()
{
    std::cout << "\n\n";
    std::cout << "Testing special functions:\n";
    std::cout << "--------------------------\n";

    checkResult(QStringLiteral("log10(KNumber(5))"), log10(KNumber(5)), QStringLiteral("0.698970004336"), KNumber::TypeFloat);
    checkResult(QStringLiteral("log10(pow(KNumber(10), KNumber(308)))"), log10(pow(KNumber(10), KNumber(308))), QStringLiteral("308"), KNumber::TypeInteger);

    // TODO: enable this check once MPFR is commonly enabled
    // checkResult(QStringLiteral("log10(pow(KNumber(10), KNumber(309)))"), log10(pow(KNumber(10), KNumber(309))), QLatin1String("309"), KNumber::TypeInteger);

    checkResult(QStringLiteral("exp(KNumber(4.34))"), exp(KNumber(4.34)), QStringLiteral("76.7075393383"), KNumber::TypeFloat);
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

void testingConstructors()
{
    std::cout << "Testing Constructors:\n";
    std::cout << "---------------------\n";

    checkResult(QStringLiteral("KNumber(5)"), KNumber(5), QStringLiteral("5"), KNumber::TypeInteger);
    checkType(QStringLiteral("KNumber(5.3)"), KNumber(5.3).type(), KNumber::TypeFloat);
    checkType(QStringLiteral("KNumber(0.0)"), KNumber(0.0).type(), KNumber::TypeInteger);

    checkResult(QStringLiteral("KNumber(\"5\")"), KNumber(QStringLiteral("5")), QStringLiteral("5"), KNumber::TypeInteger);
    checkResult(QStringLiteral("KNumber(\"5/3\")"), KNumber(QStringLiteral("5/3")), QStringLiteral("5/3"), KNumber::TypeFraction);
    checkResult(QStringLiteral("KNumber(\"5/1\")"), KNumber(QStringLiteral("5/1")), QStringLiteral("5"), KNumber::TypeInteger);
    checkResult(QStringLiteral("KNumber(\"0/12\")"), KNumber(QStringLiteral("0/12")), QStringLiteral("0"), KNumber::TypeInteger);
    KNumber::setDefaultFractionalInput(true);
    std::cout << "Read decimals as fractions:\n";
    checkResult(QStringLiteral("KNumber(\"5\")"), KNumber(QStringLiteral("5")), QStringLiteral("5"), KNumber::TypeInteger);
    checkResult(QStringLiteral("KNumber(\"1.2\")"), KNumber(QStringLiteral("1.2")), QStringLiteral("6/5"), KNumber::TypeFraction);
    checkResult(QStringLiteral("KNumber(\"-0.02\")"), KNumber(QStringLiteral("-0.02")), QStringLiteral("-1/50"), KNumber::TypeFraction);
    checkResult(QStringLiteral("KNumber(\"5e-2\")"), KNumber(QStringLiteral("5e-2")), QStringLiteral("1/20"), KNumber::TypeFraction);
    checkResult(QStringLiteral("KNumber(\"1.2e3\")"), KNumber(QStringLiteral("1.2e3")), QStringLiteral("1200"), KNumber::TypeInteger);
    checkResult(QStringLiteral("KNumber(\"0.02e+1\")"), KNumber(QStringLiteral("0.02e+1")), QStringLiteral("1/5"), KNumber::TypeFraction);

    KNumber::setDefaultFractionalInput(false);
    std::cout << "Read decimals as floats:\n";
    checkResult(QStringLiteral("KNumber(\"5.3\")"), KNumber(QStringLiteral("5.3")), QStringLiteral("5.3"), KNumber::TypeFloat);

    checkResult(QStringLiteral("KNumber(\"nan\")"), KNumber(QStringLiteral("nan")), QStringLiteral("nan"), KNumber::TypeError);
    checkResult(QStringLiteral("KNumber(\"inf\")"), KNumber(QStringLiteral("inf")), QStringLiteral("inf"), KNumber::TypeError);
    checkResult(QStringLiteral("KNumber(\"-inf\")"), KNumber(QStringLiteral("-inf")), QStringLiteral("-inf"), KNumber::TypeError);

    // test accepting of non-US number formatted strings
    KNumber::setDecimalSeparator(QStringLiteral(","));
    checkResult(QStringLiteral("KNumber(\"5,2\")"), KNumber(QStringLiteral("5,2")), QStringLiteral("5,2"), KNumber::TypeFloat);
    KNumber::setDecimalSeparator(QStringLiteral("."));

    checkResult(QStringLiteral("KNumber(\"0b1110001111\")"), KNumber(QStringLiteral("0b1110001111")), QStringLiteral("911"), KNumber::TypeInteger);
    checkResult(QStringLiteral("KNumber(\"0b00010101010111\")"), KNumber(QStringLiteral("0b00010101010111")), QStringLiteral("1367"), KNumber::TypeInteger);
    checkResult(QStringLiteral("KNumber(\"0b11\")"), KNumber(QStringLiteral("0b11")), QStringLiteral("3"), KNumber::TypeInteger);
    checkResult(QStringLiteral("KNumber(\"0b00000000000000001\")"), KNumber(QStringLiteral("0b00000000000000001")), QStringLiteral("1"), KNumber::TypeInteger);

    checkResult(QStringLiteral("KNumber(\"077\")"), KNumber(QStringLiteral("077")), QStringLiteral("63"), KNumber::TypeInteger);
    checkResult(QStringLiteral("KNumber(\"0077\")"), KNumber(QStringLiteral("0077")), QStringLiteral("63"), KNumber::TypeInteger);

    checkResult(QStringLiteral("KNumber(\"0xFF\")"), KNumber(QStringLiteral("0xFF")), QStringLiteral("255"), KNumber::TypeInteger);
    checkResult(QStringLiteral("KNumber(\"0xFf\")"), KNumber(QStringLiteral("0xFf")), QStringLiteral("255"), KNumber::TypeInteger);
    checkResult(QStringLiteral("KNumber(\"0x0A\")"), KNumber(QStringLiteral("0x0A")), QStringLiteral("10"), KNumber::TypeInteger);
    checkResult(QStringLiteral("KNumber(\"0x0a\")"), KNumber(QStringLiteral("0x0a")), QStringLiteral("10"), KNumber::TypeInteger);
    checkResult(QStringLiteral("KNumber(\"0x0000a\")"), KNumber(QStringLiteral("0x0000a")), QStringLiteral("10"), KNumber::TypeInteger);
    checkResult(QStringLiteral("KNumber(\"0xabcdef\")"), KNumber(QStringLiteral("0xabcdef")), QStringLiteral("11259375"), KNumber::TypeInteger);
    checkResult(QStringLiteral("KNumber(\"0xAbCdEf\")"), KNumber(QStringLiteral("0xAbCdEf")), QStringLiteral("11259375"), KNumber::TypeInteger);
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
    testingConstructors();
    testingCompare();
    testingFloatPrecision();
    testingSpecial();
    testingOutput();
    testingRound();
    std::cout << "SUCCESS" << std::endl;
}
