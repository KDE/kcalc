//
// Author: Klaus Niederkrueger <kniederk@math.uni-koeln.de>
//

#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <qstring.h>

#include "knumbertest.h"

QString const  numtypeToString(KNumber::NumType arg)
{
  switch(arg) {
  case KNumber::SpecialType:
    return QString("Special");
  case KNumber::IntegerType:
    return QString("Integer");
  case KNumber::FractionType:
    return QString("Fraction");
  case KNumber::FloatType:
    return QString("Float");
  default:
    return QString("Unknown:") + QString::number(static_cast<int>(arg));

  }
}

void checkResult(QString const &string, KNumber const & result,
		 QString const & desired_string, KNumber::NumType desired)
{
  std::cout << "Testing result of: " << string.ascii() <<
    " should give " << desired_string.ascii() << " and gives " <<
    result.toQString(8).ascii() << "....\n";
  std::cout << "The type of the result should be " <<
    numtypeToString(desired).ascii() << " and gives " <<
    numtypeToString(result.type()).ascii() << "....  ";

  if (result.type() == desired  &&
      result.toQString(8) == desired_string) {
    std::cout << "OK\n";
    return;
  }
  
  std::cout << "Failed\n";
  exit(1);
}

void checkTruth(QString const &string, bool computation,
		bool desired_result)
{
  std::cout << "Testing truth of: " << string.ascii() <<
    " should be " << desired_result << " and is " <<
    computation << "....\n";

  if (computation == desired_result) {
    std::cout << "OK\n";
    return;
  }
  
  std::cout << "Failed\n";
  exit(1);
}


void checkType(QString const &string, KNumber::NumType test_arg,
	       KNumber::NumType desired)
{
  std::cout << "Testing type of: " << string.ascii() << " should give " <<
    numtypeToString(desired).ascii() << " and gives " <<
    numtypeToString(test_arg).ascii() << "....";

  if (test_arg == desired) {
    std::cout << "OK\n";
    return;
  }

  std::cout << "Failed\n";
  exit(1);
    
}


void testingCompare(void)
{
  std::cout << "\n\nTesting Compare:\n";
  
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

  checkTruth("KNumber(5) == KNumber(\"1/2\")", KNumber(5) == KNumber("1/2"), false);
  checkTruth("KNumber(5) > KNumber(\"1/2\")", KNumber(5) > KNumber("1/2"), true);
  checkTruth("KNumber(5) < KNumber(\"1/2\")", KNumber(5) < KNumber("1/2"), false);
  checkTruth("KNumber(5) >= KNumber(\"1/2\")", KNumber(5) >= KNumber("1/2"), true);
  checkTruth("KNumber(5) <= KNumber(\"1/2\")", KNumber(5) <= KNumber("1/2"), false);
  checkTruth("KNumber(5) != KNumber(\"1/2\")", KNumber(5) != KNumber("1/2"), true);

  checkTruth("KNumber(\"1/2\") == KNumber(\"1/2\")", KNumber("1/2") == KNumber("1/2"), true);
  checkTruth("KNumber(\"1/2\") > KNumber(\"1/2\")", KNumber("1/2") > KNumber("1/2"), false);
  checkTruth("KNumber(\"1/2\") < KNumber(\"1/2\")", KNumber("1/2") < KNumber("1/2"), false);
  checkTruth("KNumber(\"1/2\") >= KNumber(\"1/2\")", KNumber("1/2") >= KNumber("1/2"), true);
  checkTruth("KNumber(\"1/2\") <= KNumber(\"1/2\")", KNumber("1/2") <= KNumber("1/2"), true);
  checkTruth("KNumber(\"1/2\") != KNumber(\"1/2\")", KNumber("1/2") != KNumber("1/2"), false);

  checkTruth("KNumber(\"3/2\") == KNumber(\"1/2\")", KNumber("3/2") == KNumber("1/2"), false);
  checkTruth("KNumber(\"3/2\") > KNumber(\"1/2\")", KNumber("3/2") > KNumber("1/2"), true);
  checkTruth("KNumber(\"3/2\") < KNumber(\"1/2\")", KNumber("3/2") < KNumber("1/2"), false);
  checkTruth("KNumber(\"3/2\") >= KNumber(\"1/2\")", KNumber("3/2") >= KNumber("1/2"), true);
  checkTruth("KNumber(\"3/2\") <= KNumber(\"1/2\")", KNumber("3/2") <= KNumber("1/2"), false);
  checkTruth("KNumber(\"3/2\") != KNumber(\"1/2\")", KNumber("3/2") != KNumber("1/2"), true);

}


void testingAdditions(void)
{
  std::cout << "\n\nTesting additions:\n";
  
  checkResult("KNumber(5) + KNumber(2)", KNumber(5) + KNumber(2), "7", KNumber::IntegerType);
  checkResult("KNumber(5) + KNumber(\"2/3\")", KNumber(5) + KNumber("2/3"), "17/3", KNumber::FractionType);
  checkResult("KNumber(5) + KNumber(\"2.3\")", KNumber(5) + KNumber("2.3"), "7.3", KNumber::FloatType);
  
  checkResult("KNumber(\"5/3\") + KNumber(2)", KNumber("5/3") + KNumber(2), "11/3", KNumber::FractionType);
  checkResult("KNumber(\"5/3\") + KNumber(\"2/3\")", KNumber("5/3") + KNumber("2/3"), "7/3", KNumber::FractionType);
  checkResult("KNumber(\"5/3\") + KNumber(\"1/3\")", KNumber("5/3") + KNumber("1/3"), "2", KNumber::IntegerType);
  checkResult("KNumber(\"5/3\") + KNumber(\"-26/3\")", KNumber("5/3") + KNumber("-26/3"), "-7", KNumber::IntegerType);
  checkResult("KNumber(\"5/2\") + KNumber(2.3)", KNumber("5/2") + KNumber(2.3), "4.8", KNumber::FloatType);
  
  checkResult("KNumber(5.3) + KNumber(2)", KNumber(5.3) + KNumber(2), "7.3", KNumber::FloatType);
  checkResult("KNumber(5.3) + KNumber(\"2/4\")", KNumber(5.3) + KNumber("2/4"), "5.8", KNumber::FloatType);
  checkResult("KNumber(5.3) + KNumber(2.3)", KNumber(5.3) + KNumber(2.3), "7.6", KNumber::FloatType);

}

