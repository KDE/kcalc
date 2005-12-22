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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef _KCALC_CORE_H
#define _KCALC_CORE_H

#include "stats.h"
#include <qvaluestack.h>
#include "knumber.h"

#define		POS_ZERO	 1e-19L	 /* What we consider zero is */
#define		NEG_ZERO	-1e-19L	 /* anything between these two */


typedef	KNumber	(*Arith)(const KNumber &, const KNumber &);
typedef	KNumber	(*Prcnt)(const KNumber &, const KNumber &);

#define UNUSED(x) ((void)(x))



struct operator_data {
  int precedence;  // priority of operators in " enum Operation"
  Arith arith_ptr;
  Prcnt prcnt_ptr;
};

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
    FUNC_INTDIV,
    FUNC_POWER,
    FUNC_PWR_ROOT
  };

  CalcEngine();
  
  KNumber lastOutput(bool &error) const;

  void enterOperation(KNumber num, Operation func);


  void ArcCosDeg(KNumber input);
  void ArcCosRad(KNumber input);
  void ArcCosGrad(KNumber input);
  void ArcSinDeg(KNumber input);
  void ArcSinRad(KNumber input);
  void ArcSinGrad(KNumber input);
  void ArcTangensDeg(KNumber input);
  void ArcTangensRad(KNumber input);
  void ArcTangensGrad(KNumber input);
  void AreaCosHyp(KNumber input);
  void AreaSinHyp(KNumber input);
  void AreaTangensHyp(KNumber input);
  void Complement(KNumber input);
  void CosDeg(KNumber input);
  void CosRad(KNumber input);
  void CosGrad(KNumber input);
  void CosHyp(KNumber input);
  void Cube(KNumber input);
  void CubeRoot(KNumber input);
  void Exp(KNumber input);
  void Exp10(KNumber input);
  void Factorial(KNumber input);
  void InvertSign(KNumber input);
  void Ln(KNumber input);
  void Log10(KNumber input);
  void ParenClose(KNumber input);
  void ParenOpen(KNumber input);
  void Reciprocal(KNumber input);
  void SinDeg(KNumber input);
  void SinGrad(KNumber input);
  void SinRad(KNumber input);
  void SinHyp(KNumber input);
  void Square(KNumber input);
  void SquareRoot(KNumber input);
  void StatClearAll(KNumber input);
  void StatCount(KNumber input);
  void StatDataNew(KNumber input);
  void StatDataDel(KNumber input);
  void StatMean(KNumber input);
  void StatMedian(KNumber input);
  void StatStdDeviation(KNumber input);
  void StatStdSample(KNumber input);
  void StatSum(KNumber input);
  void StatSumSquares(KNumber input);
  void TangensDeg(KNumber input);
  void TangensRad(KNumber input);
  void TangensGrad(KNumber input);
  void TangensHyp(KNumber input);

  void Reset();
 private:
  KStats	stats;

  typedef struct {
    KNumber number;
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

  KNumber _last_number;

  bool _percent_mode;


  static const struct operator_data Operator[];

  bool evalStack(void);

  KNumber evalOperation(KNumber arg1, Operation operation, KNumber arg2);

  const KNumber Deg2Rad(const KNumber &x) const
  { return KNumber(2) * KNumber::Pi / KNumber(360) * x; }
  const KNumber Gra2Rad(const KNumber &x) const
  { return KNumber(2)*KNumber::Pi / KNumber(400) * x; }
  const KNumber Rad2Deg(const KNumber &x) const
  { return KNumber(360) / (KNumber(2) * KNumber::Pi) * x; }
  const KNumber Rad2Gra(const KNumber &x) const
  { return KNumber(400) / (KNumber(2)*KNumber::Pi) * x; }

};


#endif  //_KCALC_CORE_H
