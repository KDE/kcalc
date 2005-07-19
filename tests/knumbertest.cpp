//
// Author: Klaus Niederkrueger <kniederk@math.uni-koeln.de>
//

#include <stdlib.h>
#include <stdio.h>

#include <iostream>

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
  checkResult("KNumber(\"5/3\") - KNumber(\"2/3\")", KNumber("5/3") - KNumber("2/3"), "1", KNumber::FractionType);
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
  checkResult("KNumber(5) * KNumber(2.3)", KNumber(5) * KNumber(2.3), "11.5", KNumber::FloatType);
  
  checkResult("KNumber(\"5/3\") * KNumber(2)", KNumber("5/3") * KNumber(2), "10/3", KNumber::FractionType);
  checkResult("KNumber(\"5/3\") * KNumber(\"2/3\")", KNumber("5/3") * KNumber("2/3"), "10/9", KNumber::FractionType);
  checkResult("KNumber(\"5/2\") * KNumber(2.3)", KNumber("5/2") * KNumber(2.3), "5.75",KNumber::FloatType);
  
  checkResult("KNumber(5.3) * KNumber(2)", KNumber(5.3) * KNumber(2), "10.6", KNumber::FloatType);
  checkResult("KNumber(5.3) * KNumber(\"1/2\")", KNumber(5.3) * KNumber("1/2"), "2.65", KNumber::FloatType);
  checkResult("KNumber(5.3) * KNumber(2.3)", KNumber(5.3) * KNumber(2.3), "12.19", KNumber::FloatType);

}

void testingDivisions(void)
{
  std::cout << "Testing divisions:\n";
  
  checkResult("KNumber(5) / KNumber(2)", KNumber(5) / KNumber(2), "5/2", KNumber::FractionType);
  checkResult("KNumber(5) / KNumber(\"2/3\")", KNumber(5) / KNumber("2/3"), "15/2", KNumber::FractionType);
  checkResult("KNumber(5) / KNumber(2.5)", KNumber(5) / KNumber(2.5), "2", KNumber::FloatType);
  
  checkResult("KNumber(\"5/3\") / KNumber(2)", KNumber("5/3") / KNumber(2), "5/6", KNumber::FractionType);
  checkResult("KNumber(\"5/3\") / KNumber(\"2/3\")", KNumber("5/3") / KNumber("2/3"), "5/2", KNumber::FractionType);
  checkResult("KNumber(\"5/2\") / KNumber(2.5)", KNumber("5/2") / KNumber(2.5), "1", KNumber::FloatType);
  
  checkResult("KNumber(5.3) / KNumber(2)", KNumber(5.3) / KNumber(2), "2.65", KNumber::FloatType);
  checkResult("KNumber(5.3) / KNumber(\"2/3\")", KNumber(5.3) / KNumber("2/3"), "7.95", KNumber::FloatType);
  checkResult("KNumber(5.5) / KNumber(2.5)", KNumber(5.5) / KNumber(2.5), "2.2", KNumber::FloatType);

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


int main(void)
{
  std::cout << "Testing Constructors:\n";

  checkResult("KNumber(5)", KNumber(5), "5", KNumber::IntegerType);
  checkType("KNumber(5.3)", KNumber(5.3).type(), KNumber::FloatType);

  checkResult("KNumber(\"5\")", KNumber("5"), "5", KNumber::IntegerType);
  checkResult("KNumber(\"5/3\")", KNumber("5/3"), "5/3", KNumber::FractionType);
  checkResult("KNumber(\"5.3\")", KNumber("5.3"), "5.3", KNumber::FloatType);

  std::cout << "\n\nConstants:\n";

  checkType("KNumber::ZeroInteger", KNumber::ZeroInteger.type(), KNumber::IntegerType);
  checkType("KNumber::OneInteger", KNumber::OneInteger.type(), KNumber::IntegerType);
  checkType("KNumber::MinusOneInteger", KNumber::MinusOneInteger.type(), KNumber::IntegerType);

  std::cout << "\n";

  checkType("KNumber::ZeroFloat", KNumber::ZeroFloat.type(), KNumber::FloatType);
  checkType("KNumber::OneFloat", KNumber::OneFloat.type(), KNumber::FloatType);
  checkType("KNumber::MinusOneFloat", KNumber::MinusOneFloat.type(), KNumber::FloatType);

  testingAdditions();
  testingSubtractions();
  testingMultiplications();
  testingDivisions();
 

  testingAbs();

  return 0;
}


