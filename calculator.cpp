/*
    $Id$

    kCalculator, a scientific calculator for the X window system using the
    Qt widget libraries, available at no cost at http://www.troll.no

    The stack engine conatined in this file was take from
    Martin Bartlett's xfrmcalc

    portions:	Copyright (C) 1996 Bernd Johannes Wuebben
                                   wuebben@math.cornell.edu
								   
    portions:	Copyright (C) 2001 Evan Teran
                                   emt3734@rit.edu

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include "calculator.h"
#include <exception>
#include <cstdio>
#include <cerrno>
#include <climits>

#define UNUSED(x) ((void)(x))

using std::runtime_error;


stack_ptr	Calculator::top_of_stack		= NULL;	
stack_ptr	Calculator::top_type_stack[2]	= { NULL, NULL };	
int 		Calculator::stack_next;
int 		Calculator::stack_last;
stack_item	Calculator::process_stack[STACK_SIZE];


Calculator::Calculator()
{

}

Calculator::~Calculator()
{

}

//-------------------------------------------------------------------------
// Name: InitStack()
//-------------------------------------------------------------------------
void Calculator::InitStack()
{
	stack_next = 0;
	stack_last = STACK_SIZE - 1;
	top_of_stack = top_type_stack[0] = top_type_stack[1] = NULL;
}

//-------------------------------------------------------------------------
// Name: TopTypeStack(item_type rqstd_type)
//-------------------------------------------------------------------------
item_contents *Calculator::TopTypeStack(item_type rqstd_type)
{
	// Return the top item in the stack without removing

	item_contents *return_item_ptr = NULL;

	if (top_type_stack[rqstd_type])
		return_item_ptr = &(top_type_stack[rqstd_type]->item_value);

	return return_item_ptr;
}


//-------------------------------------------------------------------------
// Name: AllocStackItem()
//-------------------------------------------------------------------------
stack_ptr Calculator::AllocStackItem()
{
	if (stack_next <= stack_last)
	{
		process_stack[stack_next].prior_item = NULL;
		process_stack[stack_next].prior_type = NULL;
		return (process_stack + (stack_next++));
	}

	//KMessageBox::error(0L, i18n("Stack Error!") );
	return (process_stack + stack_next);
}

//-------------------------------------------------------------------------
// Name: UnAllocStackItem(stack_ptr return_item)
//-------------------------------------------------------------------------
void Calculator::UnAllocStackItem(stack_ptr return_item)
{
	if (return_item != (process_stack + (--stack_next))) {
		//throw runtime_error("Stack Error!");
		//KMessageBox::error(0L, i18n("Stack Error!") );
	}
}

//-------------------------------------------------------------------------
// Name: PushStack(item_contents *add_item)
//-------------------------------------------------------------------------
void Calculator::PushStack(item_contents *add_item)
{
	// Add an item to the stack

	stack_ptr new_item = top_of_stack;

	if (!(new_item && 
		new_item->item_value.s_item_type == add_item->s_item_type))
	{
		new_item = AllocStackItem();	// Get a new item

		// Chain new item to existing stacks


		new_item->prior_item = top_of_stack;
		top_of_stack	     = new_item;
		new_item->prior_type = top_type_stack[add_item->s_item_type];
		top_type_stack[add_item->s_item_type] = new_item;
	}

	new_item->item_value  = *add_item;	// assign contents

}

//-------------------------------------------------------------------------
// Name: PopStack()
//-------------------------------------------------------------------------
item_contents *Calculator::PopStack()
{
	// Remove and return the top item in the stack

	static item_contents return_item;
	
	item_contents *return_item_ptr = NULL;
	stack_ptr return_stack_ptr;

	if ((return_stack_ptr = top_of_stack))
	{
		return_item = top_of_stack->item_value;

		top_type_stack[return_item.s_item_type] = 
			top_of_stack->prior_type;
	
		top_of_stack = top_of_stack->prior_item;

		UnAllocStackItem(return_stack_ptr);
	
		return_item_ptr = &return_item;
	}

	return return_item_ptr;
}

//-------------------------------------------------------------------------
// Name: ExecPwrRootP(CALCAMNT left_op, CALCAMNT right_op, 
//		CALCAMNT result, bool &display_error)
//-------------------------------------------------------------------------
CALCAMNT Calculator::ExecPwrRootP(CALCAMNT left_op, CALCAMNT right_op,
					 CALCAMNT result, bool &display_error)
{
	// printf("ExePwrRootP\n");
	UNUSED(result);

	if (right_op == 0)
	{
		display_error = true;
		return 0;
	}
	else
		return ExecPower(left_op, (100L / right_op), display_error);
}


//-------------------------------------------------------------------------
// Name: ExecPower(CALCAMNT left_op, CALCAMNT right_op)
//-------------------------------------------------------------------------
CALCAMNT Calculator::ExecPower(CALCAMNT left_op, CALCAMNT right_op, 
					 bool &display_error)
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
		display_error = true;
		return 0;
	}
	else
		return left_op;
}

//-------------------------------------------------------------------------
// Name: ExecOr(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
//-------------------------------------------------------------------------
CALCAMNT Calculator::ExecOr(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
{
	// printf("ExecOr\n");
	CALCAMNT	boh_work_d;
	long 		boh_work_l;
	long		boh_work_r;

	MODF(left_op, &boh_work_d);
	if (FABS(boh_work_d) > LONG_MAX)
	{
		display_error = true;
		return 0;
	}
	
	boh_work_l = (long int)boh_work_d;
	MODF(right_op, &boh_work_d);
	
	if (FABS(boh_work_d) > LONG_MAX)
	{
		display_error = true;
		return 0;
	}
	
	boh_work_r = (long int) boh_work_d;
	return (boh_work_l | boh_work_r);
}

//-------------------------------------------------------------------------
// Name: ExecXor(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
//-------------------------------------------------------------------------
CALCAMNT Calculator::ExecXor(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
{
	// printf("ExecXOr\n");
	CALCAMNT	boh_work_d;
	long 		boh_work_l;
	long		boh_work_r;

	MODF(left_op, &boh_work_d);
	if (FABS(boh_work_d) > LONG_MAX)
	{
		display_error = true;
		return 0;
	}
	
	boh_work_l = (long int)boh_work_d;
	MODF(right_op, &boh_work_d);
	
	if (FABS(boh_work_d) > LONG_MAX)
	{
		display_error = true;
		return 0;
	}
	
	boh_work_r = (long int)boh_work_d;
	return (boh_work_l ^ boh_work_r);
}

//-------------------------------------------------------------------------
// Name: ExecAnd(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
//-------------------------------------------------------------------------
CALCAMNT Calculator::ExecAnd(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
{
	// printf("ExecAnd\n");
	CALCAMNT	boh_work_d;
	long 		boh_work_l;
	long		boh_work_r;

	MODF(left_op, &boh_work_d);
	if (FABS(boh_work_d) > LONG_MAX)
	{
		display_error = true;
		return 0;
	}
	
	boh_work_l = (long int)boh_work_d;
	MODF(right_op, &boh_work_d);
	
	if (FABS(boh_work_d) > LONG_MAX)
	{
		display_error = true;
		return 0;
	}
	
	boh_work_r = (long int)boh_work_d;
	return (boh_work_l & boh_work_r);
}

//-------------------------------------------------------------------------
// Name: ExecLsh(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
//-------------------------------------------------------------------------
CALCAMNT Calculator::ExecLsh(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
{
	// printf("ExecLsh\n");
	CALCAMNT	boh_work_d;
	long 		boh_work_l;
	long		boh_work_r;

	MODF(left_op, &boh_work_d);
	if (FABS(boh_work_d) > LONG_MAX)
	{
		display_error = true;
		return 0;
	}
	
	boh_work_l = (long int) boh_work_d;
	MODF(right_op, &boh_work_d);
	
	if (FABS(boh_work_d) > LONG_MAX)
	{
		display_error = true;
		return 0;
	}
	
	boh_work_r = (long int ) boh_work_d;
	return (boh_work_l << boh_work_r);
}

//-------------------------------------------------------------------------
// Name: ExecRsh(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
//-------------------------------------------------------------------------
CALCAMNT Calculator::ExecRsh(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
{
	// printf("ExecRsh\n");
	CALCAMNT	boh_work_d;
	long 		boh_work_l;
	long		boh_work_r;

	MODF(left_op, &boh_work_d);
	if (FABS(boh_work_d) > LONG_MAX)
	{
		display_error = true;
		return 0;
	}
	
	boh_work_l = (long int)boh_work_d;
	MODF(right_op, &boh_work_d);
	
	if (FABS(boh_work_d) > LONG_MAX)
	{
		display_error = true;
		return 0;
	}
	
	boh_work_r = (long int)boh_work_d;
	return (boh_work_l >> boh_work_r);
}

//-------------------------------------------------------------------------
// Name: ExecAdd(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
//-------------------------------------------------------------------------
CALCAMNT Calculator::ExecAdd(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
{
	// printf("ExecAdd\n");
	return left_op + right_op;
}

//-------------------------------------------------------------------------
// Name: ExecSubtract(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
//-------------------------------------------------------------------------
CALCAMNT Calculator::ExecSubtract(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
{
	// printf("ExecSubtract\n");
	return left_op - right_op;
}

//-------------------------------------------------------------------------
// Name: ExecMultiply(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
//-------------------------------------------------------------------------
CALCAMNT Calculator::ExecMultiply(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
{
	// printf("ExecMulti\n");
	return left_op * right_op;
}

//-------------------------------------------------------------------------
// Name: ExecMod(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
//-------------------------------------------------------------------------
CALCAMNT Calculator::ExecDivide(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
{
	// printf("ExecDivide\n");
	if (right_op == 0)
	{
		display_error = true;
		return 0L;
	}
	else
		return left_op / right_op;
}

//-------------------------------------------------------------------------
// Name: QExecMod(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
//-------------------------------------------------------------------------
CALCAMNT Calculator::ExecMod(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
{
	// printf("ExecMod\n");
	CALCAMNT temp = 0.0;

	if (right_op == 0)
	{
		display_error = true;
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

//-------------------------------------------------------------------------
// Name: ExecIntDiv(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
//-------------------------------------------------------------------------
CALCAMNT Calculator::ExecIntDiv(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
{
	// printf("IndDiv\n");
	if (right_op == 0)
	{
		display_error = true;
		return 0L;
	}
	else 
	{
		MODF(left_op / right_op, &left_op);
		return left_op;
	}
}



//-------------------------------------------------------------------------
// Name: ExecPwrRoot(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
//-------------------------------------------------------------------------
CALCAMNT Calculator::ExecPwrRoot(CALCAMNT left_op, CALCAMNT right_op, bool &display_error)
{
	// printf("ExecPwrRoot  %g left_op, %g right_op\n", left_op, right_op);
	if (right_op == 0)
	{
		display_error = true;
		return 0L;
	}
	
	if (left_op < 0 && isoddint(right_op))
		left_op = -1L * POW((-1L * left_op), (1L)/right_op);
	else
		left_op = POW(left_op, (1L)/right_op);
		
	if (errno == EDOM || errno == ERANGE)
	{
		display_error = true;
		return 0;
	}
	else
		return left_op;
}

//-------------------------------------------------------------------------
// Name: ExecAddSubP(CALCAMNT left_op, CALCAMNT right_op, CALCAMNT result, bool &display_error)
//-------------------------------------------------------------------------	
CALCAMNT Calculator::ExecAddSubP(CALCAMNT left_op, CALCAMNT right_op, CALCAMNT result, bool &display_error)
{
	// printf("ExecAddsubP\n");
	UNUSED(left_op);

	if (result == 0)
	{
		display_error = true;
		return 0;
	} 
	else
		return (result * 100L) / right_op;
}

//-------------------------------------------------------------------------
// Name: ExecMultiplyP(CALCAMNT left_op, CALCAMNT right_op, CALCAMNT result, bool &display_error)
//-------------------------------------------------------------------------
CALCAMNT Calculator::ExecMultiplyP(CALCAMNT left_op, CALCAMNT right_op, CALCAMNT result, bool &display_error)
{
	// printf("ExecMultiplyP\n");
	UNUSED(left_op);
	UNUSED(right_op);
	return (result / 100L);
}

//-------------------------------------------------------------------------
// Name: ExecDivideP(CALCAMNT left_op, CALCAMNT right_op, CALCAMNT result, bool &display_error)
//-------------------------------------------------------------------------
CALCAMNT Calculator::ExecDivideP(CALCAMNT left_op, CALCAMNT right_op, CALCAMNT result, bool &display_error)
{
	// printf("ExecDivideP\n");
	UNUSED(left_op);
	UNUSED(right_op);
	return (result * 100L);
}

//-------------------------------------------------------------------------
// Name: ExecPowerP(CALCAMNT left_op, CALCAMNT right_op, CALCAMNT result, bool &display_error)
//-------------------------------------------------------------------------
CALCAMNT Calculator::ExecPowerP(CALCAMNT left_op, CALCAMNT right_op, CALCAMNT result, bool &display_error)
{
	// printf("ExecPowerP\n");
	UNUSED(result);
	return ExecPower(left_op, (right_op / 100L), display_error);
}

//-------------------------------------------------------------------------
// Name: isoddint(CALCAMNT input)
//-------------------------------------------------------------------------
int Calculator::isoddint(CALCAMNT input)
{
	CALCAMNT dummy;
	
	// Routine to check if CALCAMNT is an Odd integer
	return (MODF(input, &dummy) == 0.0 && MODF(input / 2, &dummy) == 0.5);
}
