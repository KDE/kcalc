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

    void testKNumberShift_data();
    void testKNumberShift();

    void testKNumberAnd_data();
    void testKNumberAnd();

    void testKNumberOr_data();
    void testKNumberOr();

    void cleanupTestCase();

private:
    static const int precision = 12;
};

void KNumberBitWiseOperatorsTest::initTestCase()
{
}

void KNumberBitWiseOperatorsTest::testKNumberShift_data()
{
    QTest::addColumn<KNumber>("result");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(16) << KNumber(2)") << (KNumber(16) << KNumber(2)) << QStringLiteral("64") << KNumber::TypeInteger;
    QTest::newRow("KNumber(16) >> KNumber(2)") << (KNumber(16) >> KNumber(2)) << QStringLiteral("4") << KNumber::TypeInteger;
}

void KNumberBitWiseOperatorsTest::testKNumberShift()
{
    QFETCH(KNumber, result);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberBitWiseOperatorsTest::testKNumberAnd_data()
{
    QTest::addColumn<KNumber>("result");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(17) & KNumber(9)") << (KNumber(17) & KNumber(9)) << QStringLiteral("1") << KNumber::TypeInteger;
    QTest::newRow("KNumber(1023) & KNumber(255)") << (KNumber(1023) & KNumber(255)) << QStringLiteral("255") << KNumber::TypeInteger;
}

void KNumberBitWiseOperatorsTest::testKNumberAnd()
{
    QFETCH(KNumber, result);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberBitWiseOperatorsTest::testKNumberOr_data()
{
    QTest::addColumn<KNumber>("result");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(17) | KNumber(9)") << (KNumber(17) | KNumber(9)) << QStringLiteral("25") << KNumber::TypeInteger;
    QTest::newRow("KNumber(1023) | KNumber(255)") << (KNumber(1023) | KNumber(255)) << QStringLiteral("1023") << KNumber::TypeInteger;
}

void KNumberBitWiseOperatorsTest::testKNumberOr()
{
    QFETCH(KNumber, result);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberBitWiseOperatorsTest::cleanupTestCase()
{
}

QTEST_GUILESS_MAIN(KNumberBitWiseOperatorsTest)

#include "knumber_bitwise_operators_test.moc"
