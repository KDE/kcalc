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
		return QLatin1String("Special");
	case 1:
		return QLatin1String("Integer");
	case 3:
		return QLatin1String("Fraction");
	case 2:
		return QLatin1String("Float");
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

	checkTruth("KNumber(5) == KNumber(2)", KNumber(5) == KNumber(2), false);
	checkTruth("KNumber(5) > KNumber(2)", KNumber(5) > KNumber(2), true);
	checkTruth("KNumber(5) < KNumber(2)", KNumber(5) < KNumber(2), false);
	checkTruth("KNumber(5) < KNumber(0)", KNumber(5) < KNumber(0), false);
	checkTruth("KNumber(-5) < KNumber(0)", KNumber(-5) < KNumber(0), true);
	checkTruth("KNumber(5) >= KNumber(2)", KNumber(5) >= KNumber(2), true);
	checkTruth("KNumber(5) <= KNumber(2)", KNumber(5) <= KNumber(2), false);
	checkTruth("KNumber(5) != KNumber(2)", KNumber(5) != KNumber(2), true);

	checkTruth("KNumber(2) == KNumber(2)", KNumber(2) == KNumber(2), true);
	checkTruth("KNumber(2) > KNumber(2)", KNumber(2) > KNumber(2), false);
	checkTruth("KNumber(2) < KNumber(2)", KNumber(2) < KNumber(2), false);
	checkTruth("KNumber(2) >= KNumber(2)", KNumber(2) >= KNumber(2), true);
	checkTruth("KNumber(2) <= KNumber(2)", KNumber(2) <= KNumber(2), true);
	checkTruth("KNumber(2) != KNumber(2)", KNumber(2) != KNumber(2), false);

	checkTruth("KNumber(5) == KNumber(\"1/2\")", KNumber(5) == KNumber(QLatin1String("1/2")), false);
	checkTruth("KNumber(5) > KNumber(\"1/2\")", KNumber(5) > KNumber(QLatin1String("1/2")), true);
	checkTruth("KNumber(5) < KNumber(\"1/2\")", KNumber(5) < KNumber(QLatin1String("1/2")), false);
	checkTruth("KNumber(5) >= KNumber(\"1/2\")", KNumber(5) >= KNumber(QLatin1String("1/2")), true);
	checkTruth("KNumber(5) <= KNumber(\"1/2\")", KNumber(5) <= KNumber(QLatin1String("1/2")), false);
	checkTruth("KNumber(5) != KNumber(\"1/2\")", KNumber(5) != KNumber(QLatin1String("1/2")), true);

	checkTruth("KNumber(\"1/2\") == KNumber(\"1/2\")", KNumber(QLatin1String("1/2")) == KNumber(QLatin1String("1/2")), true);
	checkTruth("KNumber(\"1/2\") > KNumber(\"1/2\")", KNumber(QLatin1String("1/2")) > KNumber(QLatin1String("1/2")), false);
	checkTruth("KNumber(\"1/2\") < KNumber(\"1/2\")", KNumber(QLatin1String("1/2")) < KNumber(QLatin1String("1/2")), false);
	checkTruth("KNumber(\"1/2\") >= KNumber(\"1/2\")", KNumber(QLatin1String("1/2")) >= KNumber(QLatin1String("1/2")), true);
	checkTruth("KNumber(\"1/2\") <= KNumber(\"1/2\")", KNumber(QLatin1String("1/2")) <= KNumber(QLatin1String("1/2")), true);
	checkTruth("KNumber(\"1/2\") != KNumber(\"1/2\")", KNumber(QLatin1String("1/2")) != KNumber(QLatin1String("1/2")), false);

	checkTruth("KNumber(\"3/2\") == KNumber(\"1/2\")", KNumber(QLatin1String("3/2")) == KNumber(QLatin1String("1/2")), false);
	checkTruth("KNumber(\"3/2\") > KNumber(\"1/2\")", KNumber(QLatin1String("3/2")) > KNumber(QLatin1String("1/2")), true);
	checkTruth("KNumber(\"3/2\") < KNumber(\"1/2\")", KNumber(QLatin1String("3/2")) < KNumber(QLatin1String("1/2")), false);
	checkTruth("KNumber(\"3/2\") >= KNumber(\"1/2\")", KNumber(QLatin1String("3/2")) >= KNumber(QLatin1String("1/2")), true);
	checkTruth("KNumber(\"3/2\") <= KNumber(\"1/2\")", KNumber(QLatin1String("3/2")) <= KNumber(QLatin1String("1/2")), false);
	checkTruth("KNumber(\"3/2\") != KNumber(\"1/2\")", KNumber(QLatin1String("3/2")) != KNumber(QLatin1String("1/2")), true);

	checkTruth("KNumber(3.2) != KNumber(3)", KNumber(3.2) != KNumber(3), true);
	checkTruth("KNumber(3.2) > KNumber(3)", KNumber(3.2) > KNumber(3), true);
	checkTruth("KNumber(3.2) < KNumber(3)", KNumber(3.2) < KNumber(3), false);

	checkTruth("KNumber(3.2) < KNumber(\"3/5\")", KNumber(3.2) < KNumber(QLatin1String("3/5")), false);
}


