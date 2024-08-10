/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2003-2005 Klaus Niederkrueger <kniederk@math.uni-koeln.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knumber.h"

#include <QString>
#include <QTest>

class KNumberMiscTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void init();
    void cleanup();

    void testKNumberConstructors_data();
    void testKNumberConstructors();

    void testKNumberConstructorsAsFraction_data();
    void testKNumberConstructorsAsFraction();

    void testKNumberConstructorsDecimalSeparator_data();
    void testKNumberConstructorsDecimalSeparator();

    void testKNumberCompare_data();
    void testKNumberCompare();

    void testKNumberFloatPrecision_data();
    void testKNumberFloatPrecision();

    void testKNumberFloatPrecisionFuctions();

    void testKNumberFloatOutput_data();
    void testKNumberFloatOutput();

    void testKNumberSplitoffIntegerForFractionOutput_data();
    void testKNumberSplitoffIntegerForFractionOutput();

    void testKNumberRound_data();
    void testKNumberRound();

    void cleanupTestCase();

private:
    static const int precision = 12;
};

void KNumberMiscTest::initTestCase()
{
}

void KNumberMiscTest::init()
{
}

void KNumberMiscTest::cleanup()
{
    KNumber::setDefaultFractionalInput(false);
    KNumber::setDecimalSeparator(QStringLiteral("."));
    KNumber::setDefaultFloatOutput(false);
    KNumber::setSplitoffIntegerForFractionOutput(false);
    KNumber::setDefaultFloatPrecision(20);
}

