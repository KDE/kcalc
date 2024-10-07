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

    void testKNumberAddition_data();
    void testKNumberAddition();

    void testKNumberSubtraction_data();
    void testKNumberSubtraction();

    void testKNumberMultiplication_data();
    void testKNumberMultiplication();

    void testKNumberDivision_data();
    void testKNumberDivision();

    void cleanupTestCase();

private:
    static const int precision = 12;
};

void KNumberAritmeticTest::initTestCase()
{
}

void KNumberAritmeticTest::testKNumberAddition_data()
{
    QTest::addColumn<KNumber>("op1");
    QTest::addColumn<KNumber>("op2");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(5) + KNumber(2)") << KNumber(5) << KNumber(2) << QStringLiteral("7") << KNumber::TypeInteger;
    QTest::newRow("KNumber(5) + KNumber(\"2/3\")") << KNumber(5) << KNumber(QStringLiteral("2/3")) << QStringLiteral("17/3") << KNumber::TypeFraction;
    QTest::newRow("KNumber(5) + KNumber(\"2.3\")") << KNumber(5) << KNumber(QStringLiteral("2.3")) << QStringLiteral("7.3") << KNumber::TypeFloat;

    QTest::newRow("KNumber(\"5/3\") + KNumber(2)") << KNumber(QStringLiteral("5/3")) << KNumber(2) << QStringLiteral("11/3") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"5/3\") + KNumber(\"2/3\")")
        << KNumber(QStringLiteral("5/3")) << KNumber(QStringLiteral("2/3")) << QStringLiteral("7/3") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"5/3\") + KNumber(\"1/3\")")
        << KNumber(QStringLiteral("5/3")) << KNumber(QStringLiteral("1/3")) << QStringLiteral("2") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"5/3\") + KNumber(\"-26/3\")")
        << KNumber(QStringLiteral("5/3")) << KNumber(QStringLiteral("-26/3")) << QStringLiteral("-7") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"5/2\") + KNumber(2.3)") << KNumber(QStringLiteral("5/2")) << KNumber(2.3) << QStringLiteral("4.8") << KNumber::TypeFloat;

    QTest::newRow("KNumber(5.3) + KNumber(2)") << KNumber(5.3) << KNumber(2) << QStringLiteral("7.3") << KNumber::TypeFloat;
    QTest::newRow("KNumber(5.3) + KNumber(\"2/4\")") << KNumber(5.3) << KNumber(QStringLiteral("2/4")) << QStringLiteral("5.8") << KNumber::TypeFloat;
    QTest::newRow("KNumber(5.3) + KNumber(2.3)") << KNumber(5.3) << KNumber(2.3) << QStringLiteral("7.6") << KNumber::TypeFloat;

    QTest::newRow("inf + KNumber(2)") << KNumber::PosInfinity << KNumber(2) << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(-5) + inf") << KNumber(-5) << KNumber::PosInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("inf + KNumber(\"1/2\")") << KNumber::PosInfinity << KNumber(QStringLiteral("1/2")) << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-5/3\") + inf") << KNumber(QStringLiteral("-5/3")) << KNumber::PosInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("inf + KNumber(2.01)") << KNumber::PosInfinity << KNumber(2.01) << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(-5.4) + inf") << KNumber(-5.4) << KNumber::PosInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("-inf + KNumber(2)") << KNumber::NegInfinity << KNumber(2) << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("KNumber(-5) + -inf") << KNumber(-5) << KNumber::NegInfinity << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("-inf + KNumber(\"1/2\")") << KNumber::NegInfinity << KNumber(QStringLiteral("1/2")) << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-5/3\") + -inf") << KNumber(QStringLiteral("-5/3")) << KNumber::NegInfinity << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("-inf + KNumber(2.01)") << KNumber::NegInfinity << KNumber(2.01) << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("KNumber(-5.4) + -inf") << KNumber(-5.4) << KNumber::NegInfinity << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("nan + KNumber(2)") << KNumber::NaN << KNumber(2) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-5) + nan") << KNumber(-5) << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("nan + KNumber(\"1/2\")") << KNumber::NaN << KNumber(QStringLiteral("1/2")) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-5/3\") + nan") << KNumber(QStringLiteral("-5/3")) << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("nan + KNumber(2.01)") << KNumber::NaN << KNumber(2.01) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-5.4) + nan") << KNumber(-5.4) << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("inf + inf") << KNumber::PosInfinity << KNumber::PosInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("inf + -inf") << KNumber::PosInfinity << KNumber::NegInfinity << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("-inf + inf") << KNumber::NegInfinity << KNumber::PosInfinity << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("-inf + -inf") << KNumber::NegInfinity << KNumber::NegInfinity << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("inf + nan") << KNumber::PosInfinity << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("-inf + nan") << KNumber::NegInfinity << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("nan + inf") << KNumber::NaN << KNumber::PosInfinity << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("-inf + nan") << KNumber::NegInfinity << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
}

