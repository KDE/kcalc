/*

    KCalc, a scientific calculator for the X window system using the
    Qt widget libraries, available at no cost at http://www.troll.no

    Copyright (C) 1996 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef QTCALC_CORE_H
#define QTCALC_CORE_H

#include "stats.h"
#include <qvaluestack.h>
#include "kcalctype.h"

#define		POS_ZERO	 1e-19L	 /* What we consider zero is */
#define		NEG_ZERO	-1e-19L	 /* anything between these two */


typedef	CALCAMNT	(*Arith)(CALCAMNT, CALCAMNT);
typedef	CALCAMNT	(*Prcnt)(CALCAMNT, CALCAMNT);
typedef	CALCAMNT	(*Trig)(CALCAMNT);

#define UNUSED(x) ((void)(x))



typedef struct {
	int item_function;
	int item_precedence;
} func_data;

class CalcEngine
{
 public:
  // operations that can be stored in calculation stack 
  enum Operation {
    FUNC_EQUAL,
    FUNC_PERCENT,
    FUNC_BRACKET,
    FUNC_OR,
    FUNC_XOR,
    FUNC_AND,
    FUNC_LSH,
    FUNC_RSH,
    FUNC_ADD,
    FUNC_SUBTRACT,
    FUNC_MULTIPLY,
    FUNC_DIVIDE,
    FUNC_MOD,
    FUNC_POWER,
    FUNC_PWR_ROOT,
    FUNC_INTDIV
  };

  CalcEngine();
  
  CALCAMNT lastOutput(bool &error) const;

  void enterOperation(CALCAMNT num, Operation func);


  void ArcCosDeg(CALCAMNT input);
  void ArcCosRad(CALCAMNT input);
  void ArcCosGrad(CALCAMNT input);
  void ArcSinDeg(CALCAMNT input);
  void ArcSinRad(CALCAMNT input);
  void ArcSinGrad(CALCAMNT input);
  void ArcTangensDeg(CALCAMNT input);
  void ArcTangensRad(CALCAMNT input);
  void ArcTangensGrad(CALCAMNT input);
  void AreaCosHyp(CALCAMNT input);
  void AreaSinHyp(CALCAMNT input);
  void AreaTangensHyp(CALCAMNT input);
  void Complement(CALCAMNT input);
  void CosDeg(CALCAMNT input);
  void CosRad(CALCAMNT input);
  void CosGrad(CALCAMNT input);
  void CosHyp(CALCAMNT input);
  void Exp(CALCAMNT input);
  void Exp10(CALCAMNT input);
  void Factorial(CALCAMNT input);
  void InvertSign(CALCAMNT input);
  void Ln(CALCAMNT input);
  void Log10(CALCAMNT input);
  void ParenClose(CALCAMNT input);
  void ParenOpen(CALCAMNT input);
  void Reciprocal(CALCAMNT input);
  void SinDeg(CALCAMNT input);
  void SinGrad(CALCAMNT input);
  void SinRad(CALCAMNT input);
  void SinHyp(CALCAMNT input);
  void Square(CALCAMNT input);
  void SquareRoot(CALCAMNT input);
  void StatClearAll(CALCAMNT input);
  void StatCount(CALCAMNT input);
  void StatDataNew(CALCAMNT input);
  void StatDataDel(CALCAMNT input);
  void StatMean(CALCAMNT input);
  void StatMedian(CALCAMNT input);
  void StatStdDeviation(CALCAMNT input);
  void StatStdSample(CALCAMNT input);
  void StatSum(CALCAMNT input);
  void StatSumSquares(CALCAMNT input);
  void TangensDeg(CALCAMNT input);
  void TangensRad(CALCAMNT input);
  void TangensGrad(CALCAMNT input);
  void TangensHyp(CALCAMNT input);

  void Reset();
 private:
  KStats	stats;

  typedef struct {
    CALCAMNT number;
    Operation operation;
  } _node;

  // Stack holds all operations and numbers that have not yet been
  // processed, e.g. user types "2+3*", the calculation can not be
  // executed, because "*" has a higher precedence than "+", so we
  // need to wait for the next number.
  //
  // In the stack this would be stored as ((2,+),(3,*),...)
  //
  // "enterOperation": If the introduced Operation has lower priority
  // than the preceding operations in the stack, then we can start to
  // evaluate the stack (with "evalStack"). Otherwise we append the new
  // Operation and number to the stack.
  //
  // E.g. "2*3+" evaluates to "6+", but "2+3*" can not be evaluated
  // yet.
  //
  // We also take care of brackets, by writing a marker "FUNC_BRACKET"
  // into the stack, each time the user opens one.  When a bracket is
  // closed, everything in the stack is evaluated until the first
  // marker "FUNC_BRACKET" found.
  QValueStack<_node> _stack;

  CALCAMNT _last_number;

  bool _percent_mode;

  int _precedence_list[20]; // priority of operators in " enum Operation"
  static const CALCAMNT pi;

  Arith Arith_ops[20];
  Prcnt Prcnt_ops[20];

  bool evalStack(void);

  CALCAMNT evalOperation(CALCAMNT arg1, Operation operation, CALCAMNT arg2);

  CALCAMNT Deg2Rad(CALCAMNT x)	{ return (((2L * pi) / 360L) * x); }
  CALCAMNT Gra2Rad(CALCAMNT x)	{ return ((pi / 200L) * x); }
  CALCAMNT Rad2Deg(CALCAMNT x)	{ return ((360L / (2L * pi)) * x); }
  CALCAMNT Rad2Gra(CALCAMNT x)	{ return ((200L / pi) * x); }

};


#endif  //QTCALC_CORE_H