void testingSubtractions(void)
{
  std::cout << "\n\nTesting subtractions:\n";
  
  checkResult("KNumber(5) - KNumber(2)", KNumber(5) - KNumber(2), "3", KNumber::IntegerType);
  checkResult("KNumber(5) - KNumber(\"2/3\")", KNumber(5) - KNumber("2/3"), "13/3", KNumber::FractionType);
  checkResult("KNumber(5) - KNumber(2.3)", KNumber(5) - KNumber(2.3), "2.7", KNumber::FloatType);
  
  checkResult("KNumber(\"5/3\") - KNumber(2)", KNumber("5/3") - KNumber(2), "-1/3", KNumber::FractionType);
  checkResult("KNumber(\"5/3\") - KNumber(\"1/3\")", KNumber("5/3") - KNumber("1/3"), "4/3", KNumber::FractionType);
  checkResult("KNumber(\"5/3\") - KNumber(\"2/3\")", KNumber("5/3") - KNumber("2/3"), "1", KNumber::IntegerType);
  checkResult("KNumber(\"-5/3\") - KNumber(\"4/3\")", KNumber("-5/3") - KNumber("4/3"), "-3", KNumber::IntegerType);
  checkResult("KNumber(\"5/4\") - KNumber(2.2)", KNumber("5/4") - KNumber(2.2), "-0.95", KNumber::FloatType);
  
  checkResult("KNumber(5.3) - KNumber(2)", KNumber(5.3) - KNumber(2), "3.3", KNumber::FloatType);
  checkResult("KNumber(5.3) - KNumber(\"3/4\")", KNumber(5.3) - KNumber("3/4"), "4.55", KNumber::FloatType);
  checkResult("KNumber(5.3) - KNumber(2.3)", KNumber(5.3) - KNumber(2.3), "3", KNumber::FloatType);

}


void testingMultiplications(void)
{
  std::cout << "\n\nTesting multiplications:\n";
  
  checkResult("KNumber(5) * KNumber(2)", KNumber(5) * KNumber(2), "10", KNumber::IntegerType);
  checkResult("KNumber(5) * KNumber(\"2/3\")", KNumber(5) * KNumber("2/3"), "10/3", KNumber::FractionType);
  checkResult("KNumber(5) * KNumber(\"2/5\")", KNumber(5) * KNumber("2/5"), "2", KNumber::IntegerType);
  checkResult("KNumber(5) * KNumber(2.3)", KNumber(5) * KNumber(2.3), "11.5", KNumber::FloatType);
  checkResult("KNumber(0) * KNumber(\"2/5\")", KNumber(0) * KNumber("2/5"), "0", KNumber::IntegerType);
  checkResult("KNumber(0) * KNumber(2.3)", KNumber(0) * KNumber(2.3), "0", KNumber::IntegerType);
  
  checkResult("KNumber(\"5/3\") * KNumber(2)", KNumber("5/3") * KNumber(2), "10/3", KNumber::FractionType);
  checkResult("KNumber(\"5/3\") * KNumber(0)", KNumber("5/3") * KNumber(0), "0", KNumber::IntegerType);
  checkResult("KNumber(\"5/3\") * KNumber(\"2/3\")", KNumber("5/3") * KNumber("2/3"), "10/9", KNumber::FractionType);
  checkResult("KNumber(\"25/6\") * KNumber(\"12/5\")", KNumber("25/6") * KNumber("12/5"), "10", KNumber::IntegerType);
  checkResult("KNumber(\"5/2\") * KNumber(2.3)", KNumber("5/2") * KNumber(2.3), "5.75",KNumber::FloatType);
  
  checkResult("KNumber(5.3) * KNumber(2)", KNumber(5.3) * KNumber(2), "10.6", KNumber::FloatType);
  checkResult("KNumber(5.3) * KNumber(0)", KNumber(5.3) * KNumber(0), "0", KNumber::IntegerType);
  checkResult("KNumber(5.3) * KNumber(\"1/2\")", KNumber(5.3) * KNumber("1/2"), "2.65", KNumber::FloatType);
  checkResult("KNumber(5.3) * KNumber(2.3)", KNumber(5.3) * KNumber(2.3), "12.19", KNumber::FloatType);

}