void testingAdditions() {

	std::cout << "\n\n";
	std::cout << "Testing additions:\n";
	std::cout << "------------------\n";

	checkResult("KNumber(5) + KNumber(2)", KNumber(5) + KNumber(2), QLatin1String("7"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(5) + KNumber(\"2/3\")", KNumber(5) + KNumber(QLatin1String("2/3")), QLatin1String("17/3"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(5) + KNumber(\"2.3\")", KNumber(5) + KNumber(QLatin1String("2.3")), QLatin1String("7.3"), KNumber::TYPE_FLOAT);

	checkResult("KNumber(\"5/3\") + KNumber(2)", KNumber(QLatin1String("5/3")) + KNumber(2), QLatin1String("11/3"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(\"5/3\") + KNumber(\"2/3\")", KNumber(QLatin1String("5/3")) + KNumber(QLatin1String("2/3")), QLatin1String("7/3"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(\"5/3\") + KNumber(\"1/3\")", KNumber(QLatin1String("5/3")) + KNumber(QLatin1String("1/3")), QLatin1String("2"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"5/3\") + KNumber(\"-26/3\")", KNumber(QLatin1String("5/3")) + KNumber(QLatin1String("-26/3")), QLatin1String("-7"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"5/2\") + KNumber(2.3)", KNumber(QLatin1String("5/2")) + KNumber(2.3), QLatin1String("4.8"), KNumber::TYPE_FLOAT);

	checkResult("KNumber(5.3) + KNumber(2)", KNumber(5.3) + KNumber(2), QLatin1String("7.3"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(5.3) + KNumber(\"2/4\")", KNumber(5.3) + KNumber(QLatin1String("2/4")), QLatin1String("5.8"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(5.3) + KNumber(2.3)", KNumber(5.3) + KNumber(2.3), QLatin1String("7.6"), KNumber::TYPE_FLOAT);
}

void testingSubtractions() {

	std::cout << "\n\n";
	std::cout << "Testing subtractions:\n";
	std::cout << "---------------------\n";

	checkResult("KNumber(5) - KNumber(2)", KNumber(5) - KNumber(2), QLatin1String("3"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(5) - KNumber(\"2/3\")", KNumber(5) - KNumber(QLatin1String("2/3")), QLatin1String("13/3"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(5) - KNumber(2.3)", KNumber(5) - KNumber(2.3), QLatin1String("2.7"), KNumber::TYPE_FLOAT);

	checkResult("KNumber(\"5/3\") - KNumber(2)", KNumber(QLatin1String("5/3")) - KNumber(2), QLatin1String("-1/3"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(\"5/3\") - KNumber(\"1/3\")", KNumber(QLatin1String("5/3")) - KNumber(QLatin1String("1/3")), QLatin1String("4/3"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(\"5/3\") - KNumber(\"2/3\")", KNumber(QLatin1String("5/3")) - KNumber(QLatin1String("2/3")), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"-5/3\") - KNumber(\"4/3\")", KNumber(QLatin1String("-5/3")) - KNumber(QLatin1String("4/3")), QLatin1String("-3"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"5/4\") - KNumber(2.2)", KNumber(QLatin1String("5/4")) - KNumber(2.2), QLatin1String("-0.95"), KNumber::TYPE_FLOAT);

	checkResult("KNumber(5.3) - KNumber(2)", KNumber(5.3) - KNumber(2), QLatin1String("3.3"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(5.3) - KNumber(\"3/4\")", KNumber(5.3) - KNumber(QLatin1String("3/4")), QLatin1String("4.55"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(5.3) - KNumber(2.3)", KNumber(5.3) - KNumber(2.3), QLatin1String("3"), KNumber::TYPE_INTEGER);
}

void testingSpecial() {

	std::cout << "\n\n";
	std::cout << "Testing special functions:\n";
	std::cout << "--------------------------\n";

	checkResult("log10(KNumber(5))", log10(KNumber(5)), QLatin1String("0.698970004336"), KNumber::TYPE_FLOAT);
	checkResult("log10(pow(KNumber(10), KNumber(308)))", log10(pow(KNumber(10), KNumber(308))), QLatin1String("308"), KNumber::TYPE_INTEGER);

	// TODO: enable this check once MPFR is commonly enabled
	// checkResult("log10(pow(KNumber(10), KNumber(309)))", log10(pow(KNumber(10), KNumber(309))), QLatin1String("309"), KNumber::TYPE_INTEGER);

	checkResult("exp(KNumber(4.34))", exp(KNumber(4.34)), QLatin1String("76.7075393383"), KNumber::TYPE_FLOAT);
}

void testingTrig() {

	std::cout << "\n\n";
	std::cout << "Testing trig functions:\n";
	std::cout << "-----------------------\n";

	checkResult("sin(KNumber(5))", sin(KNumber(5)), QLatin1String("-0.958924274663"), KNumber::TYPE_FLOAT);
	checkResult("cos(KNumber(5))", cos(KNumber(5)), QLatin1String("0.283662185463"), KNumber::TYPE_FLOAT);
	checkResult("tan(KNumber(5))", tan(KNumber(5)), QLatin1String("-3.38051500625"), KNumber::TYPE_FLOAT);
	checkResult("sin(KNumber(-5))", sin(KNumber(-5)), QLatin1String("0.958924274663"), KNumber::TYPE_FLOAT);
	checkResult("cos(KNumber(-5))", cos(KNumber(-5)), QLatin1String("0.283662185463"), KNumber::TYPE_FLOAT);
	checkResult("tan(KNumber(-5))", tan(KNumber(-5)), QLatin1String("3.38051500625"), KNumber::TYPE_FLOAT);

	checkResult("sin(KNumber(\"5/2\"))", sin(KNumber(QLatin1String("5/2"))), QLatin1String("0.598472144104"), KNumber::TYPE_FLOAT);
	checkResult("cos(KNumber(\"5/2\"))", cos(KNumber(QLatin1String("5/2"))), QLatin1String("-0.801143615547"), KNumber::TYPE_FLOAT);
	checkResult("tan(KNumber(\"5/2\"))", tan(KNumber(QLatin1String("5/2"))), QLatin1String("-0.747022297239"), KNumber::TYPE_FLOAT);
	checkResult("sin(KNumber(\"-5/2\"))", sin(KNumber(QLatin1String("-5/2"))), QLatin1String("-0.598472144104"), KNumber::TYPE_FLOAT);
	checkResult("cos(KNumber(\"-5/2\"))", cos(KNumber(QLatin1String("-5/2"))), QLatin1String("-0.801143615547"), KNumber::TYPE_FLOAT);
	checkResult("tan(KNumber(\"-5/2\"))", tan(KNumber(QLatin1String("-5/2"))), QLatin1String("0.747022297239"), KNumber::TYPE_FLOAT);

	checkResult("sin(KNumber(5.3))", sin(KNumber(5.3)), QLatin1String("-0.832267442224"), KNumber::TYPE_FLOAT);
	checkResult("cos(KNumber(5.3))", cos(KNumber(5.3)), QLatin1String("0.554374336179"), KNumber::TYPE_FLOAT);
	checkResult("tan(KNumber(5.3))", tan(KNumber(5.3)), QLatin1String("-1.50127339581"), KNumber::TYPE_FLOAT);
	checkResult("sin(KNumber(-5.3))", sin(KNumber(-5.3)), QLatin1String("0.832267442224"), KNumber::TYPE_FLOAT);
	checkResult("cos(KNumber(-5.3))", cos(KNumber(-5.3)), QLatin1String("0.554374336179"), KNumber::TYPE_FLOAT);
	checkResult("tan(KNumber(-5.3))", tan(KNumber(-5.3)), QLatin1String("1.50127339581"), KNumber::TYPE_FLOAT);

	checkResult("asin(KNumber(5))", asin(KNumber(5)), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("acos(KNumber(5))", acos(KNumber(5)), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("atan(KNumber(5))", atan(KNumber(5)), QLatin1String("1.37340076695"), KNumber::TYPE_FLOAT);
	checkResult("asin(KNumber(-5))", asin(KNumber(-5)), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("acos(KNumber(-5))", acos(KNumber(-5)), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("atan(KNumber(-5))", atan(KNumber(-5)), QLatin1String("-1.37340076695"), KNumber::TYPE_FLOAT);

	checkResult("asin(KNumber(\"5/2\"))", asin(KNumber(QLatin1String("5/2"))), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("acos(KNumber(\"5/2\"))", acos(KNumber(QLatin1String("5/2"))), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("atan(KNumber(\"5/2\"))", atan(KNumber(QLatin1String("5/2"))), QLatin1String("1.19028994968"), KNumber::TYPE_FLOAT);
	checkResult("asin(KNumber(\"-5/2\"))", asin(KNumber(QLatin1String("-5/2"))), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("acos(KNumber(\"-5/2\"))", acos(KNumber(QLatin1String("-5/2"))), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("atan(KNumber(\"-5/2\"))", atan(KNumber(QLatin1String("-5/2"))), QLatin1String("-1.19028994968"), KNumber::TYPE_FLOAT);

	checkResult("asin(KNumber(5.3))", asin(KNumber(5.3)), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("acos(KNumber(5.3))", acos(KNumber(5.3)), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("atan(KNumber(5.3))", atan(KNumber(5.3)), QLatin1String("1.38430942513"), KNumber::TYPE_FLOAT);
	checkResult("asin(KNumber(-5.3))", asin(KNumber(-5.3)), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("acos(KNumber(-5.3))", acos(KNumber(-5.3)), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("atan(KNumber(-5.3))", atan(KNumber(-5.3)), QLatin1String("-1.38430942513"), KNumber::TYPE_FLOAT);


	checkResult("asin(KNumber(\"2/5\"))", asin(KNumber(QLatin1String("2/5"))), QLatin1String("0.411516846067"), KNumber::TYPE_FLOAT);
	checkResult("acos(KNumber(\"2/5\"))", acos(KNumber(QLatin1String("2/5"))), QLatin1String("1.15927948073"), KNumber::TYPE_FLOAT);
	checkResult("atan(KNumber(\"2/5\"))", atan(KNumber(QLatin1String("2/5"))), QLatin1String("0.380506377112"), KNumber::TYPE_FLOAT);
	checkResult("asin(KNumber(\"-2/5\"))", asin(KNumber(QLatin1String("-2/5"))), QLatin1String("-0.411516846067"), KNumber::TYPE_FLOAT);
	checkResult("acos(KNumber(\"-2/5\"))", acos(KNumber(QLatin1String("-2/5"))), QLatin1String("1.98231317286"), KNumber::TYPE_FLOAT);
	checkResult("atan(KNumber(\"-2/5\"))", atan(KNumber(QLatin1String("-2/5"))), QLatin1String("-0.380506377112"), KNumber::TYPE_FLOAT);

	checkResult("asin(KNumber(0.3))", asin(KNumber(0.3)), QLatin1String("0.304692654015"), KNumber::TYPE_FLOAT);
	checkResult("acos(KNumber(0.3))", acos(KNumber(0.3)), QLatin1String("1.26610367278"), KNumber::TYPE_FLOAT);
	checkResult("atan(KNumber(0.3))", atan(KNumber(0.3)), QLatin1String("0.291456794478"), KNumber::TYPE_FLOAT);
	checkResult("asin(KNumber(-0.3))", asin(KNumber(-0.3)), QLatin1String("-0.304692654015"), KNumber::TYPE_FLOAT);
	checkResult("acos(KNumber(-0.3))", acos(KNumber(-0.3)), QLatin1String("1.87548898081"), KNumber::TYPE_FLOAT);
	checkResult("atan(KNumber(-0.3))", atan(KNumber(-0.3)), QLatin1String("-0.291456794478"), KNumber::TYPE_FLOAT);
}


void testingMultiplications() {

	std::cout << "\n\n";
	std::cout << "Testing multiplications:\n";
	std::cout << "------------------------\n";

	checkResult("KNumber(5) * KNumber(2)", KNumber(5) * KNumber(2), QLatin1String("10"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(5) * KNumber(\"2/3\")", KNumber(5) * KNumber(QLatin1String("2/3")), QLatin1String("10/3"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(5) * KNumber(\"2/5\")", KNumber(5) * KNumber(QLatin1String("2/5")), QLatin1String("2"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(5) * KNumber(2.3)", KNumber(5) * KNumber(2.3), QLatin1String("11.5"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(0) * KNumber(\"2/5\")", KNumber(0) * KNumber(QLatin1String("2/5")), QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(0) * KNumber(2.3)", KNumber(0) * KNumber(2.3), QLatin1String("0"), KNumber::TYPE_INTEGER);

	checkResult("KNumber(\"5/3\") * KNumber(2)", KNumber(QLatin1String("5/3")) * KNumber(2), QLatin1String("10/3"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(\"5/3\") * KNumber(0)", KNumber(QLatin1String("5/3")) * KNumber(0), QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"5/3\") * KNumber(\"2/3\")", KNumber(QLatin1String("5/3")) * KNumber(QLatin1String("2/3")), QLatin1String("10/9"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(\"25/6\") * KNumber(\"12/5\")", KNumber(QLatin1String("25/6")) * KNumber(QLatin1String("12/5")), QLatin1String("10"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"5/2\") * KNumber(2.3)", KNumber(QLatin1String("5/2")) * KNumber(2.3), QLatin1String("5.75"), KNumber::TYPE_FLOAT);

	checkResult("KNumber(5.3) * KNumber(2)", KNumber(5.3) * KNumber(2), QLatin1String("10.6"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(5.3) * KNumber(0)", KNumber(5.3) * KNumber(0), QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(5.3) * KNumber(\"1/2\")", KNumber(5.3) * KNumber(QLatin1String("1/2")), QLatin1String("2.65"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(5.3) * KNumber(2.3)", KNumber(5.3) * KNumber(2.3), QLatin1String("12.19"), KNumber::TYPE_FLOAT);
}

void testingDivisions() {

	std::cout << "\n\n";
	std::cout << "Testing divisions:\n";
	std::cout << "------------------\n";

	checkResult("KNumber(5) / KNumber(2)", KNumber(5) / KNumber(2), QLatin1String("5/2"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(122) / KNumber(2)", KNumber(122) / KNumber(2), QLatin1String("61"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(12) / KNumber(0)", KNumber(12) / KNumber(0), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-12) / KNumber(0)", KNumber(-12) / KNumber(0), QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(5) / KNumber(\"2/3\")", KNumber(5) / KNumber(QLatin1String("2/3")), QLatin1String("15/2"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(6) / KNumber(\"2/3\")", KNumber(6) / KNumber(QLatin1String("2/3")), QLatin1String("9"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(5) / KNumber(2.5)", KNumber(5) / KNumber(2.5), QLatin1String("2"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(5) / KNumber(0.0)", KNumber(5) / KNumber(0.0), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5) / KNumber(0.0)", KNumber(-5) / KNumber(0.0), QLatin1String("-inf"), KNumber::TYPE_ERROR);

	checkResult("KNumber(\"5/3\") / KNumber(2)", KNumber(QLatin1String("5/3")) / KNumber(2), QLatin1String("5/6"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(\"5/3\") / KNumber(0)", KNumber(QLatin1String("5/3")) / KNumber(0), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-5/3\") / KNumber(0)", KNumber(QLatin1String("-5/3")) / KNumber(0), QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"5/3\") / KNumber(\"2/3\")", KNumber(QLatin1String("5/3")) / KNumber(QLatin1String("2/3")), QLatin1String("5/2"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(\"49/3\") / KNumber(\"7/9\")", KNumber(QLatin1String("49/3")) / KNumber(QLatin1String("7/9")), QLatin1String("21"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"5/2\") / KNumber(2.5)", KNumber(QLatin1String("5/2")) / KNumber(2.5), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"5/2\") / KNumber(0.0)", KNumber(QLatin1String("5/2")) / KNumber(0.0), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-5/2\") / KNumber(0.0)", KNumber(QLatin1String("-5/2")) / KNumber(0.0), QLatin1String("-inf"), KNumber::TYPE_ERROR);

	checkResult("KNumber(5.3) / KNumber(2)", KNumber(5.3) / KNumber(2), QLatin1String("2.65"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(5.3) / KNumber(0)", KNumber(5.3) / KNumber(0), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5.3) / KNumber(0)", KNumber(-5.3) / KNumber(0), QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(5.3) / KNumber(\"2/3\")", KNumber(5.3) / KNumber(QLatin1String("2/3")), QLatin1String("7.95"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(5.5) / KNumber(2.5)", KNumber(5.5) / KNumber(2.5), QLatin1String("2.2"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(5.5) / KNumber(0.0)", KNumber(5.5) / KNumber(0.0), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5.5) / KNumber(0.0)", KNumber(-5.5) / KNumber(0.0), QLatin1String("-inf"), KNumber::TYPE_ERROR);
}

void testingModulus() {

	std::cout << "\n\n";
	std::cout << "Testing modulus:\n";
	std::cout << "----------------\n";

	checkResult("KNumber(23) % KNumber(4)", KNumber(23) % KNumber(4), QLatin1String("3"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(12) % KNumber(-5)", KNumber(12) % KNumber(-5), QLatin1String("2"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(-12) % KNumber(5)", KNumber(-12) % KNumber(5), QLatin1String("3"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(12) % KNumber(0)", KNumber(-12) % KNumber(0), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-12) % KNumber(0)", KNumber(-12) % KNumber(0), QLatin1String("nan"), KNumber::TYPE_ERROR);

#ifdef __GNUC__
	#warning test for other types
#endif
}

void testingAndOr() {

	std::cout << "\n\n";
	std::cout << "Testing And/Or:\n";
	std::cout << "---------------\n";

	checkResult("KNumber(17) & KNumber(9)", KNumber(17) & KNumber(9), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(17) | KNumber(9)", KNumber(17) | KNumber(9), QLatin1String("25"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(1023) & KNumber(255)", KNumber(1023) & KNumber(255), QLatin1String("255"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(1023) | KNumber(255)", KNumber(1023) | KNumber(255), QLatin1String("1023"), KNumber::TYPE_INTEGER);

#ifdef __GNUC__
	#warning test for other types
#endif
}


void testingAbs() {

	std::cout << "\n\n";
	std::cout << "Testing absolute value:\n";
	std::cout << "-----------------------\n";

	checkResult("KNumber(5).abs()", KNumber(5).abs(), QLatin1String("5"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"2/3\").abs()", KNumber(QLatin1String("2/3")).abs(), QLatin1String("2/3"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(\"2.3\").abs()", KNumber(QLatin1String("2.3")).abs(), QLatin1String("2.3"), KNumber::TYPE_FLOAT);

	checkResult("KNumber(-5).abs()", KNumber(-5).abs(), QLatin1String("5"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"-2/3\").abs()", KNumber(QLatin1String("-2/3")).abs(), QLatin1String("2/3"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(\"-2.3\").abs()", KNumber(QLatin1String("-2.3")).abs(), QLatin1String("2.3"), KNumber::TYPE_FLOAT);
}

void testingTruncateToInteger() {

	std::cout << "\n\n";
	std::cout << "Testing truncate to an integer:\n";
	std::cout << "-------------------------------\n";

	checkResult("KNumber(16).integerPart()", KNumber(16).integerPart(), QLatin1String("16"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"43/9\").integerPart()", KNumber(QLatin1String("43/9")).integerPart(), QLatin1String("4"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"-43/9\").integerPart()", KNumber(QLatin1String("-43/9")).integerPart(), QLatin1String("-4"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"5.25\").integerPart()", KNumber(QLatin1String("5.25")).integerPart(), QLatin1String("5"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"-5.25\").integerPart()", KNumber(QLatin1String("-5.25")).integerPart(), QLatin1String("-5"), KNumber::TYPE_INTEGER);
}


void testingSqrt() {

	std::cout << "\n\n";
	std::cout << "Testing square root, cubic root:\n";
	std::cout << "--------------------------------\n";

	checkResult("KNumber(16).sqrt()", KNumber(16).sqrt(), QLatin1String("4"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(-16).sqrt()", KNumber(-16).sqrt(), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"16/9\").sqrt()", KNumber(QLatin1String("16/9")).sqrt(), QLatin1String("4/3"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(\"-16/9\").sqrt()", KNumber(QLatin1String("-16/9")).sqrt(), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(2).sqrt()", KNumber(2).sqrt(), QLatin1String("1.41421356237"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(\"2/3\").sqrt()", KNumber(QLatin1String("2/3")).sqrt(), QLatin1String("0.816496580928"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(\"0.25\").sqrt()", KNumber(QLatin1String("0.25")).sqrt(), QLatin1String("0.5"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(\"-0.25\").sqrt()", KNumber(QLatin1String("-0.25")).sqrt(), QLatin1String("nan"), KNumber::TYPE_ERROR);


	checkResult("KNumber(27).cbrt()", KNumber(27).cbrt(), QLatin1String("3"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(-27).cbrt()", KNumber(-27).cbrt(), QLatin1String("-3"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"27/8\").cbrt()", KNumber(QLatin1String("27/8")).cbrt(), QLatin1String("3/2"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(\"-8/27\").cbrt()", KNumber(QLatin1String("-8/27")).cbrt(), QLatin1String("-2/3"), KNumber::TYPE_FRACTION);
#ifdef __GNUC__
	#warning need to check non-perfect cube roots
#endif
	checkResult("KNumber(2).cbrt()", KNumber(2).cbrt(), QLatin1String("1.25992104989"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(\"2/3\").cbrt()", KNumber(QLatin1String("2/3")).cbrt(), QLatin1String("0.873580464736"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(\"0.25\").cbrt()", KNumber(QLatin1String("0.25")).cbrt(), QLatin1String("0.629960524947"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(\"-0.25\").cbrt()", KNumber(QLatin1String("-0.25")).cbrt(), QLatin1String("-0.629960524947"), KNumber::TYPE_FLOAT);

}

void testingFactorial() {

	std::cout << "\n\n";
	std::cout << "Testing factorial:\n";
	std::cout << "------------------\n";

	checkResult("KNumber(-1).factorial()", KNumber(-1).factorial(), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-2).factorial()", KNumber(-2).factorial(), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-20).factorial()", KNumber(-20).factorial(), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-1/2).factorial()", KNumber(QLatin1String("-1/2")).factorial(), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-0.5).factorial()", KNumber(QLatin1String("-0.5")).factorial(), QLatin1String("nan"), KNumber::TYPE_ERROR);

	checkResult("KNumber(0).factorial()", KNumber(0).factorial(), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(1).factorial()", KNumber(1).factorial(), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(2).factorial()", KNumber(2).factorial(), QLatin1String("2"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(3).factorial()", KNumber(3).factorial(), QLatin1String("6"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(4).factorial()", KNumber(4).factorial(), QLatin1String("24"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(5).factorial()", KNumber(5).factorial(), QLatin1String("120"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(6).factorial()", KNumber(6).factorial(), QLatin1String("720"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(9).factorial()", KNumber(9).factorial(), QLatin1String("362880"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(12).factorial()", KNumber(12).factorial(), QLatin1String("479001600"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(13).factorial()", KNumber(13).factorial(), QLatin1String("6227020800"), KNumber::TYPE_INTEGER);

	checkResult("KNumber(1/2).factorial()", KNumber(QLatin1String("1/2")).factorial(), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(2/1).factorial()", KNumber(QLatin1String("2/1")).factorial(), QLatin1String("2"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(3/2).factorial()", KNumber(QLatin1String("3/2")).factorial(), QLatin1String("1"), KNumber::TYPE_INTEGER);

	checkResult("KNumber(0.1).factorial()", KNumber(0.1).factorial(), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(0.5).factorial()", KNumber(0.5).factorial(), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(1.5).factorial()", KNumber(1.5).factorial(), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(2.5).factorial()", KNumber(2.5).factorial(), QLatin1String("2"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(3.5).factorial()", KNumber(3.5).factorial(), QLatin1String("6"), KNumber::TYPE_INTEGER);
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
	checkResult("~KNumber(0)", ~KNumber(0), QLatin1String("-1"), KNumber::TYPE_INTEGER);
	checkResult("~KNumber(1)", ~KNumber(1), QLatin1String("-2"), KNumber::TYPE_INTEGER);
	checkResult("~KNumber(2)", ~KNumber(2), QLatin1String("-3"), KNumber::TYPE_INTEGER);
	checkResult("~KNumber(8)", ~KNumber(8), QLatin1String("-9"), KNumber::TYPE_INTEGER);
	checkResult("~KNumber(15)", ~KNumber(15), QLatin1String("-16"), KNumber::TYPE_INTEGER);
	checkResult("~KNumber(-1)", ~KNumber(-1), QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("~KNumber(-2)", ~KNumber(-2), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("~KNumber(-3)", ~KNumber(-3), QLatin1String("2"), KNumber::TYPE_INTEGER);
	checkResult("~KNumber(-9)", ~KNumber(-9), QLatin1String("8"), KNumber::TYPE_INTEGER);
	checkResult("~KNumber(-16)", ~KNumber(-16), QLatin1String("15"), KNumber::TYPE_INTEGER);
#endif

	checkResult("~KNumber(0.12345)", ~KNumber(0.12345), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("~KNumber(-0.12345)", ~KNumber(-0.12345), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("~KNumber(\"1/2\")", ~KNumber(QLatin1String("1/2")), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("~KNumber(\"-1/2\")", ~KNumber(QLatin1String("-1/2")), QLatin1String("nan"), KNumber::TYPE_ERROR);
}

void testingShifts() {

	std::cout << "\n\n";
	std::cout << "Testing left/right shift:\n";
	std::cout << "-------------------------\n";

	checkResult("KNumber(16) << KNumber(2)", KNumber(16) << KNumber(2), QLatin1String("64"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(16) >> KNumber(2)", KNumber(16) >> KNumber(2), QLatin1String("4"), KNumber::TYPE_INTEGER);
}

void testingPower() {

	std::cout << "\n\n";
	std::cout << "Testing Power:\n";
	std::cout << "--------------\n";

	checkResult("KNumber(0) ^ KNumber(-4)", KNumber(0).pow(KNumber(-4)), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(5) ^ KNumber(4)", KNumber(5).pow(KNumber(4)), QLatin1String("625"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(122) ^ KNumber(0)", KNumber(122).pow(KNumber(0)), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(-5) ^ KNumber(0)", KNumber(-5).pow(KNumber(0)), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(-2) ^ KNumber(3)", KNumber(-2).pow(KNumber(3)), QLatin1String("-8"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(-2) ^ KNumber(4)", KNumber(-2).pow(KNumber(4)), QLatin1String("16"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(5) ^ KNumber(-2)", KNumber(5).pow(KNumber(-2)), QLatin1String("1/25"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(8) ^ KNumber(\"2/3\")", KNumber(8).pow(KNumber(QLatin1String("2/3"))), QLatin1String("4"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(8) ^ KNumber(\"-2/3\")", KNumber(8).pow(KNumber(QLatin1String("-2/3"))), QLatin1String("1/4"), KNumber::TYPE_FRACTION);

	checkResult("KNumber(-16) ^ KNumber(\"1/4\")", KNumber(-16).pow(KNumber(QLatin1String("1/4"))), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-8) ^ KNumber(\"1/3\")", KNumber(-8).pow(KNumber(QLatin1String("1/3"))), QLatin1String("-2"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(5) ^ KNumber(0.0)", KNumber(5).pow(KNumber(0.0)), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(-5) ^ KNumber(0.0)", KNumber(-5).pow(KNumber(0.0)), QLatin1String("1"), KNumber::TYPE_INTEGER);

	checkResult("KNumber(\"5/3\") ^ KNumber(2)", KNumber(QLatin1String("5/3")).pow(KNumber(2)), QLatin1String("25/9"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(\"5/3\") ^ KNumber(0)", KNumber(QLatin1String("5/3")).pow(KNumber(0)), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"-5/3\") ^ KNumber(0)", KNumber(QLatin1String("-5/3")).pow(KNumber(0)), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"8/27\") ^ KNumber(\"2/3\")", KNumber(QLatin1String("8/27")).pow(KNumber(QLatin1String("2/3"))), QLatin1String("4/9"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(\"49/3\") ^ KNumber(\"7/9\")", KNumber(QLatin1String("49/3")).pow(KNumber(QLatin1String("7/9"))), QLatin1String("8.78016428243"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(\"5/2\") ^ KNumber(2.5)", KNumber(QLatin1String("5/2")).pow(KNumber(2.5)), QLatin1String("9.88211768803"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(\"5/2\") ^ KNumber(0.0)", KNumber(QLatin1String("5/2")).pow(KNumber(0.0)), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"-5/2\") ^ KNumber(0.0)", KNumber(QLatin1String("-5/2")).pow(KNumber(0.0)), QLatin1String("1"), KNumber::TYPE_INTEGER);

	checkResult("KNumber(5.3) ^ KNumber(2)", KNumber(5.3).pow(KNumber(2)), QLatin1String("28.09"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(5.3) ^ KNumber(0)", KNumber(5.3).pow(KNumber(0)), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(-5.3) ^ KNumber(0)", KNumber(-5.3).pow(KNumber(0)), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(5.3) ^ KNumber(\"2/3\")", KNumber(5.3).pow(KNumber(QLatin1String("2/3"))), QLatin1String("3.03983898039"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(5.5) ^ KNumber(2.5)", KNumber(5.5).pow(KNumber(2.5)), QLatin1String("70.9425383673"), KNumber::TYPE_FLOAT);
	checkResult("KNumber(5.5) ^ KNumber(0.0)", KNumber(5.5).pow(KNumber(0.0)), QLatin1String("1"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(-5.5) ^ KNumber(0.0)", KNumber(-5.5).pow(KNumber(0.0)), QLatin1String("1"), KNumber::TYPE_INTEGER);

	checkResult("KNumber::Pi() ^ KNumber::Pi()", KNumber::Pi().pow(KNumber::Pi()), QLatin1String("36.4621596072"), KNumber::TYPE_FLOAT);
	checkResult("KNumber::Euler() ^ KNumber::Pi()", KNumber::Euler().pow(KNumber::Pi()), QLatin1String("23.1406926328"), KNumber::TYPE_FLOAT);
	
	// TODO: kinda odd that this ends up being an integer
	// i guess since my euler constant is only 100 digits, we've exceeded the fractional part
	checkResult("KNumber::Euler() ^ 1000", KNumber::Euler().pow(KNumber(1000)), QLatin1String("1.97007111402e+434"), KNumber::TYPE_INTEGER);

	// TODO: make this test pass
	// the problem is that it is using the libc exp function which has limits that GMP does not
	// we should basically make this equivalent to KNumber::Euler().pow(KNumber(1000))
	// which does work
#if 0
	checkResult("KNumber(1000).exp()", KNumber(1000).exp(), QLatin1String("23.1406926328"), KNumber::TYPE_FLOAT);
#endif

	KNumber::setDefaultFractionalInput(true);
	checkResult("KNumber(\"3.1415926\") ^ KNumber(\"3.1415926\")", KNumber(QLatin1String("3.1415926")).pow(KNumber(QLatin1String("3.1415926"))), QLatin1String("36.4621554164"), KNumber::TYPE_FLOAT);
	KNumber::setDefaultFractionalInput(false);
}

void testingInfArithmetic() {

	std::cout << "\n\n";
	std::cout << "Testing inf/nan-arithmetics:\n";
	std::cout << "----------------------------\n";

	checkResult("inf + KNumber(2)", KNumber::PosInfinity + KNumber(2), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5) + inf", KNumber(-5) + KNumber::PosInfinity, QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("inf + KNumber(\"1/2\")", KNumber::PosInfinity + KNumber(QLatin1String("1/2")), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-5/3\") + inf", KNumber(QLatin1String("-5/3")) + KNumber::PosInfinity, QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("inf + KNumber(2.01)", KNumber::PosInfinity + KNumber(2.01), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5.4) + inf", KNumber(-5.4) + KNumber::PosInfinity, QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("-inf + KNumber(2)", KNumber::NegInfinity + KNumber(2), QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5) + -inf", KNumber(-5) + KNumber::NegInfinity, QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("-inf + KNumber(\"1/2\")", KNumber::NegInfinity + KNumber(QLatin1String("1/2")), QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-5/3\") + -inf", KNumber(QLatin1String("-5/3")) + KNumber::NegInfinity, QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("-inf + KNumber(2.01)", KNumber::NegInfinity + KNumber(2.01), QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5.4) + -inf", KNumber(-5.4) + KNumber::NegInfinity, QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("nan + KNumber(2)", KNumber::NaN + KNumber(2), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5) + nan", KNumber(-5) + KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("nan + KNumber(\"1/2\")", KNumber::NaN + KNumber(QLatin1String("1/2")), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-5/3\") + nan", KNumber(QLatin1String("-5/3")) + KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("nan + KNumber(2.01)", KNumber::NaN + KNumber(2.01), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5.4) + nan", KNumber(-5.4) + KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("inf + inf", KNumber::PosInfinity + KNumber::PosInfinity, QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("inf + -inf", KNumber::PosInfinity + KNumber::NegInfinity, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("-inf + inf", KNumber::NegInfinity + KNumber::PosInfinity, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("-inf + -inf", KNumber::NegInfinity + KNumber::NegInfinity, QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("inf + nan", KNumber::PosInfinity + KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("-inf + nan", KNumber::NegInfinity + KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("nan + inf", KNumber::NaN + KNumber::PosInfinity, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("-inf + nan", KNumber::NegInfinity + KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);

	checkResult("inf - KNumber(2)", KNumber::PosInfinity - KNumber(2), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5) - inf", KNumber(-5) - KNumber::PosInfinity, QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("inf - KNumber(\"1/2\")", KNumber::PosInfinity - KNumber(QLatin1String("1/2")), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-5/3\") - inf", KNumber(QLatin1String("-5/3")) - KNumber::PosInfinity, QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("inf - KNumber(2.01)", KNumber::PosInfinity - KNumber(2.01), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5.4) - inf", KNumber(-5.4) - KNumber::PosInfinity, QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("-inf - KNumber(2)", KNumber::NegInfinity - KNumber(2), QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5) - -inf", KNumber(-5) - KNumber::NegInfinity, QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("-inf - KNumber(\"1/2\")", KNumber::NegInfinity - KNumber(QLatin1String("1/2")), QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-5/3\") - -inf", KNumber(QLatin1String("-5/3")) - KNumber::NegInfinity, QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("-inf - KNumber(2.01)", KNumber::NegInfinity - KNumber(2.01), QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5.4) - -inf", KNumber(-5.4) - KNumber::NegInfinity, QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("nan - KNumber(2)", KNumber::NaN - KNumber(2), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5) - nan", KNumber(-5) - KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("nan - KNumber(\"1/2\")", KNumber::NaN - KNumber(QLatin1String("1/2")), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-5/3\") - nan", KNumber(QLatin1String("-5/3")) - KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("nan - KNumber(2.01)", KNumber::NaN - KNumber(2.01), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5.4) - nan", KNumber(-5.4) - KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("inf - inf", KNumber::PosInfinity - KNumber::PosInfinity, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("inf - -inf", KNumber::PosInfinity - KNumber::NegInfinity, QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("-inf - inf", KNumber::NegInfinity - KNumber::PosInfinity, QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("-inf - -inf", KNumber::NegInfinity - KNumber::NegInfinity, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("inf - nan", KNumber::PosInfinity - KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("-inf - nan", KNumber::NegInfinity - KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("nan - inf", KNumber::NaN - KNumber::PosInfinity, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("-inf - nan", KNumber::NegInfinity - KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);

	checkResult("inf * KNumber(2)", KNumber::PosInfinity * KNumber(2), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5) * inf", KNumber(-5) * KNumber::PosInfinity, QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("inf * KNumber(\"1/2\")", KNumber::PosInfinity * KNumber(QLatin1String("1/2")), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-5/3\") * inf", KNumber(QLatin1String("-5/3")) * KNumber::PosInfinity, QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("inf * KNumber(2.01)", KNumber::PosInfinity * KNumber(2.01), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5.4) * inf", KNumber(-5.4) * KNumber::PosInfinity, QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("-inf * KNumber(2)", KNumber::NegInfinity * KNumber(2), QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5) * -inf", KNumber(-5) * KNumber::NegInfinity, QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("-inf * KNumber(\"1/2\")", KNumber::NegInfinity * KNumber(QLatin1String("1/2")), QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-5/3\") * -inf", KNumber(QLatin1String("-5/3")) * KNumber::NegInfinity, QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("-inf * KNumber(2.01)", KNumber::NegInfinity * KNumber(2.01), QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5.4) * -inf", KNumber(-5.4) * KNumber::NegInfinity, QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("nan * KNumber(2)", KNumber::NaN * KNumber(2), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5) * nan", KNumber(-5) * KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("nan * KNumber(\"1/2\")", KNumber::NaN * KNumber(QLatin1String("1/2")), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-5/3\") * nan", KNumber(QLatin1String("-5/3")) * KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("nan * KNumber(2.01)", KNumber::NaN * KNumber(2.01), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5.4) * nan", KNumber(-5.4) * KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("inf * inf", KNumber::PosInfinity * KNumber::PosInfinity, QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("inf * -inf", KNumber::PosInfinity * KNumber::NegInfinity, QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("-inf * inf", KNumber::NegInfinity * KNumber::PosInfinity, QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("-inf * -inf", KNumber::NegInfinity * KNumber::NegInfinity, QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("inf * nan", KNumber::PosInfinity * KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("-inf * nan", KNumber::NegInfinity * KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("nan * inf", KNumber::NaN * KNumber::PosInfinity, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("-inf * nan", KNumber::NegInfinity * KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(0) * inf", KNumber(0) * KNumber::PosInfinity, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(0) * -inf", KNumber(0) * KNumber::NegInfinity, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("inf * KNumber(0)", KNumber::PosInfinity * KNumber(0), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("-inf * KNumber(0)", KNumber::NegInfinity * KNumber(0), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(0.0) * inf", KNumber(0.0) * KNumber::PosInfinity, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(0.0) * -inf", KNumber(0.0) * KNumber::NegInfinity, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("inf * KNumber(0.0)", KNumber::PosInfinity * KNumber(0.0), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("-inf * KNumber(0.0)", KNumber::NegInfinity * KNumber(0.0), QLatin1String("nan"), KNumber::TYPE_ERROR);

	checkResult("inf / KNumber(2)", KNumber::PosInfinity / KNumber(2), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5) / inf", KNumber(-5) / KNumber::PosInfinity, QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("inf / KNumber(\"1/2\")", KNumber::PosInfinity / KNumber(QLatin1String("1/2")), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-5/3\") / inf", KNumber(QLatin1String("-5/3")) / KNumber::PosInfinity, QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("inf / KNumber(2.01)", KNumber::PosInfinity / KNumber(2.01), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5.4) / inf", KNumber(-5.4) / KNumber::PosInfinity, QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("-inf / KNumber(2)", KNumber::NegInfinity / KNumber(2), QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5) / -inf", KNumber(-5) / KNumber::NegInfinity, QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("-inf / KNumber(\"1/2\")", KNumber::NegInfinity / KNumber(QLatin1String("1/2")), QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-5/3\") / -inf", KNumber(QLatin1String("-5/3")) / KNumber::NegInfinity, QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("-inf / KNumber(2.01)", KNumber::NegInfinity / KNumber(2.01), QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5.4) / -inf", KNumber(-5.4) / KNumber::NegInfinity, QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("nan / KNumber(2)", KNumber::NaN / KNumber(2), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5) / nan", KNumber(-5) / KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("nan / KNumber(\"1/2\")", KNumber::NaN / KNumber(QLatin1String("1/2")), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-5/3\") / nan", KNumber(QLatin1String("-5/3")) / KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("nan / KNumber(2.01)", KNumber::NaN / KNumber(2.01), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5.4) / nan", KNumber(-5.4) / KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("inf / inf", KNumber::PosInfinity / KNumber::PosInfinity, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("inf / -inf", KNumber::PosInfinity / KNumber::NegInfinity, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("-inf / inf", KNumber::NegInfinity / KNumber::PosInfinity, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("-inf / -inf", KNumber::NegInfinity / KNumber::NegInfinity, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("inf / nan", KNumber::PosInfinity / KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("-inf / nan", KNumber::NegInfinity / KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("nan / inf", KNumber::NaN / KNumber::PosInfinity, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("-inf / nan", KNumber::NegInfinity / KNumber::NaN, QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(0) / inf", KNumber(0) / KNumber::PosInfinity, QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(0) / -inf", KNumber(0) / KNumber::NegInfinity, QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("inf / KNumber(0)", KNumber::PosInfinity / KNumber(0), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("-inf / KNumber(0)", KNumber::NegInfinity / KNumber(0), QLatin1String("-inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(0.0) / inf", KNumber(0.0) / KNumber::PosInfinity, QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(0.0) / -inf", KNumber(0.0) / KNumber::NegInfinity, QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("inf / KNumber(0.0)", KNumber::PosInfinity / KNumber(0.0), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("-inf / KNumber(0.0)", KNumber::NegInfinity / KNumber(0.0), QLatin1String("-inf"), KNumber::TYPE_ERROR);

	checkResult("KNumber(5) ^ KNumber(\"inf\")", KNumber(5).pow(KNumber::PosInfinity), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5) ^ KNumber(\"inf\")", KNumber(-5).pow(KNumber::PosInfinity), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"5/2\") ^ KNumber(\"inf\")", KNumber(QLatin1String("5/2")).pow(KNumber::PosInfinity), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-5/2\") ^ KNumber(\"inf\")", KNumber(QLatin1String("-5/2")).pow(KNumber::PosInfinity), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"5.2\") ^ KNumber(\"inf\")", KNumber(QLatin1String("5.2")).pow(KNumber::PosInfinity), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-5.2\") ^ KNumber(\"inf\")", KNumber(QLatin1String("-5.2")).pow(KNumber::PosInfinity), QLatin1String("inf"), KNumber::TYPE_ERROR);

	checkResult("KNumber(5) ^ KNumber(\"-inf\")", KNumber(5).pow(KNumber::NegInfinity), QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(-5) ^ KNumber(\"-inf\")", KNumber(-5).pow(KNumber::NegInfinity), QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"5/2\") ^ KNumber(\"-inf\")", KNumber(QLatin1String("5/2")).pow(KNumber::NegInfinity), QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"-5/2\") ^ KNumber(\"-inf\")", KNumber(QLatin1String("-5/2")).pow(KNumber::NegInfinity), QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"5.2\") ^ KNumber(\"-inf\")", KNumber(QLatin1String("5.2")).pow(KNumber::NegInfinity), QLatin1String("0"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"-5.2\") ^ KNumber(\"-inf\")", KNumber(QLatin1String("-5.2")).pow(KNumber::NegInfinity), QLatin1String("0"), KNumber::TYPE_INTEGER);

	checkResult("KNumber(5) ^ KNumber(\"nan\")", KNumber(5).pow(KNumber::NaN), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(-5) ^ KNumber(\"nan\")", KNumber(-5).pow(KNumber::NaN), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"5/2\") ^ KNumber(\"nan\")", KNumber(QLatin1String("5/2")).pow(KNumber::NaN), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-5/2\") ^ KNumber(\"nan\")", KNumber(QLatin1String("-5/2")).pow(KNumber::NaN), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"5.2\") ^ KNumber(\"nan\")", KNumber(QLatin1String("5.2")).pow(KNumber::NaN), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-5.2\") ^ KNumber(\"nan\")", KNumber(QLatin1String("-5.2")).pow(KNumber::NaN), QLatin1String("nan"), KNumber::TYPE_ERROR);


	checkResult("KNumber(\"nan\") ^ KNumber(\"nan\")", KNumber::NaN.pow(KNumber::NaN), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"nan\") ^ KNumber(\"inf\")", KNumber::NaN.pow(KNumber::PosInfinity), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"nan\") ^ KNumber(\"-inf\")", KNumber::NaN.pow(KNumber::NegInfinity), QLatin1String("nan"), KNumber::TYPE_ERROR);

	checkResult("KNumber(\"inf\") ^ KNumber(\"nan\")", KNumber::PosInfinity.pow(KNumber::NaN), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"inf\") ^ KNumber(\"inf\")", KNumber::PosInfinity.pow(KNumber::PosInfinity), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"inf\") ^ KNumber(\"-inf\")", KNumber::PosInfinity.pow(KNumber::NegInfinity), QLatin1String("0"), KNumber::TYPE_INTEGER);

	checkResult("KNumber(\"-inf\") ^ KNumber(\"nan\")", KNumber::NegInfinity.pow(KNumber::NaN), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-inf\") ^ KNumber(\"inf\")", KNumber::NegInfinity.pow(KNumber::PosInfinity), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-inf\") ^ KNumber(\"-inf\")", KNumber::NegInfinity.pow(KNumber::NegInfinity), QLatin1String("0"), KNumber::TYPE_INTEGER);
}

void testingFloatPrecision() {

	KNumber::setDefaultFloatPrecision(100);
	checkResult("Precision >= 100: (KNumber(1) + KNumber(\"1e-80\")) - KNumber(1)", (KNumber(1) + KNumber(QLatin1String("1e-80"))) - KNumber(1), QLatin1String("1e-80"), KNumber::TYPE_FLOAT);
	checkResult("Precision >= 100: (KNumber(1) + KNumber(\"1e-980\")) - KNumber(1)", (KNumber(1) + KNumber(QLatin1String("1e-980"))) - KNumber(1), QLatin1String("0"), KNumber::TYPE_INTEGER);

	KNumber::setDefaultFloatPrecision(1000);
	checkResult("Precision >= 1000: (KNumber(1) + KNumber(\"1e-980\")) - KNumber(1)", (KNumber(1) + KNumber(QLatin1String("1e-980"))) - KNumber(1), QLatin1String("1e-980"), KNumber::TYPE_FLOAT);
	
	KNumber::setDefaultFloatPrecision(20);
	checkResult("Precision >= 20: sin(KNumber(30))", sin(KNumber(30) * (KNumber::Pi() / KNumber(180))), QLatin1String("0.5"), KNumber::TYPE_FLOAT);
	
}

void testingOutput() {

	KNumber::setDefaultFloatOutput(false);
	checkResult("Fractional output: KNumber(\"1/4\")", KNumber(QLatin1String("1/4")), QLatin1String("1/4"), KNumber::TYPE_FRACTION);
	
	KNumber::setDefaultFloatOutput(true);
	checkResult("Float: KNumber(\"1/4\")", KNumber(QLatin1String("1/4")), QLatin1String("0.25"), KNumber::TYPE_FRACTION);
	
	KNumber::setDefaultFloatOutput(false);
	KNumber::setSplitoffIntegerForFractionOutput(true);
	checkResult("Fractional output: KNumber(\"1/4\")", KNumber(QLatin1String("1/4")), QLatin1String("1/4"), KNumber::TYPE_FRACTION);
	checkResult("Fractional output: KNumber(\"-1/4\")", KNumber(QLatin1String("-1/4")), QLatin1String("-1/4"), KNumber::TYPE_FRACTION);
	checkResult("Fractional output: KNumber(\"21/4\")", KNumber(QLatin1String("21/4")), QLatin1String("5 1/4"), KNumber::TYPE_FRACTION);
	checkResult("Fractional output: KNumber(\"-21/4\")", KNumber(QLatin1String("-21/4")), QLatin1String("-5 1/4"), KNumber::TYPE_FRACTION);
	
	KNumber::setSplitoffIntegerForFractionOutput(false);
	checkResult("Fractional output: KNumber(\"1/4\")", KNumber(QLatin1String("1/4")), QLatin1String("1/4"), KNumber::TYPE_FRACTION);
	checkResult("Fractional output: KNumber(\"-1/4\")", KNumber(QLatin1String("-1/4")), QLatin1String("-1/4"), KNumber::TYPE_FRACTION);
	checkResult("Fractional output: KNumber(\"21/4\")", KNumber(QLatin1String("21/4")), QLatin1String("21/4"), KNumber::TYPE_FRACTION);
	checkResult("Fractional output: KNumber(\"-21/4\")", KNumber(QLatin1String("-21/4")), QLatin1String("-21/4"), KNumber::TYPE_FRACTION);
}

void testingConstructors() {
	std::cout << "Testing Constructors:\n";
	std::cout << "---------------------\n";

	checkResult("KNumber(5)", KNumber(5), QLatin1String("5"), KNumber::TYPE_INTEGER);
	checkType(QLatin1String("KNumber(5.3)"), KNumber(5.3).type(), KNumber::TYPE_FLOAT);
	checkType(QLatin1String("KNumber(0.0)"), KNumber(0.0).type(), KNumber::TYPE_INTEGER);

	checkResult("KNumber(\"5\")", KNumber(QLatin1String("5")), QLatin1String("5"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"5/3\")", KNumber(QLatin1String("5/3")), QLatin1String("5/3"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(\"5/1\")", KNumber(QLatin1String("5/1")), QLatin1String("5"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"0/12\")", KNumber(QLatin1String("0/12")), QLatin1String("0"), KNumber::TYPE_INTEGER);
	KNumber::setDefaultFractionalInput(true);
	std::cout << "Read decimals as fractions:\n";
	checkResult("KNumber(\"5\")", KNumber(QLatin1String("5")), QLatin1String("5"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"1.2\")", KNumber(QLatin1String("1.2")), QLatin1String("6/5"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(\"-0.02\")", KNumber(QLatin1String("-0.02")), QLatin1String("-1/50"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(\"5e-2\")", KNumber(QLatin1String("5e-2")), QLatin1String("1/20"), KNumber::TYPE_FRACTION);
	checkResult("KNumber(\"1.2e3\")", KNumber(QLatin1String("1.2e3")), QLatin1String("1200"), KNumber::TYPE_INTEGER);
	checkResult("KNumber(\"0.02e+1\")", KNumber(QLatin1String("0.02e+1")), QLatin1String("1/5"), KNumber::TYPE_FRACTION);

	KNumber::setDefaultFractionalInput(false);
	std::cout << "Read decimals as floats:\n";
	checkResult("KNumber(\"5.3\")", KNumber(QLatin1String("5.3")), QLatin1String("5.3"), KNumber::TYPE_FLOAT);

	checkResult("KNumber(\"nan\")", KNumber(QLatin1String("nan")), QLatin1String("nan"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"inf\")", KNumber(QLatin1String("inf")), QLatin1String("inf"), KNumber::TYPE_ERROR);
	checkResult("KNumber(\"-inf\")", KNumber(QLatin1String("-inf")), QLatin1String("-inf"), KNumber::TYPE_ERROR);

	// test accepting of non-US number formatted strings
	KNumber::setDecimalSeparator(",");
	checkResult("KNumber(\"5,2\")", KNumber(QLatin1String("5,2")), QLatin1String("5.2"), KNumber::TYPE_FLOAT);
	KNumber::setDecimalSeparator(".");
}

void testingConstants() {
	std::cout << "\n\n";
	std::cout << "Constants:\n";
	std::cout << "----------\n";

	checkType(QLatin1String("KNumber::Zero"),   KNumber::Zero.type(),    KNumber::TYPE_INTEGER);
	checkType(QLatin1String("KNumber::One"),    KNumber::One.type(),     KNumber::TYPE_INTEGER);
	checkType(QLatin1String("KNumber::NegOne"), KNumber::NegOne.type(),  KNumber::TYPE_INTEGER);
	checkType(QLatin1String("KNumber::Pi"),     KNumber::Pi().type(),    KNumber::TYPE_FLOAT);
	checkType(QLatin1String("KNumber::Euler"),  KNumber::Euler().type(), KNumber::TYPE_FLOAT);
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

