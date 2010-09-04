/*
    kCalculator, a scientific calculator for the X window system using the
    Qt widget libraries, available at no cost at http://www.troll.no

    The stack engine contained in this file was take from
    Martin Bartlett's xfrmcalc

    portions: Copyright (C) 2003-2006 Klaus Niederkrueger
                                        kniederk@ulb.ac.be

    portions: Copyright (C) 1996 Bernd Johannes Wuebben
                                   wuebben@math.cornell.edu

    portions:  Copyright (C) 1995 Martin Bartlett

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
// On Solaris, need to include math.h (or <cmath>) before any STL headers,
// because of ambiguity around the identifier "exception".
#include <cmath>

#include <config-kcalc.h>

#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include <stdlib.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

static void fpe_handler(int fpe_parm)
{
    Q_UNUSED(fpe_parm);
    // display_error = true;
    //tmp_number = 0L;
}


static bool error_;

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
    return ((input / KNumber(2)).type() == KNumber::IntegerType);
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
    while (tmp_count > right_op) {
        tmp_result = tmp_count * tmp_result;
        tmp_count -= KNumber::One;
    }

    tmp_count = left_op - right_op;
    while (tmp_count > KNumber::One) {
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
    return left_op * (KNumber::One + right_op / KNumber(100));
}

static KNumber ExecSubP(const KNumber & left_op, const KNumber & right_op)
{
    return left_op * (KNumber::One - right_op / KNumber(100));
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
        :   percent_mode_(false)
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
    if (input.type() == KNumber::SpecialType  ||
            input < -KNumber::One  ||  input > KNumber::One) {
        last_number_ = KNumber("nan");
        return;
    }

    if (input.type() == KNumber::IntegerType) {
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
    last_number_ = Rad2Deg(KNumber(acos(static_cast<double>(input))));
}

void CalcEngine::ArcCosRad(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType  ||
            input < -KNumber::One  ||  input > KNumber::One) {
        last_number_ = KNumber("nan");
        return;
    }
    last_number_ = KNumber(acos(static_cast<double>(input)));
}

void CalcEngine::ArcCosGrad(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType  ||
            input < -KNumber::One  ||  input > KNumber::One) {
        last_number_ = KNumber("nan");
        return;
    }
    if (input.type() == KNumber::IntegerType) {
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
    last_number_ = Rad2Gra(KNumber(acos(static_cast<double>(input))));
}

void CalcEngine::ArcSinDeg(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType  ||
            input < -KNumber::One  ||  input > KNumber::One) {
        last_number_ = KNumber("nan");
        return;
    }
    if (input.type() == KNumber::IntegerType) {
        if (input == KNumber::One) {
            last_number_ = KNumber(90);
            return;
        }
        if (input == - KNumber::One) {
            last_number_ = KNumber(-90);
            return;
        }
        if (input == KNumber::Zero) {
            last_number_ = KNumber(0);
            return;
        }
    }
    last_number_ = Rad2Deg(KNumber(asin(static_cast<double>(input))));
}

void CalcEngine::ArcSinRad(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType  ||
            input < -KNumber::One  ||  input > KNumber::One) {
        last_number_ = KNumber("nan");
        return;
    }
    last_number_ = KNumber(asin(static_cast<double>(input)));
}

void CalcEngine::ArcSinGrad(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType  ||
            input < -KNumber::One  ||  input > KNumber::One) {
        last_number_ = KNumber("nan");
        return;
    }
    if (input.type() == KNumber::IntegerType) {
        if (input == KNumber::One) {
            last_number_ = KNumber(100);
            return;
        }
        if (input == - KNumber::One) {
            last_number_ = KNumber(-100);
            return;
        }
        if (input == KNumber::Zero) {
            last_number_ = KNumber(0);
            return;
        }
    }
    last_number_ = Rad2Gra(KNumber(asin(static_cast<double>(input))));
}

void CalcEngine::ArcTangensDeg(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType) {
        if (input == KNumber("nan")) last_number_ = KNumber("nan");
        if (input == KNumber("inf")) last_number_ = KNumber(90);
        if (input == KNumber("-inf"))  last_number_ = KNumber(-90);
        return;
    }

    last_number_ = Rad2Deg(KNumber(atan(static_cast<double>(input))));
}

void CalcEngine::ArcTangensRad(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType) {
        if (input == KNumber("nan")) last_number_ = KNumber("nan");
        if (input == KNumber("inf"))
            last_number_ = KNumber::Pi() / KNumber(2);
        if (input == KNumber("-inf"))
            last_number_ = -KNumber::Pi() / KNumber(2);
        return;
    }

    last_number_ = KNumber(atan(static_cast<double>(input)));
}

void CalcEngine::ArcTangensGrad(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType) {
        if (input == KNumber("nan")) last_number_ = KNumber("nan");
        if (input == KNumber("inf")) last_number_ = KNumber(100);
        if (input == KNumber("-inf"))  last_number_ = KNumber(-100);
        return;
    }

    last_number_ = Rad2Gra(KNumber(atan(static_cast<double>(input))));
}

void CalcEngine::AreaCosHyp(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType) {
        if (input == KNumber("nan")) last_number_ = KNumber("nan");
        if (input == KNumber("inf")) last_number_ = KNumber("inf");
        if (input == KNumber("-inf"))  last_number_ = KNumber("nan");
        return;
    }

    if (input < KNumber::One) {
        last_number_ = KNumber("nan");
        return;
    }
    if (input == KNumber::One) {
        last_number_ = KNumber::Zero;
        return;
    }
    last_number_ = KNumber(acosh(static_cast<double>(input)));
}

void CalcEngine::AreaSinHyp(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType) {
        if (input == KNumber("nan")) last_number_ = KNumber("nan");
        if (input == KNumber("inf")) last_number_ = KNumber("inf");
        if (input == KNumber("-inf"))  last_number_ = KNumber("-inf");
        return;
    }

    if (input == KNumber::Zero) {
        last_number_ = KNumber::Zero;
        return;
    }
    last_number_ = KNumber(asinh(static_cast<double>(input)));
}

void CalcEngine::AreaTangensHyp(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType) {
        last_number_ = KNumber("nan");
        return;
    }

    if (input < -KNumber::One  ||  input > KNumber::One) {
        last_number_ = KNumber("nan");
        return;
    }
    if (input == KNumber::One) {
        last_number_ = KNumber("inf");
        return;
    }
    if (input == - KNumber::One) {
        last_number_ = KNumber("-inf");
        return;
    }
    last_number_ = KNumber(atanh(static_cast<double>(input)));
}

void CalcEngine::Complement(const KNumber &input)
{
    if (input.type() != KNumber::IntegerType) {
        last_number_ = KNumber("nan");
        return;
    }
    // Note: assumes 64-bit "logic mode"
    quint64 value = static_cast<quint64>(input);
    last_number_ = KNumber(~value);
}



// move a number into the interval [0,360) by adding multiples of 360
static KNumber const moveIntoDegInterval(KNumber const &num)
{
    KNumber tmp_num = num - (num / KNumber(360)).integerPart() * KNumber(360);
    if (tmp_num < KNumber::Zero)
        return tmp_num + KNumber(360);
    return tmp_num;
}

// move a number into the interval [0,400) by adding multiples of 400
static KNumber const moveIntoGradInterval(KNumber const &num)
{
    KNumber tmp_num = num - (num / KNumber(400)).integerPart() * KNumber(400);
    if (tmp_num < KNumber::Zero)
        return tmp_num + KNumber(400);
    return tmp_num;
}


void CalcEngine::CosDeg(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType) {
        last_number_ = KNumber("nan");
        return;
    }
    KNumber trunc_input = moveIntoDegInterval(input);
    if (trunc_input.type() == KNumber::IntegerType) {
        KNumber mult = trunc_input / KNumber(90);
        if (mult.type() == KNumber::IntegerType) {
            if (mult == KNumber::Zero)
                last_number_ = 1;
            else if (mult == KNumber(1))
                last_number_ = 0;
            else if (mult == KNumber(2))
                last_number_ = -1;
            else if (mult == KNumber(3))
                last_number_ = 0;
            else kDebug() << "Something wrong in CalcEngine::CosDeg";
            return;
        }
    }
    trunc_input = Deg2Rad(trunc_input);

    last_number_ = KNumber(cos(static_cast<double>(trunc_input)));
}

void CalcEngine::CosRad(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType) {
        last_number_ = KNumber("nan");
        return;
    }
    last_number_ = KNumber(cos(static_cast<double>(input)));
}

void CalcEngine::CosGrad(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType) {
        last_number_ = KNumber("nan");
        return;
    }
    KNumber trunc_input = moveIntoGradInterval(input);
    if (trunc_input.type() == KNumber::IntegerType) {
        KNumber mult = trunc_input / KNumber(100);
        if (mult.type() == KNumber::IntegerType) {
            if (mult == KNumber::Zero)
                last_number_ = 1;
            else if (mult == KNumber(1))
                last_number_ = 0;
            else if (mult == KNumber(2))
                last_number_ = -1;
            else if (mult == KNumber(3))
                last_number_ = 0;
            else kDebug() << "Something wrong in CalcEngine::CosGrad";
            return;
        }
    }
    trunc_input = Gra2Rad(trunc_input);

    last_number_ = KNumber(cos(static_cast<double>(trunc_input)));
}

void CalcEngine::CosHyp(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType) {
        if (input == KNumber("nan")) last_number_ = KNumber("nan");
        if (input == KNumber("inf")) last_number_ = KNumber("inf");
        if (input == KNumber("-inf"))  last_number_ = KNumber("inf");
        return;
    }

    last_number_ = KNumber(cosh(static_cast<double>(input)));
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
    if (input.type() == KNumber::SpecialType) {
        if (input == KNumber("nan")) last_number_ = KNumber("nan");
        if (input == KNumber("inf")) last_number_ = KNumber("inf");
        if (input == KNumber("-inf"))  last_number_ = KNumber::Zero;
        return;
    }
    last_number_ = KNumber(exp(static_cast<double>(input)));
}

void CalcEngine::Exp10(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType) {
        if (input == KNumber("nan")) last_number_ = KNumber("nan");
        if (input == KNumber("inf")) last_number_ = KNumber("inf");
        if (input == KNumber("-inf"))  last_number_ = KNumber::Zero;
        return;
    }
    last_number_ = KNumber(10).power(input);
}


void CalcEngine::Factorial(const KNumber &input)
{
    if (input == KNumber("inf")) return;
    if (input < KNumber::Zero || input.type() == KNumber::SpecialType) {
        error_ = true;
        last_number_ = KNumber("nan");
        return;
    }
    KNumber tmp_amount = input.integerPart();

    last_number_ = tmp_amount.factorial();
}

void CalcEngine::InvertSign(const KNumber &input)
{
    last_number_ = -input;
}

void CalcEngine::Ln(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType) {
        if (input == KNumber("nan")) last_number_ = KNumber("nan");
        if (input == KNumber("inf")) last_number_ = KNumber("inf");
        if (input == KNumber("-inf"))  last_number_ = KNumber("nan");
        return;
    }
    if (input < KNumber::Zero)
        last_number_ = KNumber("nan");
    else if (input == KNumber::Zero)
        last_number_ = KNumber("-inf");
    else if (input == KNumber::One)
        last_number_ = 0;
    else {
        last_number_ = KNumber(log(static_cast<double>(input)));
    }
}

void CalcEngine::Log10(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType) {
        if (input == KNumber("nan")) last_number_ = KNumber("nan");
        if (input == KNumber("inf")) last_number_ = KNumber("inf");
        if (input == KNumber("-inf"))  last_number_ = KNumber("nan");
        return;
    }
    if (input < KNumber::Zero)
        last_number_ = KNumber("nan");
    else if (input == KNumber::Zero)
        last_number_ = KNumber("-inf");
    else if (input == KNumber::One)
        last_number_ = 0;
    else {
        last_number_ = KNumber(log10(static_cast<double>(input)));
    }
}

void CalcEngine::ParenClose(KNumber input)
{
    // evaluate stack until corresponding opening bracket
    while (!stack_.isEmpty()) {
        Node tmp_node = stack_.pop();
        if (tmp_node.operation == FUNC_BRACKET)
            break;
        input = evalOperation(tmp_node.number, tmp_node.operation,
                              input);
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
    if (input.type() == KNumber::SpecialType) {
        last_number_ = KNumber("nan");
        return;
    }

    KNumber trunc_input = moveIntoDegInterval(input);
    if (trunc_input.type() == KNumber::IntegerType) {
        KNumber mult = trunc_input / KNumber(90);
        if (mult.type() == KNumber::IntegerType) {
            if (mult == KNumber::Zero)
                last_number_ = 0;
            else if (mult == KNumber(1))
                last_number_ = 1;
            else if (mult == KNumber(2))
                last_number_ = 0;
            else if (mult == KNumber(3))
                last_number_ = -1;
            else kDebug() << "Something wrong in CalcEngine::SinDeg";
            return;
        }
    }
    trunc_input = Deg2Rad(trunc_input);

    last_number_ = KNumber(sin(static_cast<double>(trunc_input)));
}

void CalcEngine::SinRad(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType) {
        last_number_ = KNumber("nan");
        return;
    }

    last_number_ = KNumber(sin(static_cast<double>(input)));
}

void CalcEngine::SinGrad(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType) {
        last_number_ = KNumber("nan");
        return;
    }

    KNumber trunc_input = moveIntoGradInterval(input);
    if (trunc_input.type() == KNumber::IntegerType) {
        KNumber mult = trunc_input / KNumber(100);
        if (mult.type() == KNumber::IntegerType) {
            if (mult == KNumber::Zero)
                last_number_ = 0;
            else if (mult == KNumber(1))
                last_number_ = 1;
            else if (mult == KNumber(2))
                last_number_ = 0;
            else if (mult == KNumber(3))
                last_number_ = -1;
            else kDebug() << "Something wrong in CalcEngine::SinGrad";
            return;
        }
    }

    trunc_input = Gra2Rad(trunc_input);

    last_number_ = KNumber(sin(static_cast<double>(trunc_input)));
}

void CalcEngine::SinHyp(const KNumber &input)
{
    if (input.type() == KNumber::SpecialType) {
        if (input == KNumber("nan")) last_number_ = KNumber("nan");
        if (input == KNumber("inf")) last_number_ = KNumber("inf");
        if (input == KNumber("-inf"))  last_number_ = KNumber("-inf");
        return;
    }

    last_number_ = KNumber(sinh(static_cast<double>(input)));
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
    if (input.type() == KNumber::SpecialType) {
        last_number_ = KNumber("nan");
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
    if (input.type() == KNumber::SpecialType) {
        last_number_ = KNumber("nan");
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
    if (input.type() == KNumber::SpecialType) {
        last_number_ = KNumber("nan");
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
    if (input.type() == KNumber::SpecialType) {
        if (input == KNumber("nan")) last_number_ = KNumber("nan");
        if (input == KNumber("inf")) last_number_ = KNumber::One;
        if (input == KNumber("-inf"))  last_number_ = KNumber::MinusOne;
        return;
    }

    last_number_ = KNumber(tanh(static_cast<double>(input)));
}

KNumber CalcEngine::evalOperation(const KNumber &arg1, Operation operation,
                                  const KNumber &arg2)
{
    if (!percent_mode_ || Operator[operation].prcnt_ptr == NULL) {
        return (Operator[operation].arith_ptr)(arg1, arg2);
    } else {
        percent_mode_ = false;
        return (Operator[operation].prcnt_ptr)(arg1, arg2);
    }
}

void CalcEngine::enterOperation(KNumber number, Operation func)
{
    Node tmp_node;

    if (func == FUNC_BRACKET) {
        tmp_node.number = 0;
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
            KNumber tmp_result =
                evalOperation(tmp_node2.number, tmp_node2.operation,
                              tmp_node.number);
            tmp_node.number = tmp_result;
        } else {
            stack_.push(tmp_node2);
            break;
        }

    }

    if (tmp_node.operation != FUNC_EQUAL  &&  tmp_node.operation != FUNC_PERCENT)
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