void KNumberMiscTest::testKNumberConstructors_data()
{
    QTest::addColumn<KNumber>("result");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(5)") << KNumber(5) << QStringLiteral("5") << KNumber::TypeInteger;
    QTest::newRow("KNumber(5.3)") << KNumber(5.3) << QStringLiteral("5.3") << KNumber::TypeFloat;
    QTest::newRow("KNumber(0.0)") << KNumber(0.0) << QStringLiteral("0") << KNumber::TypeInteger;

    QTest::newRow("KNumber(\"5\")") << KNumber(QStringLiteral("5")) << QStringLiteral("5") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"5/3\")") << KNumber(QStringLiteral("5/3")) << QStringLiteral("5/3") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"5/1\")") << KNumber(QStringLiteral("5/1")) << QStringLiteral("5") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"0/12\")") << KNumber(QStringLiteral("0/12")) << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"5.3\")") << KNumber(QStringLiteral("5.3")) << QStringLiteral("5.3") << KNumber::TypeFloat;

    QTest::newRow("KNumber(\"i\")") << KNumber(QStringLiteral("i")) << QStringLiteral("i") << KNumber::TypeComplex;
    QTest::newRow("KNumber(\"2+i\")") << KNumber(QStringLiteral("2+i")) << QStringLiteral("2+i") << KNumber::TypeComplex;
    QTest::newRow("KNumber(\"1+2i\")") << KNumber(QStringLiteral("1+2i")) << QStringLiteral("1+2i") << KNumber::TypeComplex;
    QTest::newRow("KNumber(\"1-2i\")") << KNumber(QStringLiteral("1-2i")) << QStringLiteral("1-2i") << KNumber::TypeComplex;
    QTest::newRow("KNumber(\"1.4e20+1.6e-30i\")") << KNumber(QStringLiteral("1.4e20+1.6e-30i")) << QStringLiteral("1.4e+20+1.6e-30i") << KNumber::TypeComplex;
    QTest::newRow("KNumber(\"1.88e-3+i\")") << KNumber(QStringLiteral("1.88e-3+i")) << QStringLiteral("0.00188+i") << KNumber::TypeComplex;
    QTest::newRow("KNumber(\"-1.88e-3-i\")") << KNumber(QStringLiteral("-1.88e-3-i")) << QStringLiteral("-0.00188-i") << KNumber::TypeComplex;
    QTest::newRow("KNumber(\"1+1.88e-3i\")") << KNumber(QStringLiteral("1+1.88e-3i")) << QStringLiteral("1+0.00188i") << KNumber::TypeComplex;
    QTest::newRow("KNumber(\"1-1.88e-3i\")") << KNumber(QStringLiteral("1-1.88e-3i")) << QStringLiteral("1-0.00188i") << KNumber::TypeComplex;
    QTest::newRow("KNumber(\"-i\")") << KNumber(QStringLiteral("-i")) << QStringLiteral("-i") << KNumber::TypeComplex;
    QTest::newRow("KNumber(\".5i\")") << KNumber(QStringLiteral(".5i")) << QStringLiteral("0.5i") << KNumber::TypeComplex;
    QTest::newRow("KNumber(\".5e20i\")") << KNumber(QStringLiteral(".5e20i")) << QStringLiteral("5e+19i") << KNumber::TypeComplex;
    QTest::newRow("KNumber(\".7e20+.5e10i\")") << KNumber(QStringLiteral(".7e20+.5e10i")) << QStringLiteral("7e+19+5000000000i") << KNumber::TypeComplex;
    QTest::newRow("KNumber(\"7.7e20+.5e10i\")") << KNumber(QStringLiteral("7.7e20+.5e10i")) << QStringLiteral("7.7e+20+5000000000i") << KNumber::TypeComplex;
    QTest::newRow("KNumber(\"+1+i\")") << KNumber(QStringLiteral("+1+i")) << QStringLiteral("1+i") << KNumber::TypeComplex;
    QTest::newRow("KNumber(\"-3.55+i\")") << KNumber(QStringLiteral("-3.55+i")) << QStringLiteral("-3.55+i") << KNumber::TypeComplex;
    QTest::newRow("KNumber(\"+3.55+4.66e20i\")") << KNumber(QStringLiteral("+3.55+4.66e20i")) << QStringLiteral("3.55+4.66e+20i") << KNumber::TypeComplex;

    QTest::newRow("KNumber(\"+1.2\")") << KNumber(QStringLiteral("+1.2")) << QStringLiteral("1.2") << KNumber::TypeFloat;
    QTest::newRow("KNumber(\"-1.2\")") << KNumber(QStringLiteral("-1.2")) << QStringLiteral("-1.2") << KNumber::TypeFloat;
    QTest::newRow("KNumber(\"-1.3e5\")") << KNumber(QStringLiteral("-1.3e5")) << QStringLiteral("-130000") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"-1.3e-5\")") << KNumber(QStringLiteral("-1.3e-5")) << QStringLiteral("-1.3e-05") << KNumber::TypeFloat;
    QTest::newRow("KNumber(\"1.3e555\")") << KNumber(QStringLiteral("1.3e555")) << QStringLiteral("1.3e+555") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"+1.3e5\")") << KNumber(QStringLiteral("+1.3e5")) << QStringLiteral("130000") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"1.3e5423\")") << KNumber(QStringLiteral("1.3e5423")) << QStringLiteral("1.3e+5423") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"+1.3e65\")") << KNumber(QStringLiteral("+1.3e65")) << QStringLiteral("1.3e+65") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"+1.3e+55\")") << KNumber(QStringLiteral("+1.3e+55")) << QStringLiteral("1.3e+55") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\".5e-100\")") << KNumber(QStringLiteral(".5e-100")) << QStringLiteral("5e-101") << KNumber::TypeFloat;
    QTest::newRow("KNumber(\"+.5e-100\")") << KNumber(QStringLiteral("+.5e-100")) << QStringLiteral("5e-101") << KNumber::TypeFloat;
    QTest::newRow("KNumber(\"-.5e-100\")") << KNumber(QStringLiteral("-.5e-100")) << QStringLiteral("-5e-101") << KNumber::TypeFloat;

    QTest::newRow("KNumber(\"nan\")") << KNumber(QStringLiteral("nan")) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(\"inf\")") << KNumber(QStringLiteral("inf")) << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-inf\")") << KNumber(QStringLiteral("-inf")) << QStringLiteral("-inf") << KNumber::TypeError;

    QTest::newRow("KNumber(\"0b1110001111\")") << KNumber(QStringLiteral("0b1110001111")) << QStringLiteral("911") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"0b00010101010111\")") << KNumber(QStringLiteral("0b00010101010111")) << QStringLiteral("1367") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"0b11\")") << KNumber(QStringLiteral("0b11")) << QStringLiteral("3") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"0b00000000000000001\")") << KNumber(QStringLiteral("0b00000000000000001")) << QStringLiteral("1") << KNumber::TypeInteger;

    QTest::newRow("KNumber(\"077\")") << KNumber(QStringLiteral("077")) << QStringLiteral("63") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"0077\")") << KNumber(QStringLiteral("0077")) << QStringLiteral("63") << KNumber::TypeInteger;

    QTest::newRow("KNumber(\"0xFF\")") << KNumber(QStringLiteral("0xFF")) << QStringLiteral("255") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"0xFf\")") << KNumber(QStringLiteral("0xFf")) << QStringLiteral("255") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"0x0A\")") << KNumber(QStringLiteral("0x0A")) << QStringLiteral("10") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"0x0a\")") << KNumber(QStringLiteral("0x0a")) << QStringLiteral("10") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"0x0000a\")") << KNumber(QStringLiteral("0x0000a")) << QStringLiteral("10") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"0xabcdef\")") << KNumber(QStringLiteral("0xabcdef")) << QStringLiteral("11259375") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"0xAbCdEf\")") << KNumber(QStringLiteral("0xAbCdEf")) << QStringLiteral("11259375") << KNumber::TypeInteger;
}

