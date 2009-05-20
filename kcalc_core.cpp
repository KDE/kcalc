/*
    kCalculator, a scientific calculator for the X window system using the
    Qt widget libraries, available at no cost at http://www.troll.no

    The stack engine contained in this file was take from
    Martin Bartlett's xfrmcalc

    portions:	Copyright (C) 2003-2006 Klaus Niederkrueger
                                        kniederk@ulb.ac.be

    portions:	Copyright (C) 1996 Bernd Johannes Wuebben
                                   wuebben@math.cornell.edu

    portions: 	Copyright (C) 1995 Martin Bartlett

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

#include "kcalc_core.h"

#include <config-kcalc.h>

#if defined(_ISOC99_SOURCE)
	#include <cassert>
	#include <cstdio>
	#include <climits>
	#include <csignal>
	#include <cerrno>
	#include <cstring>
	using namespace std;
#else
	#include <limits.h>
	#include <stdio.h>
	#include <assert.h>	
	#include <signal.h>
	#include <errno.h>
	#include <string.h>
#endif

#include <stdlib.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include "kcalctype.h"

#ifndef HAVE_FUNC_ISINF
        #ifdef HAVE_IEEEFP_H
                #include <ieeefp.h>
        #else
                #include <math.h>
        #endif

#undef isinf
int isinf(double x) 
{
#ifdef _HPUX_SOURCE
return !isfinite(x) && x == x;
#else
return !finite(x) && x==x; 
#endif
}

#endif


static void fpe_handler(int fpe_parm)
{
	UNUSED(fpe_parm);
	//	display_error = true;
	//tmp_number = 0L;
}


static bool _error;

static KNumber ExecOr(const KNumber & left_op, const KNumber & right_op)
{
	return (left_op | right_op);
}

static KNumber ExecXor(const KNumber & left_op, const KNumber & right_op)
{
	return (left_op | right_op) - (left_op & right_op);
}

static KNumber ExecAnd(const KNumber & left_op, const KNumber & right_op)
{
	return (left_op & right_op);
}

static KNumber ExecLsh(const KNumber & left_op, const KNumber & right_op)
{
	return left_op << right_op;
}

static KNumber ExecRsh(const KNumber & left_op, const KNumber & right_op)
{
	return left_op >> right_op;
}

static KNumber ExecAdd(const KNumber & left_op, const KNumber & right_op)
{
	return left_op + right_op;
}

static KNumber ExecSubtract(const KNumber & left_op, const KNumber & right_op)
{
	return left_op - right_op;
}

static KNumber ExecMultiply(const KNumber & left_op, const KNumber & right_op)
{
	return left_op * right_op;
}

static KNumber ExecDivide(const KNumber & left_op, const KNumber & right_op)
{
	return left_op / right_op;
}

static KNumber ExecMod(const KNumber & left_op, const KNumber & right_op)
{
	return left_op % right_op;
}

static KNumber ExecIntDiv(const KNumber & left_op, const KNumber & right_op)
{
  	return (left_op / right_op).integerPart();
}

bool isoddint(const KNumber & input)
{
	if (input.type() != KNumber::IntegerType) return false;
	// Routine to check if KNumber is an Odd integer
	return ( (input / KNumber(2)).type() == KNumber::IntegerType);
}

  
static KNumber ExecBinom(const KNumber & left_op, const KNumber & right_op) 
{
    // TODO: use libgmp mpz_bin_ui

	if (left_op.type() != KNumber::IntegerType
	    ||  right_op.type() != KNumber::IntegerType
	    ||  right_op > left_op  ||  left_op < KNumber::Zero)
		return KNumber::UndefinedNumber;

	KNumber tmp_count = left_op;
	KNumber tmp_result = KNumber::One;

	// don't do recursive factorial,
	// because large numbers lead to
	// stack overflows
	while (tmp_count > right_op)
	{
		tmp_result = tmp_count * tmp_result;
		tmp_count -= KNumber::One;
	}

	tmp_count = left_op-right_op;
	while (tmp_count > KNumber::One)
	{
		tmp_result = tmp_result / tmp_count;
		tmp_count -= KNumber::One;
	}


	return tmp_result;
} 

static KNumber ExecPower(const KNumber & left_op, const KNumber & right_op)
{
	return left_op.power(right_op);
}

static KNumber ExecPwrRoot(const KNumber & left_op, const KNumber & right_op)
{
	return left_op.power(KNumber::One / right_op);
}

static KNumber ExecAddP(const KNumber & left_op, const KNumber & right_op)
{
	return left_op * (KNumber::One + right_op/KNumber(100));
}

static KNumber ExecSubP(const KNumber & left_op, const KNumber & right_op)
{
	return left_op * (KNumber::One - right_op/KNumber(100));
}

static KNumber ExecMultiplyP(const KNumber & left_op, const KNumber & right_op)
{
	return left_op * right_op / KNumber(100);
}

static KNumber ExecDivideP(const KNumber & left_op, const KNumber & right_op)
{
	return left_op * KNumber(100) / right_op;
}


// build precedence list
const struct operator_data CalcEngine::Operator[] = {
  { 0, NULL,     NULL}, // FUNC_EQUAL
  { 0, NULL,     NULL}, // FUNC_PERCENT
  { 0, NULL,     NULL}, // FUNC_BRACKET
  { 1, ExecOr,   NULL}, // FUNC_OR
  { 2, ExecXor,  NULL}, // FUNC_XOR
  { 3, ExecAnd,  NULL}, // FUNC_AND
  { 4, ExecLsh,  NULL}, // FUNC_LSH
  { 4, ExecRsh,  NULL}, // FUNC_RSH
  { 5, ExecAdd,  ExecAddP}, // FUNC_ADD
  { 5, ExecSubtract, ExecSubP}, // FUNC_SUBTRACT
  { 6, ExecMultiply, ExecMultiplyP}, // FUNC_MULTIPLY
  { 6, ExecDivide,   ExecDivideP}, // FUNC_DIVIDE
  { 6, ExecMod,  NULL}, // FUNC_MOD
  { 6, ExecIntDiv, NULL}, // FUNC_INTDIV
  { 7, ExecBinom, NULL},  // FUNC_BINOM 
  { 7, ExecPower,  NULL}, // FUNC_POWER
  { 7, ExecPwrRoot, NULL} // FUNC_PWR_ROOT
};


CalcEngine::CalcEngine()
  :   _percent_mode(false)
{
	//
	// Basic initialization involves initializing the calcultion
	// stack, and setting up the floating point excetion signal
	// handler to trap the errors that the code can/has not been
	// written to trap.
	//
#ifndef Q_OS_WIN
	struct sigaction fpe_trap;

	sigemptyset(&fpe_trap.sa_mask);
	fpe_trap.sa_handler = &fpe_handler;
#ifdef SA_RESTART
	fpe_trap.sa_flags = SA_RESTART;
#endif
	sigaction(SIGFPE, &fpe_trap, NULL);
#endif
	_last_number = KNumber::Zero;
	_error = false;
}

KNumber CalcEngine::lastOutput(bool &error) const
{
	error = _error;
	return _last_number;
}

void CalcEngine::ArcCosDeg(KNumber input)
{
	if (input.type() == KNumber::SpecialType  ||
	    input < -KNumber::One  ||  input > KNumber::One) {
		_last_number = KNumber("nan");
		return;
	}

	if (input.type() == KNumber::IntegerType) {
		if (input == KNumber::One) {
		  _last_number = KNumber::Zero;
			return;
		}
		if (input == - KNumber::One) {
			_last_number = KNumber(180);
			return;
		}
		if (input == KNumber::Zero) {
			_last_number = KNumber(90);
			return;
		}
	}
	_last_number = Rad2Deg(KNumber(acos(static_cast<double>(input))));
}

void CalcEngine::ArcCosRad(KNumber input)
{
	if (input.type() == KNumber::SpecialType  ||
	    input < -KNumber::One  ||  input > KNumber::One) {
		_last_number = KNumber("nan");
		return;
	}
	_last_number = KNumber(acos(static_cast<double>(input)));
}

void CalcEngine::ArcCosGrad(KNumber input)
{
	if (input.type() == KNumber::SpecialType  ||
	    input < -KNumber::One  ||  input > KNumber::One) {
		_last_number = KNumber("nan");
		return;
	}
	if (input.type() == KNumber::IntegerType) {
		if (input == KNumber::One) {
			_last_number = KNumber::Zero;
			return;
		}
		if (input == - KNumber::One) {
			_last_number = KNumber(200);
			return;
		}
		if (input == KNumber::Zero) {
			_last_number = KNumber(100);
			return;
		}
	}
	_last_number = Rad2Gra(KNumber(acos(static_cast<double>(input))));
}

void CalcEngine::ArcSinDeg(KNumber input)
{
	if (input.type() == KNumber::SpecialType  ||
	    input < -KNumber::One  ||  input > KNumber::One) {
		_last_number = KNumber("nan");
		return;
	}
	if (input.type() == KNumber::IntegerType) {
		if (input == KNumber::One) {
			_last_number = KNumber(90);
			return;
		}
		if (input == - KNumber::One) {
			_last_number = KNumber(-90);
			return;
		}
		if (input == KNumber::Zero) {
			_last_number = KNumber(0);
			return;
		}
	}
	_last_number = Rad2Deg(KNumber(asin(static_cast<double>(input))));
}

void CalcEngine::ArcSinRad(KNumber input)
{
	if (input.type() == KNumber::SpecialType  ||
	    input < -KNumber::One  ||  input > KNumber::One) {
		_last_number = KNumber("nan");
		return;
	}
	_last_number = KNumber(asin(static_cast<double>(input)));
}

void CalcEngine::ArcSinGrad(KNumber input)
{
	if (input.type() == KNumber::SpecialType  ||
	    input < -KNumber::One  ||  input > KNumber::One) {
		_last_number = KNumber("nan");
		return;
	}
	if (input.type() == KNumber::IntegerType) {
		if (input == KNumber::One) {
			_last_number = KNumber(100);
			return;
		}
		if (input == - KNumber::One) {
			_last_number = KNumber(-100);
			return;
		}
		if (input == KNumber::Zero) {
			_last_number = KNumber(0);
			return;
		}
	}
	_last_number = Rad2Gra(KNumber(asin(static_cast<double>(input))));
}

void CalcEngine::ArcTangensDeg(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		if (input == KNumber("nan")) _last_number = KNumber("nan");
		if (input == KNumber("inf")) _last_number = KNumber(90);
		if (input == KNumber("-inf"))  _last_number = KNumber(-90);
		return;
	}

	_last_number = Rad2Deg(KNumber(atan(static_cast<double>(input))));
}

void CalcEngine::ArcTangensRad(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		if (input == KNumber("nan")) _last_number = KNumber("nan");
		if (input == KNumber("inf"))
			_last_number = KNumber::Pi()/KNumber(2);
		if (input == KNumber("-inf"))
			_last_number = -KNumber::Pi()/KNumber(2);
		return;
	}

	_last_number = KNumber(atan(static_cast<double>(input)));
}

void CalcEngine::ArcTangensGrad(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		if (input == KNumber("nan")) _last_number = KNumber("nan");
		if (input == KNumber("inf")) _last_number = KNumber(100);
		if (input == KNumber("-inf"))  _last_number = KNumber(-100);
		return;
	}

	_last_number = Rad2Gra(KNumber(atan(static_cast<double>(input))));
}

void CalcEngine::AreaCosHyp(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		if (input == KNumber("nan")) _last_number = KNumber("nan");
		if (input == KNumber("inf")) _last_number = KNumber("inf");
		if (input == KNumber("-inf"))  _last_number = KNumber("nan");
		return;
	}

	if (input < KNumber::One) {
	  _last_number = KNumber("nan");
	  return;
	}
	if (input == KNumber::One) {
	  _last_number = KNumber::Zero;
	  return;
	}
	_last_number = KNumber(acosh(static_cast<double>(input)));
}

void CalcEngine::AreaSinHyp(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		if (input == KNumber("nan")) _last_number = KNumber("nan");
		if (input == KNumber("inf")) _last_number = KNumber("inf");
		if (input == KNumber("-inf"))  _last_number = KNumber("-inf");
		return;
	}

	if (input == KNumber::Zero) {
	  _last_number = KNumber::Zero;
	  return;
	}
	_last_number = KNumber(asinh(static_cast<double>(input)));
}

void CalcEngine::AreaTangensHyp(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		_last_number = KNumber("nan");
		return;
	}

	if (input < -KNumber::One  ||  input > KNumber::One) {
		_last_number = KNumber("nan");
		return;
	}
	if (input == KNumber::One) {
		_last_number = KNumber("inf");
		return;
	}
	if (input == - KNumber::One) {
		_last_number = KNumber("-inf");
		return;
	}
	_last_number = KNumber(atanh(static_cast<double>(input)));
}

#include "knumber/knumber_priv.h"
void CalcEngine::Complement(KNumber input)
{
	if (input.type() != KNumber::IntegerType)
	{
		_last_number = KNumber("nan");
		return;
	}
	// Note: assumes 64-bit "logic mode"
	quint64 value = static_cast<quint64>(input);
	_last_number = KNumber(~value);
}



// move a number into the interval [0,360) by adding multiples of 360
static KNumber const moveIntoDegInterval(KNumber const &num)
{
	KNumber tmp_num = num - (num/KNumber(360)).integerPart() * KNumber(360);
	if(tmp_num < KNumber::Zero)
		return tmp_num + KNumber(360);
	return tmp_num;
}

// move a number into the interval [0,400) by adding multiples of 400
static KNumber const moveIntoGradInterval(KNumber const &num)
{
	KNumber tmp_num = num - (num/KNumber(400)).integerPart() * KNumber(400);
	if(tmp_num < KNumber::Zero)
		return tmp_num + KNumber(400);
	return tmp_num;
}


void CalcEngine::CosDeg(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		_last_number = KNumber("nan");
		return;
	}
	KNumber trunc_input = moveIntoDegInterval(input);
	if (trunc_input.type() == KNumber::IntegerType) {
		KNumber mult = trunc_input/KNumber(90);
		if (mult.type() == KNumber::IntegerType) {
		  if (mult == KNumber::Zero)
		  	_last_number = 1;
		  else if (mult == KNumber(1))
		  	_last_number = 0;
		  else if (mult == KNumber(2))
		  	_last_number = -1;
		  else if (mult == KNumber(3))
		  	_last_number = 0;
		  else kDebug("Something wrong in CalcEngine::CosDeg\n");
		  return;
		}
	}
	trunc_input = Deg2Rad(trunc_input);

	_last_number = KNumber(cos(static_cast<double>(trunc_input)));
}

void CalcEngine::CosRad(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		_last_number = KNumber("nan");
		return;
	}
	_last_number = KNumber(cos(static_cast<double>(input)));
}

void CalcEngine::CosGrad(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		_last_number = KNumber("nan");
		return;
	}
	KNumber trunc_input = moveIntoGradInterval(input);
	if (trunc_input.type() == KNumber::IntegerType) {
		KNumber mult = trunc_input/KNumber(100);
		if (mult.type() == KNumber::IntegerType) {
		  if (mult == KNumber::Zero)
		  	_last_number = 1;
		  else if (mult == KNumber(1))
		  	_last_number = 0;
		  else if (mult == KNumber(2))
		  	_last_number = -1;
		  else if (mult == KNumber(3))
		  	_last_number = 0;
		  else kDebug("Something wrong in CalcEngine::CosGrad\n");
		  return;
		}
	}
	trunc_input = Gra2Rad(trunc_input);

	_last_number = KNumber(cos(static_cast<double>(trunc_input)));
}

void CalcEngine::CosHyp(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		if (input == KNumber("nan")) _last_number = KNumber("nan");
		if (input == KNumber("inf")) _last_number = KNumber("inf");
		if (input == KNumber("-inf"))  _last_number = KNumber("inf");
		return;
	}

	_last_number = KNumber(cosh(static_cast<double>(input)));
}

void CalcEngine::Cube(KNumber input)
{
	_last_number = input*input*input;
}

void CalcEngine::CubeRoot(KNumber input)
{
	_last_number = input.cbrt();
}

void CalcEngine::Exp(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		if (input == KNumber("nan")) _last_number = KNumber("nan");
		if (input == KNumber("inf")) _last_number = KNumber("inf");
		if (input == KNumber("-inf"))  _last_number = KNumber::Zero;
		return;
	}
	_last_number = KNumber(exp(static_cast<double>(input)));
}

void CalcEngine::Exp10(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		if (input == KNumber("nan")) _last_number = KNumber("nan");
		if (input == KNumber("inf")) _last_number = KNumber("inf");
		if (input == KNumber("-inf"))  _last_number = KNumber::Zero;
		return;
	}
	_last_number = KNumber(10).power(input);
}


void CalcEngine::Factorial(KNumber input)
{
	if (input == KNumber("inf")) return;
	if (input < KNumber::Zero || input.type() == KNumber::SpecialType)
	{
		_error = true;
		_last_number = KNumber("nan");
		return;
	}
	KNumber tmp_amount = input.integerPart();

	_last_number = tmp_amount.factorial();
}

void CalcEngine::InvertSign(KNumber input)
{
	_last_number = -input;
}

void CalcEngine::Ln(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		if (input == KNumber("nan")) _last_number = KNumber("nan");
		if (input == KNumber("inf")) _last_number = KNumber("inf");
		if (input == KNumber("-inf"))  _last_number = KNumber("nan");
		return;
	}
	if (input < KNumber::Zero)
		_last_number = KNumber("nan");
	else if (input == KNumber::Zero)
		_last_number = KNumber("-inf");
	else if (input == KNumber::One)
		_last_number = 0;
	else {
		_last_number = KNumber(log(static_cast<double>(input)));
	}
}

void CalcEngine::Log10(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		if (input == KNumber("nan")) _last_number = KNumber("nan");
		if (input == KNumber("inf")) _last_number = KNumber("inf");
		if (input == KNumber("-inf"))  _last_number = KNumber("nan");
		return;
	}
	if (input < KNumber::Zero)
		_last_number = KNumber("nan");
	else if (input == KNumber::Zero)
		_last_number = KNumber("-inf");
	else if (input == KNumber::One)
		_last_number = 0;
	else {
		_last_number = KNumber(log10(static_cast<double>(input)));
	}
}

void CalcEngine::ParenClose(KNumber input)
{
 	// evaluate stack until corresponding opening bracket
	while (!_stack.isEmpty())
	{
		_node tmp_node = _stack.pop();
		if (tmp_node.operation == FUNC_BRACKET)
			break;
		input = evalOperation(tmp_node.number, tmp_node.operation,
				      input);
	}
	_last_number = input;
	return;
}

void CalcEngine::ParenOpen(KNumber input)
{
	enterOperation(input, FUNC_BRACKET);
}

void CalcEngine::Reciprocal(KNumber input)
{
	_last_number = KNumber::One/input;
}


void CalcEngine::SinDeg(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		_last_number = KNumber("nan");
		return;
	}

 	KNumber trunc_input = moveIntoDegInterval(input);
	if (trunc_input.type() == KNumber::IntegerType) {
		KNumber mult = trunc_input/KNumber(90);
		if (mult.type() == KNumber::IntegerType) {
		  if (mult == KNumber::Zero)
		  	_last_number = 0;
		  else if (mult == KNumber(1))
		  	_last_number = 1;
		  else if (mult == KNumber(2))
		  	_last_number = 0;
		  else if (mult == KNumber(3))
		  	_last_number = -1;
		  else kDebug("Something wrong in CalcEngine::SinDeg\n");
		  return;
		}
	}
	trunc_input = Deg2Rad(trunc_input);

	_last_number = KNumber(sin(static_cast<double>(trunc_input)));
}

void CalcEngine::SinRad(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		_last_number = KNumber("nan");
		return;
	}

	_last_number = KNumber(sin(static_cast<double>(input)));
}

void CalcEngine::SinGrad(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		_last_number = KNumber("nan");
		return;
	}

	KNumber trunc_input = moveIntoGradInterval(input);
	if (trunc_input.type() == KNumber::IntegerType) {
		KNumber mult = trunc_input/KNumber(100);
		if (mult.type() == KNumber::IntegerType) {
		  if (mult == KNumber::Zero)
		  	_last_number = 0;
		  else if (mult == KNumber(1))
		  	_last_number = 1;
		  else if (mult == KNumber(2))
		  	_last_number = 0;
		  else if (mult == KNumber(3))
		  	_last_number = -1;
		  else kDebug("Something wrong in CalcEngine::SinGrad\n");
		  return;
		}
	}

	trunc_input = Gra2Rad(trunc_input);

	_last_number = KNumber(sin(static_cast<double>(trunc_input)));
}

void CalcEngine::SinHyp(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		if (input == KNumber("nan")) _last_number = KNumber("nan");
		if (input == KNumber("inf")) _last_number = KNumber("inf");
		if (input == KNumber("-inf"))  _last_number = KNumber("-inf");
		return;
	}

	_last_number = KNumber(sinh(static_cast<double>(input)));
}

void CalcEngine::Square(KNumber input)
{
	_last_number = input*input;
}

void CalcEngine::SquareRoot(KNumber input)
{
	_last_number = input.sqrt();
}

void CalcEngine::StatClearAll(KNumber input)
{
	UNUSED(input);
	stats.clearAll();
}

void CalcEngine::StatCount(KNumber input)
{
	UNUSED(input);
	_last_number = KNumber(stats.count());
}

void CalcEngine::StatDataNew(KNumber input)
{
	stats.enterData(input);
	_last_number = KNumber(stats.count());
}

void CalcEngine::StatDataDel(KNumber input)
{
	UNUSED(input);
	stats.clearLast();
	_last_number = KNumber::Zero;
}

void CalcEngine::StatMean(KNumber input)
{
	UNUSED(input);
	_last_number = stats.mean();

	_error = stats.error();
}

void CalcEngine::StatMedian(KNumber input)
{
	UNUSED(input);
	_last_number = stats.median();

	_error = stats.error();
}

void CalcEngine::StatStdDeviation(KNumber input)
{
	UNUSED(input);
	_last_number = stats.std();

	_error = stats.error();
}

void CalcEngine::StatStdSample(KNumber input)
{
	UNUSED(input);
	_last_number = stats.sample_std();

	_error = stats.error();
}

void CalcEngine::StatSum(KNumber input)
{
	UNUSED(input);
	_last_number = stats.sum();
}

void CalcEngine::StatSumSquares(KNumber input)
{
	UNUSED(input);
	_last_number = stats.sum_of_squares();

	_error = stats.error();
}

void CalcEngine::TangensDeg(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		_last_number = KNumber("nan");
		return;
	}

	SinDeg(input);
	KNumber arg1 = _last_number;
	CosDeg(input);
	KNumber arg2 = _last_number;
	_last_number = arg1 / arg2;
}

void CalcEngine::TangensRad(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		_last_number = KNumber("nan");
		return;
	}

	SinRad(input);
	KNumber arg1 = _last_number;
	CosRad(input);
	KNumber arg2 = _last_number;
	_last_number = arg1 / arg2;
}

void CalcEngine::TangensGrad(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		_last_number = KNumber("nan");
		return;
	}

	SinGrad(input);
	KNumber arg1 = _last_number;
	CosGrad(input);
	KNumber arg2 = _last_number;
	_last_number = arg1 / arg2;
}

void CalcEngine::TangensHyp(KNumber input)
{
	if (input.type() == KNumber::SpecialType) {
		if (input == KNumber("nan")) _last_number = KNumber("nan");
		if (input == KNumber("inf")) _last_number = KNumber::One;
		if (input == KNumber("-inf"))  _last_number = KNumber::MinusOne;
		return;
	}

	_last_number = KNumber(tanh(static_cast<double>(input)));
}

KNumber CalcEngine::evalOperation(KNumber arg1, Operation operation,
				   KNumber arg2)
{
	if (!_percent_mode || Operator[operation].prcnt_ptr == NULL)
	{
		return (Operator[operation].arith_ptr)(arg1, arg2);
	} else {
		_percent_mode = false;
		return (Operator[operation].prcnt_ptr)(arg1, arg2);
	}
}

void CalcEngine::enterOperation(KNumber number, Operation func)
{
	_node tmp_node;

	if (func == FUNC_BRACKET)
	{
		tmp_node.number = 0;
		tmp_node.operation = FUNC_BRACKET;
	
		_stack.push(tmp_node);

		return;
	}
	
	if (func == FUNC_PERCENT)
	{
		_percent_mode = true;
	}

	tmp_node.number = number;
	tmp_node.operation = func;
	
	_stack.push(tmp_node);

	evalStack();
}

bool CalcEngine::evalStack(void)
{
	// this should never happen
	Q_ASSERT(!_stack.isEmpty());

	_node tmp_node = _stack.pop();

	while (! _stack.isEmpty())
	{
		_node tmp_node2 = _stack.pop();
		if (Operator[tmp_node.operation].precedence <=
		    Operator[tmp_node2.operation].precedence)
		{
			if (tmp_node2.operation == FUNC_BRACKET) continue;
			KNumber tmp_result =
			  evalOperation(tmp_node2.number, tmp_node2.operation,
					tmp_node.number);
			tmp_node.number = tmp_result;
		}
		else
		{
			_stack.push(tmp_node2);
			break;
		}  
		  
	}

	if(tmp_node.operation != FUNC_EQUAL  &&  tmp_node.operation != FUNC_PERCENT)
		_stack.push(tmp_node);

	_last_number = tmp_node.number;
	return true;
}

void CalcEngine::Reset()
{
	_error = false;
	_last_number = KNumber::Zero;

	_stack.clear();
}


