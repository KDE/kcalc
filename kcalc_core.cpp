/*
Copyright (C) 2001 - 2013 Evan Teran
                          evan.teran@gmail.com

Copyright (C) 2003 - 2005 Klaus Niederkrueger
                          kniederk@math.uni-koeln.de

Copyright (C) 1996 - 2000 Bernd Johannes Wuebben
                          wuebben@kde.org
						  
Copyright (C) 1995        Martin Bartlett

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kcalc_core.h"
#include <config-kcalc.h>
#include <csignal>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

namespace {

void fpe_handler(int fpe_parm) {
    Q_UNUSED(fpe_parm);
    // display_error = true;
    //tmp_number = 0L;
}

KNumber Deg2Rad(const KNumber &x) {
	return x * (KNumber::Pi() / KNumber(180));
}

KNumber Gra2Rad(const KNumber &x) {
	return x * (KNumber::Pi() / KNumber(200));
}

KNumber Rad2Deg(const KNumber &x) {
	 return x * (KNumber(180) / KNumber::Pi());
}

KNumber Rad2Gra(const KNumber &x) {
	return x * (KNumber(200) / KNumber::Pi());
}

bool error_;

KNumber ExecOr(const KNumber &left_op, const KNumber &right_op) {
    return left_op | right_op;
}

KNumber ExecXor(const KNumber &left_op, const KNumber &right_op) {
    return left_op ^ right_op;
}

KNumber ExecAnd(const KNumber &left_op, const KNumber &right_op) {
    return left_op & right_op;
}

KNumber ExecLsh(const KNumber &left_op, const KNumber &right_op) {
    return left_op << right_op;
}

KNumber ExecRsh(const KNumber &left_op, const KNumber &right_op) {
    return left_op >> right_op;
}

KNumber ExecAdd(const KNumber &left_op, const KNumber &right_op) {
    return left_op + right_op;
}

KNumber ExecSubtract(const KNumber &left_op, const KNumber &right_op) {
    return left_op - right_op;
}

KNumber ExecMultiply(const KNumber &left_op, const KNumber &right_op) {
    return left_op * right_op;
}

KNumber ExecDivide(const KNumber &left_op, const KNumber &right_op) {
    return left_op / right_op;
}

KNumber ExecMod(const KNumber &left_op, const KNumber &right_op) {
    return left_op % right_op;
}

KNumber ExecIntDiv(const KNumber &left_op, const KNumber &right_op) {
    return (left_op / right_op).integerPart();
}

KNumber ExecBinom(const KNumber &left_op, const KNumber &right_op) {
	return left_op.bin(right_op);
}

KNumber ExecPower(const KNumber &left_op, const KNumber &right_op) {
    return left_op.pow(right_op);
}

KNumber ExecPwrRoot(const KNumber &left_op, const KNumber &right_op) {
    return left_op.pow(KNumber::One / right_op);
}

KNumber ExecAddP(const KNumber &left_op, const KNumber &right_op) {
    return left_op * (KNumber::One + right_op / KNumber(100));
}

KNumber ExecSubP(const KNumber &left_op, const KNumber &right_op) {
    return left_op * (KNumber::One - right_op / KNumber(100));
}

KNumber ExecMultiplyP(const KNumber &left_op, const KNumber &right_op) {
    return left_op * right_op / KNumber(100);
}

KNumber ExecDivideP(const KNumber &left_op, const KNumber &right_op) {
    return left_op * KNumber(100) / right_op;
}

// move a number into the interval [0,360) by adding multiples of 360
KNumber moveIntoDegInterval(const KNumber &num) {
    KNumber tmp_num = num - (num / KNumber(360)).integerPart() * KNumber(360);
    if (tmp_num < KNumber::Zero)
        return tmp_num + KNumber(360);
    return tmp_num;
}

// move a number into the interval [0,400) by adding multiples of 400
KNumber moveIntoGradInterval(const KNumber &num) {
    KNumber tmp_num = num - (num / KNumber(400)).integerPart() * KNumber(400);
    if (tmp_num < KNumber::Zero)
        return tmp_num + KNumber(400);
    return tmp_num;
}

typedef KNumber(*Arith)(const KNumber &, const KNumber &);
typedef KNumber(*Prcnt)(const KNumber &, const KNumber &);

struct operator_data {
    int precedence;  // priority of operators in " enum Operation"
    Arith arith_ptr;
    Prcnt prcnt_ptr;
};

// build precedence list
const struct operator_data Operator[] = {
    { 0, NULL,         NULL},          // FUNC_EQUAL
    { 0, NULL,         NULL},          // FUNC_PERCENT
    { 0, NULL,         NULL},          // FUNC_BRACKET
    { 1, ExecOr,       NULL},          // FUNC_OR
    { 2, ExecXor,      NULL},          // FUNC_XOR
    { 3, ExecAnd,      NULL},          // FUNC_AND
    { 4, ExecLsh,      NULL},          // FUNC_LSH
    { 4, ExecRsh,      NULL},          // FUNC_RSH
    { 5, ExecAdd,      ExecAddP},      // FUNC_ADD
    { 5, ExecSubtract, ExecSubP},      // FUNC_SUBTRACT
    { 6, ExecMultiply, ExecMultiplyP}, // FUNC_MULTIPLY
    { 6, ExecDivide,   ExecDivideP},   // FUNC_DIVIDE
    { 6, ExecMod,      NULL},          // FUNC_MOD
    { 6, ExecIntDiv,   NULL},          // FUNC_INTDIV
    { 7, ExecBinom,    NULL},          // FUNC_BINOM
    { 7, ExecPower,    NULL},          // FUNC_POWER
    { 7, ExecPwrRoot,  NULL}           // FUNC_PWR_ROOT
};

}


CalcEngine::CalcEngine() : percent_mode_(false) {
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
    last_number_ = KNumber::Zero;
    error_ = false;
}

KNumber CalcEngine::lastOutput(bool &error) const
{
    error = error_;
    return last_number_;
}

void CalcEngine::ArcCosDeg(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR || input < -KNumber::One || input > KNumber::One) {
        last_number_ = KNumber::NaN;
        return;
    }

    if (input.type() == KNumber::TYPE_INTEGER) {
        if (input == KNumber::One) {
            last_number_ = KNumber::Zero;
            return;
        }
        if (input == - KNumber::One) {
            last_number_ = KNumber(180);
            return;
        }
        if (input == KNumber::Zero) {
            last_number_ = KNumber(90);
            return;
        }
    }
    last_number_ = Rad2Deg(input.acos());
}

void CalcEngine::ArcCosRad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR || input < -KNumber::One || input > KNumber::One) {
        last_number_ = KNumber::NaN;
        return;
    }
    last_number_ = input.acos();
}

void CalcEngine::ArcCosGrad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR || input < -KNumber::One || input > KNumber::One) {
        last_number_ = KNumber::NaN;
        return;
    }
    if (input.type() == KNumber::TYPE_INTEGER) {
        if (input == KNumber::One) {
            last_number_ = KNumber::Zero;
            return;
        }
        if (input == - KNumber::One) {
            last_number_ = KNumber(200);
            return;
        }
        if (input == KNumber::Zero) {
            last_number_ = KNumber(100);
            return;
        }
    }
    last_number_ = Rad2Gra(input.acos());
}

void CalcEngine::ArcSinDeg(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR  ||
            input < -KNumber::One || input > KNumber::One) {
        last_number_ = KNumber::NaN;
        return;
    }
    if (input.type() == KNumber::TYPE_INTEGER) {
        if (input == KNumber::One) {
            last_number_ = KNumber(90);
            return;
        }
        if (input == - KNumber::One) {
            last_number_ = KNumber(-90);
            return;
        }
        if (input == KNumber::Zero) {
            last_number_ = KNumber::Zero;
            return;
        }
    }
    last_number_ = Rad2Deg(input.asin());
}

void CalcEngine::ArcSinRad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR  ||
            input < -KNumber::One || input > KNumber::One) {
        last_number_ = KNumber::NaN;
        return;
    }
    last_number_ = input.asin();
}

void CalcEngine::ArcSinGrad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR  ||
            input < -KNumber::One || input > KNumber::One) {
        last_number_ = KNumber::NaN;
        return;
    }
    if (input.type() == KNumber::TYPE_INTEGER) {
        if (input == KNumber::One) {
            last_number_ = KNumber(100);
            return;
        }
        if (input == - KNumber::One) {
            last_number_ = KNumber(-100);
            return;
        }
        if (input == KNumber::Zero) {
            last_number_ = KNumber::Zero;
            return;
        }
    }
    last_number_ = Rad2Gra(input.asin());
}

void CalcEngine::ArcTangensDeg(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN) last_number_ = KNumber::NaN;
        if (input == KNumber::PosInfinity) last_number_ = KNumber(90);
        if (input == KNumber::NegInfinity)  last_number_ = KNumber(-90);
        return;
    }

    last_number_ = Rad2Deg(input.atan());
}

void CalcEngine::ArcTangensRad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN) last_number_ = KNumber::NaN;
        if (input == KNumber::PosInfinity)
            last_number_ = KNumber::Pi() / KNumber(2);
        if (input == KNumber::NegInfinity)
            last_number_ = -KNumber::Pi() / KNumber(2);
        return;
    }

    last_number_ = input.atan();
}

void CalcEngine::ArcTangensGrad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)         last_number_ = KNumber::NaN;
        if (input == KNumber::PosInfinity) last_number_ = KNumber(100);
        if (input == KNumber::NegInfinity) last_number_ = KNumber(-100);
        return;
    }

    last_number_ = Rad2Gra(input.atan());
}

void CalcEngine::AreaCosHyp(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)         last_number_ = KNumber::NaN;
        if (input == KNumber::PosInfinity) last_number_ = KNumber::PosInfinity;
        if (input == KNumber::NegInfinity) last_number_ = KNumber::NaN;
        return;
    }

    if (input < KNumber::One) {
        last_number_ = KNumber::NaN;
        return;
    }
    if (input == KNumber::One) {
        last_number_ = KNumber::Zero;
        return;
    }
    last_number_ = input.acosh();
}

void CalcEngine::AreaSinHyp(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)         last_number_ = KNumber::NaN;
        if (input == KNumber::PosInfinity) last_number_ = KNumber::PosInfinity;
        if (input == KNumber::NegInfinity) last_number_ = KNumber::NegInfinity;
        return;
    }

    if (input == KNumber::Zero) {
        last_number_ = KNumber::Zero;
        return;
    }
    last_number_ = input.asinh();
}

void CalcEngine::AreaTangensHyp(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        last_number_ = KNumber::NaN;
        return;
    }

    if (input < -KNumber::One || input > KNumber::One) {
        last_number_ = KNumber::NaN;
        return;
    }
    if (input == KNumber::One) {
        last_number_ = KNumber::PosInfinity;
        return;
    }
    if (input == - KNumber::One) {
        last_number_ = KNumber::NegInfinity;
        return;
    }
    last_number_ = input.atanh();
}

void CalcEngine::Complement(const KNumber &input)
{
    if (input.type() != KNumber::TYPE_INTEGER) {
        last_number_ = KNumber::NaN;
        return;
    }

    last_number_ = ~input;
}

void CalcEngine::CosDeg(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        last_number_ = KNumber::NaN;
        return;
    }
	
    KNumber trunc_input = moveIntoDegInterval(input);
    
	if (trunc_input.type() == KNumber::TYPE_INTEGER) {
        KNumber mult = trunc_input / KNumber(90);
        if (mult.type() == KNumber::TYPE_INTEGER) {
            if (mult == KNumber::Zero)
                last_number_ = KNumber::One;
            else if (mult == KNumber::One)
                last_number_ = KNumber::Zero;
            else if (mult == KNumber(2))
                last_number_ = KNumber::NegOne;
            else if (mult == KNumber(3))
                last_number_ = KNumber::Zero;
            else kDebug() << "Something wrong in CalcEngine::CosDeg";
            return;
        }
    }
		
    trunc_input = Deg2Rad(trunc_input);
    last_number_ = trunc_input.cos();
}

void CalcEngine::CosRad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        last_number_ = KNumber::NaN;
        return;
    }
	
    last_number_ = input.cos();
}

void CalcEngine::CosGrad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        last_number_ = KNumber::NaN;
        return;
    }
    KNumber trunc_input = moveIntoGradInterval(input);
    if (trunc_input.type() == KNumber::TYPE_INTEGER) {
        KNumber mult = trunc_input / KNumber(100);
        if (mult.type() == KNumber::TYPE_INTEGER) {
            if (mult == KNumber::Zero)
                last_number_ = KNumber::One;
            else if (mult == KNumber::One)
                last_number_ = KNumber::Zero;
            else if (mult == KNumber(2))
                last_number_ = KNumber::NegOne;
            else if (mult == KNumber(3))
                last_number_ = KNumber::Zero;
            else kDebug() << "Something wrong in CalcEngine::CosGrad";
            return;
        }
    }
    trunc_input = Gra2Rad(trunc_input);

    last_number_ = trunc_input.cos();
}

void CalcEngine::CosHyp(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)         last_number_ = KNumber::NaN;
        if (input == KNumber::PosInfinity) last_number_ = KNumber::PosInfinity;
		// YES, this should be *positive* infinity. We mimic the behavior of 
		// libc which says the following for cosh
		//
		// "If x is positive infinity or negative infinity, positive infinity is returned."
        if (input == KNumber::NegInfinity) last_number_ = KNumber::PosInfinity;
        return;
    }

    last_number_ = input.cosh();
}

void CalcEngine::Cube(const KNumber &input)
{
    last_number_ = input * input * input;
}

void CalcEngine::CubeRoot(const KNumber &input)
{
    last_number_ = input.cbrt();
}

void CalcEngine::Exp(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)         last_number_ = KNumber::NaN;
        if (input == KNumber::PosInfinity) last_number_ = KNumber::PosInfinity;
        if (input == KNumber::NegInfinity) last_number_ = KNumber::Zero;
        return;
    }
    last_number_ = KNumber::Euler().pow(input);
}

void CalcEngine::Exp10(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)         last_number_ = KNumber::NaN;
        if (input == KNumber::PosInfinity) last_number_ = KNumber::PosInfinity;
        if (input == KNumber::NegInfinity) last_number_ = KNumber::Zero;
        return;
    }
    last_number_ = KNumber(10).pow(input);
}


void CalcEngine::Factorial(const KNumber &input)
{
    if (input == KNumber::PosInfinity) return;
    if (input < KNumber::Zero || input.type() == KNumber::TYPE_ERROR) {
        error_ = true;
        last_number_ = KNumber::NaN;
        return;
    }
	
    last_number_ = input.integerPart().factorial();
}

void CalcEngine::InvertSign(const KNumber &input)
{
    last_number_ = -input;
}

void CalcEngine::Ln(const KNumber &input)
{
    if (input < KNumber::Zero)
        last_number_ = KNumber::NaN;
    else if (input == KNumber::Zero)
        last_number_ = KNumber::NegInfinity;
    else if (input == KNumber::One)
        last_number_ = KNumber::Zero;
    else {
        last_number_ = input.ln();
    }
}

void CalcEngine::Log10(const KNumber &input)
{
    if (input < KNumber::Zero)
        last_number_ = KNumber::NaN;
    else if (input == KNumber::Zero)
        last_number_ = KNumber::NegInfinity;
    else if (input == KNumber::One)
        last_number_ = KNumber::Zero;
    else {
        last_number_ = input.log10();
    }
}

void CalcEngine::ParenClose(KNumber input)
{
    // evaluate stack until corresponding opening bracket
    while (!stack_.isEmpty()) {
        Node tmp_node = stack_.pop();
        if (tmp_node.operation == FUNC_BRACKET)
            break;
        input = evalOperation(tmp_node.number, tmp_node.operation, input);
    }
    last_number_ = input;
    return;
}

void CalcEngine::ParenOpen(const KNumber &input)
{
    enterOperation(input, FUNC_BRACKET);
}

void CalcEngine::Reciprocal(const KNumber &input)
{
    last_number_ = KNumber::One / input;
}


void CalcEngine::SinDeg(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        last_number_ = KNumber::NaN;
        return;
    }

    KNumber trunc_input = moveIntoDegInterval(input);
    if (trunc_input.type() == KNumber::TYPE_INTEGER) {
        KNumber mult = trunc_input / KNumber(90);
        if (mult.type() == KNumber::TYPE_INTEGER) {
            if (mult == KNumber::Zero)
                last_number_ = KNumber::Zero;
            else if (mult == KNumber::One)
                last_number_ = KNumber::One;
            else if (mult == KNumber(2))
                last_number_ = KNumber::Zero;
            else if (mult == KNumber(3))
                last_number_ = KNumber::NegOne;
            else kDebug() << "Something wrong in CalcEngine::SinDeg";
            return;
        }
    }
    trunc_input = Deg2Rad(trunc_input);

    last_number_ = trunc_input.sin();
}

void CalcEngine::SinRad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        last_number_ = KNumber::NaN;
        return;
    }

    last_number_ = input.sin();
}

void CalcEngine::SinGrad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        last_number_ = KNumber::NaN;
        return;
    }

    KNumber trunc_input = moveIntoGradInterval(input);
    if (trunc_input.type() == KNumber::TYPE_INTEGER) {
        KNumber mult = trunc_input / KNumber(100);
        if (mult.type() == KNumber::TYPE_INTEGER) {
            if (mult == KNumber::Zero)
                last_number_ = KNumber::Zero;
            else if (mult == KNumber::One)
                last_number_ = KNumber::One;
            else if (mult == KNumber(2))
                last_number_ = KNumber::Zero;
            else if (mult == KNumber(3))
                last_number_ = KNumber::NegOne;
            else kDebug() << "Something wrong in CalcEngine::SinGrad";
            return;
        }
    }

    trunc_input = Gra2Rad(trunc_input);

    last_number_ = trunc_input.sin();
}

void CalcEngine::SinHyp(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)         last_number_ = KNumber::NaN;
        if (input == KNumber::PosInfinity) last_number_ = KNumber::PosInfinity;
        if (input == KNumber::NegInfinity) last_number_ = KNumber::NegInfinity;
        return;
    }

    last_number_ = input.sinh();
}

void CalcEngine::Square(const KNumber &input)
{
    last_number_ = input * input;
}

void CalcEngine::SquareRoot(const KNumber &input)
{
    last_number_ = input.sqrt();
}

void CalcEngine::StatClearAll(const KNumber &input)
{
    Q_UNUSED(input);
    stats.clearAll();
}

void CalcEngine::StatCount(const KNumber &input)
{
    Q_UNUSED(input);
    last_number_ = KNumber(stats.count());
}

void CalcEngine::StatDataNew(const KNumber &input)
{
    stats.enterData(input);
    last_number_ = KNumber(stats.count());
}

void CalcEngine::StatDataDel(const KNumber &input)
{
    Q_UNUSED(input);
    stats.clearLast();
    last_number_ = KNumber::Zero;
}

void CalcEngine::StatMean(const KNumber &input)
{
    Q_UNUSED(input);
    last_number_ = stats.mean();

    error_ = stats.error();
}

void CalcEngine::StatMedian(const KNumber &input)
{
    Q_UNUSED(input);
    last_number_ = stats.median();

    error_ = stats.error();
}

void CalcEngine::StatStdDeviation(const KNumber &input)
{
    Q_UNUSED(input);
    last_number_ = stats.std();

    error_ = stats.error();
}

void CalcEngine::StatStdSample(const KNumber &input)
{
    Q_UNUSED(input);
    last_number_ = stats.sample_std();

    error_ = stats.error();
}

void CalcEngine::StatSum(const KNumber &input)
{
    Q_UNUSED(input);
    last_number_ = stats.sum();
}

void CalcEngine::StatSumSquares(const KNumber &input)
{
    Q_UNUSED(input);
    last_number_ = stats.sum_of_squares();

    error_ = stats.error();
}

void CalcEngine::TangensDeg(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        last_number_ = KNumber::NaN;
        return;
    }

    SinDeg(input);
    KNumber arg1 = last_number_;
    CosDeg(input);
    KNumber arg2 = last_number_;
    last_number_ = arg1 / arg2;
}

void CalcEngine::TangensRad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        last_number_ = KNumber::NaN;
        return;
    }

    SinRad(input);
    KNumber arg1 = last_number_;
    CosRad(input);
    KNumber arg2 = last_number_;
    last_number_ = arg1 / arg2;
}

void CalcEngine::TangensGrad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        last_number_ = KNumber::NaN;
        return;
    }

    SinGrad(input);
    KNumber arg1 = last_number_;
    CosGrad(input);
    KNumber arg2 = last_number_;
    last_number_ = arg1 / arg2;
}

void CalcEngine::TangensHyp(const KNumber &input)
{
	if (input.type() == KNumber::TYPE_ERROR) {
		if (input == KNumber::NaN)         last_number_ = KNumber::NaN;
		if (input == KNumber::PosInfinity) last_number_ = KNumber::One;
		if (input == KNumber::NegInfinity) last_number_ = KNumber::NegOne;
		return;
	}

	last_number_ = input.tanh();
}

KNumber CalcEngine::evalOperation(const KNumber &arg1, Operation operation, const KNumber &arg2)
{
    if (!percent_mode_ || Operator[operation].prcnt_ptr == NULL) {
        return (Operator[operation].arith_ptr)(arg1, arg2);
    } else {
        percent_mode_ = false;
        return (Operator[operation].prcnt_ptr)(arg1, arg2);
    }
}

void CalcEngine::enterOperation(const KNumber &number, Operation func)
{
    Node tmp_node;

    if (func == FUNC_BRACKET) {
        tmp_node.number = KNumber::Zero;
        tmp_node.operation = FUNC_BRACKET;

        stack_.push(tmp_node);

        return;
    }

    if (func == FUNC_PERCENT) {
        percent_mode_ = true;
    }

    tmp_node.number = number;
    tmp_node.operation = func;

    stack_.push(tmp_node);

    evalStack();
}

bool CalcEngine::evalStack()
{
    // this should never happen
    Q_ASSERT(!stack_.isEmpty());

    Node tmp_node = stack_.pop();

    while (! stack_.isEmpty()) {
        Node tmp_node2 = stack_.pop();
        if (Operator[tmp_node.operation].precedence <=
                Operator[tmp_node2.operation].precedence) {
            if (tmp_node2.operation == FUNC_BRACKET) continue;
			const KNumber tmp_result = evalOperation(tmp_node2.number, tmp_node2.operation, tmp_node.number);
            tmp_node.number = tmp_result;
        } else {
            stack_.push(tmp_node2);
            break;
        }

    }

    if (tmp_node.operation != FUNC_EQUAL && tmp_node.operation != FUNC_PERCENT)
        stack_.push(tmp_node);

    last_number_ = tmp_node.number;
    return true;
}

void CalcEngine::Reset()
{
    error_ = false;
    last_number_ = KNumber::Zero;

    stack_.clear();
}


