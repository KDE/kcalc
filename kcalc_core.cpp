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

int precedence[14] = { 0, 1, 2, 3, 4, 4, 5, 5, 6, 6, 6, 7, 7, 6 };

/*
int adjust_op[14][3] =
{
	{FUNC_NULL,     FUNC_NULL,     FUNC_NULL},
	{FUNC_OR,       FUNC_OR,       FUNC_XOR },
	{FUNC_XOR,      FUNC_XOR,      FUNC_XOR },
	{FUNC_AND,      FUNC_AND,      FUNC_AND },
	{FUNC_LSH,      FUNC_LSH,      FUNC_RSH },
	{FUNC_RSH,      FUNC_RSH,      FUNC_RSH },
	{FUNC_ADD,      FUNC_ADD,      FUNC_ADD },
	{FUNC_SUBTRACT, FUNC_SUBTRACT, FUNC_SUBTRACT},
	{FUNC_MULTIPLY, FUNC_MULTIPLY, FUNC_MULTIPLY},
	{FUNC_DIVIDE,   FUNC_DIVIDE,   FUNC_DIVIDE},
	{FUNC_MOD,      FUNC_MOD,      FUNC_INTDIV },
	{FUNC_POWER,    FUNC_POWER,    FUNC_PWR_ROOT},
	{FUNC_PWR_ROOT, FUNC_PWR_ROOT, FUNC_PWR_ROOT},
	{FUNC_INTDIV,   FUNC_INTDIV,   FUNC_INTDIV},
};
*/

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
	// printf("ExecPowser %g left_op, %g right_op\n",left_op, right_op);
	if (right_op == 0)
		return 1L;

	if (left_op < 0 && isoddint(1 / right_op))
		left_op = -1L * POW((-1L * left_op), right_op);
	else
		left_op = POW(left_op, right_op);

	if (errno == EDOM || errno == ERANGE)
	{
		_error = true;
		return 0;
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
  :   precedence_base(0), percent_mode(false)
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

	//	RefreshEngine();

	Arith_ops[0] = NULL,
	  Arith_ops[1] = ExecOr,
	  Arith_ops[2] = ExecXor,
	  Arith_ops[3] = ExecAnd,
	  Arith_ops[4] = ExecLsh,
	  Arith_ops[5] = ExecRsh,
	  Arith_ops[6] = ExecAdd,
	  Arith_ops[7] = ExecSubtract,
	  Arith_ops[8] = ExecMultiply,
	  Arith_ops[9] = ExecDivide,
	  Arith_ops[10] = ExecMod,
	  Arith_ops[11] = ExecPower,
	  Arith_ops[12] = ExecPwrRoot,
	  Arith_ops[13] = ExecIntDiv;

	Prcnt_ops[0] = NULL,
	  Prcnt_ops[1] = NULL,
	  Prcnt_ops[2] = NULL,
	  Prcnt_ops[3] = NULL,
	  Prcnt_ops[4] = NULL,
	  Prcnt_ops[5] = NULL,
	  Prcnt_ops[6] = ExecAddP,
	  Prcnt_ops[7] = ExecSubP,
	  Prcnt_ops[8] = ExecMultiplyP,
	  Prcnt_ops[9] = ExecDivideP,
	  Prcnt_ops[10] = NULL,
	  Prcnt_ops[11] = NULL,
	  Prcnt_ops[12] = NULL,
	  Prcnt_ops[13] = NULL;

	_last_result = 0L;
	_error = false;
}

CALCAMNT CalcEngine::last_output(bool &error) const
{
	error = _error;
	return _last_result;
}


/*
void CalcEngine::RefreshEngine()
{
	display_error = false;
	DISPLAY_AMOUNT = 0L;
	decimal_point = 0;
	input_count = 0;
	UpdateDisplay();
	last_input = DIGIT; // must set last to DIGIT after Update Display in order
						// not to get a display holding e.g. 0.000
}
*/


void CalcEngine::And(CALCAMNT input)
{
	EnterStackFunction(FUNC_AND, input);
}

void CalcEngine::ArcCos(CALCAMNT input)
{
	CALCAMNT tmp = ACOS(input);

	switch (_angle_mode)
	{
	case ANG_DEGREE:
		_last_result = Rad2Deg(tmp);
		break;
	case ANG_GRADIENT:
		_last_result = Rad2Gra(tmp);
		break;
	case ANG_RADIAN:
		_last_result = tmp;
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
	case ANG_DEGREE:
		_last_result = Rad2Deg(tmp);
		break;
	case ANG_GRADIENT:
		_last_result = Rad2Gra(tmp);
		break;
	case ANG_RADIAN:
		_last_result = tmp;
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
	case ANG_DEGREE:
		_last_result = Rad2Deg(tmp);
		break;
	case ANG_GRADIENT:
		_last_result = Rad2Gra(tmp);
		break;
	case ANG_RADIAN:
		_last_result = tmp;
		break;
	}

	//if (errno == EDOM || errno == ERANGE)
	//	_error = true;
}

