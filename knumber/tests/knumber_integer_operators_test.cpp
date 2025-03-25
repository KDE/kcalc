/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2003-2005 Klaus Niederkrueger <kniederk@math.uni-koeln.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knumber.h"

#include <QString>
#include <QTest>

class KNumberIntegerOperatorsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testKNumberTruncate_data();
    void testKNumberTruncate();

    void testKNumberModulus_data();
    void testKNumberModulus();

    void testKNumberFactorial_data();
    void testKNumberFactorial();

    void cleanupTestCase();

private:
    static const int precision = 12;
};

void KNumberIntegerOperatorsTest::initTestCase()
{
}

void KNumberIntegerOperatorsTest::testKNumberTruncate_data()
{
    QTest::addColumn<KNumber>("operand");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(16).integerPart()") << KNumber(16) << QStringLiteral("16") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"43/9\").integerPart()") << KNumber(QStringLiteral("43/9")) << QStringLiteral("4") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"-43/9\").integerPart()") << KNumber(QStringLiteral("-43/9")) << QStringLiteral("-4") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"5.25\").integerPart()") << KNumber(QStringLiteral("5.25")) << QStringLiteral("5") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"-5.25\").integerPart()") << KNumber(QStringLiteral("-5.25")) << QStringLiteral("-5") << KNumber::TypeInteger;
}

void KNumberIntegerOperatorsTest::testKNumberTruncate()
{
    QFETCH(KNumber, operand);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = operand.integerPart();

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberIntegerOperatorsTest::testKNumberModulus_data()
{
    QTest::addColumn<KNumber>("operand1");
    QTest::addColumn<KNumber>("operand2");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(23) % KNumber(4)") << KNumber(23) << KNumber(4) << QStringLiteral("3") << KNumber::TypeInteger;
    QTest::newRow("KNumber(12) % KNumber(-5)") << KNumber(12) << KNumber(-5) << QStringLiteral("2") << KNumber::TypeInteger;
    QTest::newRow("KNumber(-12) % KNumber(5)") << KNumber(-12) << KNumber(5) << QStringLiteral("3") << KNumber::TypeInteger;
    QTest::newRow("KNumber(12) % KNumber(0)") << KNumber(-12) << KNumber(0) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-12) % KNumber(0)") << KNumber(-12) << KNumber(0) << QStringLiteral("nan") << KNumber::TypeError;
}

void KNumberIntegerOperatorsTest::testKNumberModulus()
{
    QFETCH(KNumber, operand1);
    QFETCH(KNumber, operand2);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = operand1 % operand2;

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberIntegerOperatorsTest::testKNumberFactorial_data()
{
    QTest::addColumn<KNumber>("operand");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(-1).factorial()") << KNumber(-1) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-2).factorial()") << KNumber(-2) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-20).factorial()") << KNumber(-20) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-1/2).factorial()") << KNumber(QStringLiteral("-1/2")) << QStringLiteral("1.77245385091") << KNumber::TypeFloat;
    QTest::newRow("KNumber(-0.5).factorial()") << KNumber(QStringLiteral("-0.5")) << QStringLiteral("1.77245385091") << KNumber::TypeFloat;

    QTest::newRow("KNumber(0).factorial()") << KNumber(0) << QStringLiteral("1") << KNumber::TypeInteger;
    QTest::newRow("KNumber(1).factorial()") << KNumber(1) << QStringLiteral("1") << KNumber::TypeInteger;
    QTest::newRow("KNumber(2).factorial()") << KNumber(2) << QStringLiteral("2") << KNumber::TypeInteger;
    QTest::newRow("KNumber(3).factorial()") << KNumber(3) << QStringLiteral("6") << KNumber::TypeInteger;
    QTest::newRow("KNumber(4).factorial()") << KNumber(4) << QStringLiteral("24") << KNumber::TypeInteger;
    QTest::newRow("KNumber(5).factorial()") << KNumber(5) << QStringLiteral("120") << KNumber::TypeInteger;
    QTest::newRow("KNumber(6).factorial()") << KNumber(6) << QStringLiteral("720") << KNumber::TypeInteger;
    QTest::newRow("KNumber(9).factorial()") << KNumber(9) << QStringLiteral("362880") << KNumber::TypeInteger;
    QTest::newRow("KNumber(12).factorial()") << KNumber(12) << QStringLiteral("479001600") << KNumber::TypeInteger;
    QTest::newRow("KNumber(13).factorial()") << KNumber(13) << QStringLiteral("6227020800") << KNumber::TypeInteger;

    QTest::newRow("KNumber(1/2).factorial()") << KNumber(QStringLiteral("1/2")) << QStringLiteral("0.886226925453") << KNumber::TypeFloat;
    QTest::newRow("KNumber(2/1).factorial()") << KNumber(QStringLiteral("2/1")) << QStringLiteral("2") << KNumber::TypeInteger;
    QTest::newRow("KNumber(3/2).factorial()") << KNumber(QStringLiteral("3/2")) << QStringLiteral("1.32934038818") << KNumber::TypeFloat;

    QTest::newRow("KNumber(0.1).factorial()") << KNumber(0.1) << QStringLiteral("0.951350769867") << KNumber::TypeFloat;
    QTest::newRow("KNumber(0.5).factorial()") << KNumber(0.5) << QStringLiteral("0.886226925453") << KNumber::TypeFloat;
    QTest::newRow("KNumber(1.5).factorial()") << KNumber(1.5) << QStringLiteral("1.32934038818") << KNumber::TypeFloat;
    QTest::newRow("KNumber(2.5).factorial()") << KNumber(2.5) << QStringLiteral("3.32335097045") << KNumber::TypeFloat;
    QTest::newRow("KNumber(3.5).factorial()") << KNumber(3.5) << QStringLiteral("11.6317283966") << KNumber::TypeFloat;
}

void KNumberIntegerOperatorsTest::testKNumberFactorial()
{
    QFETCH(KNumber, operand);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = operand.factorial();
    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberIntegerOperatorsTest::cleanupTestCase()
{
}

QTEST_GUILESS_MAIN(KNumberIntegerOperatorsTest)

#include "knumber_integer_operators_test.moc"
