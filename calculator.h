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

#ifndef _CALCULATOR_H__
#define _CALCULATOR_H__

#ifdef HAVE_CONFIG_H
	#include "../config.h"
#endif

// IMPORTANT this has to come after ../config.h
#include "kcalctype.h"

#define STACK_SIZE	100

typedef enum _item_type
{ 
	ITEM_FUNCTION, 
	ITEM_AMOUNT 
} item_type;

typedef struct _func_data
{ 
	int item_function;
	int item_precedence;
} func_data;

typedef	union  _item_data
{
	CALCAMNT	item_amount;	// an amount
	func_data	item_func_data;	// or a function
} item_data;



typedef struct _item_contents
{
	item_type	s_item_type;	// a type flag
	item_data	s_item_data; 	// and data
} item_contents;

typedef struct stack_item *stack_ptr;

typedef struct stack_item
{
	// Contents of an item on the input stack

	stack_ptr prior_item;		// Pointer to prior item
	stack_ptr prior_type;		// Pointer to prior type
	item_contents item_value;	// The value of the item
} stack_item;


class Calculator
{
public:
	Calculator();
	virtual ~Calculator();
	
private:
	static stack_ptr	top_of_stack;
	static stack_ptr	top_type_stack[2];
	static int 		stack_next, stack_last;
	static stack_item process_stack[STACK_SIZE];
	
public:
	static void InitStack();
	static item_contents *TopTypeStack(item_type rqstd_type);
	static item_contents *PopStack();
	static void PushStack(item_contents *add_item);
	static void UnAllocStackItem(stack_ptr return_item);
	static stack_ptr AllocStackItem();
	
public:
	static CALCAMNT ExecPwrRootP(CALCAMNT left_op, CALCAMNT right_op, CALCAMNT result, bool &display_error);
	static CALCAMNT ExecPower(CALCAMNT left_op, CALCAMNT right_op, bool &display_error);
	static CALCAMNT ExecOr(CALCAMNT left_op, CALCAMNT right_op, bool &display_error);
	static CALCAMNT ExecXor(CALCAMNT left_op, CALCAMNT right_op, bool &display_error);
	static CALCAMNT ExecAnd(CALCAMNT left_op, CALCAMNT right_op, bool &display_error);
	static CALCAMNT ExecLsh(CALCAMNT left_op, CALCAMNT right_op, bool &display_error);
	static CALCAMNT ExecRsh(CALCAMNT left_op, CALCAMNT right_op, bool &display_error);
	static CALCAMNT ExecAdd(CALCAMNT left_op, CALCAMNT right_op, bool &display_error);
	static CALCAMNT ExecSubtract(CALCAMNT left_op, CALCAMNT right_op, bool &display_error);
	static CALCAMNT ExecMultiply(CALCAMNT left_op, CALCAMNT right_op, bool &display_error);
	static CALCAMNT ExecDivide(CALCAMNT left_op, CALCAMNT right_op, bool &display_error);
	static CALCAMNT ExecMod(CALCAMNT left_op, CALCAMNT right_op, bool &display_error);
	static CALCAMNT ExecPwrRoot(CALCAMNT left_op, CALCAMNT right_op, bool &display_error);
	static CALCAMNT ExecIntDiv(CALCAMNT left_op, CALCAMNT right_op, bool &display_error);
	static CALCAMNT ExecAddSubP(CALCAMNT left_op, CALCAMNT right_op, CALCAMNT result, bool &display_error);
	static CALCAMNT ExecMultiplyP(CALCAMNT left_op, CALCAMNT right_op, CALCAMNT result, bool &display_error);
	static CALCAMNT ExecDivideP(CALCAMNT left_op, CALCAMNT right_op, CALCAMNT result, bool &display_error);
	static CALCAMNT ExecPowerP(CALCAMNT left_op, CALCAMNT right_op, CALCAMNT result, bool &display_error);
	static CALCAMNT ExecFunction(CALCAMNT left_op, int function, CALCAMNT right_op, bool &display_error);
private:
	static int isoddint(CALCAMNT input);
};

#endif
