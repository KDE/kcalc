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

    void testKNumberConstructors_data();
    void testKNumberConstructors();

    void testKNumberConstructorsAsFraction_data();
    void testKNumberConstructorsAsFraction();
    /*
         void testKNumberConstructorsDecimalSeparator_data();
         void testKNumberConstructorsDecimalSeparator();
    */
    void testKNumberCompare_data();
    void testKNumberCompare();
    /*
         void testKNumberFloatPrecision_data();
         void testKNumberFloatPrecision();

         void testKNumberOutput_data();
         void testKNumberOutput();

         void testKNumberRound_data();
         void testKNumberRound();*/

    void cleanupTestCase();

private:
    static const int precision = 12;
    bool fractionalInput = false;
    QString decimalSeparator = QStringLiteral(".");
};

void KNumberMiscTest::initTestCase()
{
    KNumber::setDefaultFractionalInput(fractionalInput);
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
    // KNumber::setDefaultFractionalInput(true);
    fractionalInput = true;

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
/*
void KNumberMiscTest::testKNumberConstructorsDecimalSeparator_data()
{
    QTest::addColumn<KNumber>("result");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(\"5,2\")") << KNumber(QStringLiteral("5,2")) << QStringLiteral("5,2") << KNumber::TypeFloat;
}

void KNumberMiscTest::testKNumberConstructorsDecimalSeparator()
{
    KNumber::setDecimalSeparator(QStringLiteral(","));

    QFETCH(KNumber, result);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);

    KNumber::setDecimalSeparator(QStringLiteral("."));
}
*/
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
/*
void KNumberMiscTest::testKNumberFloatPrecision_data()
{
    QTest::addColumn<KNumber>("result");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");
}

void KNumberMiscTest::testKNumberFloatPrecision()
{
    QFETCH(KNumber, result);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberMiscTest::testKNumberOutput_data()
{
    QTest::addColumn<KNumber>("result");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");
}

void KNumberMiscTest::testKNumberOutput()
{
    QFETCH(KNumber, result);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberMiscTest::testKNumberRound_data()
{
    QTest::addColumn<KNumber>("result");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");
}

void KNumberMiscTest::testKNumberRound()
{
    QFETCH(KNumber, result);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}
*/
void KNumberMiscTest::cleanupTestCase()
{
    fractionalInput = false;
    KNumber::setDefaultFractionalInput(false);
}

QTEST_GUILESS_MAIN(KNumberMiscTest)

#include "knumber_misc_test.moc"
