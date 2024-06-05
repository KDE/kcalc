/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "../kcalc_core.h"
#include "../kcalc_parser.h"

#include <QQueue>
#include <QTest>

#define QS(s) QStringLiteral(s)

/**
 * The main test class.
 */
class KCalcParserCoreTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testParserCore_data();
    void testParserCore();

    void testParserError_data();
    void testParserError();

    void testCalculationError_data();
    void testCalculationError();

    void cleanupTestCase();

private:
    QQueue<KCalcToken> token_Queue_;
    CalcEngine core;
    KCalcParser parser;

    int input_error_index_;
};

void KCalcParserCoreTest::initTestCase()
{
    // load constants into parser
}

void KCalcParserCoreTest::testParserCore_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("base");
    QTest::addColumn<QString>("expectedResult");

    // Simple operations //
    QTest::newRow("addition test") << QS("1+2") << 10 << QS("3");
    QTest::newRow("subtaction test") << QS("7-2") << 10 << QS("5");
    QTest::newRow("multiplication test 1") << QS("50*2") << 10 << QS("100");
    QTest::newRow("multiplication test 2") << QS("50×2") << 10 << QS("100");
    QTest::newRow("division test") << QS("50÷12") << 10 << QS("25/6");
    QTest::newRow("div test using slash") << QS("1/2") << 10 << QS("1/2");
    QTest::newRow("percentage") << QS("10×50%") << 10 << QS("5");
    QTest::newRow("inverse test 1") << QS("10⁻¹") << 10 << QS("1/10");
    QTest::newRow("inverse test 2") << QS("10⁻¹⁻¹") << 10 << QS("10");
    QTest::newRow("sqrt test 2") << QS("√5") << 10 << QS("2.2360679775");
    QTest::newRow("sqrt test 2") << QS("∛5") << 10 << QS("1.70997594668");
    QTest::newRow("square test 1") << QS("2.2360679775²") << 10 << QS("5");
    QTest::newRow("square test 2") << QS("2.2360679775²²") << 10 << QS("25");
    QTest::newRow("power test 1") << QS("5^5") << 10 << QS("3125");
    QTest::newRow("power test 2") << QS("2^2^2") << 10 << QS("16");
    QTest::newRow("sin test") << QS("sin(90)") << 10 << QS("1");
    QTest::newRow("sin test2") << QS("sin(45)") << 10 << QS("0.707106781187");
    QTest::newRow("cos test") << QS("cos(90)") << 10 << QS("0");
    QTest::newRow("tan test") << QS("tan(45)") << 10 << QS("1");
    QTest::newRow("asin test") << QS("asin(0.707106781187)") << 10 << QS("45");
    QTest::newRow("asin test") << QS("asin(1)") << 10 << QS("90");
    QTest::newRow("acos test") << QS("acos(0)") << 10 << QS("90");
    QTest::newRow("atan test") << QS("atan(1)") << 10 << QS("45");

    QTest::newRow("sinh test 1") << QS("sinh(1)") << 10 << QS("1.17520119364");
    QTest::newRow("cosh test 1") << QS("cosh(1)") << 10 << QS("1.54308063482");
    QTest::newRow("tanh test 1") << QS("tanh(2000.90645)") << 10 << QS("1");
    QTest::newRow("tanh test 2") << QS("tanh(0.5)") << 10 << QS("0.46211715726");
    QTest::newRow("asinh test 1") << QS("asinh(1.17520119364)") << 10 << QS("0.999999999998");
    QTest::newRow("acosh test 2") << QS("acosh(1.54308063481)") << 10 << QS("0.999999999996");
    QTest::newRow("atanh test 3") << QS("atanh(0.46211715726)") << 10 << QS("0.5");

    QTest::newRow("ln test 1") << QS("ln(2.71)") << 10 << QS("0.996948634892");
    QTest::newRow("ln test 2") << QS("ln(2.718281828460)") << 10 << QS("1");
    QTest::newRow("log test 1") << QS("log(10)") << 10 << QS("1");
    QTest::newRow("log test 2") << QS("log(100)") << 10 << QS("2");
    QTest::newRow("log test 3") << QS("log(1000)") << 10 << QS("3");

    QTest::newRow("exp test") << QS("exp(1)") << 10 << QS("2.71828182846");
    QTest::newRow("exp10 test") << QS("5⏨2") << 10 << QS("500");

    QTest::newRow("- test 1") << QS("-590") << 10 << QS("-590");
    QTest::newRow("- test 2") << QS("-1cos(45)") << 10 << QS("-0.707106781187");
    QTest::newRow("- test 3") << QS("-cos(45)") << 10 << QS("-0.707106781187");
    QTest::newRow("- test 4") << QS("-20-cos(45)") << 10 << QS("-20.7071067812");
    QTest::newRow("- test 5") << QS("-1×-1") << 10 << QS("1");
    QTest::newRow("- test 6") << QS("-1÷-1") << 10 << QS("1");
    QTest::newRow("- test 7") << QS("-7 mod 5") << 10 << QS("-2");
    QTest::newRow("- test 8") << QS("5×--5") << 10 << QS("25");
    QTest::newRow("- test 9") << QS("5×---5") << 10 << QS("-25");
    QTest::newRow("- test 10") << QS("7 mod -5") << 10 << QS("2");
    QTest::newRow("-- test 1") << QS("--5") << 10 << QS("5");
    QTest::newRow("-- test 2") << QS("--5+6") << 10 << QS("11");
    QTest::newRow("-- test 3") << QS("--4--9+100") << 10 << QS("113");
    QTest::newRow("-- test 4") << QS("--4--9-100") << 10 << QS("-87");
    QTest::newRow("-- test 5") << QS("--4--9×100") << 10 << QS("904");
    QTest::newRow("-- test 6") << QS("--4×45--9×100") << 10 << QS("1080");
    QTest::newRow("-- test 7") << QS("--4×45--9×100") << 10 << QS("1080");

    QTest::newRow("+ test 1") << QS("+7") << 10 << QS("7");
    QTest::newRow("++ test 1") << QS("++7") << 10 << QS("7");
    QTest::newRow("+++ test 1") << QS("+++7") << 10 << QS("7");
    QTest::newRow("+++ test 2") << QS("+++7-7²") << 10 << QS("-42");
    QTest::newRow("+++ test 3") << QS("+++7--7²") << 10 << QS("56");
    QTest::newRow("+ test 2") << QS("+7--1") << 10 << QS("8");
    QTest::newRow("+ test 3") << QS("+7--20") << 10 << QS("27");
    QTest::newRow("+ test 4") << QS("+7-20") << 10 << QS("-13");
    QTest::newRow("+ test 5") << QS("+7-20+30") << 10 << QS("17");
    QTest::newRow("+ test 6") << QS("+7----1") << 10 << QS("8");

    QTest::newRow("& test 1") << QS("0b10 & 0b100") << 10 << QS("0");
    QTest::newRow("& test 2") << QS("0b10 & 0b11 & 0b110") << 10 << QS("2");
    QTest::newRow("| test 1") << QS("0b10 | 0b1") << 10 << QS("3");
    QTest::newRow("~ test 1") << QS("~~0b10") << 10 << QS("2");
    QTest::newRow("<< test 1") << QS("0b10<<1") << 10 << QS("4");
    QTest::newRow(">> test 1") << QS("0b10>>1") << 10 << QS("1");

    QTest::newRow("mod test 1") << QS("15 mod 10") << 10 << QS("5");
    QTest::newRow("mod test 2") << QS("100 mod 101 mod 90 mod 8") << 10 << QS("2");
    QTest::newRow("div test 1") << QS("15 div 10") << 10 << QS("1");
    QTest::newRow("div test 2") << QS("25 div 10") << 10 << QS("2");
    QTest::newRow("factorial test 1") << QS("5!") << 10 << QS("120");

    QTest::newRow("0x test 1") << QS("0xA+10") << 10 << QS("20");
    QTest::newRow("0b test 1") << QS("0b100+1") << 10 << QS("5");
    QTest::newRow("0 test 1") << QS("010+8") << 10 << QS("16");
    QTest::newRow("base test 1") << QS("0xFF & 0b1111 & 07") << 10 << QS("7");
    // parenthesis input
    QTest::newRow("parenthesis test 1") << QS("(√5)²") << 10 << QS("5");
    QTest::newRow("parenthesis test 2") << QS("(1+3)") << 10 << QS("4");
    QTest::newRow("parenthesis test 3") << QS("(1+3)²") << 10 << QS("16");
    QTest::newRow("parenthesis test 4") << QS("5×(1+3)") << 10 << QS("20");
    QTest::newRow("parenthesis test 5") << QS("5×(1+3)+2") << 10 << QS("22");
    QTest::newRow("parenthesis test 6") << QS("5×(1+3)-2") << 10 << QS("18");
    QTest::newRow("parenthesis test 7") << QS("2(5×(1+3)-2)") << 10 << QS("36");
    QTest::newRow("parenthesis test 8") << QS("5×(1+3)²") << 10 << QS("80");
    QTest::newRow("parenthesis test 9") << QS("cos(1+2+87)") << 10 << QS("0");
    QTest::newRow("parenthesis test 10") << QS("(1)(2)(3)") << 10 << QS("6");
    QTest::newRow("parenthesis test 11") << QS("(((((-1)))))") << 10 << QS("-1");
    QTest::newRow("parenthesis test 12") << QS("(((((-1)))))+2") << 10 << QS("1");
    QTest::newRow("parenthesis test 13") << QS("√(1+2+3!+91)") << 10 << QS("10");
    QTest::newRow("parenthesis test 14") << QS("sin(sin(sin(sin(0))))") << 10 << QS("0");
    QTest::newRow("parenthesis test 15") << QS("ln(exp(1))") << 10 << QS("1");
    QTest::newRow("parenthesis test 16") << QS("ln(exp(100.45))") << 10 << QS("100.45");
    // random input test (these are inputs that generated errors on production
    // or variations of those )
    QTest::newRow("Minus (-) second to last test 1") << QS("8-3*2+1") << 10 << QS("3");
    QTest::newRow("Minus (-) second to last test 2") << QS("8-3*2-1") << 10 << QS("1");
    QTest::newRow("Minus (-) second to last test 3") << QS("8-(3*2)+1") << 10 << QS("3");
    QTest::newRow("Minus (-) second to last test 4") << QS("8-(3*2+1)+1") << 10 << QS("2");
    QTest::newRow("Minus (-) second to last test 5") << QS("8-3*2-1") << 10 << QS("1");
    QTest::newRow("Minus (-) second to last test 6") << QS("8-6-1") << 10 << QS("1");
    QTest::newRow("Minus (-) second to last test 7") << QS("10-1-1-1-1-1-1") << 10 << QS("4");
    QTest::newRow("Minus (-) second to last test 8") << QS("-10-1-1-1-1-1-1") << 10 << QS("-16");
    QTest::newRow("Minus (-) second to last test 9") << QS("--10-1+1-1+1-1+1") << 10 << QS("10");
    QTest::newRow("Minus (-) second to last test 10") << QS("--10-1+1-1+1-1+1/5") << 10 << QS("46/5");
    QTest::newRow("Minus (-) second to last test 11") << QS("8 - 7 mod 2 +1") << 10 << QS("8");
    QTest::newRow("Minus (-) second to last test 12") << QS("8-sin(90)-1") << 10 << QS("6");
    QTest::newRow("Minus (-) second to last test 13") << QS("8-sin(90)+1") << 10 << QS("8");
    QTest::newRow("Minus (-) second to last test 14") << QS("8-cos(0)sin(90)-1") << 10 << QS("6");
    QTest::newRow("Minus (-) second to last test 15") << QS("8-cos(0)sin(90)+1") << 10 << QS("8");
    QTest::newRow("Minus (-) second to last test 16") << QS("8-cos(90)sin(90)+1") << 10 << QS("9");
    QTest::newRow("Minus (-) second to last test 17") << QS("8-√4+1") << 10 << QS("7");
    QTest::newRow("Minus (-) second to last test 18") << QS("8-√4-1") << 10 << QS("5");
}

