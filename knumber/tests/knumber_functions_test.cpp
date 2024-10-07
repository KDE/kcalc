/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2003-2005 Klaus Niederkrueger <kniederk@math.uni-koeln.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knumber.h"

#include <QString>
#include <QTest>

class KNumberFunctionsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testKNumberSqrt_data();
    void testKNumberSqrt();

    void testKNumberCbrt_data();
    void testKNumberCbrt();

    void testKNumberAbs_data();
    void testKNumberAbs();

    void testKNumberSin_data();
    void testKNumberSin();

    void testKNumberCos_data();
    void testKNumberCos();

    void testKNumberTan_data();
    void testKNumberTan();

    void testKNumberAsin_data();
    void testKNumberAsin();

    void testKNumberAcos_data();
    void testKNumberAcos();

    void testKNumberAtan_data();
    void testKNumberAtan();

    void testKNumberPower_data();
    void testKNumberPower();

    void testKNumberLog_data();
    void testKNumberLog();

    void testKNumberExp_data();
    void testKNumberExp();

    void cleanupTestCase();

private:
    static const int precision = 12;
};

void KNumberFunctionsTest::initTestCase()
{
}

void KNumberFunctionsTest::testKNumberSqrt_data()
{
    QTest::addColumn<KNumber>("op");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(16).sqrt()") << KNumber(16) << QStringLiteral("4") << KNumber::TypeInteger;
    QTest::newRow("KNumber(-16).sqrt()") << KNumber(-16) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(\"16/9\").sqrt()") << KNumber(QStringLiteral("16/9")) << QStringLiteral("4/3") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"-16/9\").sqrt()") << KNumber(QStringLiteral("-16/9")) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(2).sqrt()") << KNumber(2) << QStringLiteral("1.41421356237") << KNumber::TypeFloat;
    QTest::newRow("KNumber(\"2/3\").sqrt()") << KNumber(QStringLiteral("2/3")) << QStringLiteral("0.816496580928") << KNumber::TypeFloat;
    QTest::newRow("KNumber(\"0.25\").sqrt()") << KNumber(QStringLiteral("0.25")) << QStringLiteral("0.5") << KNumber::TypeFloat;
    QTest::newRow("KNumber(\"-0.25\").sqrt()") << KNumber(QStringLiteral("-0.25")) << QStringLiteral("nan") << KNumber::TypeError;
}

void KNumberFunctionsTest::testKNumberSqrt()
{
    QFETCH(KNumber, op);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = op.sqrt();

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberFunctionsTest::testKNumberCbrt_data()
{
    QTest::addColumn<KNumber>("op");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(27).cbrt()") << KNumber(27) << QStringLiteral("3") << KNumber::TypeInteger;
    QTest::newRow("KNumber(-27).cbrt()") << KNumber(-27) << QStringLiteral("-3") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"27/8\").cbrt()") << KNumber(QStringLiteral("27/8")) << QStringLiteral("3/2") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"-8/27\").cbrt()") << KNumber(QStringLiteral("-8/27")) << QStringLiteral("-2/3") << KNumber::TypeFraction;
    // TODO: need to check non-perfect cube roots
    QTest::newRow("KNumber(2).cbrt()") << KNumber(2) << QStringLiteral("1.25992104989") << KNumber::TypeFloat;
    QTest::newRow("KNumber(\"2/3\").cbrt()") << KNumber(QStringLiteral("2/3")) << QStringLiteral("0.873580464736") << KNumber::TypeFloat;
    QTest::newRow("KNumber(\"0.25\").cbrt()") << KNumber(QStringLiteral("0.25")) << QStringLiteral("0.629960524947") << KNumber::TypeFloat;
    QTest::newRow("KNumber(\"-0.25\").cbrt()") << KNumber(QStringLiteral("-0.25")) << QStringLiteral("-0.629960524947") << KNumber::TypeFloat;
}

void KNumberFunctionsTest::testKNumberCbrt()
{
    QFETCH(KNumber, op);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = op.cbrt();

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberFunctionsTest::testKNumberAbs_data()
{
    QTest::addColumn<KNumber>("op");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(5).abs()") << KNumber(5) << QStringLiteral("5") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"2/3\").abs()") << KNumber(QStringLiteral("2/3")) << QStringLiteral("2/3") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"2.3\").abs()") << KNumber(QStringLiteral("2.3")) << QStringLiteral("2.3") << KNumber::TypeFloat;

    QTest::newRow("KNumber(-5).abs()") << KNumber(-5) << QStringLiteral("5") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"-2/3\").abs()") << KNumber(QStringLiteral("-2/3")) << QStringLiteral("2/3") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"-2.3\").abs()") << KNumber(QStringLiteral("-2.3")) << QStringLiteral("2.3") << KNumber::TypeFloat;
}

