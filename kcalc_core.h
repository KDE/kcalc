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

#define STACK_SIZE	100
#define PRECEDENCE_INCR	20

#define FUNC_NULL		0
#define FUNC_OR			1
#define FUNC_XOR		2
#define FUNC_AND		3
#define FUNC_LSH		4
#define FUNC_RSH		5
#define FUNC_ADD		6
#define FUNC_SUBTRACT		7
#define FUNC_MULTIPLY		8
#define FUNC_DIVIDE		9
#define FUNC_MOD		10
#define FUNC_POWER		11
#define FUNC_PWR_ROOT		12
#define FUNC_INTDIV		13


#define		POS_ZERO	 1e-19L	 /* What we consider zero is */
#define		NEG_ZERO	-1e-19L	 /* anything between these two */


typedef	CALCAMNT	(*Arith)(CALCAMNT, CALCAMNT);
typedef	CALCAMNT	(*Prcnt)(CALCAMNT, CALCAMNT);
typedef	CALCAMNT	(*Trig)(CALCAMNT);

#define UNUSED(x) ((void)(x))

typedef enum {
	ANG_DEGREE = 0,
	ANG_RADIAN = 1,
	ANG_GRADIENT = 2
} angle_type;


typedef struct {
	int item_function;
	int item_precedence;
} func_data;

class CalcEngine
{
 public:
  CalcEngine();
  
  CALCAMNT last_output(bool &error) const;

  void Equal(CALCAMNT input);
  
  void And(CALCAMNT input);
  void ArcCos(CALCAMNT input);
  void ArcSin(CALCAMNT input);
  void ArcTangens(CALCAMNT input);
  void AreaCosHyp(CALCAMNT input);
  void AreaSinHyp(CALCAMNT input);
  void AreaTangensHyp(CALCAMNT input);
  void Complement(CALCAMNT input);
  void Cos(CALCAMNT input);
  void CosHyp(CALCAMNT input);
  void Divide(CALCAMNT input);
  void Exp(CALCAMNT input);
  void Exp10(CALCAMNT input);
  void Factorial(CALCAMNT input);
  void InvMod(CALCAMNT input);
  void InvPower(CALCAMNT input);
  void Ln(CALCAMNT input);
  void Log10(CALCAMNT input);
  void Minus(CALCAMNT input);
  void Multiply(CALCAMNT input);
  void Mod(CALCAMNT input);
  void Or(CALCAMNT input);
  void ParenClose(CALCAMNT input);
  void ParenOpen(CALCAMNT input);
  void Percent(CALCAMNT input);
  void Plus(CALCAMNT input);
  void Power(CALCAMNT input);
  void Reciprocal(CALCAMNT input);
  void ShiftLeft(CALCAMNT input);
  void ShiftRight(CALCAMNT input);
  void Sin(CALCAMNT input);
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
  void Tangens(CALCAMNT input);
  void TangensHyp(CALCAMNT input);
  void Xor(CALCAMNT input);

  void Reset();
  void SetAngleMode(int mode) { _angle_mode = mode; };
 private:
  KStats	stats;

  CALCAMNT _last_result;
  int _angle_mode;

  QValueStack<CALCAMNT>	amount_stack;
  QValueStack<func_data> func_stack;

  int precedence_base;
  bool percent_mode;

  static const CALCAMNT pi;

  Arith Arith_ops[14];
  Prcnt Prcnt_ops[14];

  void EnterStackFunction(int func, CALCAMNT num);
  int UpdateStack(int run_precedence);

  CALCAMNT Deg2Rad(CALCAMNT x)	{ return (((2L * pi) / 360L) * x); }
  CALCAMNT Gra2Rad(CALCAMNT x)	{ return ((pi / 200L) * x); }
  CALCAMNT Rad2Deg(CALCAMNT x)	{ return ((360L / (2L * pi)) * x); }
  CALCAMNT Rad2Gra(CALCAMNT x)	{ return ((200L / pi) * x); }

};


#endif  //QTCALC_CORE_H
