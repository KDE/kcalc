/*
    kCalculator, a scientific calculator for the X window system using the
    Qt widget libraries, available at no cost at http://www.troll.no

    The stack engine conatined in this file was take from
    Martin Bartlett's xfrmcalc

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/


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

#include <config.h>
#include "kcalc_core.h"
#include <stdlib.h>
#include <klocale.h>
#include <kmessagebox.h>

#ifndef HAVE_FUNC_ISINF
        #ifdef HAVE_IEEEFP_H
                #include <ieeefp.h>
        #else
                #include <math.h>
        #endif

int isinf(double x) { return !finite(x) && x==x; }
#endif


static void fpe_handler(int fpe_parm)
{
	UNUSED(fpe_parm);
	//	display_error = true;
	//tmp_number = 0L;
}


const CALCAMNT CalcEngine::pi = (ASIN(1L) * 2L);

static CALCAMNT _error;

static CALCAMNT ExecOr(CALCAMNT left_op, CALCAMNT right_op)
{
	// printf("ExecOr\n");
	CALCAMNT	boh_work_d;
	KCALC_LONG	boh_work_l;
	KCALC_LONG	boh_work_r;

	MODF(left_op, &boh_work_d);
	if (FABS(boh_work_d) > KCALC_LONG_MAX)
	{
	  //	display_error = true;
		return 0;
	}

	boh_work_l = (KCALC_LONG)boh_work_d;
	MODF(right_op, &boh_work_d);

	if (FABS(boh_work_d) > KCALC_LONG_MAX)
	{
	  //display_error = true;
		return 0;
	}

	boh_work_r = (KCALC_LONG) boh_work_d;
	return (boh_work_l | boh_work_r);
}

static CALCAMNT ExecXor(CALCAMNT left_op, CALCAMNT right_op)
{
	// printf("ExecXOr\n");
	CALCAMNT	boh_work_d;
	KCALC_LONG	boh_work_l;
	KCALC_LONG	boh_work_r;

	MODF(left_op, &boh_work_d);
	if (FABS(boh_work_d) > KCALC_LONG_MAX)
	{
	  //display_error = true;
		return 0;
	}

	boh_work_l = (KCALC_LONG)boh_work_d;
	MODF(right_op, &boh_work_d);

	if (FABS(boh_work_d) > KCALC_LONG_MAX)
	{
	  //display_error = true;
		return 0;
	}

	boh_work_r = (KCALC_LONG)boh_work_d;
	return (boh_work_l ^ boh_work_r);
}

static CALCAMNT ExecAnd(CALCAMNT left_op, CALCAMNT right_op)
{
	// printf("ExecAnd\n");
	CALCAMNT	boh_work_d;
	KCALC_LONG	boh_work_l;
	KCALC_LONG	boh_work_r;

	MODF(left_op, &boh_work_d);
	if (FABS(boh_work_d) > KCALC_LONG_MAX)
	{
	  //display_error = true;
		return 0;
	}

	boh_work_l = (KCALC_LONG)boh_work_d;
	MODF(right_op, &boh_work_d);

	if (FABS(boh_work_d) > KCALC_LONG_MAX)
	{
	  //display_error = true;
		return 0;
	}

	boh_work_r = (KCALC_LONG)boh_work_d;
	return (boh_work_l & boh_work_r);
}

static CALCAMNT ExecLsh(CALCAMNT left_op, CALCAMNT right_op)
{
	// printf("ExecLsh\n");
	CALCAMNT	boh_work_d;
	KCALC_LONG	boh_work_l;
	KCALC_LONG	boh_work_r;

	MODF(left_op, &boh_work_d);
	if (FABS(boh_work_d) > KCALC_LONG_MAX)
	{
	  //display_error = true;
		return 0;
	}

	boh_work_l = (KCALC_LONG) boh_work_d;
	MODF(right_op, &boh_work_d);

	if (FABS(boh_work_d) > KCALC_LONG_MAX)
	{
	  //display_error = true;
		return 0;
	}

	boh_work_r = (KCALC_LONG) boh_work_d;
	return (boh_work_l << boh_work_r);
}

static CALCAMNT ExecRsh(CALCAMNT left_op, CALCAMNT right_op)
{
	// printf("ExecRsh\n");
	CALCAMNT	boh_work_d;
	KCALC_LONG	boh_work_l;
	KCALC_LONG	boh_work_r;

	MODF(left_op, &boh_work_d);
	if (FABS(boh_work_d) > KCALC_LONG_MAX)
	{
	  //display_error = true;
		return 0;
	}

	boh_work_l = (KCALC_LONG)boh_work_d;
	MODF(right_op, &boh_work_d);

	if (FABS(boh_work_d) > KCALC_LONG_MAX)
	{
	  //display_error = true;
		return 0;
	}

	boh_work_r = (KCALC_LONG)boh_work_d;
	return (boh_work_l >> boh_work_r);
}

static CALCAMNT ExecAdd(CALCAMNT left_op, CALCAMNT right_op)
{
	// printf("ExecAdd\n");
	return left_op + right_op;
}

static CALCAMNT ExecSubtract(CALCAMNT left_op, CALCAMNT right_op)
{
	// printf("ExecSubtract\n");
	return left_op - right_op;
}

static CALCAMNT ExecMultiply(CALCAMNT left_op, CALCAMNT right_op)
{
	// printf("ExecMulti\n");
	return left_op * right_op;
}

static CALCAMNT ExecDivide(CALCAMNT left_op, CALCAMNT right_op)
{
	// printf("ExecDivide\n");
	if (right_op == 0)
	{
		_error = true;
		return 0L;
	}
	else
		return left_op / right_op;
}

static CALCAMNT ExecMod(CALCAMNT left_op, CALCAMNT right_op)
{
	// printf("ExecMod\n");
	CALCAMNT temp = 0.0;

	if (right_op == 0)
	{
		_error = true;
		return 0L;
	}
	else
	{
		// x mod y should be the same as x mod -y, thus:
		right_op = FABS(right_op);

		temp = FMOD(left_op, right_op);

		// let's make sure that -7 mod 3 = 2 and NOT -1.
		// In other words we wand x mod 3 to be a _positive_ number
		// that is 0,1 or 2.
		if( temp < 0 )
			temp = right_op + temp;

		return FABS(temp);
	}
}

static CALCAMNT ExecIntDiv(CALCAMNT left_op, CALCAMNT right_op)
{
	// printf("IndDiv\n");
	if (right_op == 0)
	{
		_error = true;
		return 0L;
	}
	else
	{
		MODF(left_op / right_op, &left_op);
		return left_op;
	}
}

int isoddint(CALCAMNT input)
{
	CALCAMNT	dummy;

	// Routine to check if CALCAMNT is an Odd integer
	return (MODF(input, &dummy) == 0.0 && MODF(input / 2, &dummy) == 0.5);
}

static CALCAMNT ExecPower(CALCAMNT left_op, CALCAMNT right_op)
{
	if (right_op == 0)
		if (left_op == 0) // 0^0 not defined
		{
			_error = true;		
			return 0L;
		}
		else
			return 1L;

	if (left_op < 0 && isoddint(1 / right_op))
		left_op = -1L * POW((-1L * left_op), right_op);
	else
		left_op = POW(left_op, right_op);

	if (errno == EDOM || errno == ERANGE)
	{
		_error = true;
		return 0L;
	}
	else
		return left_op;
}

static CALCAMNT ExecPwrRoot(CALCAMNT left_op, CALCAMNT right_op)
{
	// printf("ExecPwrRoot  %g left_op, %g right_op\n", left_op, right_op);
	if (right_op == 0)
	{
		_error = true;
		return 0L;
	}

	if (left_op < 0 && isoddint(right_op))
		left_op = -1L * POW((-1L * left_op), (1L)/right_op);
	else
		left_op = POW(left_op, (1L)/right_op);

	if (errno == EDOM || errno == ERANGE)
	{
		_error = true;
		return 0;
	}
	else
		return left_op;
}

static CALCAMNT ExecAddP(CALCAMNT left_op, CALCAMNT right_op)
{
    return left_op * (100.0 + right_op) / 100.0;
}

static CALCAMNT ExecSubP(CALCAMNT left_op, CALCAMNT right_op)
{
    return left_op * (100.0 - right_op) / 100.0;
}

static CALCAMNT ExecMultiplyP(CALCAMNT left_op, CALCAMNT right_op)
{
    return left_op*right_op / 100;
}

static CALCAMNT ExecDivideP(CALCAMNT left_op, CALCAMNT right_op)
{
    return left_op * 100 / right_op;
}



CalcEngine::CalcEngine()
  :   _percent_mode(false)
{
	//
	// Basic initialization involves initializing the calcultion
	// stack, and setting up the floating point excetion signal
	// handler to trap the errors that the code can/has not been
	// written to trap.
	//

	struct sigaction fpe_trap;

	sigemptyset(&fpe_trap.sa_mask);
	fpe_trap.sa_handler = &fpe_handler;
#ifdef SA_RESTART
	fpe_trap.sa_flags = SA_RESTART;
#endif
	sigaction(SIGFPE, &fpe_trap, NULL);

	// build precedence list
	_precedence_list[FUNC_EQUAL] = 0;
	Arith_ops[FUNC_EQUAL] = NULL;
	Prcnt_ops[FUNC_EQUAL] = NULL;

	_precedence_list[FUNC_BRACKET] = 0;
	Arith_ops[FUNC_BRACKET] = NULL;
	Prcnt_ops[FUNC_BRACKET] = NULL;

	_precedence_list[FUNC_OR] = 1;
	Arith_ops[FUNC_OR] = ExecOr;
	Prcnt_ops[FUNC_OR] = NULL;

	_precedence_list[FUNC_XOR] = 2;
	Arith_ops[FUNC_XOR] = ExecXor;
	Prcnt_ops[FUNC_XOR] = NULL;

	_precedence_list[FUNC_AND] = 3;
	Arith_ops[FUNC_AND] = ExecAnd;
	Prcnt_ops[FUNC_AND] = NULL;

	_precedence_list[FUNC_LSH] = 4;
	Arith_ops[FUNC_LSH] = ExecLsh;
	Prcnt_ops[FUNC_LSH] = NULL;

	_precedence_list[FUNC_RSH] = 4;
	Arith_ops[FUNC_RSH] = ExecRsh;
	Prcnt_ops[FUNC_RSH] = NULL;

	_precedence_list[FUNC_ADD] = 5;
	Arith_ops[FUNC_ADD] = ExecAdd;
	Prcnt_ops[FUNC_ADD] = ExecAddP;

	_precedence_list[FUNC_SUBTRACT] = 5;
	Arith_ops[FUNC_SUBTRACT] = ExecSubtract;
	Prcnt_ops[FUNC_SUBTRACT] = ExecSubP;

	_precedence_list[FUNC_MULTIPLY] = 6;
	Arith_ops[FUNC_MULTIPLY] = ExecMultiply;
	Prcnt_ops[FUNC_MULTIPLY] = ExecMultiplyP;

	_precedence_list[FUNC_DIVIDE] = 6;
	Arith_ops[FUNC_DIVIDE] = ExecDivide;
	Prcnt_ops[FUNC_DIVIDE] = ExecDivideP;

	_precedence_list[FUNC_MOD] = 6;
	Arith_ops[FUNC_MOD] = ExecMod;
	Prcnt_ops[FUNC_MOD] = NULL;

	_precedence_list[FUNC_POWER] = 7;
	Arith_ops[FUNC_POWER] = ExecPower;
	Prcnt_ops[FUNC_POWER] = NULL;

	_precedence_list[FUNC_PWR_ROOT] = 7;
	Arith_ops[FUNC_PWR_ROOT] = ExecPwrRoot;
	Prcnt_ops[FUNC_PWR_ROOT] = NULL;
	
	_precedence_list[FUNC_INTDIV] = 6;
	Arith_ops[FUNC_INTDIV] = ExecIntDiv;	
	Prcnt_ops[FUNC_INTDIV] = NULL;

	_last_number = 0L;
	_error = false;
}

CALCAMNT CalcEngine::lastOutput(bool &error) const
{
	error = _error;
	return _last_number;
}

void CalcEngine::And(CALCAMNT input)
{
	enterOperation(FUNC_AND, input);
}

void CalcEngine::ArcCos(CALCAMNT input)
{
	CALCAMNT tmp = ACOS(input);

	switch (_angle_mode)
	{
	case AngleDegree:
		_last_number = Rad2Deg(tmp);
		break;
	case AngleGradient:
		_last_number = Rad2Gra(tmp);
		break;
	case AngleRadian:
		_last_number = tmp;
		break;
	}

	//if (errno == EDOM || errno == ERANGE)
	//	_error = true;
}

void CalcEngine::ArcSin(CALCAMNT input)
{
	CALCAMNT tmp = ASIN(input);

	switch (_angle_mode)
	{
	case AngleDegree:
		_last_number = Rad2Deg(tmp);
		break;
	case AngleGradient:
		_last_number = Rad2Gra(tmp);
		break;
	case AngleRadian:
		_last_number = tmp;
		break;
	}

	//if (errno == EDOM || errno == ERANGE)
	//	_error = true;
}

void CalcEngine::ArcTangens(CALCAMNT input)
{
	CALCAMNT tmp = ATAN(input);

	switch (_angle_mode)
	{
	case AngleDegree:
		_last_number = Rad2Deg(tmp);
		break;
	case AngleGradient:
		_last_number = Rad2Gra(tmp);
		break;
	case AngleRadian:
		_last_number = tmp;
		break;
	}

	//if (errno == EDOM || errno == ERANGE)
	//	_error = true;
}

void CalcEngine::AreaCosHyp(CALCAMNT input)
{
	_last_number = ACOSH(input);
	//if (errno == EDOM || errno == ERANGE)
	//	_error = true;
}

void CalcEngine::AreaSinHyp(CALCAMNT input)
{
	_last_number = ASINH(input);
	//if (errno == EDOM || errno == ERANGE)
	//	_error = true;
}

void CalcEngine::AreaTangensHyp(CALCAMNT input)
{
	_last_number = ATANH(input);
	//if (errno == EDOM || errno == ERANGE)
	//	_error = true;
}

void CalcEngine::Complement(CALCAMNT input)
{
	CALCAMNT boh_work_d;

	MODF(input, &boh_work_d);

	if (FABS(boh_work_d) > KCALC_LONG_MAX)
	{
		_error = true;
		return;
	}

	KCALC_LONG boh_work = (KCALC_LONG)boh_work_d;

	_last_number = ~boh_work;
}

void CalcEngine::Cos(CALCAMNT input)
{
	CALCAMNT tmp = input;

	switch (_angle_mode)
	{
	case AngleDegree:
		tmp = Deg2Rad(input);
		break;
	case AngleGradient:
		tmp = Gra2Rad(input);
		break;
	case AngleRadian:
		tmp = input;
		break;
	}

	_last_number = COS(tmp);

	// Now a cheat to help the weird case of COS 90 degrees not being 0!!!
	if (_last_number < POS_ZERO && _last_number > NEG_ZERO)
		_last_number = 0.0;

	//if (errno == EDOM || errno == ERANGE)
	//	_error = true;
}

void CalcEngine::CosHyp(CALCAMNT input)
{
	_last_number = COSH(input);
}

void CalcEngine::Divide(CALCAMNT input)
{
	enterOperation(FUNC_DIVIDE, input);
}

void CalcEngine::Exp(CALCAMNT input)
{
	_last_number = EXP(input);
}

void CalcEngine::Exp10(CALCAMNT input)
{
	_last_number = POW(10, input);
}

static CALCAMNT _factorial(CALCAMNT input)
{
	CALCAMNT tmp_amount = input;

	// don't do recursive factorial,
	// because large numbers lead to
	// stack overflows
	while (tmp_amount > 1.0)
	{
		tmp_amount--;

		input *= tmp_amount;

		if(ISINF(input))
		{
			_error = true;
			return 0;
		}
	}

	if (tmp_amount < 1)
		return 1;
	return input;
}

void CalcEngine::Factorial(CALCAMNT input)
{
	CALCAMNT tmp_amount;

	if (input < 0)
	{
		_error = true;
		return;
	}

	MODF(input, &tmp_amount);

	_last_number = _factorial(tmp_amount);

}

void CalcEngine::InvertSign(CALCAMNT input)
{
	_last_number = -input;
}

void CalcEngine::InvMod(CALCAMNT input)
{
	enterOperation(FUNC_INTDIV, input);
}

void CalcEngine::InvPower(CALCAMNT input)
{
	enterOperation(FUNC_PWR_ROOT, input);
}

void CalcEngine::Ln(CALCAMNT input)
{
	if (input <= 0.0)
		_error = true;
	else
		_last_number = LN(input);
}

void CalcEngine::Log10(CALCAMNT input)
{
	if (input <= 0.0)
		_error = true;
	else
		_last_number = LOG_TEN(input);
}

void CalcEngine::Minus(CALCAMNT input)
{
	enterOperation(FUNC_SUBTRACT, input);
}

void CalcEngine::Mod(CALCAMNT input)
{
	enterOperation(FUNC_MOD, input);
}

void CalcEngine::Multiply(CALCAMNT input)
{
	enterOperation(FUNC_MULTIPLY, input);
}

void CalcEngine::Or(CALCAMNT input)
{
	enterOperation(FUNC_OR, input);
}

void CalcEngine::ParenClose(CALCAMNT input)
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

void CalcEngine::ParenOpen(CALCAMNT input)
{
	enterOperation(FUNC_BRACKET, input);
}

void CalcEngine::Plus(CALCAMNT input)
{
	enterOperation(FUNC_ADD, input);
}

void CalcEngine::Power(CALCAMNT input)
{
	enterOperation(FUNC_POWER, input);
}

void CalcEngine::Reciprocal(CALCAMNT input)
{
	if (input == 0.0)
		_error = true;
	else
		_last_number = 1/input;
}
		
void CalcEngine::ShiftLeft(CALCAMNT input)
{
	enterOperation(FUNC_LSH, input);
}

void CalcEngine::ShiftRight(CALCAMNT input)
{
	enterOperation(FUNC_RSH, input);
}

void CalcEngine::Sin(CALCAMNT input)
{
	CALCAMNT tmp = input;

	switch (_angle_mode)
	{
	case AngleDegree:
		tmp = Deg2Rad(input);
		break;
	case AngleGradient:
		tmp = Gra2Rad(input);
		break;
	case AngleRadian:
		tmp = input;
		break;
	}

	_last_number = SIN(tmp);

	// Now a cheat to help the weird case of COS 90 degrees not being 0!!!
	if (_last_number < POS_ZERO && _last_number > NEG_ZERO)
		_last_number = 0.0;

	//if (errno == EDOM || errno == ERANGE)
	//	_error = true;
}

void CalcEngine::SinHyp(CALCAMNT input)
{
	_last_number = SINH(input);
}

void CalcEngine::Square(CALCAMNT input)
{
	if (ISINF(_last_number = input*input))
		_error = true;
}

void CalcEngine::SquareRoot(CALCAMNT input)
{
	if (input < 0.0)
		_error = true;
	else
		_last_number = SQRT(input);
}

void CalcEngine::StatClearAll(CALCAMNT input)
{
	UNUSED(input);
	stats.clearAll();
}

void CalcEngine::StatCount(CALCAMNT input)
{
	UNUSED(input);
	_last_number = stats.count();
}

void CalcEngine::StatDataNew(CALCAMNT input)
{
	stats.enterData(input);
	_last_number = stats.count();
}

void CalcEngine::StatDataDel(CALCAMNT input)
{
	UNUSED(input);
	stats.clearLast();
	_last_number = 0L;
}

void CalcEngine::StatMean(CALCAMNT input)
{
	UNUSED(input);
	_last_number = stats.mean();

	_error = stats.error();
}

void CalcEngine::StatMedian(CALCAMNT input)
{
	UNUSED(input);
	_last_number = stats.median();

	_error = stats.error();
}

void CalcEngine::StatStdDeviation(CALCAMNT input)
{
	UNUSED(input);
	_last_number = stats.std();

	_error = stats.error();
}

void CalcEngine::StatStdSample(CALCAMNT input)
{
	UNUSED(input);
	_last_number = stats.sample_std();

	_error = stats.error();
}

void CalcEngine::StatSum(CALCAMNT input)
{
	UNUSED(input);
	_last_number = stats.sum();
}

void CalcEngine::StatSumSquares(CALCAMNT input)
{
	UNUSED(input);
	_last_number = stats.sum_of_squares();

	_error = stats.error();
}

void CalcEngine::Tangens(CALCAMNT input)
{
	CALCAMNT aux, tmp = input;

	switch (_angle_mode)
	{
	case AngleDegree:
		tmp = Deg2Rad(input);
		break;
	case AngleGradient:
		tmp = Gra2Rad(input);
		break;
	case AngleRadian:
		tmp = input;
		break;
	}
	
	aux = tmp;
	// make aux positive
	if (aux < 0) aux = -aux;
	// put aux between 0 and pi
	while (aux > pi) aux -= pi;
	// if were are really close to pi/2 throw an error
	// tan(pi/2) => inf
	// using the 10 factor because without it 270º tan still gave a result
	if ( (aux - pi/2 < POS_ZERO * 10) && (aux - pi/2 > NEG_ZERO * 10) )
		_error = true;
	else
		_last_number = TAN(tmp);

	// Now a cheat to help the weird case of TAN 0 degrees not being 0!!!
	if (_last_number < POS_ZERO && _last_number > NEG_ZERO)
		_last_number = 0.0;

	//if (errno == EDOM || errno == ERANGE)
	//	_error = true;
}

void CalcEngine::TangensHyp(CALCAMNT input)
{
	_last_number = TANH(input);
}

void CalcEngine::Xor(CALCAMNT input)
{
	enterOperation(FUNC_XOR, input);
}


void CalcEngine::Equal(CALCAMNT input)
{
	enterOperation(FUNC_EQUAL, input);
}

void CalcEngine::Percent(CALCAMNT input)
{
	_percent_mode = true;
	
	Equal(input);
}

CALCAMNT CalcEngine::evalOperation(CALCAMNT arg1, Operation operation,
				   CALCAMNT arg2)
{
	if (!_percent_mode || Prcnt_ops[operation] == NULL)
	{
		return (Arith_ops[operation])(arg1, arg2);
	} else {
		_percent_mode = false;
		return (Prcnt_ops[operation])(arg1, arg2);
	}
}

void CalcEngine::enterOperation(Operation func, CALCAMNT number)
{
	_node tmp_node;

	if (func == FUNC_BRACKET)
	{
		tmp_node.number = 0;
		tmp_node.operation = FUNC_BRACKET;
	
		_stack.push(tmp_node);
	}
	else
	{
		tmp_node.number = number;
		tmp_node.operation = func;

		_stack.push(tmp_node);

		evalStack();
	}
}

bool CalcEngine::evalStack(void)
{
	// this should never happen
	if (_stack.isEmpty()) KMessageBox::error(0L, i18n("Stack processing error - empty stack"));

	_node tmp_node = _stack.pop();

	while (! _stack.isEmpty())
	{
		_node tmp_node2 = _stack.pop();
		if (_precedence_list[tmp_node.operation] <=
		    _precedence_list[tmp_node2.operation])
		{
			if (tmp_node2.operation == FUNC_BRACKET) continue;
			CALCAMNT tmp_result =
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

	if(tmp_node.operation != FUNC_EQUAL) _stack.push(tmp_node);

	_last_number = tmp_node.number;
	return true;
}

void CalcEngine::Reset()
{
	_error = false;
	_last_number = 0L;

	_stack.clear();
}