void testingDivisions(void)
{
  std::cout << "\n\nTesting divisions:\n";
  
  checkResult("KNumber(5) / KNumber(2)", KNumber(5) / KNumber(2), "5/2", KNumber::FractionType);
  checkResult("KNumber(122) / KNumber(2)", KNumber(122) / KNumber(2), "61", KNumber::IntegerType);
  checkResult("KNumber(12) / KNumber(0)", KNumber(12) / KNumber(0), "inf", KNumber::SpecialType);
  checkResult("KNumber(-12) / KNumber(0)", KNumber(-12) / KNumber(0), "-inf", KNumber::SpecialType);
  checkResult("KNumber(5) / KNumber(\"2/3\")", KNumber(5) / KNumber("2/3"), "15/2", KNumber::FractionType);
  checkResult("KNumber(6) / KNumber(\"2/3\")", KNumber(6) / KNumber("2/3"), "9", KNumber::IntegerType);
  checkResult("KNumber(5) / KNumber(2.5)", KNumber(5) / KNumber(2.5), "2", KNumber::FloatType);
  checkResult("KNumber(5) / KNumber(0.0)", KNumber(5) / KNumber(0.0), "inf", KNumber::SpecialType);
  checkResult("KNumber(-5) / KNumber(0.0)", KNumber(-5) / KNumber(0.0), "-inf", KNumber::SpecialType);
  
  checkResult("KNumber(\"5/3\") / KNumber(2)", KNumber("5/3") / KNumber(2), "5/6", KNumber::FractionType);
  checkResult("KNumber(\"5/3\") / KNumber(0)", KNumber("5/3") / KNumber(0), "inf", KNumber::SpecialType);
  checkResult("KNumber(\"-5/3\") / KNumber(0)", KNumber("-5/3") / KNumber(0), "-inf", KNumber::SpecialType);
  checkResult("KNumber(\"5/3\") / KNumber(\"2/3\")", KNumber("5/3") / KNumber("2/3"), "5/2", KNumber::FractionType);
  checkResult("KNumber(\"49/3\") / KNumber(\"7/9\")", KNumber("49/3") / KNumber("7/9"), "21", KNumber::IntegerType);
  checkResult("KNumber(\"5/2\") / KNumber(2.5)", KNumber("5/2") / KNumber(2.5), "1", KNumber::FloatType);
  checkResult("KNumber(\"5/2\") / KNumber(0.0)", KNumber("5/2") / KNumber(0.0), "inf", KNumber::SpecialType);
  checkResult("KNumber(\"-5/2\") / KNumber(0.0)", KNumber("-5/2") / KNumber(0.0), "-inf", KNumber::SpecialType);
  
  checkResult("KNumber(5.3) / KNumber(2)", KNumber(5.3) / KNumber(2), "2.65", KNumber::FloatType);
  checkResult("KNumber(5.3) / KNumber(0)", KNumber(5.3) / KNumber(0), "inf", KNumber::SpecialType);
  checkResult("KNumber(-5.3) / KNumber(0)", KNumber(-5.3) / KNumber(0), "-inf", KNumber::SpecialType);
  checkResult("KNumber(5.3) / KNumber(\"2/3\")", KNumber(5.3) / KNumber("2/3"), "7.95", KNumber::FloatType);
  checkResult("KNumber(5.5) / KNumber(2.5)", KNumber(5.5) / KNumber(2.5), "2.2", KNumber::FloatType);
  checkResult("KNumber(5.5) / KNumber(0.0)", KNumber(5.5) / KNumber(0.0), "inf", KNumber::SpecialType);
  checkResult("KNumber(-5.5) / KNumber(0.0)", KNumber(-5.5) / KNumber(0.0), "-inf", KNumber::SpecialType);
}

void testingModulus(void)
{
  std::cout << "\n\nTesting modulus:\n";
  
  checkResult("KNumber(23) % KNumber(4)", KNumber(23) % KNumber(4), "3", KNumber::IntegerType);
  checkResult("KNumber(12) % KNumber(-5)", KNumber(12) % KNumber(-5), "2", KNumber::IntegerType);
  checkResult("KNumber(-12) % KNumber(5)", KNumber(-12) % KNumber(5), "3", KNumber::IntegerType);
  checkResult("KNumber(12) % KNumber(0)", KNumber(-12) % KNumber(0), "nan", KNumber::SpecialType);
  checkResult("KNumber(-12) % KNumber(0)", KNumber(-12) % KNumber(0), "nan", KNumber::SpecialType);

#warning test for other types

}

void testingAndOr(void)
{
  std::cout << "\n\nTesting And/Or:\n";
  
  checkResult("KNumber(17) & KNumber(9)", KNumber(17) & KNumber(9), "1", KNumber::IntegerType);
  checkResult("KNumber(17) | KNumber(9)", KNumber(17) | KNumber(9), "25", KNumber::IntegerType);
  checkResult("KNumber(1023) & KNumber(255)", KNumber(1023) & KNumber(255), "255", KNumber::IntegerType);
  checkResult("KNumber(1023) | KNumber(255)", KNumber(1023) | KNumber(255), "1023", KNumber::IntegerType);

#warning test for other types

}


void testingAbs(void)
{
  std::cout << "\n\nTesting absolute value:\n";
  
  checkResult("KNumber(5).abs()", KNumber(5).abs(), "5", KNumber::IntegerType);
  checkResult("KNumber(\"2/3\").abs()", KNumber("2/3").abs(), "2/3", KNumber::FractionType);
  checkResult("KNumber(\"2.3\").abs()", KNumber("2.3").abs(), "2.3", KNumber::FloatType);
  
  checkResult("KNumber(-5).abs()", KNumber(-5).abs(), "5", KNumber::IntegerType);
  checkResult("KNumber(\"-2/3\").abs()", KNumber("-2/3").abs(), "2/3", KNumber::FractionType);
  checkResult("KNumber(\"-2.3\").abs()", KNumber("-2.3").abs(), "2.3", KNumber::FloatType);
}

void testingTruncateToInteger(void)
{
  std::cout << "\n\nTesting truncate to an integer:\n";
  
  checkResult("KNumber(16).integerPart()", KNumber(16).integerPart(), "16", KNumber::IntegerType);
  checkResult("KNumber(\"43/9\").integerPart()", KNumber("43/9").integerPart(), "4", KNumber::IntegerType);
  checkResult("KNumber(\"-43/9\").integerPart()", KNumber("-43/9").integerPart(), "-4", KNumber::IntegerType);
  checkResult("KNumber(\"5.25\").integerPart()", KNumber("5.25").integerPart(), "5", KNumber::IntegerType);
  checkResult("KNumber(\"-5.25\").integerPart()", KNumber("-5.25").integerPart(), "-5", KNumber::IntegerType);  
}