void KNumberMiscTest::testKNumberConstructors()
{
    QFETCH(KNumber, result);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberMiscTest::testKNumberConstructorsAsFraction_data()
{
    KNumber::setDefaultFractionalInput(true);

    QTest::addColumn<KNumber>("result");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(\"5\")") << KNumber(QStringLiteral("5")) << QStringLiteral("5") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"1.2\")") << KNumber(QStringLiteral("1.2")) << QStringLiteral("6/5") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"-0.02\")") << KNumber(QStringLiteral("-0.02")) << QStringLiteral("-1/50") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"5e-2\")") << KNumber(QStringLiteral("5e-2")) << QStringLiteral("1/20") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"1.2e3\")") << KNumber(QStringLiteral("1.2e3")) << QStringLiteral("1200") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"0.02e+1\")") << KNumber(QStringLiteral("0.02e+1")) << QStringLiteral("1/5") << KNumber::TypeFraction;
}

void KNumberMiscTest::testKNumberConstructorsAsFraction()
{
    QFETCH(KNumber, result);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberMiscTest::testKNumberConstructorsDecimalSeparator_data()
{
    KNumber::setDecimalSeparator(QStringLiteral(","));

    QTest::addColumn<KNumber>("result");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(\"5,2\")") << KNumber(QStringLiteral("5,2")) << QStringLiteral("5,2") << KNumber::TypeFloat;
}

void KNumberMiscTest::testKNumberConstructorsDecimalSeparator()
{
    QFETCH(KNumber, result);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberMiscTest::testKNumberCompare_data()
{
    QTest::addColumn<bool>("result");
    QTest::addColumn<bool>("expectedResult");

    QTest::newRow("KNumber(5) == KNumber(2)") << (KNumber(5) == KNumber(2)) << false;
    QTest::newRow("KNumber(5) > KNumber(2)") << (KNumber(5) > KNumber(2)) << true;
    QTest::newRow("KNumber(5) < KNumber(2)") << (KNumber(5) < KNumber(2)) << false;
    QTest::newRow("KNumber(5) < KNumber(0)") << (KNumber(5) < KNumber(0)) << false;
    QTest::newRow("KNumber(-5) < KNumber(0)") << (KNumber(-5) < KNumber(0)) << true;
    QTest::newRow("KNumber(5) >= KNumber(2)") << (KNumber(5) >= KNumber(2)) << true;
    QTest::newRow("KNumber(5) <= KNumber(2)") << (KNumber(5) <= KNumber(2)) << false;
    QTest::newRow("KNumber(5) != KNumber(2)") << (KNumber(5) != KNumber(2)) << true;

    QTest::newRow("KNumber(2) == KNumber(2)") << (KNumber(2) == KNumber(2)) << true;
    QTest::newRow("KNumber(2) > KNumber(2)") << (KNumber(2) > KNumber(2)) << false;
    QTest::newRow("KNumber(2) < KNumber(2)") << (KNumber(2) < KNumber(2)) << false;
    QTest::newRow("KNumber(2) >= KNumber(2)") << (KNumber(2) >= KNumber(2)) << true;
    QTest::newRow("KNumber(2) <= KNumber(2)") << (KNumber(2) <= KNumber(2)) << true;
    QTest::newRow("KNumber(2) != KNumber(2)") << (KNumber(2) != KNumber(2)) << false;

    QTest::newRow("KNumber(5) == KNumber(\"1/2\")") << (KNumber(5) == KNumber(QStringLiteral("1/2"))) << false;
    QTest::newRow("KNumber(5) > KNumber(\"1/2\")") << (KNumber(5) > KNumber(QStringLiteral("1/2"))) << true;
    QTest::newRow("KNumber(5) < KNumber(\"1/2\")") << (KNumber(5) < KNumber(QStringLiteral("1/2"))) << false;
    QTest::newRow("KNumber(5) >= KNumber(\"1/2\")") << (KNumber(5) >= KNumber(QStringLiteral("1/2"))) << true;
    QTest::newRow("KNumber(5) <= KNumber(\"1/2\")") << (KNumber(5) <= KNumber(QStringLiteral("1/2"))) << false;
    QTest::newRow("KNumber(5) != KNumber(\"1/2\")") << (KNumber(5) != KNumber(QStringLiteral("1/2"))) << true;

    QTest::newRow("KNumber(\"1/2\") == KNumber(\"1/2\")") << (KNumber(QStringLiteral("1/2")) == KNumber(QStringLiteral("1/2"))) << true;
    QTest::newRow("KNumber(\"1/2\") > KNumber(\"1/2\")") << (KNumber(QStringLiteral("1/2")) > KNumber(QStringLiteral("1/2"))) << false;
    QTest::newRow("KNumber(\"1/2\") < KNumber(\"1/2\")") << (KNumber(QStringLiteral("1/2")) < KNumber(QStringLiteral("1/2"))) << false;
    QTest::newRow("KNumber(\"1/2\") >= KNumber(\"1/2\")") << (KNumber(QStringLiteral("1/2")) >= KNumber(QStringLiteral("1/2"))) << true;
    QTest::newRow("KNumber(\"1/2\") <= KNumber(\"1/2\")") << (KNumber(QStringLiteral("1/2")) <= KNumber(QStringLiteral("1/2"))) << true;
    QTest::newRow("KNumber(\"1/2\") != KNumber(\"1/2\")") << (KNumber(QStringLiteral("1/2")) != KNumber(QStringLiteral("1/2"))) << false;

    QTest::newRow("KNumber(\"3/2\") == KNumber(\"1/2\")") << (KNumber(QStringLiteral("3/2")) == KNumber(QStringLiteral("1/2"))) << false;
    QTest::newRow("KNumber(\"3/2\") > KNumber(\"1/2\")") << (KNumber(QStringLiteral("3/2")) > KNumber(QStringLiteral("1/2"))) << true;
    QTest::newRow("KNumber(\"3/2\") < KNumber(\"1/2\")") << (KNumber(QStringLiteral("3/2")) < KNumber(QStringLiteral("1/2"))) << false;
    QTest::newRow("KNumber(\"3/2\") >= KNumber(\"1/2\")") << (KNumber(QStringLiteral("3/2")) >= KNumber(QStringLiteral("1/2"))) << true;
    QTest::newRow("KNumber(\"3/2\") <= KNumber(\"1/2\")") << (KNumber(QStringLiteral("3/2")) <= KNumber(QStringLiteral("1/2"))) << false;
    QTest::newRow("KNumber(\"3/2\") != KNumber(\"1/2\")") << (KNumber(QStringLiteral("3/2")) != KNumber(QStringLiteral("1/2"))) << true;

    QTest::newRow("KNumber(3.2) != KNumber(3)") << (KNumber(3.2) != KNumber(3)) << true;
    QTest::newRow("KNumber(3.2) > KNumber(3)") << (KNumber(3.2) > KNumber(3)) << true;
    QTest::newRow("KNumber(3.2) < KNumber(3)") << (KNumber(3.2) < KNumber(3)) << false;

    QTest::newRow("KNumber(3.2) < KNumber(\"3/5\")") << (KNumber(3.2) < KNumber(QStringLiteral("3/5"))) << false;

    QTest::newRow("KNumber(\"i\") == KNumber(\"0+i\")") << (KNumber(QStringLiteral("i")) == KNumber(QStringLiteral("0+i"))) << true;
    QTest::newRow("KNumber(\"i\") != KNumber(\"1+i\")") << (KNumber(QStringLiteral("i")) != KNumber(QStringLiteral("1+i"))) << true;
    QTest::newRow("KNumber(\"1\") != KNumber(\"i\")") << (KNumber(QStringLiteral("1")) == KNumber(QStringLiteral("i"))) << false;

    QTest::newRow("KNumber(-inf) == KNumber::NegInfinity") << (KNumber(QStringLiteral("-inf")) == KNumber::NegInfinity) << true;
    QTest::newRow("KNumber(inf) == KNumber::PosInfinity") << (KNumber(QStringLiteral("inf")) == KNumber::PosInfinity) << true;
    QTest::newRow("KNumber(inf) == KNumber(inf)") << (KNumber(QStringLiteral("inf")) == KNumber(QStringLiteral("inf"))) << true;
    QTest::newRow("KNumber(-inf) == KNumber(-inf)") << (KNumber(QStringLiteral("-inf")) == KNumber(QStringLiteral("-inf"))) << true;
    QTest::newRow("KNumber(inf) > KNumber(5)") << (KNumber(QStringLiteral("inf")) > KNumber(5)) << true;

    QTest::newRow("KNumber(-inf) < KNumber(5)") << (KNumber(QStringLiteral("-inf")) < KNumber(5)) << true;
    QTest::newRow("KNumber(nan) != KNumber(inf)") << (KNumber(QStringLiteral("nan")) != KNumber(QStringLiteral("inf"))) << true;
    QTest::newRow("KNumber(-inf) < KNumber(inf)") << (KNumber(QStringLiteral("-inf")) < KNumber(QStringLiteral("inf"))) << true;
    QTest::newRow("KNumber(nan) != KNumber(-inf)") << (KNumber(QStringLiteral("nan")) != KNumber(QStringLiteral("-inf"))) << true;
    QTest::newRow("KNumber(inf) != KNumber(-inf)") << (KNumber(QStringLiteral("inf")) != KNumber(QStringLiteral("-inf"))) << true;
    QTest::newRow("KNumber(-inf) > KNumber(inf)") << (KNumber(QStringLiteral("-inf")) > KNumber(QStringLiteral("inf"))) << false;
    QTest::newRow("KNumber(inf) > KNumber(-inf)") << (KNumber(QStringLiteral("inf")) > KNumber(QStringLiteral("-inf"))) << true;
}

void KNumberMiscTest::testKNumberCompare()
{
    QFETCH(bool, result);
    QFETCH(bool, expectedResult);

    QCOMPARE(result, expectedResult);
}

void KNumberMiscTest::testKNumberFloatPrecision_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("floatPrecision");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("Precision >= 100: (KNumber(1) + KNumber(\"1e-80\")) - KNumber(1)")
        << QStringLiteral("1e-80") << 100 << QStringLiteral("1e-80") << KNumber::TypeFloat;
    QTest::newRow("Precision >= 100: (KNumber(1) + KNumber(\"1e-980\")) - KNumber(1)")
        << QStringLiteral("1e-980") << 100 << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("Precision >= 1000: (KNumber(1) + KNumber(\"1e-980\")) - KNumber(1)")
        << QStringLiteral("1e-980") << 1000 << QStringLiteral("1e-980") << KNumber::TypeFloat;
}

void KNumberMiscTest::testKNumberFloatPrecision()
{
    QFETCH(QString, input);
    QFETCH(int, floatPrecision);
    QFETCH(QString, expectedResultToQString);
    QFETCH(KNumber::Type, expectedResultType);

    KNumber::setDefaultFloatPrecision(floatPrecision);

    KNumber result = KNumber(1) + KNumber(input) - KNumber(1);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberMiscTest::testKNumberFloatPrecisionFuctions()
{
    KNumber::setDefaultFloatPrecision(20);
    KNumber result = sin(KNumber(30) * (KNumber::Pi() / KNumber(180)));
    QCOMPARE(result.type(), KNumber::TypeFloat);
    QCOMPARE(result.toQString(precision), QStringLiteral("0.5"));
}

void KNumberMiscTest::testKNumberFloatOutput_data()
{
    QTest::addColumn<bool>("setDefaultFloatOutput");
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("Fractional output: KNumber(\"1/4\")") << false << QStringLiteral("1/4") << QStringLiteral("1/4") << KNumber::TypeFraction;
    QTest::newRow("Float: KNumber(\"1/4\")") << true << QStringLiteral("1/4") << QStringLiteral("0.25") << KNumber::TypeFraction;
}

void KNumberMiscTest::testKNumberFloatOutput()
{
    QFETCH(bool, setDefaultFloatOutput);
    QFETCH(QString, input);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber::setDefaultFloatOutput(setDefaultFloatOutput);
    KNumber result = KNumber(input);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberMiscTest::testKNumberSplitoffIntegerForFractionOutput_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<bool>("splitInt");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("Fractional output: KNumber(\"1/4\")") << QStringLiteral("1/4") << false << QStringLiteral("1/4") << KNumber::TypeFraction;
    QTest::newRow("Fractional output: KNumber(\"-1/4\")") << QStringLiteral("-1/4") << false << QStringLiteral("-1/4") << KNumber::TypeFraction;
    QTest::newRow("Fractional output: KNumber(\"21/4\")") << QStringLiteral("21/4") << false << QStringLiteral("21/4") << KNumber::TypeFraction;
    QTest::newRow("Fractional output: KNumber(\"-21/4\")") << QStringLiteral("-21/4") << false << QStringLiteral("-21/4") << KNumber::TypeFraction;

    QTest::newRow("Fractional output: KNumber(\"1/4\")") << QStringLiteral("1/4") << true << QStringLiteral("1/4") << KNumber::TypeFraction;
    QTest::newRow("Fractional output: KNumber(\"-1/4\")") << QStringLiteral("-1/4") << true << QStringLiteral("-1/4") << KNumber::TypeFraction;
    QTest::newRow("Fractional output: KNumber(\"21/4\")") << QStringLiteral("21/4") << true << QStringLiteral("5 1/4") << KNumber::TypeFraction;
    QTest::newRow("Fractional output: KNumber(\"-21/4\")") << QStringLiteral("-21/4") << true << QStringLiteral("-5 1/4") << KNumber::TypeFraction;
}

void KNumberMiscTest::testKNumberSplitoffIntegerForFractionOutput()
{
    QFETCH(QString, input);
    QFETCH(bool, splitInt);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber::setSplitoffIntegerForFractionOutput(splitInt);

    KNumber result = KNumber(input);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberMiscTest::testKNumberRound_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("decimalSeparator");
    QTest::addColumn<int>("numberOfDecimals");
    QTest::addColumn<QString>("expectedResultToQString");

    QTest::newRow("KNumber(3)     -> 3.00") << QStringLiteral("3") << QStringLiteral(".") << 2 << QStringLiteral("3.00");
    QTest::newRow("KNumber(3.)    -> 3.00") << QStringLiteral("3.") << QStringLiteral(".") << 2 << QStringLiteral("3.00");
    QTest::newRow("KNumber(3.0)   -> 3.00") << QStringLiteral("3.0") << QStringLiteral(".") << 2 << QStringLiteral("3.00");
    QTest::newRow("KNumber(3.00)  -> 3.00") << QStringLiteral("3.00") << QStringLiteral(".") << 2 << QStringLiteral("3.00");
    QTest::newRow("KNumber(3.02)  -> 3.02") << QStringLiteral("3.02") << QStringLiteral(".") << 2 << QStringLiteral("3.02");
    QTest::newRow("KNumber(3.09)  -> 3.09") << QStringLiteral("3.09") << QStringLiteral(".") << 2 << QStringLiteral("3.09");
    QTest::newRow("KNumber(3.001) -> 3.00") << QStringLiteral("3.001") << QStringLiteral(".") << 2 << QStringLiteral("3.00");
    QTest::newRow("KNumber(3.004) -> 3.00") << QStringLiteral("3.004") << QStringLiteral(".") << 2 << QStringLiteral("3.00");
    QTest::newRow("KNumber(3.005) -> 3.01") << QStringLiteral("3.005") << QStringLiteral(".") << 2 << QStringLiteral("3.01");
    QTest::newRow("KNumber(3.006) -> 3.01") << QStringLiteral("3.006") << QStringLiteral(".") << 2 << QStringLiteral("3.01");
    QTest::newRow("KNumber(3.091) -> 3.09") << QStringLiteral("3.091") << QStringLiteral(".") << 2 << QStringLiteral("3.09");
    QTest::newRow("KNumber(3.099) -> 3.10") << QStringLiteral("3.099") << QStringLiteral(".") << 2 << QStringLiteral("3.10");
    QTest::newRow("KNumber(3.999) -> 4.00") << QStringLiteral("3.999") << QStringLiteral(".") << 2 << QStringLiteral("4.00");
    QTest::newRow("KNumber(3.9099)-> 3.91") << QStringLiteral("3.9099") << QStringLiteral(".") << 2 << QStringLiteral("3.91");
    QTest::newRow("KNumber(9.999) -> 10.00") << QStringLiteral("9.999") << QStringLiteral(".") << 2 << QStringLiteral("10.00");

    QTest::newRow("KNumber(3)     -> 3,00") << QStringLiteral("3") << QStringLiteral(",") << 2 << QStringLiteral("3,00");
    QTest::newRow("KNumber(3,)    -> 3,00") << QStringLiteral("3,") << QStringLiteral(",") << 2 << QStringLiteral("3,00");
    QTest::newRow("KNumber(3,0)   -> 3,00") << QStringLiteral("3,0") << QStringLiteral(",") << 2 << QStringLiteral("3,00");
    QTest::newRow("KNumber(3,00)  -> 3,00") << QStringLiteral("3,00") << QStringLiteral(",") << 2 << QStringLiteral("3,00");
    QTest::newRow("KNumber(3,02)  -> 3,02") << QStringLiteral("3,02") << QStringLiteral(",") << 2 << QStringLiteral("3,02");
    QTest::newRow("KNumber(3,09)  -> 3,09") << QStringLiteral("3,09") << QStringLiteral(",") << 2 << QStringLiteral("3,09");
    QTest::newRow("KNumber(3,001) -> 3,00") << QStringLiteral("3,001") << QStringLiteral(",") << 2 << QStringLiteral("3,00");
    QTest::newRow("KNumber(3,005) -> 3,01") << QStringLiteral("3,005") << QStringLiteral(",") << 2 << QStringLiteral("3,01");
    QTest::newRow("KNumber(3,091) -> 3,09") << QStringLiteral("3,091") << QStringLiteral(",") << 2 << QStringLiteral("3,09");
    QTest::newRow("KNumber(3,099) -> 3,10") << QStringLiteral("3,099") << QStringLiteral(",") << 2 << QStringLiteral("3,10");
    QTest::newRow("KNumber(3,999) -> 4,00") << QStringLiteral("3,999") << QStringLiteral(",") << 2 << QStringLiteral("4,00");
    QTest::newRow("KNumber(3,9099)-> 3,91") << QStringLiteral("3,9099") << QStringLiteral(",") << 2 << QStringLiteral("3,91");
    QTest::newRow("KNumber(9,999) -> 10,00") << QStringLiteral("9,999") << QStringLiteral(",") << 2 << QStringLiteral("10,00");
}

void KNumberMiscTest::testKNumberRound()
{
    QFETCH(QString, input);
    QFETCH(QString, decimalSeparator);
    QFETCH(int, numberOfDecimals);
    QFETCH(QString, expectedResultToQString);

    KNumber::setDecimalSeparator(decimalSeparator);
    KNumber result = KNumber(input);

    QCOMPARE(result.toQString(-1, numberOfDecimals), expectedResultToQString);
}

void KNumberMiscTest::cleanupTestCase()
{
    KNumber::setDefaultFractionalInput(false);
}

QTEST_GUILESS_MAIN(KNumberMiscTest)

#include "knumber_misc_test.moc"