void KNumberFunctionsTest::testKNumberAbs()
{
    QFETCH(KNumber, op);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = op.abs();

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberFunctionsTest::testKNumberSin_data()
{
    QTest::addColumn<KNumber>("op");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("sin(KNumber(5))") << KNumber(5) << QStringLiteral("-0.958924274663") << KNumber::TypeFloat;
    QTest::newRow("sin(KNumber(-5))") << KNumber(-5) << QStringLiteral("0.958924274663") << KNumber::TypeFloat;
    QTest::newRow("sin(KNumber(\"5/2\"))") << KNumber(QStringLiteral("5/2")) << QStringLiteral("0.598472144104") << KNumber::TypeFloat;
    QTest::newRow("sin(KNumber(\"-5/2\"))") << KNumber(QStringLiteral("-5/2")) << QStringLiteral("-0.598472144104") << KNumber::TypeFloat;
    QTest::newRow("sin(KNumber(5.3))") << KNumber(5.3) << QStringLiteral("-0.832267442224") << KNumber::TypeFloat;
    QTest::newRow("sin(KNumber(-5.3))") << KNumber(-5.3) << QStringLiteral("0.832267442224") << KNumber::TypeFloat;
}

void KNumberFunctionsTest::testKNumberSin()
{
    QFETCH(KNumber, op);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = sin(op);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberFunctionsTest::testKNumberCos_data()
{
    QTest::addColumn<KNumber>("op");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("cos(KNumber(5))") << KNumber(5) << QStringLiteral("0.283662185463") << KNumber::TypeFloat;
    QTest::newRow("cos(KNumber(-5))") << KNumber(-5) << QStringLiteral("0.283662185463") << KNumber::TypeFloat;
    QTest::newRow("cos(KNumber(\"5/2\"))") << KNumber(QStringLiteral("5/2")) << QStringLiteral("-0.801143615547") << KNumber::TypeFloat;
    QTest::newRow("cos(KNumber(\"-5/2\"))") << KNumber(QStringLiteral("-5/2")) << QStringLiteral("-0.801143615547") << KNumber::TypeFloat;
    QTest::newRow("cos(KNumber(5.3))") << KNumber(5.3) << QStringLiteral("0.554374336179") << KNumber::TypeFloat;
    QTest::newRow("cos(KNumber(-5.3))") << KNumber(-5.3) << QStringLiteral("0.554374336179") << KNumber::TypeFloat;
}

void KNumberFunctionsTest::testKNumberCos()
{
    QFETCH(KNumber, op);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = cos(op);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberFunctionsTest::testKNumberTan_data()
{
    QTest::addColumn<KNumber>("op");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("tan(KNumber(5))") << KNumber(5) << QStringLiteral("-3.38051500625") << KNumber::TypeFloat;
    QTest::newRow("tan(KNumber(-5))") << KNumber(-5) << QStringLiteral("3.38051500625") << KNumber::TypeFloat;
    QTest::newRow("tan(KNumber(\"5/2\"))") << KNumber(QStringLiteral("5/2")) << QStringLiteral("-0.747022297239") << KNumber::TypeFloat;
    QTest::newRow("tan(KNumber(\"-5/2\"))") << KNumber(QStringLiteral("-5/2")) << QStringLiteral("0.747022297239") << KNumber::TypeFloat;
    QTest::newRow("tan(KNumber(5.3))") << KNumber(5.3) << QStringLiteral("-1.50127339581") << KNumber::TypeFloat;
    QTest::newRow("tan(KNumber(-5.3))") << KNumber(-5.3) << QStringLiteral("1.50127339581") << KNumber::TypeFloat;
}

void KNumberFunctionsTest::testKNumberTan()
{
    QFETCH(KNumber, op);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = tan(op);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberFunctionsTest::testKNumberAsin_data()
{
    QTest::addColumn<KNumber>("op");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("asin(KNumber(5))") << KNumber(5) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("asin(KNumber(-5))") << KNumber(-5) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("asin(KNumber(\"5/2\"))") << KNumber(QStringLiteral("5/2")) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("asin(KNumber(\"-5/2\"))") << KNumber(QStringLiteral("-5/2")) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("asin(KNumber(5.3))") << KNumber(5.3) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("asin(KNumber(-5.3))") << KNumber(-5.3) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("asin(KNumber(\"2/5\"))") << KNumber(QStringLiteral("2/5")) << QStringLiteral("0.411516846067") << KNumber::TypeFloat;
    QTest::newRow("asin(KNumber(\"-2/5\"))") << KNumber(QStringLiteral("-2/5")) << QStringLiteral("-0.411516846067") << KNumber::TypeFloat;
    QTest::newRow("asin(KNumber(0.3))") << KNumber(0.3) << QStringLiteral("0.304692654015") << KNumber::TypeFloat;
    QTest::newRow("asin(KNumber(-0.3))") << KNumber(-0.3) << QStringLiteral("-0.304692654015") << KNumber::TypeFloat;
}

void KNumberFunctionsTest::testKNumberAsin()
{
    QFETCH(KNumber, op);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = asin(op);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberFunctionsTest::testKNumberAcos_data()
{
    QTest::addColumn<KNumber>("op");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("acos(KNumber(5))") << KNumber(5) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("acos(KNumber(-5))") << KNumber(-5) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("acos(KNumber(\"5/2\"))") << KNumber(QStringLiteral("5/2")) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("acos(KNumber(\"-5/2\"))") << KNumber(QStringLiteral("-5/2")) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("acos(KNumber(5.3))") << KNumber(5.3) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("acos(KNumber(-5.3))") << KNumber(-5.3) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("acos(KNumber(\"2/5\"))") << KNumber(QStringLiteral("2/5")) << QStringLiteral("1.15927948073") << KNumber::TypeFloat;
    QTest::newRow("acos(KNumber(\"-2/5\"))") << KNumber(QStringLiteral("-2/5")) << QStringLiteral("1.98231317286") << KNumber::TypeFloat;

    QTest::newRow("acos(KNumber(0.3))") << KNumber(0.3) << QStringLiteral("1.26610367278") << KNumber::TypeFloat;
    QTest::newRow("acos(KNumber(-0.3))") << KNumber(-0.3) << QStringLiteral("1.87548898081") << KNumber::TypeFloat;
}

void KNumberFunctionsTest::testKNumberAcos()
{
    QFETCH(KNumber, op);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = acos(op);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberFunctionsTest::testKNumberAtan_data()
{
    QTest::addColumn<KNumber>("op");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("atan(KNumber(5))") << KNumber(5) << QStringLiteral("1.37340076695") << KNumber::TypeFloat;
    QTest::newRow("atan(KNumber(-5))") << KNumber(-5) << QStringLiteral("-1.37340076695") << KNumber::TypeFloat;
    QTest::newRow("atan(KNumber(\"5/2\"))") << KNumber(QStringLiteral("5/2")) << QStringLiteral("1.19028994968") << KNumber::TypeFloat;
    QTest::newRow("atan(KNumber(\"-5/2\"))") << KNumber(QStringLiteral("-5/2")) << QStringLiteral("-1.19028994968") << KNumber::TypeFloat;
    QTest::newRow("atan(KNumber(5.3))") << KNumber(5.3) << QStringLiteral("1.38430942513") << KNumber::TypeFloat;
    QTest::newRow("atan(KNumber(-5.3))") << KNumber(-5.3) << QStringLiteral("-1.38430942513") << KNumber::TypeFloat;
    QTest::newRow("atan(KNumber(\"2/5\"))") << KNumber(QStringLiteral("2/5")) << QStringLiteral("0.380506377112") << KNumber::TypeFloat;
    QTest::newRow("atan(KNumber(\"-2/5\"))") << KNumber(QStringLiteral("-2/5")) << QStringLiteral("-0.380506377112") << KNumber::TypeFloat;
    QTest::newRow("atan(KNumber(0.3))") << KNumber(0.3) << QStringLiteral("0.291456794478") << KNumber::TypeFloat;
    QTest::newRow("atan(KNumber(-0.3))") << KNumber(-0.3) << QStringLiteral("-0.291456794478") << KNumber::TypeFloat;
}

void KNumberFunctionsTest::testKNumberAtan()
{
    QFETCH(KNumber, op);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = atan(op);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberFunctionsTest::testKNumberPower_data()
{
    QTest::addColumn<KNumber>("op1");
    QTest::addColumn<KNumber>("op2");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("KNumber(0) ^ KNumber(0)") << KNumber(0) << KNumber(0) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(0) ^ KNumber(-4)") << KNumber(0) << KNumber(-4) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(5) ^ KNumber(4)") << KNumber(5) << KNumber(4) << QStringLiteral("625") << KNumber::TypeInteger;
    QTest::newRow("KNumber(122) ^ KNumber(0)") << KNumber(122) << KNumber(0) << QStringLiteral("1") << KNumber::TypeInteger;
    QTest::newRow("KNumber(-5) ^ KNumber(0)") << KNumber(-5) << KNumber(0) << QStringLiteral("1") << KNumber::TypeInteger;
    QTest::newRow("KNumber(-2) ^ KNumber(3)") << KNumber(-2) << KNumber(3) << QStringLiteral("-8") << KNumber::TypeInteger;
    QTest::newRow("KNumber(-2) ^ KNumber(4)") << KNumber(-2) << KNumber(4) << QStringLiteral("16") << KNumber::TypeInteger;
    QTest::newRow("KNumber(5) ^ KNumber(-2)") << KNumber(5) << KNumber(-2) << QStringLiteral("1/25") << KNumber::TypeFraction;
    QTest::newRow("KNumber(8) ^ KNumber(\"2/3\")") << KNumber(8) << KNumber(QStringLiteral("2/3")) << QStringLiteral("4") << KNumber::TypeInteger;
    QTest::newRow("KNumber(8) ^ KNumber(\"-2/3\")") << KNumber(8) << KNumber(QStringLiteral("-2/3")) << QStringLiteral("1/4") << KNumber::TypeFraction;

    QTest::newRow("KNumber(-16) ^ KNumber(\"1/4\")") << KNumber(-16) << KNumber(QStringLiteral("1/4")) << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-8) ^ KNumber(\"1/3\")") << KNumber(-8) << KNumber(QStringLiteral("1/3")) << QStringLiteral("-2") << KNumber::TypeInteger;
    QTest::newRow("KNumber(5) ^ KNumber(0.0)") << KNumber(5) << KNumber(0.0) << QStringLiteral("1") << KNumber::TypeInteger;
    QTest::newRow("KNumber(-5) ^ KNumber(0.0)") << KNumber(-5) << KNumber(0.0) << QStringLiteral("1") << KNumber::TypeInteger;

    QTest::newRow("KNumber(\"5/3\") ^ KNumber(2)") << KNumber(QStringLiteral("5/3")) << KNumber(2) << QStringLiteral("25/9") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"5/3\") ^ KNumber(0)") << KNumber(QStringLiteral("5/3")) << KNumber(0) << QStringLiteral("1") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"-5/3\") ^ KNumber(0)") << KNumber(QStringLiteral("-5/3")) << KNumber(0) << QStringLiteral("1") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"8/27\") ^ KNumber(\"2/3\")")
        << KNumber(QStringLiteral("8/27")) << KNumber(QStringLiteral("2/3")) << QStringLiteral("4/9") << KNumber::TypeFraction;
    QTest::newRow("KNumber(\"49/3\") ^ KNumber(\"7/9\")")
        << KNumber(QStringLiteral("49/3")) << KNumber(QStringLiteral("7/9")) << QStringLiteral("8.78016428243") << KNumber::TypeFloat;
    QTest::newRow("KNumber(\"5/2\") ^ KNumber(2.5)") << KNumber(QStringLiteral("5/2")) << KNumber(2.5) << QStringLiteral("9.88211768803") << KNumber::TypeFloat;
    QTest::newRow("KNumber(\"5/2\") ^ KNumber(0.0)") << KNumber(QStringLiteral("5/2")) << KNumber(0.0) << QStringLiteral("1") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"-5/2\") ^ KNumber(0.0)") << KNumber(QStringLiteral("-5/2")) << KNumber(0.0) << QStringLiteral("1") << KNumber::TypeInteger;

    QTest::newRow("KNumber(5.3) ^ KNumber(2)") << KNumber(5.3) << KNumber(2) << QStringLiteral("28.09") << KNumber::TypeFloat;
    QTest::newRow("KNumber(5.3) ^ KNumber(0)") << KNumber(5.3) << KNumber(0) << QStringLiteral("1") << KNumber::TypeInteger;
    QTest::newRow("KNumber(-5.3) ^ KNumber(0)") << KNumber(-5.3) << KNumber(0) << QStringLiteral("1") << KNumber::TypeInteger;
    QTest::newRow("KNumber(5.3) ^ KNumber(\"2/3\")") << KNumber(5.3) << KNumber(QStringLiteral("2/3")) << QStringLiteral("3.03983898039") << KNumber::TypeFloat;
    QTest::newRow("KNumber(5.5) ^ KNumber(2.5)") << KNumber(5.5) << KNumber(2.5) << QStringLiteral("70.9425383673") << KNumber::TypeFloat;
    QTest::newRow("KNumber(5.5) ^ KNumber(0.0)") << KNumber(5.5) << KNumber(0.0) << QStringLiteral("1") << KNumber::TypeInteger;
    QTest::newRow("KNumber(-5.5) ^ KNumber(0.0)") << KNumber(-5.5) << KNumber(0.0) << QStringLiteral("1") << KNumber::TypeInteger;

    QTest::newRow("KNumber::Pi() ^ KNumber::Pi()") << KNumber::Pi() << KNumber::Pi() << QStringLiteral("36.4621596072") << KNumber::TypeFloat;
    QTest::newRow("KNumber::Euler() ^ KNumber::Pi()") << KNumber::Euler() << KNumber::Pi() << QStringLiteral("23.1406926328") << KNumber::TypeFloat;

    QTest::newRow("KNumber(2.0) ^ KNumber(0.5)") << KNumber(2.0) << KNumber(0.5) << QStringLiteral("1.41421356237") << KNumber::TypeFloat;
    QTest::newRow("KNumber(2.0) ^ KNumber(-0.5)") << KNumber(2.0) << KNumber(-0.5) << QStringLiteral("0.707106781187") << KNumber::TypeFloat;

    // QTest::newRow("KNumber(-2.0).exp()") << KNumber(-2.0).exp() << QStringLiteral("0.135335283237") << KNumber::TypeFloat;
    QTest::newRow("KNumber::Euler() ^ KNumber(-2.0)") << KNumber::Euler() << KNumber(-2.0) << QStringLiteral("0.135335283237") << KNumber::TypeFloat;

    QTest::newRow("KNumber(2.0).exp()") << KNumber::Euler() << KNumber(2.0) << QStringLiteral("7.38905609893") << KNumber::TypeFloat;
    QTest::newRow("KNumber::Euler() ^ KNumber(2.0)") << KNumber::Euler() << KNumber(2.0) << QStringLiteral("7.38905609893") << KNumber::TypeFloat;

    // TODO: kinda odd that this ends up being an integer
    // i guess since my euler constant is only 100 digits << we've exceeded the fractional part
    QTest::newRow("KNumber::Euler() ^ 1000") << KNumber::Euler() << KNumber(1000) << QStringLiteral("1.97007111402e+434") << KNumber::TypeInteger;

    // TODO: make this test pass
    // the problem is that it is using the libc exp function which has limits that GMP does not
    // we should basically make this equivalent to KNumber::Euler() << KNumber(1000))
    // which does work
#if 0
    QTest::newRow("KNumber(1000).exp()") << KNumber(1000).exp() << QLatin1String("23.1406926328") << KNumber::TypeFloat;
#endif

    QTest::newRow("KNumber(\"3.1415926\") ^ KNumber(\"3.1415926\")")
        << KNumber(QStringLiteral("3.1415926")) << KNumber(QStringLiteral("3.1415926")) << QStringLiteral("36.4621554164") << KNumber::TypeFloat;

    QTest::newRow("KNumber(5) ^ KNumber(\"inf\")") << KNumber(5) << KNumber::PosInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(-5) ^ KNumber(\"inf\")") << KNumber(-5) << KNumber::PosInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(\"5/2\") ^ KNumber(\"inf\")")
        << KNumber(QStringLiteral("5/2")) << KNumber::PosInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-5/2\") ^ KNumber(\"inf\")")
        << KNumber(QStringLiteral("-5/2")) << KNumber::PosInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(\"5.2\") ^ KNumber(\"inf\")")
        << KNumber(QStringLiteral("5.2")) << KNumber::PosInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-5.2\") ^ KNumber(\"inf\")")
        << KNumber(QStringLiteral("-5.2")) << KNumber::PosInfinity << QStringLiteral("inf") << KNumber::TypeError;

    QTest::newRow("KNumber(5) ^ KNumber(\"-inf\")") << KNumber(5) << KNumber::NegInfinity << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("KNumber(-5) ^ KNumber(\"-inf\")") << KNumber(-5) << KNumber::NegInfinity << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"5/2\") ^ KNumber(\"-inf\")")
        << KNumber(QStringLiteral("5/2")) << KNumber::NegInfinity << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"-5/2\") ^ KNumber(\"-inf\")")
        << KNumber(QStringLiteral("-5/2")) << KNumber::NegInfinity << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"5.2\") ^ KNumber(\"-inf\")")
        << KNumber(QStringLiteral("5.2")) << KNumber::NegInfinity << QStringLiteral("0") << KNumber::TypeInteger;
    QTest::newRow("KNumber(\"-5.2\") ^ KNumber(\"-inf\")")
        << KNumber(QStringLiteral("-5.2")) << KNumber::NegInfinity << QStringLiteral("0") << KNumber::TypeInteger;

    QTest::newRow("KNumber(5) ^ KNumber(\"nan\")") << KNumber(5) << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(-5) ^ KNumber(\"nan\")") << KNumber(-5) << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(\"5/2\") ^ KNumber(\"nan\")") << KNumber(QStringLiteral("5/2")) << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-5/2\") ^ KNumber(\"nan\")") << KNumber(QStringLiteral("-5/2")) << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(\"5.2\") ^ KNumber(\"nan\")") << KNumber(QStringLiteral("5.2")) << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-5.2\") ^ KNumber(\"nan\")") << KNumber(QStringLiteral("-5.2")) << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;

    QTest::newRow("KNumber(\"nan\") ^ KNumber(\"nan\")") << KNumber::NaN << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(\"nan\") ^ KNumber(\"inf\")") << KNumber::NaN << KNumber::PosInfinity << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(\"nan\") ^ KNumber(\"-inf\")") << KNumber::NaN << KNumber::NegInfinity << QStringLiteral("nan") << KNumber::TypeError;

    QTest::newRow("KNumber(\"inf\") ^ KNumber(\"nan\")") << KNumber::PosInfinity << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(\"inf\") ^ KNumber(\"inf\")") << KNumber::PosInfinity << KNumber::PosInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(\"inf\") ^ KNumber(\"-inf\")") << KNumber::PosInfinity << KNumber::NegInfinity << QStringLiteral("0") << KNumber::TypeInteger;

    QTest::newRow("KNumber(\"-inf\") ^ KNumber(\"nan\")") << KNumber::NegInfinity << KNumber::NaN << QStringLiteral("nan") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-inf\") ^ KNumber(\"inf\")") << KNumber::NegInfinity << KNumber::PosInfinity << QStringLiteral("inf") << KNumber::TypeError;
    QTest::newRow("KNumber(\"-inf\") ^ KNumber(\"-inf\")") << KNumber::NegInfinity << KNumber::NegInfinity << QStringLiteral("0") << KNumber::TypeInteger;
}

