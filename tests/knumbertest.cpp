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
    result.toQString().ascii() << "....\n";
  std::cout << "The type of the result should be " <<
    numtypeToString(desired).ascii() << " and gives " <<
    numtypeToString(result.type()).ascii() << "....  ";

  if (result.type() == desired  &&
      result.toQString() == desired_string) {
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

void testingAdditions(void)
{
  std::cout << "Testing additions:\n";
  
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
  std::cout << "Testing subtractions:\n";
  
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
  std::cout << "Testing multiplications:\n";
  
  checkResult("KNumber(5) * KNumber(2)", KNumber(5) * KNumber(2), "10", KNumber::IntegerType);
  checkResult("KNumber(5) * KNumber(\"2/3\")", KNumber(5) * KNumber("2/3"), "10/3", KNumber::FractionType);
  checkResult("KNumber(5) * KNumber(\"2/5\")", KNumber(5) * KNumber("2/5"), "2", KNumber::IntegerType);
  checkResult("KNumber(5) * KNumber(2.3)", KNumber(5) * KNumber(2.3), "11.5", KNumber::FloatType);
  
  checkResult("KNumber(\"5/3\") * KNumber(2)", KNumber("5/3") * KNumber(2), "10/3", KNumber::FractionType);
  checkResult("KNumber(\"5/3\") * KNumber(\"2/3\")", KNumber("5/3") * KNumber("2/3"), "10/9", KNumber::FractionType);
  checkResult("KNumber(\"25/6\") * KNumber(\"12/5\")", KNumber("25/6") * KNumber("12/5"), "10", KNumber::IntegerType);
  checkResult("KNumber(\"5/2\") * KNumber(2.3)", KNumber("5/2") * KNumber(2.3), "5.75",KNumber::FloatType);
  
  checkResult("KNumber(5.3) * KNumber(2)", KNumber(5.3) * KNumber(2), "10.6", KNumber::FloatType);
  checkResult("KNumber(5.3) * KNumber(\"1/2\")", KNumber(5.3) * KNumber("1/2"), "2.65", KNumber::FloatType);
  checkResult("KNumber(5.3) * KNumber(2.3)", KNumber(5.3) * KNumber(2.3), "12.19", KNumber::FloatType);

}

void testingDivisions(void)
{
#warning test for division by zero
  std::cout << "Testing divisions:\n";
  
  checkResult("KNumber(5) / KNumber(2)", KNumber(5) / KNumber(2), "5/2", KNumber::FractionType);
  checkResult("KNumber(122) / KNumber(2)", KNumber(122) / KNumber(2), "61", KNumber::IntegerType);
  checkResult("KNumber(5) / KNumber(\"2/3\")", KNumber(5) / KNumber("2/3"), "15/2", KNumber::FractionType);
  checkResult("KNumber(6) / KNumber(\"2/3\")", KNumber(6) / KNumber("2/3"), "9", KNumber::IntegerType);
  checkResult("KNumber(5) / KNumber(2.5)", KNumber(5) / KNumber(2.5), "2", KNumber::FloatType);
  
  checkResult("KNumber(\"5/3\") / KNumber(2)", KNumber("5/3") / KNumber(2), "5/6", KNumber::FractionType);
  checkResult("KNumber(\"5/3\") / KNumber(\"2/3\")", KNumber("5/3") / KNumber("2/3"), "5/2", KNumber::FractionType);
  checkResult("KNumber(\"49/3\") / KNumber(\"7/9\")", KNumber("49/3") / KNumber("7/9"), "21", KNumber::IntegerType);
  checkResult("KNumber(\"5/2\") / KNumber(2.5)", KNumber("5/2") / KNumber(2.5), "1", KNumber::FloatType);
  
  checkResult("KNumber(5.3) / KNumber(2)", KNumber(5.3) / KNumber(2), "2.65", KNumber::FloatType);
  checkResult("KNumber(5.3) / KNumber(\"2/3\")", KNumber(5.3) / KNumber("2/3"), "7.95", KNumber::FloatType);
  checkResult("KNumber(5.5) / KNumber(2.5)", KNumber(5.5) / KNumber(2.5), "2.2", KNumber::FloatType);

}

void testingModulus(void)
{
  std::cout << "Testing modulus:\n";
  
  checkResult("KNumber(23) % KNumber(4)", KNumber(23) % KNumber(4), "3", KNumber::IntegerType);
  checkResult("KNumber(12) % KNumber(-5)", KNumber(12) % KNumber(-5), "2", KNumber::IntegerType);

#warning test for other types, division by zero

}

void testingAndOr(void)
{
  std::cout << "Testing And/Or:\n";
  
  checkResult("KNumber(17) & KNumber(9)", KNumber(17) & KNumber(9), "1", KNumber::IntegerType);
  checkResult("KNumber(17) | KNumber(9)", KNumber(17) | KNumber(9), "25", KNumber::IntegerType);
  checkResult("KNumber(1023) & KNumber(255)", KNumber(1023) & KNumber(255), "255", KNumber::IntegerType);
  checkResult("KNumber(1023) | KNumber(255)", KNumber(1023) | KNumber(255), "1023", KNumber::IntegerType);

#warning test for other types

}


void testingAbs(void)
{
  std::cout << "Testing absolute value:\n";
  
  checkResult("KNumber(5).abs()", KNumber(5).abs(), "5", KNumber::IntegerType);
  checkResult("KNumber(\"2/3\").abs()", KNumber("2/3").abs(), "2/3", KNumber::FractionType);
  checkResult("KNumber(\"2.3\").abs()", KNumber("2.3").abs(), "2.3", KNumber::FloatType);
  
  checkResult("KNumber(-5).abs()", KNumber(-5).abs(), "5", KNumber::IntegerType);
  checkResult("KNumber(\"-2/3\").abs()", KNumber("-2/3").abs(), "2/3", KNumber::FractionType);
  checkResult("KNumber(\"-2.3\").abs()", KNumber("-2.3").abs(), "2.3", KNumber::FloatType);
}

void testingTruncateToInteger(void)
{
  std::cout << "Testing truncate to an integer:\n";
  
  checkResult("KNumber(16).integerPart()", KNumber(16).integerPart(), "16", KNumber::IntegerType);
  checkResult("KNumber(\"43/9\").integerPart()", KNumber("43/9").integerPart(), "4", KNumber::IntegerType);
  checkResult("KNumber(\"-43/9\").integerPart()", KNumber("-43/9").integerPart(), "-4", KNumber::IntegerType);
  checkResult("KNumber(\"5.25\").integerPart()", KNumber("5.25").integerPart(), "5", KNumber::IntegerType);
  checkResult("KNumber(\"-5.25\").integerPart()", KNumber("-5.25").integerPart(), "-5", KNumber::IntegerType);  
}


void testingSqrt(void)
{
  std::cout << "Testing square root:\n";
  
  checkResult("KNumber(16).sqrt()", KNumber(16).sqrt(), "4", KNumber::IntegerType);
  checkResult("KNumber(\"16/9\").sqrt()", KNumber("16/9").sqrt(), "4/3", KNumber::FractionType);
  checkResult("KNumber(2).sqrt()", KNumber(2).sqrt(), "1.4142136", KNumber::FloatType);
  checkResult("KNumber(\"2/3\").sqrt()", KNumber("2/3").sqrt(), "0.81649658", KNumber::FloatType);
  checkResult("KNumber(\"0.25\").sqrt()", KNumber("0.25").sqrt(), "0.5", KNumber::FloatType);
  
}

void testingShifts(void)
{
  std::cout << "Testing left/right shift:\n";
  
  checkResult("KNumber(16) << KNumber(2)", KNumber(16) << KNumber(2), "64", KNumber::IntegerType);
  checkResult("KNumber(16) >> KNumber(2)", KNumber(16) >> KNumber(2), "4", KNumber::IntegerType);
  
}



int main(void)
{
  std::cout << "Testing Constructors:\n";

  checkResult("KNumber(5)", KNumber(5), "5", KNumber::IntegerType);
  checkType("KNumber(5.3)", KNumber(5.3).type(), KNumber::FloatType);

  checkResult("KNumber(\"5\")", KNumber("5"), "5", KNumber::IntegerType);
  checkResult("KNumber(\"5/3\")", KNumber("5/3"), "5/3", KNumber::FractionType);
  checkResult("KNumber(\"5.3\")", KNumber("5.3"), "5.3", KNumber::FloatType);

  std::cout << "\n\nConstants:\n";

  checkType("KNumber::Zero", KNumber::Zero.type(), KNumber::IntegerType);
  checkType("KNumber::One", KNumber::One.type(), KNumber::IntegerType);
  checkType("KNumber::MinusOne", KNumber::MinusOne.type(), KNumber::IntegerType);

  testingAdditions();
  testingSubtractions();
  testingMultiplications();
  testingDivisions();

  testingAndOr();
  testingModulus();

  testingAbs();
  testingSqrt();
  testingTruncateToInteger();

  testingShifts();

  return 0;
}


