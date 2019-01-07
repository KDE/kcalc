/*
Copyright (C) 2001 - 2013 Evan Teran
                          evan.teran@gmail.com

Copyright (C) 2003 - 2005 Klaus Niederkrueger
                          kniederk@math.uni-koeln.de

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "knumber.h"
#include <QString>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace {
const int precision = 12;

QString numtypeToString(int arg) {
	switch (arg) {
	case 0:
		return QStringLiteral("Special");
	case 1:
		return QStringLiteral("Integer");
	case 3:
		return QStringLiteral("Fraction");
	case 2:
		return QStringLiteral("Float");
	default:
		return QLatin1String("Unknown:") + QString::number(arg);
	}
}

void checkResult(const QString &string, const KNumber &result, const QString &desired_string, int desired) {

	std::cout
		<< "Testing result of: "
		<< qPrintable(string)
		<< " should give "
		<< qPrintable(desired_string)
		<< " and gives "
		<< qPrintable(result.toQString(precision))
		<< " ...\n";

	std::cout
		<< "The type of the result should be "
		<< qPrintable(numtypeToString(desired))
		<< " and gives "
		<< qPrintable(numtypeToString(result.type()))
		<< " ... ";

	if (result.type() == desired && result.toQString(precision) == desired_string) {
		std::cout << "OK\n";
		return;
	}

	std::cout << "Failed\n";
	exit(1);
}

void checkTruth(const QString &string, bool computation, bool desired_result) {

	std::cout
		<< "Testing truth of: "
		<< qPrintable(string)
		<< " should be "
		<< desired_result
		<< " and is "
		<< computation
		<< " ... ";

	if (computation == desired_result) {
		std::cout << "OK\n";
		return;
	}

	std::cout << "Failed\n";
	exit(1);
}

void checkType(const QString &string, int test_arg, int desired) {

	std::cout
		<< "Testing type of: "
		<< qPrintable(string)
		<< " should give "
		<< qPrintable(numtypeToString(desired))
		<< " and gives "
		<< qPrintable(numtypeToString(test_arg))
		<< " ...";

	if (test_arg == desired) {
		std::cout << "OK\n";
		return;
	}

	std::cout << "Failed\n";
	exit(1);
}

void testingCompare() {

	std::cout << "\n\n";
	std::cout << "Testing Compare:\n";
	std::cout << "----------------\n";

    checkTruth(QStringLiteral("KNumber(5) == KNumber(2)"), KNumber(5) == KNumber(2), false);
    checkTruth(QStringLiteral("KNumber(5) > KNumber(2)"), KNumber(5) > KNumber(2), true);
    checkTruth(QStringLiteral("KNumber(5) < KNumber(2)"), KNumber(5) < KNumber(2), false);
    checkTruth(QStringLiteral("KNumber(5) < KNumber(0)"), KNumber(5) < KNumber(0), false);
    checkTruth(QStringLiteral("KNumber(-5) < KNumber(0)"), KNumber(-5) < KNumber(0), true);
    checkTruth(QStringLiteral("KNumber(5) >= KNumber(2)"), KNumber(5) >= KNumber(2), true);
    checkTruth(QStringLiteral("KNumber(5) <= KNumber(2)"), KNumber(5) <= KNumber(2), false);
    checkTruth(QStringLiteral("KNumber(5) != KNumber(2)"), KNumber(5) != KNumber(2), true);

    checkTruth(QStringLiteral("KNumber(2) == KNumber(2)"), KNumber(2) == KNumber(2), true);
    checkTruth(QStringLiteral("KNumber(2) > KNumber(2)"), KNumber(2) > KNumber(2), false);
    checkTruth(QStringLiteral("KNumber(2) < KNumber(2)"), KNumber(2) < KNumber(2), false);
    checkTruth(QStringLiteral("KNumber(2) >= KNumber(2)"), KNumber(2) >= KNumber(2), true);
    checkTruth(QStringLiteral("KNumber(2) <= KNumber(2)"), KNumber(2) <= KNumber(2), true);
    checkTruth(QStringLiteral("KNumber(2) != KNumber(2)"), KNumber(2) != KNumber(2), false);

    checkTruth(QStringLiteral("KNumber(5) == KNumber(\"1/2\")"), KNumber(5) == KNumber(QStringLiteral("1/2")), false);
    checkTruth(QStringLiteral("KNumber(5) > KNumber(\"1/2\")"), KNumber(5) > KNumber(QStringLiteral("1/2")), true);
    checkTruth(QStringLiteral("KNumber(5) < KNumber(\"1/2\")"), KNumber(5) < KNumber(QStringLiteral("1/2")), false);
    checkTruth(QStringLiteral("KNumber(5) >= KNumber(\"1/2\")"), KNumber(5) >= KNumber(QStringLiteral("1/2")), true);
    checkTruth(QStringLiteral("KNumber(5) <= KNumber(\"1/2\")"), KNumber(5) <= KNumber(QStringLiteral("1/2")), false);
    checkTruth(QStringLiteral("KNumber(5) != KNumber(\"1/2\")"), KNumber(5) != KNumber(QStringLiteral("1/2")), true);

    checkTruth(QStringLiteral("KNumber(\"1/2\") == KNumber(\"1/2\")"), KNumber(QStringLiteral("1/2")) == KNumber(QStringLiteral("1/2")), true);
    checkTruth(QStringLiteral("KNumber(\"1/2\") > KNumber(\"1/2\")"), KNumber(QStringLiteral("1/2")) > KNumber(QStringLiteral("1/2")), false);
    checkTruth(QStringLiteral("KNumber(\"1/2\") < KNumber(\"1/2\")"), KNumber(QStringLiteral("1/2")) < KNumber(QStringLiteral("1/2")), false);
    checkTruth(QStringLiteral("KNumber(\"1/2\") >= KNumber(\"1/2\")"), KNumber(QStringLiteral("1/2")) >= KNumber(QStringLiteral("1/2")), true);
    checkTruth(QStringLiteral("KNumber(\"1/2\") <= KNumber(\"1/2\")"), KNumber(QStringLiteral("1/2")) <= KNumber(QStringLiteral("1/2")), true);
    checkTruth(QStringLiteral("KNumber(\"1/2\") != KNumber(\"1/2\")"), KNumber(QStringLiteral("1/2")) != KNumber(QStringLiteral("1/2")), false);

    checkTruth(QStringLiteral("KNumber(\"3/2\") == KNumber(\"1/2\")"), KNumber(QStringLiteral("3/2")) == KNumber(QStringLiteral("1/2")), false);
    checkTruth(QStringLiteral("KNumber(\"3/2\") > KNumber(\"1/2\")"), KNumber(QStringLiteral("3/2")) > KNumber(QStringLiteral("1/2")), true);
    checkTruth(QStringLiteral("KNumber(\"3/2\") < KNumber(\"1/2\")"), KNumber(QStringLiteral("3/2")) < KNumber(QStringLiteral("1/2")), false);
    checkTruth(QStringLiteral("KNumber(\"3/2\") >= KNumber(\"1/2\")"), KNumber(QStringLiteral("3/2")) >= KNumber(QStringLiteral("1/2")), true);
    checkTruth(QStringLiteral("KNumber(\"3/2\") <= KNumber(\"1/2\")"), KNumber(QStringLiteral("3/2")) <= KNumber(QStringLiteral("1/2")), false);
    checkTruth(QStringLiteral("KNumber(\"3/2\") != KNumber(\"1/2\")"), KNumber(QStringLiteral("3/2")) != KNumber(QStringLiteral("1/2")), true);

    checkTruth(QStringLiteral("KNumber(3.2) != KNumber(3)"), KNumber(3.2) != KNumber(3), true);
    checkTruth(QStringLiteral("KNumber(3.2) > KNumber(3)"), KNumber(3.2) > KNumber(3), true);
    checkTruth(QStringLiteral("KNumber(3.2) < KNumber(3)"), KNumber(3.2) < KNumber(3), false);

    checkTruth(QStringLiteral("KNumber(3.2) < KNumber(\"3/5\")"), KNumber(3.2) < KNumber(QStringLiteral("3/5")), false);
}


void testingAdditions() {

	std::cout << "\n\n";
	std::cout << "Testing additions:\n";
	std::cout << "------------------\n";

    checkResult(QStringLiteral("KNumber(5) + KNumber(2)"), KNumber(5) + KNumber(2), QStringLiteral("7"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(5) + KNumber(\"2/3\")"), KNumber(5) + KNumber(QStringLiteral("2/3")), QStringLiteral("17/3"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(5) + KNumber(\"2.3\")"), KNumber(5) + KNumber(QStringLiteral("2.3")), QStringLiteral("7.3"), KNumber::TYPE_FLOAT);

    checkResult(QStringLiteral("KNumber(\"5/3\") + KNumber(2)"), KNumber(QStringLiteral("5/3")) + KNumber(2), QStringLiteral("11/3"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(\"5/3\") + KNumber(\"2/3\")"), KNumber(QStringLiteral("5/3")) + KNumber(QStringLiteral("2/3")), QStringLiteral("7/3"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(\"5/3\") + KNumber(\"1/3\")"), KNumber(QStringLiteral("5/3")) + KNumber(QStringLiteral("1/3")), QStringLiteral("2"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"5/3\") + KNumber(\"-26/3\")"), KNumber(QStringLiteral("5/3")) + KNumber(QStringLiteral("-26/3")), QStringLiteral("-7"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"5/2\") + KNumber(2.3)"), KNumber(QStringLiteral("5/2")) + KNumber(2.3), QStringLiteral("4.8"), KNumber::TYPE_FLOAT);

    checkResult(QStringLiteral("KNumber(5.3) + KNumber(2)"), KNumber(5.3) + KNumber(2), QStringLiteral("7.3"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(5.3) + KNumber(\"2/4\")"), KNumber(5.3) + KNumber(QStringLiteral("2/4")), QStringLiteral("5.8"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(5.3) + KNumber(2.3)"), KNumber(5.3) + KNumber(2.3), QStringLiteral("7.6"), KNumber::TYPE_FLOAT);
}

void testingSubtractions() {

	std::cout << "\n\n";
	std::cout << "Testing subtractions:\n";
	std::cout << "---------------------\n";

    checkResult(QStringLiteral("KNumber(5) - KNumber(2)"), KNumber(5) - KNumber(2), QStringLiteral("3"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(5) - KNumber(\"2/3\")"), KNumber(5) - KNumber(QStringLiteral("2/3")), QStringLiteral("13/3"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(5) - KNumber(2.3)"), KNumber(5) - KNumber(2.3), QStringLiteral("2.7"), KNumber::TYPE_FLOAT);

    checkResult(QStringLiteral("KNumber(\"5/3\") - KNumber(2)"), KNumber(QStringLiteral("5/3")) - KNumber(2), QStringLiteral("-1/3"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(\"5/3\") - KNumber(\"1/3\")"), KNumber(QStringLiteral("5/3")) - KNumber(QStringLiteral("1/3")), QStringLiteral("4/3"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(\"5/3\") - KNumber(\"2/3\")"), KNumber(QStringLiteral("5/3")) - KNumber(QStringLiteral("2/3")), QStringLiteral("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"-5/3\") - KNumber(\"4/3\")"), KNumber(QStringLiteral("-5/3")) - KNumber(QStringLiteral("4/3")), QStringLiteral("-3"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"5/4\") - KNumber(2.2)"), KNumber(QStringLiteral("5/4")) - KNumber(2.2), QStringLiteral("-0.95"), KNumber::TYPE_FLOAT);

    checkResult(QStringLiteral("KNumber(5.3) - KNumber(2)"), KNumber(5.3) - KNumber(2), QStringLiteral("3.3"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(5.3) - KNumber(\"3/4\")"), KNumber(5.3) - KNumber(QStringLiteral("3/4")), QStringLiteral("4.55"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(5.3) - KNumber(2.3)"), KNumber(5.3) - KNumber(2.3), QStringLiteral("3"), KNumber::TYPE_INTEGER);
}

void testingSpecial() {

	std::cout << "\n\n";
	std::cout << "Testing special functions:\n";
	std::cout << "--------------------------\n";

    checkResult(QStringLiteral("log10(KNumber(5))"), log10(KNumber(5)), QStringLiteral("0.698970004336"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("log10(pow(KNumber(10), KNumber(308)))"), log10(pow(KNumber(10), KNumber(308))), QStringLiteral("308"), KNumber::TYPE_INTEGER);

	// TODO: enable this check once MPFR is commonly enabled
    // checkResult(QStringLiteral("log10(pow(KNumber(10), KNumber(309)))"), log10(pow(KNumber(10), KNumber(309))), QLatin1String("309"), KNumber::TYPE_INTEGER);

    checkResult(QStringLiteral("exp(KNumber(4.34))"), exp(KNumber(4.34)), QStringLiteral("76.7075393383"), KNumber::TYPE_FLOAT);
}

void testingTrig() {

	std::cout << "\n\n";
	std::cout << "Testing trig functions:\n";
	std::cout << "-----------------------\n";

    checkResult(QStringLiteral("sin(KNumber(5))"), sin(KNumber(5)), QStringLiteral("-0.958924274663"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("cos(KNumber(5))"), cos(KNumber(5)), QStringLiteral("0.283662185463"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("tan(KNumber(5))"), tan(KNumber(5)), QStringLiteral("-3.38051500625"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("sin(KNumber(-5))"), sin(KNumber(-5)), QStringLiteral("0.958924274663"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("cos(KNumber(-5))"), cos(KNumber(-5)), QStringLiteral("0.283662185463"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("tan(KNumber(-5))"), tan(KNumber(-5)), QStringLiteral("3.38051500625"), KNumber::TYPE_FLOAT);

    checkResult(QStringLiteral("sin(KNumber(\"5/2\"))"), sin(KNumber(QStringLiteral("5/2"))), QStringLiteral("0.598472144104"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("cos(KNumber(\"5/2\"))"), cos(KNumber(QStringLiteral("5/2"))), QStringLiteral("-0.801143615547"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("tan(KNumber(\"5/2\"))"), tan(KNumber(QStringLiteral("5/2"))), QStringLiteral("-0.747022297239"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("sin(KNumber(\"-5/2\"))"), sin(KNumber(QStringLiteral("-5/2"))), QStringLiteral("-0.598472144104"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("cos(KNumber(\"-5/2\"))"), cos(KNumber(QStringLiteral("-5/2"))), QStringLiteral("-0.801143615547"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("tan(KNumber(\"-5/2\"))"), tan(KNumber(QStringLiteral("-5/2"))), QStringLiteral("0.747022297239"), KNumber::TYPE_FLOAT);

    checkResult(QStringLiteral("sin(KNumber(5.3))"), sin(KNumber(5.3)), QStringLiteral("-0.832267442224"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("cos(KNumber(5.3))"), cos(KNumber(5.3)), QStringLiteral("0.554374336179"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("tan(KNumber(5.3))"), tan(KNumber(5.3)), QStringLiteral("-1.50127339581"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("sin(KNumber(-5.3))"), sin(KNumber(-5.3)), QStringLiteral("0.832267442224"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("cos(KNumber(-5.3))"), cos(KNumber(-5.3)), QStringLiteral("0.554374336179"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("tan(KNumber(-5.3))"), tan(KNumber(-5.3)), QStringLiteral("1.50127339581"), KNumber::TYPE_FLOAT);

    checkResult(QStringLiteral("asin(KNumber(5))"), asin(KNumber(5)), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("acos(KNumber(5))"), acos(KNumber(5)), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("atan(KNumber(5))"), atan(KNumber(5)), QStringLiteral("1.37340076695"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("asin(KNumber(-5))"), asin(KNumber(-5)), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("acos(KNumber(-5))"), acos(KNumber(-5)), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("atan(KNumber(-5))"), atan(KNumber(-5)), QStringLiteral("-1.37340076695"), KNumber::TYPE_FLOAT);

    checkResult(QStringLiteral("asin(KNumber(\"5/2\"))"), asin(KNumber(QStringLiteral("5/2"))), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("acos(KNumber(\"5/2\"))"), acos(KNumber(QStringLiteral("5/2"))), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("atan(KNumber(\"5/2\"))"), atan(KNumber(QStringLiteral("5/2"))), QStringLiteral("1.19028994968"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("asin(KNumber(\"-5/2\"))"), asin(KNumber(QStringLiteral("-5/2"))), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("acos(KNumber(\"-5/2\"))"), acos(KNumber(QStringLiteral("-5/2"))), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("atan(KNumber(\"-5/2\"))"), atan(KNumber(QStringLiteral("-5/2"))), QStringLiteral("-1.19028994968"), KNumber::TYPE_FLOAT);

    checkResult(QStringLiteral("asin(KNumber(5.3))"), asin(KNumber(5.3)), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("acos(KNumber(5.3))"), acos(KNumber(5.3)), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("atan(KNumber(5.3))"), atan(KNumber(5.3)), QStringLiteral("1.38430942513"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("asin(KNumber(-5.3))"), asin(KNumber(-5.3)), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("acos(KNumber(-5.3))"), acos(KNumber(-5.3)), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("atan(KNumber(-5.3))"), atan(KNumber(-5.3)), QStringLiteral("-1.38430942513"), KNumber::TYPE_FLOAT);


    checkResult(QStringLiteral("asin(KNumber(\"2/5\"))"), asin(KNumber(QStringLiteral("2/5"))), QStringLiteral("0.411516846067"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("acos(KNumber(\"2/5\"))"), acos(KNumber(QStringLiteral("2/5"))), QStringLiteral("1.15927948073"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("atan(KNumber(\"2/5\"))"), atan(KNumber(QStringLiteral("2/5"))), QStringLiteral("0.380506377112"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("asin(KNumber(\"-2/5\"))"), asin(KNumber(QStringLiteral("-2/5"))), QStringLiteral("-0.411516846067"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("acos(KNumber(\"-2/5\"))"), acos(KNumber(QStringLiteral("-2/5"))), QStringLiteral("1.98231317286"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("atan(KNumber(\"-2/5\"))"), atan(KNumber(QStringLiteral("-2/5"))), QStringLiteral("-0.380506377112"), KNumber::TYPE_FLOAT);

    checkResult(QStringLiteral("asin(KNumber(0.3))"), asin(KNumber(0.3)), QStringLiteral("0.304692654015"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("acos(KNumber(0.3))"), acos(KNumber(0.3)), QStringLiteral("1.26610367278"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("atan(KNumber(0.3))"), atan(KNumber(0.3)), QStringLiteral("0.291456794478"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("asin(KNumber(-0.3))"), asin(KNumber(-0.3)), QStringLiteral("-0.304692654015"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("acos(KNumber(-0.3))"), acos(KNumber(-0.3)), QStringLiteral("1.87548898081"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("atan(KNumber(-0.3))"), atan(KNumber(-0.3)), QStringLiteral("-0.291456794478"), KNumber::TYPE_FLOAT);
}


void testingMultiplications() {

	std::cout << "\n\n";
	std::cout << "Testing multiplications:\n";
	std::cout << "------------------------\n";

    checkResult(QStringLiteral("KNumber(5) * KNumber(2)"), KNumber(5) * KNumber(2), QStringLiteral("10"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(5) * KNumber(\"2/3\")"), KNumber(5) * KNumber(QStringLiteral("2/3")), QStringLiteral("10/3"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(5) * KNumber(\"2/5\")"), KNumber(5) * KNumber(QStringLiteral("2/5")), QStringLiteral("2"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(5) * KNumber(2.3)"), KNumber(5) * KNumber(2.3), QStringLiteral("11.5"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(0) * KNumber(\"2/5\")"), KNumber(0) * KNumber(QStringLiteral("2/5")), QStringLiteral("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(0) * KNumber(2.3)"), KNumber(0) * KNumber(2.3), QStringLiteral("0"), KNumber::TYPE_INTEGER);

    checkResult(QStringLiteral("KNumber(\"5/3\") * KNumber(2)"), KNumber(QStringLiteral("5/3")) * KNumber(2), QStringLiteral("10/3"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(\"5/3\") * KNumber(0)"), KNumber(QStringLiteral("5/3")) * KNumber(0), QStringLiteral("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"5/3\") * KNumber(\"2/3\")"), KNumber(QStringLiteral("5/3")) * KNumber(QStringLiteral("2/3")), QStringLiteral("10/9"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(\"25/6\") * KNumber(\"12/5\")"), KNumber(QStringLiteral("25/6")) * KNumber(QStringLiteral("12/5")), QStringLiteral("10"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"5/2\") * KNumber(2.3)"), KNumber(QStringLiteral("5/2")) * KNumber(2.3), QStringLiteral("5.75"), KNumber::TYPE_FLOAT);

    checkResult(QStringLiteral("KNumber(5.3) * KNumber(2)"), KNumber(5.3) * KNumber(2), QStringLiteral("10.6"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(5.3) * KNumber(0)"), KNumber(5.3) * KNumber(0), QStringLiteral("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(5.3) * KNumber(\"1/2\")"), KNumber(5.3) * KNumber(QStringLiteral("1/2")), QStringLiteral("2.65"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(5.3) * KNumber(2.3)"), KNumber(5.3) * KNumber(2.3), QStringLiteral("12.19"), KNumber::TYPE_FLOAT);
}

void testingDivisions() {

	std::cout << "\n\n";
	std::cout << "Testing divisions:\n";
	std::cout << "------------------\n";

    checkResult(QStringLiteral("KNumber(5) / KNumber(2)"), KNumber(5) / KNumber(2), QStringLiteral("5/2"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(122) / KNumber(2)"), KNumber(122) / KNumber(2), QStringLiteral("61"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(12) / KNumber(0)"), KNumber(12) / KNumber(0), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-12) / KNumber(0)"), KNumber(-12) / KNumber(0), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(5) / KNumber(\"2/3\")"), KNumber(5) / KNumber(QStringLiteral("2/3")), QStringLiteral("15/2"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(6) / KNumber(\"2/3\")"), KNumber(6) / KNumber(QStringLiteral("2/3")), QStringLiteral("9"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(5) / KNumber(2.5)"), KNumber(5) / KNumber(2.5), QStringLiteral("2"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(5) / KNumber(0.0)"), KNumber(5) / KNumber(0.0), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5) / KNumber(0.0)"), KNumber(-5) / KNumber(0.0), QStringLiteral("nan"), KNumber::TYPE_ERROR);

    checkResult(QStringLiteral("KNumber(\"5/3\") / KNumber(2)"), KNumber(QStringLiteral("5/3")) / KNumber(2), QStringLiteral("5/6"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(\"5/3\") / KNumber(0)"), KNumber(QStringLiteral("5/3")) / KNumber(0), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-5/3\") / KNumber(0)"), KNumber(QStringLiteral("-5/3")) / KNumber(0), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"5/3\") / KNumber(\"2/3\")"), KNumber(QStringLiteral("5/3")) / KNumber(QStringLiteral("2/3")), QStringLiteral("5/2"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(\"49/3\") / KNumber(\"7/9\")"), KNumber(QStringLiteral("49/3")) / KNumber(QStringLiteral("7/9")), QStringLiteral("21"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"5/2\") / KNumber(2.5)"), KNumber(QStringLiteral("5/2")) / KNumber(2.5), QStringLiteral("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"5/2\") / KNumber(0.0)"), KNumber(QStringLiteral("5/2")) / KNumber(0.0), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-5/2\") / KNumber(0.0)"), KNumber(QStringLiteral("-5/2")) / KNumber(0.0), QStringLiteral("nan"), KNumber::TYPE_ERROR);

    checkResult(QStringLiteral("KNumber(5.3) / KNumber(2)"), KNumber(5.3) / KNumber(2), QStringLiteral("2.65"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(5.3) / KNumber(0)"), KNumber(5.3) / KNumber(0), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5.3) / KNumber(0)"), KNumber(-5.3) / KNumber(0), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(5.3) / KNumber(\"2/3\")"), KNumber(5.3) / KNumber(QStringLiteral("2/3")), QStringLiteral("7.95"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(5.5) / KNumber(2.5)"), KNumber(5.5) / KNumber(2.5), QStringLiteral("2.2"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(5.5) / KNumber(0.0)"), KNumber(5.5) / KNumber(0.0), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5.5) / KNumber(0.0)"), KNumber(-5.5) / KNumber(0.0), QStringLiteral("nan"), KNumber::TYPE_ERROR);
}

void testingModulus() {

	std::cout << "\n\n";
	std::cout << "Testing modulus:\n";
	std::cout << "----------------\n";

    checkResult(QStringLiteral("KNumber(23) % KNumber(4)"), KNumber(23) % KNumber(4), QStringLiteral("3"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(12) % KNumber(-5)"), KNumber(12) % KNumber(-5), QStringLiteral("2"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(-12) % KNumber(5)"), KNumber(-12) % KNumber(5), QStringLiteral("3"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(12) % KNumber(0)"), KNumber(-12) % KNumber(0), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-12) % KNumber(0)"), KNumber(-12) % KNumber(0), QStringLiteral("nan"), KNumber::TYPE_ERROR);

#ifdef __GNUC__
	#warning test for other types
#endif
}

void testingAndOr() {

	std::cout << "\n\n";
	std::cout << "Testing And/Or:\n";
	std::cout << "---------------\n";

    checkResult(QStringLiteral("KNumber(17) & KNumber(9)"), KNumber(17) & KNumber(9), QStringLiteral("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(17) | KNumber(9)"), KNumber(17) | KNumber(9), QStringLiteral("25"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(1023) & KNumber(255)"), KNumber(1023) & KNumber(255), QStringLiteral("255"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(1023) | KNumber(255)"), KNumber(1023) | KNumber(255), QStringLiteral("1023"), KNumber::TYPE_INTEGER);

#ifdef __GNUC__
	#warning test for other types
#endif
}


void testingAbs() {

	std::cout << "\n\n";
	std::cout << "Testing absolute value:\n";
	std::cout << "-----------------------\n";

    checkResult(QStringLiteral("KNumber(5).abs()"), KNumber(5).abs(), QStringLiteral("5"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"2/3\").abs()"), KNumber(QStringLiteral("2/3")).abs(), QStringLiteral("2/3"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(\"2.3\").abs()"), KNumber(QStringLiteral("2.3")).abs(), QStringLiteral("2.3"), KNumber::TYPE_FLOAT);

    checkResult(QStringLiteral("KNumber(-5).abs()"), KNumber(-5).abs(), QStringLiteral("5"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"-2/3\").abs()"), KNumber(QStringLiteral("-2/3")).abs(), QStringLiteral("2/3"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(\"-2.3\").abs()"), KNumber(QStringLiteral("-2.3")).abs(), QStringLiteral("2.3"), KNumber::TYPE_FLOAT);
}

void testingTruncateToInteger() {

	std::cout << "\n\n";
	std::cout << "Testing truncate to an integer:\n";
	std::cout << "-------------------------------\n";

    checkResult(QStringLiteral("KNumber(16).integerPart()"), KNumber(16).integerPart(), QStringLiteral("16"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"43/9\").integerPart()"), KNumber(QStringLiteral("43/9")).integerPart(), QStringLiteral("4"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"-43/9\").integerPart()"), KNumber(QStringLiteral("-43/9")).integerPart(), QStringLiteral("-4"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"5.25\").integerPart()"), KNumber(QStringLiteral("5.25")).integerPart(), QStringLiteral("5"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"-5.25\").integerPart()"), KNumber(QStringLiteral("-5.25")).integerPart(), QStringLiteral("-5"), KNumber::TYPE_INTEGER);
}


void testingSqrt() {

	std::cout << "\n\n";
	std::cout << "Testing square root, cubic root:\n";
	std::cout << "--------------------------------\n";

    checkResult(QStringLiteral("KNumber(16).sqrt()"), KNumber(16).sqrt(), QStringLiteral("4"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(-16).sqrt()"), KNumber(-16).sqrt(), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"16/9\").sqrt()"), KNumber(QStringLiteral("16/9")).sqrt(), QStringLiteral("4/3"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(\"-16/9\").sqrt()"), KNumber(QStringLiteral("-16/9")).sqrt(), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(2).sqrt()"), KNumber(2).sqrt(), QStringLiteral("1.41421356237"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(\"2/3\").sqrt()"), KNumber(QStringLiteral("2/3")).sqrt(), QStringLiteral("0.816496580928"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(\"0.25\").sqrt()"), KNumber(QStringLiteral("0.25")).sqrt(), QStringLiteral("0.5"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(\"-0.25\").sqrt()"), KNumber(QStringLiteral("-0.25")).sqrt(), QStringLiteral("nan"), KNumber::TYPE_ERROR);


    checkResult(QStringLiteral("KNumber(27).cbrt()"), KNumber(27).cbrt(), QStringLiteral("3"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(-27).cbrt()"), KNumber(-27).cbrt(), QStringLiteral("-3"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"27/8\").cbrt()"), KNumber(QStringLiteral("27/8")).cbrt(), QStringLiteral("3/2"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(\"-8/27\").cbrt()"), KNumber(QStringLiteral("-8/27")).cbrt(), QStringLiteral("-2/3"), KNumber::TYPE_FRACTION);
#ifdef __GNUC__
	#warning need to check non-perfect cube roots
#endif
    checkResult(QStringLiteral("KNumber(2).cbrt()"), KNumber(2).cbrt(), QStringLiteral("1.25992104989"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(\"2/3\").cbrt()"), KNumber(QStringLiteral("2/3")).cbrt(), QStringLiteral("0.873580464736"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(\"0.25\").cbrt()"), KNumber(QStringLiteral("0.25")).cbrt(), QStringLiteral("0.629960524947"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(\"-0.25\").cbrt()"), KNumber(QStringLiteral("-0.25")).cbrt(), QStringLiteral("-0.629960524947"), KNumber::TYPE_FLOAT);

}

void testingFactorial() {

	std::cout << "\n\n";
	std::cout << "Testing factorial:\n";
	std::cout << "------------------\n";

    checkResult(QStringLiteral("KNumber(-1).factorial()"), KNumber(-1).factorial(), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-2).factorial()"), KNumber(-2).factorial(), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-20).factorial()"), KNumber(-20).factorial(), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-1/2).factorial()"), KNumber(QStringLiteral("-1/2")).factorial(), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-0.5).factorial()"), KNumber(QStringLiteral("-0.5")).factorial(), QStringLiteral("nan"), KNumber::TYPE_ERROR);

    checkResult(QStringLiteral("KNumber(0).factorial()"), KNumber(0).factorial(), QStringLiteral("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(1).factorial()"), KNumber(1).factorial(), QStringLiteral("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(2).factorial()"), KNumber(2).factorial(), QStringLiteral("2"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(3).factorial()"), KNumber(3).factorial(), QStringLiteral("6"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(4).factorial()"), KNumber(4).factorial(), QStringLiteral("24"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(5).factorial()"), KNumber(5).factorial(), QStringLiteral("120"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(6).factorial()"), KNumber(6).factorial(), QStringLiteral("720"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(9).factorial()"), KNumber(9).factorial(), QStringLiteral("362880"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(12).factorial()"), KNumber(12).factorial(), QStringLiteral("479001600"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(13).factorial()"), KNumber(13).factorial(), QStringLiteral("6227020800"), KNumber::TYPE_INTEGER);

    checkResult(QStringLiteral("KNumber(1/2).factorial()"), KNumber(QStringLiteral("1/2")).factorial(), QStringLiteral("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(2/1).factorial()"), KNumber(QStringLiteral("2/1")).factorial(), QStringLiteral("2"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(3/2).factorial()"), KNumber(QStringLiteral("3/2")).factorial(), QStringLiteral("1"), KNumber::TYPE_INTEGER);

    checkResult(QStringLiteral("KNumber(0.1).factorial()"), KNumber(0.1).factorial(), QStringLiteral("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(0.5).factorial()"), KNumber(0.5).factorial(), QStringLiteral("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(1.5).factorial()"), KNumber(1.5).factorial(), QStringLiteral("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(2.5).factorial()"), KNumber(2.5).factorial(), QStringLiteral("2"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(3.5).factorial()"), KNumber(3.5).factorial(), QStringLiteral("6"), KNumber::TYPE_INTEGER);
}

void testingComplement() {
	std::cout << "\n\n";
	std::cout << "Testing complement:\n";
	std::cout << "-------------------\n";

	// at first glance, these look like they should work
	// but there is an annoyance. If we use the mpz_com function
	// ~-2 == 1, but the HEX/OCT/BIN views are broken :-(
	// specifically, if the value is negative, it goes badly pretty quick..
#if 0
    checkResult(QStringLiteral("~KNumber(0)"), ~KNumber(0), QLatin1String("-1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("~KNumber(1)"), ~KNumber(1), QLatin1String("-2"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("~KNumber(2)"), ~KNumber(2), QLatin1String("-3"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("~KNumber(8)"), ~KNumber(8), QLatin1String("-9"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("~KNumber(15)"), ~KNumber(15), QLatin1String("-16"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("~KNumber(-1)"), ~KNumber(-1), QLatin1String("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("~KNumber(-2)"), ~KNumber(-2), QLatin1String("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("~KNumber(-3)"), ~KNumber(-3), QLatin1String("2"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("~KNumber(-9)"), ~KNumber(-9), QLatin1String("8"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("~KNumber(-16)"), ~KNumber(-16), QLatin1String("15"), KNumber::TYPE_INTEGER);
#endif

    checkResult(QStringLiteral("~KNumber(0.12345)"), ~KNumber(0.12345), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("~KNumber(-0.12345)"), ~KNumber(-0.12345), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("~KNumber(\"1/2\")"), ~KNumber(QStringLiteral("1/2")), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("~KNumber(\"-1/2\")"), ~KNumber(QStringLiteral("-1/2")), QStringLiteral("nan"), KNumber::TYPE_ERROR);
}

void testingShifts() {

	std::cout << "\n\n";
	std::cout << "Testing left/right shift:\n";
	std::cout << "-------------------------\n";

    checkResult(QStringLiteral("KNumber(16) << KNumber(2)"), KNumber(16) << KNumber(2), QStringLiteral("64"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(16) >> KNumber(2)"), KNumber(16) >> KNumber(2), QStringLiteral("4"), KNumber::TYPE_INTEGER);
}

void testingPower() {

	std::cout << "\n\n";
	std::cout << "Testing Power:\n";
	std::cout << "--------------\n";

    checkResult(QStringLiteral("KNumber(0) ^ KNumber(0)"), KNumber(0).pow(KNumber(0)), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(0) ^ KNumber(-4)"), KNumber(0).pow(KNumber(-4)), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(5) ^ KNumber(4)"), KNumber(5).pow(KNumber(4)), QStringLiteral("625"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(122) ^ KNumber(0)"), KNumber(122).pow(KNumber(0)), QStringLiteral("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(-5) ^ KNumber(0)"), KNumber(-5).pow(KNumber(0)), QStringLiteral("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(-2) ^ KNumber(3)"), KNumber(-2).pow(KNumber(3)), QStringLiteral("-8"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(-2) ^ KNumber(4)"), KNumber(-2).pow(KNumber(4)), QStringLiteral("16"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(5) ^ KNumber(-2)"), KNumber(5).pow(KNumber(-2)), QStringLiteral("1/25"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(8) ^ KNumber(\"2/3\")"), KNumber(8).pow(KNumber(QStringLiteral("2/3"))), QStringLiteral("4"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(8) ^ KNumber(\"-2/3\")"), KNumber(8).pow(KNumber(QStringLiteral("-2/3"))), QStringLiteral("1/4"), KNumber::TYPE_FRACTION);

    checkResult(QStringLiteral("KNumber(-16) ^ KNumber(\"1/4\")"), KNumber(-16).pow(KNumber(QStringLiteral("1/4"))), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-8) ^ KNumber(\"1/3\")"), KNumber(-8).pow(KNumber(QStringLiteral("1/3"))), QStringLiteral("-2"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(5) ^ KNumber(0.0)"), KNumber(5).pow(KNumber(0.0)), QStringLiteral("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(-5) ^ KNumber(0.0)"), KNumber(-5).pow(KNumber(0.0)), QStringLiteral("1"), KNumber::TYPE_INTEGER);

    checkResult(QStringLiteral("KNumber(\"5/3\") ^ KNumber(2)"), KNumber(QStringLiteral("5/3")).pow(KNumber(2)), QStringLiteral("25/9"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(\"5/3\") ^ KNumber(0)"), KNumber(QStringLiteral("5/3")).pow(KNumber(0)), QStringLiteral("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"-5/3\") ^ KNumber(0)"), KNumber(QStringLiteral("-5/3")).pow(KNumber(0)), QStringLiteral("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"8/27\") ^ KNumber(\"2/3\")"), KNumber(QStringLiteral("8/27")).pow(KNumber(QStringLiteral("2/3"))), QStringLiteral("4/9"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(\"49/3\") ^ KNumber(\"7/9\")"), KNumber(QStringLiteral("49/3")).pow(KNumber(QStringLiteral("7/9"))), QStringLiteral("8.78016428243"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(\"5/2\") ^ KNumber(2.5)"), KNumber(QStringLiteral("5/2")).pow(KNumber(2.5)), QStringLiteral("9.88211768803"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(\"5/2\") ^ KNumber(0.0)"), KNumber(QStringLiteral("5/2")).pow(KNumber(0.0)), QStringLiteral("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"-5/2\") ^ KNumber(0.0)"), KNumber(QStringLiteral("-5/2")).pow(KNumber(0.0)), QStringLiteral("1"), KNumber::TYPE_INTEGER);

    checkResult(QStringLiteral("KNumber(5.3) ^ KNumber(2)"), KNumber(5.3).pow(KNumber(2)), QStringLiteral("28.09"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(5.3) ^ KNumber(0)"), KNumber(5.3).pow(KNumber(0)), QStringLiteral("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(-5.3) ^ KNumber(0)"), KNumber(-5.3).pow(KNumber(0)), QStringLiteral("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(5.3) ^ KNumber(\"2/3\")"), KNumber(5.3).pow(KNumber(QStringLiteral("2/3"))), QStringLiteral("3.03983898039"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(5.5) ^ KNumber(2.5)"), KNumber(5.5).pow(KNumber(2.5)), QStringLiteral("70.9425383673"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(5.5) ^ KNumber(0.0)"), KNumber(5.5).pow(KNumber(0.0)), QStringLiteral("1"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(-5.5) ^ KNumber(0.0)"), KNumber(-5.5).pow(KNumber(0.0)), QStringLiteral("1"), KNumber::TYPE_INTEGER);

    checkResult(QStringLiteral("KNumber::Pi() ^ KNumber::Pi()"), KNumber::Pi().pow(KNumber::Pi()), QStringLiteral("36.4621596072"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber::Euler() ^ KNumber::Pi()"), KNumber::Euler().pow(KNumber::Pi()), QStringLiteral("23.1406926328"), KNumber::TYPE_FLOAT);


    checkResult(QStringLiteral("KNumber(2.0) ^ KNumber(0.5)"), KNumber(2.0).pow(KNumber(0.5)), QStringLiteral("1.41421356237"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber(2.0) ^ KNumber(-0.5)"), KNumber(2.0).pow(KNumber(-0.5)), QStringLiteral("0.707106781187"), KNumber::TYPE_FLOAT);


    checkResult(QStringLiteral("KNumber(-2.0).exp()"), KNumber(-2.0).exp(), QStringLiteral("0.135335283237"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber::Euler() ^ KNumber(-2.0)"), KNumber::Euler().pow(KNumber(-2.0)), QStringLiteral("0.135335283237"), KNumber::TYPE_FLOAT);


    checkResult(QStringLiteral("KNumber(2.0).exp()"), KNumber(2.0).exp(), QStringLiteral("7.38905609893"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("KNumber::Euler() ^ KNumber(2.0)"), KNumber::Euler().pow(KNumber(2.0)), QStringLiteral("7.38905609893"), KNumber::TYPE_FLOAT);

	// TODO: kinda odd that this ends up being an integer
	// i guess since my euler constant is only 100 digits, we've exceeded the fractional part
    checkResult(QStringLiteral("KNumber::Euler() ^ 1000"), KNumber::Euler().pow(KNumber(1000)), QStringLiteral("1.97007111402e+434"), KNumber::TYPE_INTEGER);

	// TODO: make this test pass
	// the problem is that it is using the libc exp function which has limits that GMP does not
	// we should basically make this equivalent to KNumber::Euler().pow(KNumber(1000))
	// which does work
#if 0
    checkResult(QStringLiteral("KNumber(1000).exp()"), KNumber(1000).exp(), QLatin1String("23.1406926328"), KNumber::TYPE_FLOAT);
#endif

	KNumber::setDefaultFractionalInput(true);
    checkResult(QStringLiteral("KNumber(\"3.1415926\") ^ KNumber(\"3.1415926\")"), KNumber(QStringLiteral("3.1415926")).pow(KNumber(QStringLiteral("3.1415926"))), QStringLiteral("36.4621554164"), KNumber::TYPE_FLOAT);
	KNumber::setDefaultFractionalInput(false);
}

void testingInfArithmetic() {

	std::cout << "\n\n";
	std::cout << "Testing inf/nan-arithmetics:\n";
	std::cout << "----------------------------\n";

    checkResult(QStringLiteral("inf + KNumber(2)"), KNumber::PosInfinity + KNumber(2), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5) + inf"), KNumber(-5) + KNumber::PosInfinity, QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf + KNumber(\"1/2\")"), KNumber::PosInfinity + KNumber(QStringLiteral("1/2")), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-5/3\") + inf"), KNumber(QStringLiteral("-5/3")) + KNumber::PosInfinity, QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf + KNumber(2.01)"), KNumber::PosInfinity + KNumber(2.01), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5.4) + inf"), KNumber(-5.4) + KNumber::PosInfinity, QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf + KNumber(2)"), KNumber::NegInfinity + KNumber(2), QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5) + -inf"), KNumber(-5) + KNumber::NegInfinity, QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf + KNumber(\"1/2\")"), KNumber::NegInfinity + KNumber(QStringLiteral("1/2")), QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-5/3\") + -inf"), KNumber(QStringLiteral("-5/3")) + KNumber::NegInfinity, QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf + KNumber(2.01)"), KNumber::NegInfinity + KNumber(2.01), QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5.4) + -inf"), KNumber(-5.4) + KNumber::NegInfinity, QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("nan + KNumber(2)"), KNumber::NaN + KNumber(2), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5) + nan"), KNumber(-5) + KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("nan + KNumber(\"1/2\")"), KNumber::NaN + KNumber(QStringLiteral("1/2")), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-5/3\") + nan"), KNumber(QStringLiteral("-5/3")) + KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("nan + KNumber(2.01)"), KNumber::NaN + KNumber(2.01), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5.4) + nan"), KNumber(-5.4) + KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf + inf"), KNumber::PosInfinity + KNumber::PosInfinity, QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf + -inf"), KNumber::PosInfinity + KNumber::NegInfinity, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf + inf"), KNumber::NegInfinity + KNumber::PosInfinity, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf + -inf"), KNumber::NegInfinity + KNumber::NegInfinity, QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf + nan"), KNumber::PosInfinity + KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf + nan"), KNumber::NegInfinity + KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("nan + inf"), KNumber::NaN + KNumber::PosInfinity, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf + nan"), KNumber::NegInfinity + KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);

    checkResult(QStringLiteral("inf - KNumber(2)"), KNumber::PosInfinity - KNumber(2), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5) - inf"), KNumber(-5) - KNumber::PosInfinity, QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf - KNumber(\"1/2\")"), KNumber::PosInfinity - KNumber(QStringLiteral("1/2")), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-5/3\") - inf"), KNumber(QStringLiteral("-5/3")) - KNumber::PosInfinity, QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf - KNumber(2.01)"), KNumber::PosInfinity - KNumber(2.01), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5.4) - inf"), KNumber(-5.4) - KNumber::PosInfinity, QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf - KNumber(2)"), KNumber::NegInfinity - KNumber(2), QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5) - -inf"), KNumber(-5) - KNumber::NegInfinity, QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf - KNumber(\"1/2\")"), KNumber::NegInfinity - KNumber(QStringLiteral("1/2")), QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-5/3\") - -inf"), KNumber(QStringLiteral("-5/3")) - KNumber::NegInfinity, QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf - KNumber(2.01)"), KNumber::NegInfinity - KNumber(2.01), QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5.4) - -inf"), KNumber(-5.4) - KNumber::NegInfinity, QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("nan - KNumber(2)"), KNumber::NaN - KNumber(2), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5) - nan"), KNumber(-5) - KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("nan - KNumber(\"1/2\")"), KNumber::NaN - KNumber(QStringLiteral("1/2")), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-5/3\") - nan"), KNumber(QStringLiteral("-5/3")) - KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("nan - KNumber(2.01)"), KNumber::NaN - KNumber(2.01), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5.4) - nan"), KNumber(-5.4) - KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf - inf"), KNumber::PosInfinity - KNumber::PosInfinity, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf - -inf"), KNumber::PosInfinity - KNumber::NegInfinity, QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf - inf"), KNumber::NegInfinity - KNumber::PosInfinity, QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf - -inf"), KNumber::NegInfinity - KNumber::NegInfinity, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf - nan"), KNumber::PosInfinity - KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf - nan"), KNumber::NegInfinity - KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("nan - inf"), KNumber::NaN - KNumber::PosInfinity, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf - nan"), KNumber::NegInfinity - KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);

    checkResult(QStringLiteral("inf * KNumber(2)"), KNumber::PosInfinity * KNumber(2), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5) * inf"), KNumber(-5) * KNumber::PosInfinity, QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf * KNumber(\"1/2\")"), KNumber::PosInfinity * KNumber(QStringLiteral("1/2")), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-5/3\") * inf"), KNumber(QStringLiteral("-5/3")) * KNumber::PosInfinity, QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf * KNumber(2.01)"), KNumber::PosInfinity * KNumber(2.01), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5.4) * inf"), KNumber(-5.4) * KNumber::PosInfinity, QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf * KNumber(2)"), KNumber::NegInfinity * KNumber(2), QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5) * -inf"), KNumber(-5) * KNumber::NegInfinity, QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf * KNumber(\"1/2\")"), KNumber::NegInfinity * KNumber(QStringLiteral("1/2")), QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-5/3\") * -inf"), KNumber(QStringLiteral("-5/3")) * KNumber::NegInfinity, QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf * KNumber(2.01)"), KNumber::NegInfinity * KNumber(2.01), QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5.4) * -inf"), KNumber(-5.4) * KNumber::NegInfinity, QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("nan * KNumber(2)"), KNumber::NaN * KNumber(2), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5) * nan"), KNumber(-5) * KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("nan * KNumber(\"1/2\")"), KNumber::NaN * KNumber(QStringLiteral("1/2")), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-5/3\") * nan"), KNumber(QStringLiteral("-5/3")) * KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("nan * KNumber(2.01)"), KNumber::NaN * KNumber(2.01), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5.4) * nan"), KNumber(-5.4) * KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf * inf"), KNumber::PosInfinity * KNumber::PosInfinity, QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf * -inf"), KNumber::PosInfinity * KNumber::NegInfinity, QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf * inf"), KNumber::NegInfinity * KNumber::PosInfinity, QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf * -inf"), KNumber::NegInfinity * KNumber::NegInfinity, QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf * nan"), KNumber::PosInfinity * KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf * nan"), KNumber::NegInfinity * KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("nan * inf"), KNumber::NaN * KNumber::PosInfinity, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf * nan"), KNumber::NegInfinity * KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(0) * inf"), KNumber(0) * KNumber::PosInfinity, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(0) * -inf"), KNumber(0) * KNumber::NegInfinity, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf * KNumber(0)"), KNumber::PosInfinity * KNumber(0), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf * KNumber(0)"), KNumber::NegInfinity * KNumber(0), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(0.0) * inf"), KNumber(0.0) * KNumber::PosInfinity, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(0.0) * -inf"), KNumber(0.0) * KNumber::NegInfinity, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf * KNumber(0.0)"), KNumber::PosInfinity * KNumber(0.0), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf * KNumber(0.0)"), KNumber::NegInfinity * KNumber(0.0), QStringLiteral("nan"), KNumber::TYPE_ERROR);

    checkResult(QStringLiteral("inf / KNumber(2)"), KNumber::PosInfinity / KNumber(2), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5) / inf"), KNumber(-5) / KNumber::PosInfinity, QStringLiteral("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("inf / KNumber(\"1/2\")"), KNumber::PosInfinity / KNumber(QStringLiteral("1/2")), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-5/3\") / inf"), KNumber(QStringLiteral("-5/3")) / KNumber::PosInfinity, QStringLiteral("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("inf / KNumber(2.01)"), KNumber::PosInfinity / KNumber(2.01), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5.4) / inf"), KNumber(-5.4) / KNumber::PosInfinity, QStringLiteral("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("-inf / KNumber(2)"), KNumber::NegInfinity / KNumber(2), QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5) / -inf"), KNumber(-5) / KNumber::NegInfinity, QStringLiteral("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("-inf / KNumber(\"1/2\")"), KNumber::NegInfinity / KNumber(QStringLiteral("1/2")), QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-5/3\") / -inf"), KNumber(QStringLiteral("-5/3")) / KNumber::NegInfinity, QStringLiteral("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("-inf / KNumber(2.01)"), KNumber::NegInfinity / KNumber(2.01), QStringLiteral("-inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5.4) / -inf"), KNumber(-5.4) / KNumber::NegInfinity, QStringLiteral("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("nan / KNumber(2)"), KNumber::NaN / KNumber(2), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5) / nan"), KNumber(-5) / KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("nan / KNumber(\"1/2\")"), KNumber::NaN / KNumber(QStringLiteral("1/2")), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-5/3\") / nan"), KNumber(QStringLiteral("-5/3")) / KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("nan / KNumber(2.01)"), KNumber::NaN / KNumber(2.01), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5.4) / nan"), KNumber(-5.4) / KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf / inf"), KNumber::PosInfinity / KNumber::PosInfinity, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf / -inf"), KNumber::PosInfinity / KNumber::NegInfinity, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf / inf"), KNumber::NegInfinity / KNumber::PosInfinity, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf / -inf"), KNumber::NegInfinity / KNumber::NegInfinity, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("inf / nan"), KNumber::PosInfinity / KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf / nan"), KNumber::NegInfinity / KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("nan / inf"), KNumber::NaN / KNumber::PosInfinity, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf / nan"), KNumber::NegInfinity / KNumber::NaN, QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(0) / inf"), KNumber(0) / KNumber::PosInfinity, QStringLiteral("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(0) / -inf"), KNumber(0) / KNumber::NegInfinity, QStringLiteral("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("inf / KNumber(0)"), KNumber::PosInfinity / KNumber(0), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf / KNumber(0)"), KNumber::NegInfinity / KNumber(0), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(0.0) / inf"), KNumber(0.0) / KNumber::PosInfinity, QStringLiteral("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(0.0) / -inf"), KNumber(0.0) / KNumber::NegInfinity, QStringLiteral("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("inf / KNumber(0.0)"), KNumber::PosInfinity / KNumber(0.0), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("-inf / KNumber(0.0)"), KNumber::NegInfinity / KNumber(0.0), QStringLiteral("nan"), KNumber::TYPE_ERROR);

    checkResult(QStringLiteral("KNumber(5) ^ KNumber(\"inf\")"), KNumber(5).pow(KNumber::PosInfinity), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5) ^ KNumber(\"inf\")"), KNumber(-5).pow(KNumber::PosInfinity), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"5/2\") ^ KNumber(\"inf\")"), KNumber(QStringLiteral("5/2")).pow(KNumber::PosInfinity), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-5/2\") ^ KNumber(\"inf\")"), KNumber(QStringLiteral("-5/2")).pow(KNumber::PosInfinity), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"5.2\") ^ KNumber(\"inf\")"), KNumber(QStringLiteral("5.2")).pow(KNumber::PosInfinity), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-5.2\") ^ KNumber(\"inf\")"), KNumber(QStringLiteral("-5.2")).pow(KNumber::PosInfinity), QStringLiteral("inf"), KNumber::TYPE_ERROR);

    checkResult(QStringLiteral("KNumber(5) ^ KNumber(\"-inf\")"), KNumber(5).pow(KNumber::NegInfinity), QStringLiteral("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(-5) ^ KNumber(\"-inf\")"), KNumber(-5).pow(KNumber::NegInfinity), QStringLiteral("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"5/2\") ^ KNumber(\"-inf\")"), KNumber(QStringLiteral("5/2")).pow(KNumber::NegInfinity), QStringLiteral("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"-5/2\") ^ KNumber(\"-inf\")"), KNumber(QStringLiteral("-5/2")).pow(KNumber::NegInfinity), QStringLiteral("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"5.2\") ^ KNumber(\"-inf\")"), KNumber(QStringLiteral("5.2")).pow(KNumber::NegInfinity), QStringLiteral("0"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"-5.2\") ^ KNumber(\"-inf\")"), KNumber(QStringLiteral("-5.2")).pow(KNumber::NegInfinity), QStringLiteral("0"), KNumber::TYPE_INTEGER);

    checkResult(QStringLiteral("KNumber(5) ^ KNumber(\"nan\")"), KNumber(5).pow(KNumber::NaN), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(-5) ^ KNumber(\"nan\")"), KNumber(-5).pow(KNumber::NaN), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"5/2\") ^ KNumber(\"nan\")"), KNumber(QStringLiteral("5/2")).pow(KNumber::NaN), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-5/2\") ^ KNumber(\"nan\")"), KNumber(QStringLiteral("-5/2")).pow(KNumber::NaN), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"5.2\") ^ KNumber(\"nan\")"), KNumber(QStringLiteral("5.2")).pow(KNumber::NaN), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-5.2\") ^ KNumber(\"nan\")"), KNumber(QStringLiteral("-5.2")).pow(KNumber::NaN), QStringLiteral("nan"), KNumber::TYPE_ERROR);


    checkResult(QStringLiteral("KNumber(\"nan\") ^ KNumber(\"nan\")"), KNumber::NaN.pow(KNumber::NaN), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"nan\") ^ KNumber(\"inf\")"), KNumber::NaN.pow(KNumber::PosInfinity), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"nan\") ^ KNumber(\"-inf\")"), KNumber::NaN.pow(KNumber::NegInfinity), QStringLiteral("nan"), KNumber::TYPE_ERROR);

    checkResult(QStringLiteral("KNumber(\"inf\") ^ KNumber(\"nan\")"), KNumber::PosInfinity.pow(KNumber::NaN), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"inf\") ^ KNumber(\"inf\")"), KNumber::PosInfinity.pow(KNumber::PosInfinity), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"inf\") ^ KNumber(\"-inf\")"), KNumber::PosInfinity.pow(KNumber::NegInfinity), QStringLiteral("0"), KNumber::TYPE_INTEGER);

    checkResult(QStringLiteral("KNumber(\"-inf\") ^ KNumber(\"nan\")"), KNumber::NegInfinity.pow(KNumber::NaN), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-inf\") ^ KNumber(\"inf\")"), KNumber::NegInfinity.pow(KNumber::PosInfinity), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-inf\") ^ KNumber(\"-inf\")"), KNumber::NegInfinity.pow(KNumber::NegInfinity), QStringLiteral("0"), KNumber::TYPE_INTEGER);
}

void testingFloatPrecision() {

	KNumber::setDefaultFloatPrecision(100);
    checkResult(QStringLiteral("Precision >= 100: (KNumber(1) + KNumber(\"1e-80\")) - KNumber(1)"), (KNumber(1) + KNumber(QStringLiteral("1e-80"))) - KNumber(1), QStringLiteral("1e-80"), KNumber::TYPE_FLOAT);
    checkResult(QStringLiteral("Precision >= 100: (KNumber(1) + KNumber(\"1e-980\")) - KNumber(1)"), (KNumber(1) + KNumber(QStringLiteral("1e-980"))) - KNumber(1), QStringLiteral("0"), KNumber::TYPE_INTEGER);

	KNumber::setDefaultFloatPrecision(1000);
    checkResult(QStringLiteral("Precision >= 1000: (KNumber(1) + KNumber(\"1e-980\")) - KNumber(1)"), (KNumber(1) + KNumber(QStringLiteral("1e-980"))) - KNumber(1), QStringLiteral("1e-980"), KNumber::TYPE_FLOAT);

	KNumber::setDefaultFloatPrecision(20);
    checkResult(QStringLiteral("Precision >= 20: sin(KNumber(30))"), sin(KNumber(30) * (KNumber::Pi() / KNumber(180))), QStringLiteral("0.5"), KNumber::TYPE_FLOAT);

}

void testingOutput() {

	KNumber::setDefaultFloatOutput(false);
    checkResult(QStringLiteral("Fractional output: KNumber(\"1/4\")"), KNumber(QStringLiteral("1/4")), QStringLiteral("1/4"), KNumber::TYPE_FRACTION);

	KNumber::setDefaultFloatOutput(true);
    checkResult(QStringLiteral("Float: KNumber(\"1/4\")"), KNumber(QStringLiteral("1/4")), QStringLiteral("0.25"), KNumber::TYPE_FRACTION);

	KNumber::setDefaultFloatOutput(false);
	KNumber::setSplitoffIntegerForFractionOutput(true);
    checkResult(QStringLiteral("Fractional output: KNumber(\"1/4\")"), KNumber(QStringLiteral("1/4")), QStringLiteral("1/4"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("Fractional output: KNumber(\"-1/4\")"), KNumber(QStringLiteral("-1/4")), QStringLiteral("-1/4"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("Fractional output: KNumber(\"21/4\")"), KNumber(QStringLiteral("21/4")), QStringLiteral("5 1/4"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("Fractional output: KNumber(\"-21/4\")"), KNumber(QStringLiteral("-21/4")), QStringLiteral("-5 1/4"), KNumber::TYPE_FRACTION);

	KNumber::setSplitoffIntegerForFractionOutput(false);
    checkResult(QStringLiteral("Fractional output: KNumber(\"1/4\")"), KNumber(QStringLiteral("1/4")), QStringLiteral("1/4"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("Fractional output: KNumber(\"-1/4\")"), KNumber(QStringLiteral("-1/4")), QStringLiteral("-1/4"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("Fractional output: KNumber(\"21/4\")"), KNumber(QStringLiteral("21/4")), QStringLiteral("21/4"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("Fractional output: KNumber(\"-21/4\")"), KNumber(QStringLiteral("-21/4")), QStringLiteral("-21/4"), KNumber::TYPE_FRACTION);
}

void testingConstructors() {
	std::cout << "Testing Constructors:\n";
	std::cout << "---------------------\n";

    checkResult(QStringLiteral("KNumber(5)"), KNumber(5), QStringLiteral("5"), KNumber::TYPE_INTEGER);
	checkType(QStringLiteral("KNumber(5.3)"), KNumber(5.3).type(), KNumber::TYPE_FLOAT);
	checkType(QStringLiteral("KNumber(0.0)"), KNumber(0.0).type(), KNumber::TYPE_INTEGER);

    checkResult(QStringLiteral("KNumber(\"5\")"), KNumber(QStringLiteral("5")), QStringLiteral("5"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"5/3\")"), KNumber(QStringLiteral("5/3")), QStringLiteral("5/3"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(\"5/1\")"), KNumber(QStringLiteral("5/1")), QStringLiteral("5"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"0/12\")"), KNumber(QStringLiteral("0/12")), QStringLiteral("0"), KNumber::TYPE_INTEGER);
	KNumber::setDefaultFractionalInput(true);
	std::cout << "Read decimals as fractions:\n";
    checkResult(QStringLiteral("KNumber(\"5\")"), KNumber(QStringLiteral("5")), QStringLiteral("5"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"1.2\")"), KNumber(QStringLiteral("1.2")), QStringLiteral("6/5"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(\"-0.02\")"), KNumber(QStringLiteral("-0.02")), QStringLiteral("-1/50"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(\"5e-2\")"), KNumber(QStringLiteral("5e-2")), QStringLiteral("1/20"), KNumber::TYPE_FRACTION);
    checkResult(QStringLiteral("KNumber(\"1.2e3\")"), KNumber(QStringLiteral("1.2e3")), QStringLiteral("1200"), KNumber::TYPE_INTEGER);
    checkResult(QStringLiteral("KNumber(\"0.02e+1\")"), KNumber(QStringLiteral("0.02e+1")), QStringLiteral("1/5"), KNumber::TYPE_FRACTION);

	KNumber::setDefaultFractionalInput(false);
	std::cout << "Read decimals as floats:\n";
    checkResult(QStringLiteral("KNumber(\"5.3\")"), KNumber(QStringLiteral("5.3")), QStringLiteral("5.3"), KNumber::TYPE_FLOAT);

    checkResult(QStringLiteral("KNumber(\"nan\")"), KNumber(QStringLiteral("nan")), QStringLiteral("nan"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"inf\")"), KNumber(QStringLiteral("inf")), QStringLiteral("inf"), KNumber::TYPE_ERROR);
    checkResult(QStringLiteral("KNumber(\"-inf\")"), KNumber(QStringLiteral("-inf")), QStringLiteral("-inf"), KNumber::TYPE_ERROR);

	// test accepting of non-US number formatted strings
    KNumber::setDecimalSeparator(QStringLiteral(","));
    checkResult(QStringLiteral("KNumber(\"5,2\")"), KNumber(QStringLiteral("5,2")), QStringLiteral("5.2"), KNumber::TYPE_FLOAT);
    KNumber::setDecimalSeparator(QStringLiteral("."));
}

void testingConstants() {
	std::cout << "\n\n";
	std::cout << "Constants:\n";
	std::cout << "----------\n";

	checkType(QStringLiteral("KNumber::Zero"),   KNumber::Zero.type(),    KNumber::TYPE_INTEGER);
	checkType(QStringLiteral("KNumber::One"),    KNumber::One.type(),     KNumber::TYPE_INTEGER);
	checkType(QStringLiteral("KNumber::NegOne"), KNumber::NegOne.type(),  KNumber::TYPE_INTEGER);
	checkType(QStringLiteral("KNumber::Pi"),     KNumber::Pi().type(),    KNumber::TYPE_FLOAT);
	checkType(QStringLiteral("KNumber::Euler"),  KNumber::Euler().type(), KNumber::TYPE_FLOAT);
}

}


int main() {

	testingConstants();
	testingConstructors();
	testingCompare();
	testingAdditions();
	testingSubtractions();
	testingMultiplications();
	testingDivisions();
	testingAndOr();
	testingModulus();
	testingAbs();
	testingSqrt();
	testingFactorial();
	testingComplement();
	testingPower();
	testingTruncateToInteger();
	testingShifts();
	testingInfArithmetic();
	testingFloatPrecision();
	testingTrig();
	testingSpecial();
	testingOutput();
	std::cout << "SUCCESS" << std::endl;
}