void testingSqrt(void)
{
  std::cout << "\n\nTesting square root, cubic root:\n";
  
  checkResult("KNumber(16).sqrt()", KNumber(16).sqrt(), "4", KNumber::IntegerType);
  checkResult("KNumber(-16).sqrt()", KNumber(-16).sqrt(), "nan", KNumber::SpecialType);
  checkResult("KNumber(\"16/9\").sqrt()", KNumber("16/9").sqrt(), "4/3", KNumber::FractionType);
  checkResult("KNumber(\"-16/9\").sqrt()", KNumber("-16/9").sqrt(), "nan", KNumber::SpecialType);
  checkResult("KNumber(2).sqrt()", KNumber(2).sqrt(), "1.4142136", KNumber::FloatType);
  checkResult("KNumber(\"2/3\").sqrt()", KNumber("2/3").sqrt(), "0.81649658", KNumber::FloatType);
  checkResult("KNumber(\"0.25\").sqrt()", KNumber("0.25").sqrt(), "0.5", KNumber::FloatType);
  checkResult("KNumber(\"-0.25\").sqrt()", KNumber("-0.25").sqrt(), "nan", KNumber::SpecialType);
  

  checkResult("KNumber(27).cbrt()", KNumber(27).cbrt(), "3", KNumber::IntegerType);
  checkResult("KNumber(-27).cbrt()", KNumber(-27).cbrt(), "-3", KNumber::IntegerType);
  checkResult("KNumber(\"27/8\").cbrt()", KNumber("27/8").cbrt(), "3/2", KNumber::FractionType);
  checkResult("KNumber(\"-8/27\").cbrt()", KNumber("-8/27").cbrt(), "-2/3", KNumber::FractionType);
#warning need to check non-perfect cube roots
  //  checkResult("KNumber(2).cbrt()", KNumber(2).cbrt(), "1.4142136", KNumber::FloatType);
  //  checkResult("KNumber(\"2/3\").cbrt()", KNumber("2/3").cbrt(), "0.81649658", KNumber::FloatType);
  //  checkResult("KNumber(\"0.25\").cbrt()", KNumber("0.25").cbrt(), "0.5", KNumber::FloatType);
  //  checkResult("KNumber(\"-0.25\").cbrt()", KNumber("-0.25").cbrt(), "nan", KNumber::SpecialType);

}

void testingShifts(void)
{
  std::cout << "\n\nTesting left/right shift:\n";
  
  checkResult("KNumber(16) << KNumber(2)", KNumber(16) << KNumber(2), "64", KNumber::IntegerType);
  checkResult("KNumber(16) >> KNumber(2)", KNumber(16) >> KNumber(2), "4", KNumber::IntegerType);
  
}

void testingPower(void)
{
  std::cout << "\n\nTesting Power:\n";
  
  checkResult("KNumber(0) ^ KNumber(-4)", KNumber(0).power(KNumber(-4)), "inf", KNumber::SpecialType);
  checkResult("KNumber(5) ^ KNumber(4)", KNumber(5).power(KNumber(4)), "625", KNumber::IntegerType);
  checkResult("KNumber(122) ^ KNumber(0)", KNumber(122).power(KNumber(0)), "1", KNumber::IntegerType);
  checkResult("KNumber(-5) ^ KNumber(0)", KNumber(-5).power(KNumber(0)), "nan", KNumber::SpecialType);
  checkResult("KNumber(-2) ^ KNumber(3)", KNumber(-2).power(KNumber(3)), "-8", KNumber::IntegerType);
  checkResult("KNumber(-2) ^ KNumber(4)", KNumber(-2).power(KNumber(4)), "16", KNumber::IntegerType);
  checkResult("KNumber(5) ^ KNumber(-2)", KNumber(5).power(KNumber(-2)), "1/25", KNumber::FractionType);
  checkResult("KNumber(8) ^ KNumber(\"2/3\")", KNumber(8).power(KNumber("2/3")), "4", KNumber::IntegerType);
  checkResult("KNumber(8) ^ KNumber(\"-2/3\")", KNumber(8).power(KNumber("-2/3")), "1/4", KNumber::FractionType);
  checkResult("KNumber(-16) ^ KNumber(\"1/4\")", KNumber(-16).power(KNumber("1/4")), "nan", KNumber::SpecialType);
  checkResult("KNumber(-8) ^ KNumber(\"1/3\")", KNumber(-8).power(KNumber("1/3")), "nan", KNumber::SpecialType);
  checkResult("KNumber(5) ^ KNumber(0.0)", KNumber(5).power(KNumber(0.0)), "1", KNumber::IntegerType);
  checkResult("KNumber(-5) ^ KNumber(0.0)", KNumber(-5).power(KNumber(0.0)), "nan", KNumber::SpecialType);
  
  checkResult("KNumber(\"5/3\") ^ KNumber(2)", KNumber("5/3").power(KNumber(2)), "25/9", KNumber::FractionType);
  checkResult("KNumber(\"5/3\") ^ KNumber(0)", KNumber("5/3").power(KNumber(0)), "1", KNumber::IntegerType);
  checkResult("KNumber(\"-5/3\") ^ KNumber(0)", KNumber("-5/3").power(KNumber(0)), "nan", KNumber::SpecialType);
  checkResult("KNumber(\"8/27\") ^ KNumber(\"2/3\")", KNumber("8/27").power(KNumber("2/3")), "4/9", KNumber::FractionType);
  checkResult("KNumber(\"49/3\") ^ KNumber(\"7/9\")", KNumber("49/3").power(KNumber("7/9")), "21", KNumber::IntegerType);
  checkResult("KNumber(\"5/2\") ^ KNumber(2.5)", KNumber("5/2").power(KNumber(2.5)), "1", KNumber::FloatType);
  checkResult("KNumber(\"5/2\") ^ KNumber(0.0)", KNumber("5/2").power(KNumber(0.0)), "inf", KNumber::SpecialType);
  checkResult("KNumber(\"-5/2\") ^ KNumber(0.0)", KNumber("-5/2").power(KNumber(0.0)), "-inf", KNumber::SpecialType);
  
  checkResult("KNumber(5.3) ^ KNumber(2)", KNumber(5.3).power(KNumber(2)), "2.65", KNumber::FloatType);
  checkResult("KNumber(5.3) ^ KNumber(0)", KNumber(5.3).power(KNumber(0)), "inf", KNumber::SpecialType);
  checkResult("KNumber(-5.3) ^ KNumber(0)", KNumber(-5.3).power(KNumber(0)), "-inf", KNumber::SpecialType);
  checkResult("KNumber(5.3) ^ KNumber(\"2/3\")", KNumber(5.3).power(KNumber("2/3")), "7.95", KNumber::FloatType);
  checkResult("KNumber(5.5) ^ KNumber(2.5)", KNumber(5.5).power(KNumber(2.5)), "2.2", KNumber::FloatType);
  checkResult("KNumber(5.5) ^ KNumber(0.0)", KNumber(5.5).power(KNumber(0.0)), "inf", KNumber::SpecialType);
  checkResult("KNumber(-5.5) ^ KNumber(0.0)", KNumber(-5.5).power(KNumber(0.0)), "-inf", KNumber::SpecialType);
}