void CalcEngine::AreaCosHyp(CALCAMNT input)
{
	_last_result = ACOSH(input);
	//if (errno == EDOM || errno == ERANGE)
	//	_error = true;
}

void CalcEngine::AreaSinHyp(CALCAMNT input)
{
	_last_result = ASINH(input);
	//if (errno == EDOM || errno == ERANGE)
	//	_error = true;
}

void CalcEngine::AreaTangensHyp(CALCAMNT input)
{
	_last_result = ATANH(input);
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

	_last_result = ~boh_work;
}

void CalcEngine::Cos(CALCAMNT input)
{
	CALCAMNT tmp = input;

	switch (_angle_mode)
	{
	case ANG_DEGREE:
		tmp = Deg2Rad(input);
		break;
	case ANG_GRADIENT:
		tmp = Gra2Rad(input);
		break;
	case ANG_RADIAN:
		tmp = input;
		break;
	}

	_last_result = COS(tmp);

	// Now a cheat to help the weird case of COS 90 degrees not being 0!!!
	if (_last_result < POS_ZERO && _last_result > NEG_ZERO)
		_last_result = 0.0;

	//if (errno == EDOM || errno == ERANGE)
	//	_error = true;
}

void CalcEngine::CosHyp(CALCAMNT input)
{
	_last_result = COSH(input);
}

void CalcEngine::Divide(CALCAMNT input)
{
	EnterStackFunction(FUNC_DIVIDE, input);
}

void CalcEngine::Exp(CALCAMNT input)
{
	_last_result = EXP(input);
}

void CalcEngine::Exp10(CALCAMNT input)
{
	_last_result = POW(10, input);
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

	_last_result = _factorial(tmp_amount);

}

void CalcEngine::InvMod(CALCAMNT input)
{
	EnterStackFunction(FUNC_INTDIV, input);
}

void CalcEngine::InvPower(CALCAMNT input)
{
	EnterStackFunction(FUNC_PWR_ROOT, input);
}

void CalcEngine::Ln(CALCAMNT input)
{
	if (input <= 0.0)
		_error = true;
	else
		_last_result = LN(input);
}

void CalcEngine::Log10(CALCAMNT input)
{
	if (input <= 0.0)
		_error = true;
	else
		_last_result = LOG_TEN(input);
}

void CalcEngine::Minus(CALCAMNT input)
{
	EnterStackFunction(FUNC_SUBTRACT, input);
}

void CalcEngine::Mod(CALCAMNT input)
{
	EnterStackFunction(FUNC_MOD, input);
}

void CalcEngine::Multiply(CALCAMNT input)
{
	EnterStackFunction(FUNC_MULTIPLY, input);
}

void CalcEngine::Or(CALCAMNT input)
{
	EnterStackFunction(FUNC_OR, input);
}

void CalcEngine::ParenClose(CALCAMNT input)
{
	amount_stack.push(input);

	UpdateStack(precedence_base);

	if ((precedence_base -= PRECEDENCE_INCR) < 0)
		precedence_base = 0;

	// sad, but necessary: pushing once above and once in
	// UpdateStack
	amount_stack.pop();
}

void CalcEngine::ParenOpen(CALCAMNT input)
{
	UNUSED(input);
	precedence_base += PRECEDENCE_INCR;
}

void CalcEngine::Plus(CALCAMNT input)
{
	EnterStackFunction(FUNC_ADD, input);
}

void CalcEngine::Power(CALCAMNT input)
{
	EnterStackFunction(FUNC_POWER, input);
}

void CalcEngine::Reciprocal(CALCAMNT input)
{
	if (input == 0.0)
		_error = true;
	else
		_last_result = 1/input;
}
		
void CalcEngine::ShiftLeft(CALCAMNT input)
{
	EnterStackFunction(FUNC_LSH, input);
}

void CalcEngine::ShiftRight(CALCAMNT input)
{
	EnterStackFunction(FUNC_RSH, input);
}

void CalcEngine::Sin(CALCAMNT input)
{
	CALCAMNT tmp = input;

	switch (_angle_mode)
	{
	case ANG_DEGREE:
		tmp = Deg2Rad(input);
		break;
	case ANG_GRADIENT:
		tmp = Gra2Rad(input);
		break;
	case ANG_RADIAN:
		tmp = input;
		break;
	}

	_last_result = SIN(tmp);

	// Now a cheat to help the weird case of COS 90 degrees not being 0!!!
	if (_last_result < POS_ZERO && _last_result > NEG_ZERO)
		_last_result = 0.0;

	//if (errno == EDOM || errno == ERANGE)
	//	_error = true;
}

void CalcEngine::SinHyp(CALCAMNT input)
{
	_last_result = SINH(input);
}