void KNumberAritmeticTest::testKNumberAddition()
{
    QFETCH(KNumber, op1);
    QFETCH(KNumber, op2);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = op1 + op2;

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberAritmeticTest::testKNumberSubtraction_data()
{
    QTest::addColumn<KNumber>("op1");
    QTest::addColumn<KNumber>("op2");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(5) - KNumber(2)") << KNumber(5) << KNumber(2) << QStringLiteral("3") << KNumber::TypeInteger;
    QTest::newRow("KNumber(5) - KNumber(\"2/3\")") << KNumber(5) << KNumber(QStringLiteral("2/3")) << QStringLiteral("13/3") << KNumber::TypeFraction;
    QTest::newRow("KNumber(5) - KNumber(2.3)") << KNumber(5) << KNumber(2.3) << QStringLiteral("2.7") << KNumber::TypeFloat;

    QTest::newRow("KNumber(\"5/3\") - KNumber(2)") << KNumber(QStringLiteral("5/3")) << KNumber(2) << QStringLiteral("-1/3") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"5/3\") - KNumber(\"1/3\")")
        << KNumber(QStringLiteral("5/3")) << KNumber(QStringLiteral("1/3")) << QStringLiteral("4/3") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"5/3\") - KNumber(\"2/3\")")
        << KNumber(QStringLiteral("5/3")) << KNumber(QStringLiteral("2/3")) << QStringLiteral("1") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"-5/3\") - KNumber(\"4/3\")")
        << KNumber(QStringLiteral("-5/3")) << KNumber(QStringLiteral("4/3")) << QStringLiteral("-3") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"5/4\") - KNumber(2.2)") << KNumber(QStringLiteral("5/4")) << KNumber(2.2) << QStringLiteral("-0.95") << KNumber::TypeFloat;

    QTest::newRow("KNumber(5.3) - KNumber(2)") << KNumber(5.3) << KNumber(2) << QStringLiteral("3.3") << KNumber::TypeFloat;
    QTest::newRow("KNumber(5.3) - KNumber(\"3/4\")") << KNumber(5.3) << KNumber(QStringLiteral("3/4")) << QStringLiteral("4.55") << KNumber::TypeFloat;
    QTest::newRow("KNumber(5.3) - KNumber(2.3)") << KNumber(5.3) << KNumber(2.3) << QStringLiteral("3") << KNumber::TypeInteger;

    QTest::newRow("inf - KNumber(2)") << KNumber::PosInfinity << KNumber(2) << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(-5) - inf") << KNumber(-5) << KNumber::PosInfinity << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("inf - KNumber(\"1/2\")") << KNumber::PosInfinity << KNumber(QStringLiteral("1/2")) << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-5/3\") - inf") << KNumber(QStringLiteral("-5/3")) << KNumber::PosInfinity << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("inf - KNumber(2.01)") << KNumber::PosInfinity << KNumber(2.01) << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(-5.4) - inf") << KNumber(-5.4) << KNumber::PosInfinity << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("-inf - KNumber(2)") << KNumber::NegInfinity << KNumber(2) << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("KNumber(-5) - -inf") << KNumber(-5) << KNumber::NegInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("-inf - KNumber(\"1/2\")") << KNumber::NegInfinity << KNumber(QStringLiteral("1/2")) << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-5/3\") - -inf") << KNumber(QStringLiteral("-5/3")) << KNumber::NegInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("-inf - KNumber(2.01)") << KNumber::NegInfinity << KNumber(2.01) << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("KNumber(-5.4) - -inf") << KNumber(-5.4) << KNumber::NegInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("nan - KNumber(2)") << KNumber::NaN << KNumber(2) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-5) - nan") << KNumber(-5) << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("nan - KNumber(\"1/2\")") << KNumber::NaN << KNumber(QStringLiteral("1/2")) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-5/3\") - nan") << KNumber(QStringLiteral("-5/3")) << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("nan - KNumber(2.01)") << KNumber::NaN << KNumber(2.01) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-5.4) - nan") << KNumber(-5.4) << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("inf - inf") << KNumber::PosInfinity << KNumber::PosInfinity << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("inf - -inf") << KNumber::PosInfinity << KNumber::NegInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("-inf - inf") << KNumber::NegInfinity << KNumber::PosInfinity << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("-inf - -inf") << KNumber::NegInfinity << KNumber::NegInfinity << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("inf - nan") << KNumber::PosInfinity << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("-inf - nan") << KNumber::NegInfinity << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("nan - inf") << KNumber::NaN << KNumber::PosInfinity << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("-inf - nan") << KNumber::NegInfinity << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
}