void KCalcParserCoreTest::testParserCore()
{
    QFETCH(QString, input);
    QFETCH(int, base);
    QFETCH(QString, expectedResult);

    int parsing_result, calculation_result, errorIndex;
    parsing_result = parser.stringToTokenQueue(input, base, token_Queue_, errorIndex);
    QCOMPARE(parsing_result, 0); // successful parsing

    calculation_result = core.calculate(token_Queue_, errorIndex);

    QCOMPARE(calculation_result, 0); // successful calculation

    QString coreResult = core.getResult().toQString(12, -1);
    coreResult.replace(KNumber::decimalSeparator().at(0), QLatin1Char('.'));
    coreResult.replace(QLatin1Char(','), QLatin1Char('.'));
    QCOMPARE(coreResult, expectedResult);
}

void KCalcParserCoreTest::testParserError_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("expectedErrorIndex");

    // invalid characters in input //
    QTest::newRow("parsing error test 1") << QS("1+s2") << 2;
    QTest::newRow("parsing error test 2") << QS("126d") << 3;
    QTest::newRow("parsing error test 3") << QS("1+1+1+1+1+m") << 10;
}

void KCalcParserCoreTest::testParserError()
{
    QFETCH(QString, input);
    QFETCH(int, expectedErrorIndex);

    int parsing_result, errorIndex;
    parsing_result = parser.stringToTokenQueue(input, 10, token_Queue_, errorIndex);

    QCOMPARE_NE(parsing_result, 0); // fail on parsing
    QCOMPARE(errorIndex, expectedErrorIndex); // index indicating error position
}

void KCalcParserCoreTest::testCalculationError_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("expectedErrorIndex");

    // invalid input //
    QTest::newRow("calculation error test 1") << QS("1××5") << 3;
    QTest::newRow("calculation error test 2") << QS("1+8×!+2") << 5;
    QTest::newRow("calculation error test 3") << QS("tan(%)+2+3") << 5;
}

void KCalcParserCoreTest::testCalculationError()
{
    QFETCH(QString, input);
    QFETCH(int, expectedErrorIndex);

    int parsing_result, calculation_result, errorIndex;
    parsing_result = parser.stringToTokenQueue(input, 10, token_Queue_, errorIndex);

    QCOMPARE(parsing_result, 0); // successful parsing

    calculation_result = core.calculate(token_Queue_, errorIndex);

    QCOMPARE_NE(calculation_result, 0); // fail on calculation

    errorIndex = token_Queue_.at(errorIndex).getStringIndex();

    QCOMPARE(errorIndex, expectedErrorIndex); // index indicating error position
}

void KCalcParserCoreTest::cleanupTestCase()
{
}

QTEST_GUILESS_MAIN(KCalcParserCoreTest)

#include "kcalc_parser_core_test.moc"
