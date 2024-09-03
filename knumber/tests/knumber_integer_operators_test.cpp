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

    void cleanupTestCase();

private:
    static const int precision = 12;
};

void KNumberIntegerOperatorsTest::initTestCase()
{
}

void KNumberIntegerOperatorsTest::testKNumberTruncate_data()
{
    QTest::addColumn<KNumber>("result");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(16).integerPart()") << KNumber(16).integerPart() << QStringLiteral("16") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"43/9\").integerPart()") << KNumber(QStringLiteral("43/9")).integerPart() << QStringLiteral("4") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"-43/9\").integerPart()") << KNumber(QStringLiteral("-43/9")).integerPart() << QStringLiteral("-4") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"5.25\").integerPart()") << KNumber(QStringLiteral("5.25")).integerPart() << QStringLiteral("5") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"-5.25\").integerPart()") << KNumber(QStringLiteral("-5.25")).integerPart() << QStringLiteral("-5") << KNumber::TypeInteger;
}

void KNumberIntegerOperatorsTest::testKNumberTruncate()
{
    QFETCH(KNumber, result);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberIntegerOperatorsTest::testKNumberModulus_data()
{
    QTest::addColumn<KNumber>("result");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(23) % KNumber(4)") << (KNumber(23) % KNumber(4)) << QStringLiteral("3") << KNumber::TypeInteger;
    QTest::newRow("KNumber(12) % KNumber(-5)") << (KNumber(12) % KNumber(-5)) << QStringLiteral("2") << KNumber::TypeInteger;
    QTest::newRow("KNumber(-12) % KNumber(5)") << (KNumber(-12) % KNumber(5)) << QStringLiteral("3") << KNumber::TypeInteger;
    QTest::newRow("KNumber(12) % KNumber(0)") << (KNumber(-12) % KNumber(0)) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-12) % KNumber(0)") << (KNumber(-12) % KNumber(0)) << QStringLiteral("nan") << KNumber::TypeError;
}

void KNumberIntegerOperatorsTest::testKNumberModulus()
{
    QFETCH(KNumber, result);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberIntegerOperatorsTest::cleanupTestCase()
{
}

QTEST_GUILESS_MAIN(KNumberIntegerOperatorsTest)

#include "knumber_integer_operators_test.moc"
