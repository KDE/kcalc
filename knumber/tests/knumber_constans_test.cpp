/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2003-2005 Klaus Niederkrueger <kniederk@math.uni-koeln.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knumber.h"

#include <QString>
#include <QTest>

class KNumberConstansTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testKNumberConstans_data();
    void testKNumberConstans();

    void cleanupTestCase();

private:
    static const int precision = 12;
};

void KNumberConstansTest::initTestCase()
{
}

void KNumberConstansTest::testKNumberConstans_data()
{
    QTest::addColumn<KNumber>("constant");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber::Zero") << KNumber::Zero << KNumber::TypeInteger;
    QTest::newRow("KNumber::One") << KNumber::One << KNumber::TypeInteger;
    QTest::newRow("KNumber::NegOne") << KNumber::NegOne << KNumber::TypeInteger;
    QTest::newRow("KNumber::Pi") << KNumber::Pi() << KNumber::TypeFloat;
    QTest::newRow("KNumber::Euler") << KNumber::Euler() << KNumber::TypeFloat;
    QTest::newRow("KNumber::I") << KNumber::I << KNumber::TypeComplex;
}

void KNumberConstansTest::testKNumberConstans()
{
    QFETCH(KNumber, constant);
    QFETCH(KNumber::Type, expectedResultType);

    QCOMPARE(constant.type(), expectedResultType);
}

void KNumberConstansTest::cleanupTestCase()
{
}

QTEST_GUILESS_MAIN(KNumberConstansTest)

#include "knumber_constans_test.moc"