void KNumberFunctionsTest::testKNumberPower()
{
    QFETCH(KNumber, op1);
    QFETCH(KNumber, op2);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = op1.pow(op2);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberFunctionsTest::testKNumberLog_data()
{
    QTest::addColumn<KNumber>("op");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("log10(KNumber(5))") << KNumber(5) << QStringLiteral("0.698970004336") << KNumber::TypeFloat;
    QTest::newRow("log10(pow(KNumber(10), KNumber(308)))") << pow(KNumber(10), KNumber(308)) << QStringLiteral("308") << KNumber::TypeInteger;

    // TODO: enable this check once MPFR is commonly enabled
    // QTest::newRow("log10(pow(KNumber(10) << KNumber(309)))") << log10(pow(KNumber(10) << KNumber(309))) << QLatin1String("309") << KNumber::TypeInteger;
}

void KNumberFunctionsTest::testKNumberLog()
{
    QFETCH(KNumber, op);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = log10(op);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberFunctionsTest::testKNumberExp_data()
{
    QTest::addColumn<KNumber>("op");
    QTest::addColumn<QString>("expectedResultToQString");
    QTest::addColumn<KNumber::Type>("expectedResultType");

    QTest::newRow("exp(KNumber(4.34))") << KNumber(4.34) << QStringLiteral("76.7075393383") << KNumber::TypeFloat;
}

void KNumberFunctionsTest::testKNumberExp()
{
    QFETCH(KNumber, op);
    QFETCH(KNumber::Type, expectedResultType);
    QFETCH(QString, expectedResultToQString);

    KNumber result = exp(op);

    QCOMPARE(result.type(), expectedResultType);
    QCOMPARE(result.toQString(precision), expectedResultToQString);
}

void KNumberFunctionsTest::cleanupTestCase()
{
}

QTEST_GUILESS_MAIN(KNumberFunctionsTest)

#include "knumber_functions_test.moc"