void testingInfArithmetic(void)
{
  std::cout << "\n\nTesting inf/nan-arithmetics:\n";

  KNumber tmp_inf = KNumber("inf");
  KNumber tmp_mininf = KNumber("-inf");
  KNumber tmp_nan = KNumber("nan");

  checkResult("inf + KNumber(2)", tmp_inf + KNumber(2), "inf", KNumber::SpecialType);
  checkResult("KNumber(-5) + inf", KNumber(-5) + tmp_inf, "inf", KNumber::SpecialType);
  checkResult("inf + KNumber(\"1/2\")", tmp_inf + KNumber("1/2"), "inf", KNumber::SpecialType);
  checkResult("KNumber(\"-5/3\") + inf", KNumber("-5/3") + tmp_inf, "inf", KNumber::SpecialType);
  checkResult("inf + KNumber(2.01)", tmp_inf + KNumber(2.01), "inf", KNumber::SpecialType);
  checkResult("KNumber(-5.4) + inf", KNumber(-5.4) + tmp_inf, "inf", KNumber::SpecialType);
  checkResult("mininf + KNumber(2)", tmp_mininf + KNumber(2), "-inf", KNumber::SpecialType);
  checkResult("KNumber(-5) + mininf", KNumber(-5) + tmp_mininf, "-inf", KNumber::SpecialType);
  checkResult("mininf + KNumber(\"1/2\")", tmp_mininf + KNumber("1/2"), "-inf", KNumber::SpecialType);
  checkResult("KNumber(\"-5/3\") + mininf", KNumber("-5/3") + tmp_mininf, "-inf", KNumber::SpecialType);
  checkResult("mininf + KNumber(2.01)", tmp_mininf + KNumber(2.01), "-inf", KNumber::SpecialType);
  checkResult("KNumber(-5.4) + mininf", KNumber(-5.4) + tmp_mininf, "-inf", KNumber::SpecialType);
  checkResult("nan + KNumber(2)", tmp_nan + KNumber(2), "nan", KNumber::SpecialType);
  checkResult("KNumber(-5) + nan", KNumber(-5) + tmp_nan, "nan", KNumber::SpecialType);
  checkResult("nan + KNumber(\"1/2\")", tmp_nan + KNumber("1/2"), "nan", KNumber::SpecialType);
  checkResult("KNumber(\"-5/3\") + nan", KNumber("-5/3") + tmp_nan, "nan", KNumber::SpecialType);
  checkResult("nan + KNumber(2.01)", tmp_nan + KNumber(2.01), "nan", KNumber::SpecialType);
  checkResult("KNumber(-5.4) + nan", KNumber(-5.4) + tmp_nan, "nan", KNumber::SpecialType);
  checkResult("inf + inf", tmp_inf + tmp_inf, "inf", KNumber::SpecialType);
  checkResult("inf + mininf", tmp_inf + tmp_mininf, "nan", KNumber::SpecialType);
  checkResult("mininf + inf", tmp_mininf + tmp_inf, "nan", KNumber::SpecialType);
  checkResult("mininf + mininf", tmp_mininf + tmp_mininf, "-inf", KNumber::SpecialType);
  checkResult("inf + nan", tmp_inf + tmp_nan, "nan", KNumber::SpecialType);
  checkResult("mininf + nan", tmp_mininf + tmp_nan, "nan", KNumber::SpecialType);
  checkResult("nan + inf", tmp_nan + tmp_inf, "nan", KNumber::SpecialType);
  checkResult("mininf + nan", tmp_mininf + tmp_nan, "nan", KNumber::SpecialType);


  checkResult("inf - KNumber(2)", tmp_inf - KNumber(2), "inf", KNumber::SpecialType);
  checkResult("KNumber(-5) - inf", KNumber(-5) - tmp_inf, "-inf", KNumber::SpecialType);
  checkResult("inf - KNumber(\"1/2\")", tmp_inf - KNumber("1/2"), "inf", KNumber::SpecialType);
  checkResult("KNumber(\"-5/3\") - inf", KNumber("-5/3") - tmp_inf, "-inf", KNumber::SpecialType);
  checkResult("inf - KNumber(2.01)", tmp_inf - KNumber(2.01), "inf", KNumber::SpecialType);
  checkResult("KNumber(-5.4) - inf", KNumber(-5.4) - tmp_inf, "-inf", KNumber::SpecialType);
  checkResult("mininf - KNumber(2)", tmp_mininf - KNumber(2), "-inf", KNumber::SpecialType);
  checkResult("KNumber(-5) - mininf", KNumber(-5) - tmp_mininf, "inf", KNumber::SpecialType);
  checkResult("mininf - KNumber(\"1/2\")", tmp_mininf - KNumber("1/2"), "-inf", KNumber::SpecialType);
  checkResult("KNumber(\"-5/3\") - mininf", KNumber("-5/3") - tmp_mininf, "inf", KNumber::SpecialType);
  checkResult("mininf - KNumber(2.01)", tmp_mininf - KNumber(2.01), "-inf", KNumber::SpecialType);
  checkResult("KNumber(-5.4) - mininf", KNumber(-5.4) - tmp_mininf, "inf", KNumber::SpecialType);
  checkResult("nan - KNumber(2)", tmp_nan - KNumber(2), "nan", KNumber::SpecialType);
  checkResult("KNumber(-5) - nan", KNumber(-5) - tmp_nan, "nan", KNumber::SpecialType);
  checkResult("nan - KNumber(\"1/2\")", tmp_nan - KNumber("1/2"), "nan", KNumber::SpecialType);
  checkResult("KNumber(\"-5/3\") - nan", KNumber("-5/3") - tmp_nan, "nan", KNumber::SpecialType);
  checkResult("nan - KNumber(2.01)", tmp_nan - KNumber(2.01), "nan", KNumber::SpecialType);
  checkResult("KNumber(-5.4) - nan", KNumber(-5.4) - tmp_nan, "nan", KNumber::SpecialType);
  checkResult("inf - inf", tmp_inf - tmp_inf, "nan", KNumber::SpecialType);
  checkResult("inf - mininf", tmp_inf - tmp_mininf, "inf", KNumber::SpecialType);
  checkResult("mininf - inf", tmp_mininf - tmp_inf, "-inf", KNumber::SpecialType);
  checkResult("mininf - mininf", tmp_mininf - tmp_mininf, "nan", KNumber::SpecialType);
  checkResult("inf - nan", tmp_inf - tmp_nan, "nan", KNumber::SpecialType);
  checkResult("mininf - nan", tmp_mininf - tmp_nan, "nan", KNumber::SpecialType);
  checkResult("nan - inf", tmp_nan - tmp_inf, "nan", KNumber::SpecialType);
  checkResult("mininf - nan", tmp_mininf - tmp_nan, "nan", KNumber::SpecialType);


  checkResult("inf * KNumber(2)", tmp_inf * KNumber(2), "inf", KNumber::SpecialType);
  checkResult("KNumber(-5) * inf", KNumber(-5) * tmp_inf, "-inf", KNumber::SpecialType);
  checkResult("inf * KNumber(\"1/2\")", tmp_inf * KNumber("1/2"), "inf", KNumber::SpecialType);
  checkResult("KNumber(\"-5/3\") * inf", KNumber("-5/3") * tmp_inf, "-inf", KNumber::SpecialType);
  checkResult("inf * KNumber(2.01)", tmp_inf * KNumber(2.01), "inf", KNumber::SpecialType);
  checkResult("KNumber(-5.4) * inf", KNumber(-5.4) * tmp_inf, "-inf", KNumber::SpecialType);
  checkResult("mininf * KNumber(2)", tmp_mininf * KNumber(2), "-inf", KNumber::SpecialType);
  checkResult("KNumber(-5) * mininf", KNumber(-5) * tmp_mininf, "inf", KNumber::SpecialType);
  checkResult("mininf * KNumber(\"1/2\")", tmp_mininf * KNumber("1/2"), "-inf", KNumber::SpecialType);
  checkResult("KNumber(\"-5/3\") * mininf", KNumber("-5/3") * tmp_mininf, "inf", KNumber::SpecialType);
  checkResult("mininf * KNumber(2.01)", tmp_mininf * KNumber(2.01), "-inf", KNumber::SpecialType);
  checkResult("KNumber(-5.4) * mininf", KNumber(-5.4) * tmp_mininf, "inf", KNumber::SpecialType);
  checkResult("nan * KNumber(2)", tmp_nan * KNumber(2), "nan", KNumber::SpecialType);
  checkResult("KNumber(-5) * nan", KNumber(-5) * tmp_nan, "nan", KNumber::SpecialType);
  checkResult("nan * KNumber(\"1/2\")", tmp_nan * KNumber("1/2"), "nan", KNumber::SpecialType);
  checkResult("KNumber(\"-5/3\") * nan", KNumber("-5/3") * tmp_nan, "nan", KNumber::SpecialType);
  checkResult("nan * KNumber(2.01)", tmp_nan * KNumber(2.01), "nan", KNumber::SpecialType);
  checkResult("KNumber(-5.4) * nan", KNumber(-5.4) * tmp_nan, "nan", KNumber::SpecialType);
  checkResult("inf * inf", tmp_inf * tmp_inf, "inf", KNumber::SpecialType);
  checkResult("inf * mininf", tmp_inf * tmp_mininf, "-inf", KNumber::SpecialType);
  checkResult("mininf * inf", tmp_mininf * tmp_inf, "-inf", KNumber::SpecialType);
  checkResult("mininf * mininf", tmp_mininf * tmp_mininf, "inf", KNumber::SpecialType);
  checkResult("inf * nan", tmp_inf * tmp_nan, "nan", KNumber::SpecialType);
  checkResult("mininf * nan", tmp_mininf * tmp_nan, "nan", KNumber::SpecialType);
  checkResult("nan * inf", tmp_nan * tmp_inf, "nan", KNumber::SpecialType);
  checkResult("mininf * nan", tmp_mininf * tmp_nan, "nan", KNumber::SpecialType);
  checkResult("KNumber(0) * inf", KNumber(0) * tmp_inf, "nan", KNumber::SpecialType);
  checkResult("KNumber(0) * mininf", KNumber(0) * tmp_mininf, "nan", KNumber::SpecialType);
  checkResult("inf * KNumber(0)", tmp_inf * KNumber(0), "nan", KNumber::SpecialType);
  checkResult("mininf * KNumber(0)", tmp_mininf * KNumber(0), "nan", KNumber::SpecialType);
  checkResult("KNumber(0.0) * inf", KNumber(0.0) * tmp_inf, "nan", KNumber::SpecialType);
  checkResult("KNumber(0.0) * mininf", KNumber(0.0) * tmp_mininf, "nan", KNumber::SpecialType);
  checkResult("inf * KNumber(0.0)", tmp_inf * KNumber(0.0), "nan", KNumber::SpecialType);
  checkResult("mininf * KNumber(0.0)", tmp_mininf * KNumber(0.0), "nan", KNumber::SpecialType);


  checkResult("inf / KNumber(2)", tmp_inf / KNumber(2), "inf", KNumber::SpecialType);
  checkResult("KNumber(-5) / inf", KNumber(-5) / tmp_inf, "0", KNumber::IntegerType);
  checkResult("inf / KNumber(\"1/2\")", tmp_inf / KNumber("1/2"), "inf", KNumber::SpecialType);
  checkResult("KNumber(\"-5/3\") / inf", KNumber("-5/3") / tmp_inf, "0", KNumber::IntegerType);
  checkResult("inf / KNumber(2.01)", tmp_inf / KNumber(2.01), "inf", KNumber::SpecialType);
  checkResult("KNumber(-5.4) / inf", KNumber(-5.4) / tmp_inf, "0", KNumber::IntegerType);
  checkResult("mininf / KNumber(2)", tmp_mininf / KNumber(2), "-inf", KNumber::SpecialType);
  checkResult("KNumber(-5) / mininf", KNumber(-5) / tmp_mininf, "0", KNumber::IntegerType);
  checkResult("mininf / KNumber(\"1/2\")", tmp_mininf / KNumber("1/2"), "-inf", KNumber::SpecialType);
  checkResult("KNumber(\"-5/3\") / mininf", KNumber("-5/3") / tmp_mininf, "0", KNumber::IntegerType);
  checkResult("mininf / KNumber(2.01)", tmp_mininf / KNumber(2.01), "-inf", KNumber::SpecialType);
  checkResult("KNumber(-5.4) / mininf", KNumber(-5.4) / tmp_mininf, "0", KNumber::IntegerType);
  checkResult("nan / KNumber(2)", tmp_nan / KNumber(2), "nan", KNumber::SpecialType);
  checkResult("KNumber(-5) / nan", KNumber(-5) / tmp_nan, "nan", KNumber::SpecialType);
  checkResult("nan / KNumber(\"1/2\")", tmp_nan / KNumber("1/2"), "nan", KNumber::SpecialType);
  checkResult("KNumber(\"-5/3\") / nan", KNumber("-5/3") / tmp_nan, "nan", KNumber::SpecialType);
  checkResult("nan / KNumber(2.01)", tmp_nan / KNumber(2.01), "nan", KNumber::SpecialType);
  checkResult("KNumber(-5.4) / nan", KNumber(-5.4) / tmp_nan, "nan", KNumber::SpecialType);
  checkResult("inf / inf", tmp_inf / tmp_inf, "nan", KNumber::SpecialType);
  checkResult("inf / mininf", tmp_inf / tmp_mininf, "nan", KNumber::SpecialType);
  checkResult("mininf / inf", tmp_mininf / tmp_inf, "nan", KNumber::SpecialType);
  checkResult("mininf / mininf", tmp_mininf / tmp_mininf, "nan", KNumber::SpecialType);
  checkResult("inf / nan", tmp_inf / tmp_nan, "nan", KNumber::SpecialType);
  checkResult("mininf / nan", tmp_mininf / tmp_nan, "nan", KNumber::SpecialType);
  checkResult("nan / inf", tmp_nan / tmp_inf, "nan", KNumber::SpecialType);
  checkResult("mininf / nan", tmp_mininf / tmp_nan, "nan", KNumber::SpecialType);
  checkResult("KNumber(0) / inf", KNumber(0) / tmp_inf, "0", KNumber::IntegerType);
  checkResult("KNumber(0) / mininf", KNumber(0) / tmp_mininf, "0", KNumber::IntegerType);
  checkResult("inf / KNumber(0)", tmp_inf / KNumber(0), "inf", KNumber::SpecialType);
  checkResult("mininf / KNumber(0)", tmp_mininf / KNumber(0), "-inf", KNumber::SpecialType);
  checkResult("KNumber(0.0) / inf", KNumber(0.0) / tmp_inf, "0", KNumber::IntegerType);
  checkResult("KNumber(0.0) / mininf", KNumber(0.0) / tmp_mininf, "0", KNumber::IntegerType);
  checkResult("inf / KNumber(0.0)", tmp_inf / KNumber(0.0), "inf", KNumber::SpecialType);
  checkResult("mininf / KNumber(0.0)", tmp_mininf / KNumber(0.0), "-inf", KNumber::SpecialType);
}

