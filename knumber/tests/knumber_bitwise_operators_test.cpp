/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2003-2005 Klaus Niederkrueger <kniederk@math.uni-koeln.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knumber.h"

#include <QString>
#include <QTest>

class KNumberBitWiseOperatorsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testKNumberLeftShift_data();
    void testKNumberLeftShift();

    void testKNumberRightShift_data();
    void testKNumberRightShift();

    void testKNumberAnd_data();
    void testKNumberAnd();

    void testKNumberOr_data();
    void testKNumberOr();

    void testKNumberComplement_data();
    void testKNumberComplement();

    void cleanupTestCase();

private:
    static const int precision = 12;
};

void KNumberBitWiseOperatorsTest::initTestCase()
{
}

void KNumberBitWiseOperatorsTest::testKNumberLeftShift_data()
{
    QTest::addColumn<KNumber>("op1");
    QTest::addColumn<KNumber>("op2");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(16) << KNumber(2)") << KNumber(16) << KNumber(2) << QStringLiteral("64") << KNumber::TypeInteger;
}

void KNumberBitWiseOperatorsTest::testKNumberLeftShift()
{
    QFETCH(KNumber, op1);
    QFETCH(KNumber, op2);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = op1 << op2;

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberBitWiseOperatorsTest::testKNumberRightShift_data()
{
    QTest::addColumn<KNumber>("op1");
    QTest::addColumn<KNumber>("op2");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(16) >> KNumber(2)") << KNumber(16) << KNumber(2) << QStringLiteral("4") << KNumber::TypeInteger;
}

void KNumberBitWiseOperatorsTest::testKNumberRightShift()
{
    QFETCH(KNumber, op1);
    QFETCH(KNumber, op2);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = op1 >> op2;

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberBitWiseOperatorsTest::testKNumberAnd_data()
{
    QTest::addColumn<KNumber>("op1");
    QTest::addColumn<KNumber>("op2");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(17) & KNumber(9)") << KNumber(17) << KNumber(9) << QStringLiteral("1") << KNumber::TypeInteger;
    QTest::newRow("KNumber(1023) & KNumber(255)") << KNumber(1023) << KNumber(255) << QStringLiteral("255") << KNumber::TypeInteger;
}

void KNumberBitWiseOperatorsTest::testKNumberAnd()
{
    QFETCH(KNumber, op1);
    QFETCH(KNumber, op2);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = op1 & op2;

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberBitWiseOperatorsTest::testKNumberOr_data()
{
    QTest::addColumn<KNumber>("op1");
    QTest::addColumn<KNumber>("op2");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(17) | KNumber(9)") << KNumber(17) << KNumber(9) << QStringLiteral("25") << KNumber::TypeInteger;
    QTest::newRow("KNumber(1023) | KNumber(255)") << KNumber(1023) << KNumber(255) << QStringLiteral("1023") << KNumber::TypeInteger;
}

void KNumberBitWiseOperatorsTest::testKNumberOr()
{
    QFETCH(KNumber, op1);
    QFETCH(KNumber, op2);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = op1 | op2;

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberBitWiseOperatorsTest::testKNumberComplement_data()
{
    QTest::addColumn<KNumber>("op");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    // at first glance, these look like they should work
    // but there is an annoyance. If we use the mpz_com function
    // ~-2 == 1, but the HEX/OCT/BIN views are broken :-(
    // specifically, if the value is negative, it goes badly pretty quick.
#if 0
     QTest::newRow("~KNumber(0)") << KNumber(0) << QLatin1String("-1") << KNumber::TypeInteger;
     QTest::newRow("~KNumber(1)") << KNumber(1) << QLatin1String("-2") << KNumber::TypeInteger;
     QTest::newRow("~KNumber(2)") << KNumber(2) << QLatin1String("-3") << KNumber::TypeInteger;
     QTest::newRow("~KNumber(8)") << KNumber(8) << QLatin1String("-9") << KNumber::TypeInteger;
     QTest::newRow("~KNumber(15)") <<KNumber(15) << QLatin1String("-16") << KNumber::TypeInteger;
     QTest::newRow("~KNumber(-1)") <<KNumber(-1) << QLatin1String("0") << KNumber::TypeInteger;
     QTest::newRow("~KNumber(-2)") <<KNumber(-2) << QLatin1String("1") << KNumber::TypeInteger;
     QTest::newRow("~KNumber(-3)") <<KNumber(-3) << QLatin1String("2") << KNumber::TypeInteger;
     QTest::newRow("~KNumber(-9)") <<KNumber(-9) << QLatin1String("8") << KNumber::TypeInteger;
     QTest::newRow("~KNumber(-16)") << KNumber(-16) << QLatin1String("15") << KNumber::TypeInteger;
#endif

    QTest::newRow("~KNumber(0.12345)") << KNumber(0.12345) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("~KNumber(-0.12345)") << KNumber(-0.12345) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("~KNumber(\"1/2\")") << KNumber(QStringLiteral("1/2")) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("~KNumber(\"-1/2\")") << KNumber(QStringLiteral("-1/2")) << QStringLiteral("nan") << KNumber::TypeError;
}

void KNumberBitWiseOperatorsTest::testKNumberComplement()
{
    QFETCH(KNumber, op);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = ~op;

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberBitWiseOperatorsTest::cleanupTestCase()
{
}

QTEST_GUILESS_MAIN(KNumberBitWiseOperatorsTest)

#include "knumber_bitwise_operators_test.moc"