void CalcEngine::Square(CALCAMNT input)
{
	if (ISINF(_last_result = input*input))
		_error = true;
}

void CalcEngine::SquareRoot(CALCAMNT input)
{
	if (input < 0.0)
		_error = true;
	else
		_last_result = SQRT(input);
}

void CalcEngine::StatClearAll(CALCAMNT input)
{
	UNUSED(input);
	stats.clearAll();
}

void CalcEngine::StatCount(CALCAMNT input)
{
	UNUSED(input);
	_last_result = stats.count();
}

void CalcEngine::StatDataNew(CALCAMNT input)
{
	stats.enterData(input);
	_last_result = stats.count();
}

void CalcEngine::StatDataDel(CALCAMNT input)
{
	UNUSED(input);
	stats.clearLast();
	_last_result = 0L;
}

void CalcEngine::StatMean(CALCAMNT input)
{
	UNUSED(input);
	_last_result = stats.mean();

	_error = stats.error();
}

void CalcEngine::StatMedian(CALCAMNT input)
{
	UNUSED(input);
	_last_result = stats.median();

	_error = stats.error();
}

void CalcEngine::StatStdDeviation(CALCAMNT input)
{
	UNUSED(input);
	_last_result = stats.std();

	_error = stats.error();
}

void CalcEngine::StatStdSample(CALCAMNT input)
{
	UNUSED(input);
	_last_result = stats.sample_std();

	_error = stats.error();
}

void CalcEngine::StatSum(CALCAMNT input)
{
	UNUSED(input);
	_last_result = stats.sum();
}

void CalcEngine::StatSumSquares(CALCAMNT input)
{
	UNUSED(input);
	_last_result = stats.sum_of_squares();

	_error = stats.error();
}

void CalcEngine::Tangens(CALCAMNT input)
{
	CALCAMNT tmp = input;

	switch (_angle_mode)
	{
	case ANG_DEGREE:
		tmp = Deg2Rad(input);
		break;
	case ANG_GRADIENT:
		tmp = Gra2Rad(input);
		break;
	case ANG_RADIAN:
		tmp = input;
		break;
	}

	_last_result = TAN(tmp);

	// Now a cheat to help the weird case of COS 90 degrees not being 0!!!
	if (_last_result < POS_ZERO && _last_result > NEG_ZERO)
		_last_result = 0.0;

	//if (errno == EDOM || errno == ERANGE)
	//	_error = true;
}

void CalcEngine::TangensHyp(CALCAMNT input)
{
	_last_result = TANH(input);
}


void CalcEngine::Equal(CALCAMNT input)
{
	amount_stack.push(input);
	_last_result = input;

	UpdateStack(0);

	precedence_base = 0;
}

void CalcEngine::Percent(CALCAMNT input)
{
	percent_mode = true;
	
	Equal(input);
}

void CalcEngine::EnterStackFunction(int func, CALCAMNT number)
{
	func_data new_item;
	int new_precedence;

	_last_result = number;

	amount_stack.push(number);
	
	new_item.item_function = func;
	new_item.item_precedence = new_precedence = 
		(precedence[func] + precedence_base);

	//	refresh_display = true;
	if (UpdateStack(new_precedence)) {
	  ;//	UpdateDisplay();
	}

	func_stack.push(new_item);
}

void CalcEngine::Xor(CALCAMNT input)
{
	EnterStackFunction(FUNC_XOR, input);
}

void CalcEngine::Reset()
{
	_error = false;
	_last_result = 0L;
	//decimal_point = 0;
	//input_count = 0;

	amount_stack.clear();
	func_stack.clear();
}

int CalcEngine::UpdateStack(int run_precedence)
{
	CALCAMNT left_op	= 0.0;
	CALCAMNT right_op 	= 0.0;
	CALCAMNT new_value	= 0.0;
	int op_function		= 0;
	int return_value	= 0;


	while ((func_stack.size() != 0) && 
		(func_stack.top().item_precedence >= run_precedence))
	{
		return_value = 1;

		if (amount_stack.isEmpty()) {
			KMessageBox::error(0L, i18n("Stack processing error - right_op"));
		}

		right_op = amount_stack.pop();

		if (func_stack.isEmpty()) {
			KMessageBox::error(0L, i18n("Stack processing error - function"));
		}

		op_function = func_stack.pop().item_function;

		if (amount_stack.isEmpty()) {
			KMessageBox::error(0L, i18n("Stack processing error - left_op"));
		}

		left_op = amount_stack.pop();

		if (!percent_mode || Prcnt_ops[op_function] == NULL) {
			new_value = (Arith_ops[op_function])(left_op, right_op);
		} else {
			new_value = (Prcnt_ops[op_function])(left_op, right_op);
			percent_mode = false;
		}

		amount_stack.push(new_value);
	}

	if (return_value)
		_last_result = new_value;

	//decimal_point = 1;
	return return_value;
}