void KNumberAritmeticTest::testKNumberSubtraction()
{
    QFETCH(KNumber, op1);
    QFETCH(KNumber, op2);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = op1 - op2;

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberAritmeticTest::testKNumberMultiplication_data()
{
    QTest::addColumn<KNumber>("op1");
    QTest::addColumn<KNumber>("op2");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(5) * KNumber(2)") << KNumber(5) << KNumber(2) << QStringLiteral("10") << KNumber::TypeInteger;
    QTest::newRow("KNumber(5) * KNumber(\"2/3\")") << KNumber(5) << KNumber(QStringLiteral("2/3")) << QStringLiteral("10/3") << KNumber::TypeFraction;
    QTest::newRow("KNumber(5) * KNumber(\"2/5\")") << KNumber(5) << KNumber(QStringLiteral("2/5")) << QStringLiteral("2") << KNumber::TypeInteger;
    QTest::newRow("KNumber(5) * KNumber(2.3)") << KNumber(5) << KNumber(2.3) << QStringLiteral("11.5") << KNumber::TypeFloat;
    QTest::newRow("KNumber(0) * KNumber(\"2/5\")") << KNumber(0) << KNumber(QStringLiteral("2/5")) << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("KNumber(0) * KNumber(2.3)") << KNumber(0) << KNumber(2.3) << QStringLiteral("0") << KNumber::TypeInteger;

    QTest::newRow("KNumber(\"5/3\") * KNumber(2)") << KNumber(QStringLiteral("5/3")) << KNumber(2) << QStringLiteral("10/3") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"5/3\") * KNumber(0)") << KNumber(QStringLiteral("5/3")) << KNumber(0) << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"5/3\") * KNumber(\"2/3\")")
        << KNumber(QStringLiteral("5/3")) << KNumber(QStringLiteral("2/3")) << QStringLiteral("10/9") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"25/6\") * KNumber(\"12/5\")")
        << KNumber(QStringLiteral("25/6")) << KNumber(QStringLiteral("12/5")) << QStringLiteral("10") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"5/2\") * KNumber(2.3)") << KNumber(QStringLiteral("5/2")) << KNumber(2.3) << QStringLiteral("5.75") << KNumber::TypeFloat;

    QTest::newRow("KNumber(5.3) * KNumber(2)") << KNumber(5.3) << KNumber(2) << QStringLiteral("10.6") << KNumber::TypeFloat;
    QTest::newRow("KNumber(5.3) * KNumber(0)") << KNumber(5.3) << KNumber(0) << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("KNumber(5.3) * KNumber(\"1/2\")") << KNumber(5.3) << KNumber(QStringLiteral("1/2")) << QStringLiteral("2.65") << KNumber::TypeFloat;
    QTest::newRow("KNumber(5.3) * KNumber(2.3)") << KNumber(5.3) << KNumber(2.3) << QStringLiteral("12.19") << KNumber::TypeFloat;

    QTest::newRow("inf * KNumber(2)") << KNumber::PosInfinity << KNumber(2) << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(-5) * inf") << KNumber(-5) << KNumber::PosInfinity << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("inf * KNumber(\"1/2\")") << KNumber::PosInfinity << KNumber(QStringLiteral("1/2")) << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-5/3\") * inf") << KNumber(QStringLiteral("-5/3")) << KNumber::PosInfinity << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("inf * KNumber(2.01)") << KNumber::PosInfinity << KNumber(2.01) << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(-5.4) * inf") << KNumber(-5.4) << KNumber::PosInfinity << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("-inf * KNumber(2)") << KNumber::NegInfinity << KNumber(2) << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("KNumber(-5) * -inf") << KNumber(-5) << KNumber::NegInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("-inf * KNumber(\"1/2\")") << KNumber::NegInfinity << KNumber(QStringLiteral("1/2")) << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-5/3\") * -inf") << KNumber(QStringLiteral("-5/3")) << KNumber::NegInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("-inf * KNumber(2.01)") << KNumber::NegInfinity << KNumber(2.01) << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("KNumber(-5.4) * -inf") << KNumber(-5.4) << KNumber::NegInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("nan * KNumber(2)") << KNumber::NaN << KNumber(2) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-5) * nan") << KNumber(-5) << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("nan * KNumber(\"1/2\")") << KNumber::NaN << KNumber(QStringLiteral("1/2")) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-5/3\") * nan") << KNumber(QStringLiteral("-5/3")) << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("nan * KNumber(2.01)") << KNumber::NaN << KNumber(2.01) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-5.4) * nan") << KNumber(-5.4) << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("inf * inf") << KNumber::PosInfinity << KNumber::PosInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("inf * -inf") << KNumber::PosInfinity << KNumber::NegInfinity << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("-inf * inf") << KNumber::NegInfinity << KNumber::PosInfinity << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("-inf * -inf") << KNumber::NegInfinity << KNumber::NegInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("inf * nan") << KNumber::PosInfinity << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("-inf * nan") << KNumber::NegInfinity << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("nan * inf") << KNumber::NaN << KNumber::PosInfinity << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("-inf * nan") << KNumber::NegInfinity << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(0) * inf") << KNumber(0) << KNumber::PosInfinity << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(0) * -inf") << KNumber(0) << KNumber::NegInfinity << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("inf * KNumber(0)") << KNumber::PosInfinity << KNumber(0) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("-inf * KNumber(0)") << KNumber::NegInfinity << KNumber(0) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(0.0) * inf") << KNumber(0.0) << KNumber::PosInfinity << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(0.0) * -inf") << KNumber(0.0) << KNumber::NegInfinity << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("inf * KNumber(0.0)") << KNumber::PosInfinity << KNumber(0.0) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("-inf * KNumber(0.0)") << KNumber::NegInfinity << KNumber(0.0) << QStringLiteral("nan") << KNumber::TypeError;
}