void testingFloatPrecision(void)
{
  KNumber::setDefaultFloatPrecision(100);
  checkResult("Precision >= 100: (KNumber(1) + KNumber(\"1e-80\")) - KNumber(1)",
	      (KNumber(1) + KNumber("1e-80")) - KNumber(1), "1e-80", KNumber::FloatType);
  checkResult("Precision >= 100: (KNumber(1) + KNumber(\"1e-980\")) - KNumber(1)",
	      (KNumber(1) + KNumber("1e-980")) - KNumber(1), "0", KNumber::FloatType);

  KNumber::setDefaultFloatPrecision(1000);
  checkResult("Precision >= 1000: (KNumber(1) + KNumber(\"1e-980\")) - KNumber(1)",
	      (KNumber(1) + KNumber("1e-980")) - KNumber(1), "1e-980", KNumber::FloatType);

}

void testingOutput(void)
{
  KNumber::setDefaultFloatOutput(false);
  checkResult("Fractional output: KNumber(\"1/4\")", KNumber("1/4"), "1/4", KNumber::FractionType);
  KNumber::setDefaultFloatOutput(true);
  checkResult("Float: KNumber(\"1/4\")", KNumber("1/4"), "0.25", KNumber::FractionType);
  KNumber::setDefaultFloatOutput(false);
  KNumber::setSplitoffIntegerForFractionOutput(true);
  checkResult("Fractional output: KNumber(\"1/4\")", KNumber("1/4"), "1/4", KNumber::FractionType);
  checkResult("Fractional output: KNumber(\"-1/4\")", KNumber("-1/4"), "-1/4", KNumber::FractionType);
  checkResult("Fractional output: KNumber(\"21/4\")", KNumber("21/4"), "5 1/4", KNumber::FractionType);
  checkResult("Fractional output: KNumber(\"-21/4\")", KNumber("-21/4"), "-5 1/4", KNumber::FractionType);
  KNumber::setSplitoffIntegerForFractionOutput(false);
  checkResult("Fractional output: KNumber(\"1/4\")", KNumber("1/4"), "1/4", KNumber::FractionType);
  checkResult("Fractional output: KNumber(\"-1/4\")", KNumber("-1/4"), "-1/4", KNumber::FractionType);
  checkResult("Fractional output: KNumber(\"21/4\")", KNumber("21/4"), "21/4", KNumber::FractionType);
  checkResult("Fractional output: KNumber(\"-21/4\")", KNumber("-21/4"), "-21/4", KNumber::FractionType);
}


