/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2003-2005 Klaus Niederkrueger <kniederk@math.uni-koeln.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knumber.h"

#include <QString>
#include <QTest>

class KNumberAritmeticTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testKNumberaddition_data();
    void testKNumberaddition();

    void cleanupTestCase();

private:
    static const int precision = 12;
};

void KNumberAritmeticTest::initTestCase()
{
}

void KNumberAritmeticTest::testKNumberaddition_data()
{
    QTest::addColumn<KNumber>("result");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(5) + KNumber(2)") << KNumber(5) + KNumber(2) << QStringLiteral("7") << KNumber::TypeInteger;
    QTest::newRow("KNumber(5) + KNumber(\"2/3\")") << KNumber(5) + KNumber(QStringLiteral("2/3")) << QStringLiteral("17/3") << KNumber::TypeFraction;
    QTest::newRow("KNumber(5) + KNumber(\"2.3\")") << KNumber(5) + KNumber(QStringLiteral("2.3")) << QStringLiteral("7.3") << KNumber::TypeFloat;

    QTest::newRow("KNumber(\"5/3\") + KNumber(2)") << KNumber(QStringLiteral("5/3")) + KNumber(2) << QStringLiteral("11/3") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"5/3\") + KNumber(\"2/3\")")
        << KNumber(QStringLiteral("5/3")) + KNumber(QStringLiteral("2/3")) << QStringLiteral("7/3") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"5/3\") + KNumber(\"1/3\")")
        << KNumber(QStringLiteral("5/3")) + KNumber(QStringLiteral("1/3")) << QStringLiteral("2") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"5/3\") + KNumber(\"-26/3\")")
        << KNumber(QStringLiteral("5/3")) + KNumber(QStringLiteral("-26/3")) << QStringLiteral("-7") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"5/2\") + KNumber(2.3)") << KNumber(QStringLiteral("5/2")) + KNumber(2.3) << QStringLiteral("4.8") << KNumber::TypeFloat;

    QTest::newRow("KNumber(5.3) + KNumber(2)") << KNumber(5.3) + KNumber(2) << QStringLiteral("7.3") << KNumber::TypeFloat;
    QTest::newRow("KNumber(5.3) + KNumber(\"2/4\")") << KNumber(5.3) + KNumber(QStringLiteral("2/4")) << QStringLiteral("5.8") << KNumber::TypeFloat;
    QTest::newRow("KNumber(5.3) + KNumber(2.3)") << KNumber(5.3) + KNumber(2.3) << QStringLiteral("7.6") << KNumber::TypeFloat;
}

void KNumberAritmeticTest::testKNumberaddition()
{
    QFETCH(KNumber, result);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberAritmeticTest::cleanupTestCase()
{
}

QTEST_GUILESS_MAIN(KNumberAritmeticTest)

#include "knumber_aritmetic_test.moc"