void KNumberAritmeticTest::testKNumberMultiplication()
{
    QFETCH(KNumber, op1);
    QFETCH(KNumber, op2);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = op1 * op2;

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberAritmeticTest::testKNumberDivision_data()
{
    QTest::addColumn<KNumber>("op1");
    QTest::addColumn<KNumber>("op2");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(5) / KNumber(2)") << KNumber(5) << KNumber(2) << QStringLiteral("5/2") << KNumber::TypeFraction;
    QTest::newRow("KNumber(122) / KNumber(2)") << KNumber(122) << KNumber(2) << QStringLiteral("61") << KNumber::TypeInteger;
    QTest::newRow("KNumber(12) / KNumber(0)") << KNumber(12) << KNumber(0) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-12) / KNumber(0)") << KNumber(-12) << KNumber(0) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(5) / KNumber(\"2/3\")") << KNumber(5) << KNumber(QStringLiteral("2/3")) << QStringLiteral("15/2") << KNumber::TypeFraction;
    QTest::newRow("KNumber(6) / KNumber(\"2/3\")") << KNumber(6) << KNumber(QStringLiteral("2/3")) << QStringLiteral("9") << KNumber::TypeInteger;
    QTest::newRow("KNumber(5) / KNumber(2.5)") << KNumber(5) << KNumber(2.5) << QStringLiteral("2") << KNumber::TypeInteger;
    QTest::newRow("KNumber(5) / KNumber(0.0)") << KNumber(5) << KNumber(0.0) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-5) / KNumber(0.0)") << KNumber(-5) << KNumber(0.0) << QStringLiteral("nan") << KNumber::TypeError;

    QTest::newRow("KNumber(\"5/3\") / KNumber(2)") << KNumber(QStringLiteral("5/3")) << KNumber(2) << QStringLiteral("5/6") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"5/3\") / KNumber(0)") << KNumber(QStringLiteral("5/3")) << KNumber(0) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-5/3\") / KNumber(0)") << KNumber(QStringLiteral("-5/3")) << KNumber(0) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(\"5/3\") / KNumber(\"2/3\")")
        << KNumber(QStringLiteral("5/3")) << KNumber(QStringLiteral("2/3")) << QStringLiteral("5/2") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"49/3\") / KNumber(\"7/9\")")
        << KNumber(QStringLiteral("49/3")) << KNumber(QStringLiteral("7/9")) << QStringLiteral("21") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"5/2\") / KNumber(2.5)") << KNumber(QStringLiteral("5/2")) << KNumber(2.5) << QStringLiteral("1") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"5/2\") / KNumber(0.0)") << KNumber(QStringLiteral("5/2")) << KNumber(0.0) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-5/2\") / KNumber(0.0)") << KNumber(QStringLiteral("-5/2")) << KNumber(0.0) << QStringLiteral("nan") << KNumber::TypeError;

    QTest::newRow("KNumber(5.3) / KNumber(2)") << KNumber(5.3) << KNumber(2) << QStringLiteral("2.65") << KNumber::TypeFloat;
    QTest::newRow("KNumber(5.3) / KNumber(0)") << KNumber(5.3) << KNumber(0) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-5.3) / KNumber(0)") << KNumber(-5.3) << KNumber(0) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(5.3) / KNumber(\"2/3\")") << KNumber(5.3) << KNumber(QStringLiteral("2/3")) << QStringLiteral("7.95") << KNumber::TypeFloat;
    QTest::newRow("KNumber(5.5) / KNumber(2.5)") << KNumber(5.5) << KNumber(2.5) << QStringLiteral("2.2") << KNumber::TypeFloat;
    QTest::newRow("KNumber(5.5) / KNumber(0.0)") << KNumber(5.5) << KNumber(0.0) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-5.5) / KNumber(0.0)") << KNumber(-5.5) << KNumber(0.0) << QStringLiteral("nan") << KNumber::TypeError;

    QTest::newRow("inf / KNumber(2)") << KNumber::PosInfinity << KNumber(2) << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(-5) / inf") << KNumber(-5) << KNumber::PosInfinity << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("inf / KNumber(\"1/2\")") << KNumber::PosInfinity << KNumber(QStringLiteral("1/2")) << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-5/3\") / inf") << KNumber(QStringLiteral("-5/3")) << KNumber::PosInfinity << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("inf / KNumber(2.01)") << KNumber::PosInfinity << KNumber(2.01) << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(-5.4) / inf") << KNumber(-5.4) << KNumber::PosInfinity << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("-inf / KNumber(2)") << KNumber::NegInfinity << KNumber(2) << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("KNumber(-5) / -inf") << KNumber(-5) << KNumber::NegInfinity << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("-inf / KNumber(\"1/2\")") << KNumber::NegInfinity << KNumber(QStringLiteral("1/2")) << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-5/3\") / -inf") << KNumber(QStringLiteral("-5/3")) << KNumber::NegInfinity << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("-inf / KNumber(2.01)") << KNumber::NegInfinity << KNumber(2.01) << QStringLiteral("-inf") << KNumber::TypeError;
    QTest::newRow("KNumber(-5.4) / -inf") << KNumber(-5.4) << KNumber::NegInfinity << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("nan / KNumber(2)") << KNumber::NaN << KNumber(2) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-5) / nan") << KNumber(-5) << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("nan / KNumber(\"1/2\")") << KNumber::NaN << KNumber(QStringLiteral("1/2")) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-5/3\") / nan") << KNumber(QStringLiteral("-5/3")) << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("nan / KNumber(2.01)") << KNumber::NaN << KNumber(2.01) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-5.4) / nan") << KNumber(-5.4) << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("inf / inf") << KNumber::PosInfinity << KNumber::PosInfinity << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("inf / -inf") << KNumber::PosInfinity << KNumber::NegInfinity << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("-inf / inf") << KNumber::NegInfinity << KNumber::PosInfinity << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("-inf / -inf") << KNumber::NegInfinity << KNumber::NegInfinity << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("inf / nan") << KNumber::PosInfinity << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("-inf / nan") << KNumber::NegInfinity << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("nan / inf") << KNumber::NaN << KNumber::PosInfinity << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("-inf / nan") << KNumber::NegInfinity << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(0) / inf") << KNumber(0) << KNumber::PosInfinity << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("KNumber(0) / -inf") << KNumber(0) << KNumber::NegInfinity << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("inf / KNumber(0)") << KNumber::PosInfinity << KNumber(0) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("-inf / KNumber(0)") << KNumber::NegInfinity << KNumber(0) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(0.0) / inf") << KNumber(0.0) << KNumber::PosInfinity << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("KNumber(0.0) / -inf") << KNumber(0.0) << KNumber::NegInfinity << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("inf / KNumber(0.0)") << KNumber::PosInfinity << KNumber(0.0) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("-inf / KNumber(0.0)") << KNumber::NegInfinity << KNumber(0.0) << QStringLiteral("nan") << KNumber::TypeError;
}

void KNumberAritmeticTest::testKNumberDivision()
{
    QFETCH(KNumber, op1);
    QFETCH(KNumber, op2);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = op1 / op2;

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberAritmeticTest::cleanupTestCase()
{
}

QTEST_GUILESS_MAIN(KNumberAritmeticTest)

#include "knumber_aritmetic_test.moc"