int main(void)
{
  std::cout << "Testing Constructors:\n";

  checkResult("KNumber(5)", KNumber(5), "5", KNumber::IntegerType);
  checkType("KNumber(5.3)", KNumber(5.3).type(), KNumber::FloatType);
  checkType("KNumber(0.0)", KNumber(0.0).type(), KNumber::FloatType);

  checkResult("KNumber(\"5\")", KNumber("5"), "5", KNumber::IntegerType);
  checkResult("KNumber(\"5/3\")", KNumber("5/3"), "5/3", KNumber::FractionType);
  checkResult("KNumber(\"5/1\")", KNumber("5/1"), "5", KNumber::IntegerType);
  checkResult("KNumber(\"0/12\")", KNumber("0/12"), "0", KNumber::IntegerType);
  KNumber::setDefaultFractionalInput(true);
  std::cout << "Read decimals as fractions:\n";
  checkResult("KNumber(\"5\")", KNumber("5"), "5", KNumber::IntegerType);
  checkResult("KNumber(\"1.2\")", KNumber("1.2"), "6/5", KNumber::FractionType);
  checkResult("KNumber(\"-0.02\")", KNumber("-0.02"), "-1/50", KNumber::FractionType);
  checkResult("KNumber(\"5e-2\")", KNumber("5e-2"), "1/20", KNumber::FractionType);
  checkResult("KNumber(\"1.2e3\")", KNumber("1.2e3"), "1200", KNumber::IntegerType);
  checkResult("KNumber(\"0.02e+1\")", KNumber("0.02e+1"), "1/5", KNumber::FractionType);

  KNumber::setDefaultFractionalInput(false);
  std::cout << "Read decimals as floats:\n";
  checkResult("KNumber(\"5.3\")", KNumber("5.3"), "5.3", KNumber::FloatType);

  checkResult("KNumber(\"nan\")", KNumber("nan"), "nan", KNumber::SpecialType);
  checkResult("KNumber(\"inf\")", KNumber("inf"), "inf", KNumber::SpecialType);
  checkResult("KNumber(\"-inf\")", KNumber("-inf"), "-inf", KNumber::SpecialType);

  std::cout << "\n\nConstants:\n";

  checkType("KNumber::Zero", KNumber::Zero.type(), KNumber::IntegerType);
  checkType("KNumber::One", KNumber::One.type(), KNumber::IntegerType);
  checkType("KNumber::MinusOne", KNumber::MinusOne.type(), KNumber::IntegerType);
  checkType("KNumber::Pi", KNumber::Pi.type(), KNumber::FloatType);

  testingCompare();

  testingAdditions();
  testingSubtractions();
  testingMultiplications();
  testingDivisions();

  testingAndOr();
  testingModulus();

  testingAbs();
  testingSqrt();
  //testingPower();
  testingTruncateToInteger();

  testingShifts();

  testingInfArithmetic();

  testingFloatPrecision();

  testingOutput();

  return 0;
}


