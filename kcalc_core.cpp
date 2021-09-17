/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2003-2005 Klaus Niederkrueger <kniederk@math.uni-koeln.de>
    SPDX-FileCopyrightText: 1996-2000 Bernd Johannes Wuebben <wuebben@kde.org>
    SPDX-FileCopyrightText: 1995 Martin Bartlett

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_core.h"
#include "kcalc_settings.h"
#include "kcalchistory.h"

#include <QDebug>

namespace
{
KNumber Deg2Rad(const KNumber &x)
{
    return x * (KNumber::Pi() / KNumber(180));
}

KNumber Gra2Rad(const KNumber &x)
{
    return x * (KNumber::Pi() / KNumber(200));
}

KNumber Rad2Deg(const KNumber &x)
{
    return x * (KNumber(180) / KNumber::Pi());
}

KNumber Rad2Gra(const KNumber &x)
{
    return x * (KNumber(200) / KNumber::Pi());
}

bool error_;

KNumber ExecOr(const KNumber &left_op, const KNumber &right_op)
{
    return left_op | right_op;
}

KNumber ExecXor(const KNumber &left_op, const KNumber &right_op)
{
    return left_op ^ right_op;
}

KNumber ExecAnd(const KNumber &left_op, const KNumber &right_op)
{
    return left_op & right_op;
}

KNumber ExecLsh(const KNumber &left_op, const KNumber &right_op)
{
    return left_op << right_op;
}

KNumber ExecRsh(const KNumber &left_op, const KNumber &right_op)
{
    return left_op >> right_op;
}

KNumber ExecAdd(const KNumber &left_op, const KNumber &right_op)
{
    return left_op + right_op;
}

KNumber ExecSubtract(const KNumber &left_op, const KNumber &right_op)
{
    return left_op - right_op;
}

KNumber ExecMultiply(const KNumber &left_op, const KNumber &right_op)
{
    return left_op * right_op;
}

KNumber ExecDivide(const KNumber &left_op, const KNumber &right_op)
{
    return left_op / right_op;
}

KNumber ExecMod(const KNumber &left_op, const KNumber &right_op)
{
    return left_op % right_op;
}

KNumber ExecIntDiv(const KNumber &left_op, const KNumber &right_op)
{
    return (left_op / right_op).integerPart();
}

KNumber ExecBinom(const KNumber &left_op, const KNumber &right_op)
{
    return left_op.bin(right_op);
}

KNumber ExecPower(const KNumber &left_op, const KNumber &right_op)
{
    return left_op.pow(right_op);
}

KNumber ExecPwrRoot(const KNumber &left_op, const KNumber &right_op)
{
    return left_op.pow(KNumber::One / right_op);
}

KNumber ExecAddP(const KNumber &left_op, const KNumber &right_op)
{
    return left_op * (KNumber::One + right_op / KNumber(100));
}

KNumber ExecSubP(const KNumber &left_op, const KNumber &right_op)
{
    return left_op * (KNumber::One - right_op / KNumber(100));
}

KNumber ExecMultiplyP(const KNumber &left_op, const KNumber &right_op)
{
    return left_op * right_op / KNumber(100);
}

KNumber ExecDivideP(const KNumber &left_op, const KNumber &right_op)
{
    return left_op * KNumber(100) / right_op;
}

// move a number into the interval [0,360) by adding multiples of 360
KNumber moveIntoDegInterval(const KNumber &num)
{
    KNumber tmp_num = num - (num / KNumber(360)).integerPart() * KNumber(360);
    if (tmp_num < KNumber::Zero)
        return tmp_num + KNumber(360);
    return tmp_num;
}

// move a number into the interval [0,400) by adding multiples of 400
KNumber moveIntoGradInterval(const KNumber &num)
{
    KNumber tmp_num = num - (num / KNumber(400)).integerPart() * KNumber(400);
    if (tmp_num < KNumber::Zero)
        return tmp_num + KNumber(400);
    return tmp_num;
}

typedef KNumber (*Arith)(const KNumber &, const KNumber &);
typedef KNumber (*Prcnt)(const KNumber &, const KNumber &);

struct operator_data {
    int precedence; // priority of operators in " enum Operation"
    Arith arith_ptr;
    Prcnt prcnt_ptr;
};

// build precedence list
const struct operator_data Operator[] = {
    {0, nullptr, nullptr}, // FUNC_EQUAL
    {0, nullptr, nullptr}, // FUNC_PERCENT
    {0, nullptr, nullptr}, // FUNC_BRACKET
    {1, ExecOr, nullptr}, // FUNC_OR
    {2, ExecXor, nullptr}, // FUNC_XOR
    {3, ExecAnd, nullptr}, // FUNC_AND
    {4, ExecLsh, nullptr}, // FUNC_LSH
    {4, ExecRsh, nullptr}, // FUNC_RSH
    {5, ExecAdd, ExecAddP}, // FUNC_ADD
    {5, ExecSubtract, ExecSubP}, // FUNC_SUBTRACT
    {6, ExecMultiply, ExecMultiplyP}, // FUNC_MULTIPLY
    {6, ExecDivide, ExecDivideP}, // FUNC_DIVIDE
    {6, ExecMod, nullptr}, // FUNC_MOD
    {6, ExecIntDiv, nullptr}, // FUNC_INTDIV
    {7, ExecBinom, nullptr}, // FUNC_BINOM
    {7, ExecPower, nullptr}, // FUNC_POWER
    {7, ExecPwrRoot, nullptr} // FUNC_PWR_ROOT
};

}

CalcEngine::CalcEngine()
    : only_update_operation_(false)
    , percent_mode_(false)
    , repeat_mode_(false)
{
    last_number_ = KNumber::Zero;
    error_ = false;
    last_operation_ = FUNC_EQUAL;
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
        if (input == -KNumber::One) {
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
        if (input == -KNumber::One) {
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
    if (input.type() == KNumber::TYPE_ERROR || input < -KNumber::One || input > KNumber::One) {
        last_number_ = KNumber::NaN;
        return;
    }
    if (input.type() == KNumber::TYPE_INTEGER) {
        if (input == KNumber::One) {
            last_number_ = KNumber(90);
            return;
        }
        if (input == -KNumber::One) {
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
    if (input.type() == KNumber::TYPE_ERROR || input < -KNumber::One || input > KNumber::One) {
        last_number_ = KNumber::NaN;
        return;
    }
    last_number_ = input.asin();
}

void CalcEngine::ArcSinGrad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR || input < -KNumber::One || input > KNumber::One) {
        last_number_ = KNumber::NaN;
        return;
    }
    if (input.type() == KNumber::TYPE_INTEGER) {
        if (input == KNumber::One) {
            last_number_ = KNumber(100);
            return;
        }
        if (input == -KNumber::One) {
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
        if (input == KNumber::NaN)
            last_number_ = KNumber::NaN;
        if (input == KNumber::PosInfinity)
            last_number_ = KNumber(90);
        if (input == KNumber::NegInfinity)
            last_number_ = KNumber(-90);
        return;
    }

    last_number_ = Rad2Deg(input.atan());
}

void CalcEngine::ArcTangensRad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)
            last_number_ = KNumber::NaN;
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
        if (input == KNumber::NaN)
            last_number_ = KNumber::NaN;
        if (input == KNumber::PosInfinity)
            last_number_ = KNumber(100);
        if (input == KNumber::NegInfinity)
            last_number_ = KNumber(-100);
        return;
    }

    last_number_ = Rad2Gra(input.atan());
}

void CalcEngine::AreaCosHyp(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)
            last_number_ = KNumber::NaN;
        if (input == KNumber::PosInfinity)
            last_number_ = KNumber::PosInfinity;
        if (input == KNumber::NegInfinity)
            last_number_ = KNumber::NaN;
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
        if (input == KNumber::NaN)
            last_number_ = KNumber::NaN;
        if (input == KNumber::PosInfinity)
            last_number_ = KNumber::PosInfinity;
        if (input == KNumber::NegInfinity)
            last_number_ = KNumber::NegInfinity;
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
    if (input == -KNumber::One) {
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
            else
                qDebug() << "Something wrong in CalcEngine::CosDeg";
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
            else
                qDebug() << "Something wrong in CalcEngine::CosGrad";
            return;
        }
    }
    trunc_input = Gra2Rad(trunc_input);

    last_number_ = trunc_input.cos();
}

void CalcEngine::CosHyp(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)
            last_number_ = KNumber::NaN;
        if (input == KNumber::PosInfinity)
            last_number_ = KNumber::PosInfinity;
        // YES, this should be *positive* infinity. We mimic the behavior of
        // libc which says the following for cosh
        //
        // "If x is positive infinity or negative infinity, positive infinity is returned."
        if (input == KNumber::NegInfinity)
            last_number_ = KNumber::PosInfinity;
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
        if (input == KNumber::NaN)
            last_number_ = KNumber::NaN;
        if (input == KNumber::PosInfinity)
            last_number_ = KNumber::PosInfinity;
        if (input == KNumber::NegInfinity)
            last_number_ = KNumber::Zero;
        return;
    }
    last_number_ = KNumber::Euler().pow(input);
}

void CalcEngine::Exp10(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)
            last_number_ = KNumber::NaN;
        if (input == KNumber::PosInfinity)
            last_number_ = KNumber::PosInfinity;
        if (input == KNumber::NegInfinity)
            last_number_ = KNumber::Zero;
        return;
    }
    last_number_ = KNumber(10).pow(input);
}

void CalcEngine::Factorial(const KNumber &input)
{
    if (input == KNumber::PosInfinity)
        return;
    if (input < KNumber::Zero || input.type() == KNumber::TYPE_ERROR) {
        error_ = true;
        last_number_ = KNumber::NaN;
        return;
    }

    last_number_ = input.integerPart().factorial();
}

void CalcEngine::Gamma(const KNumber &input)
{
    if (input == KNumber::PosInfinity)
        return;
    if (input < KNumber::Zero || input.type() == KNumber::TYPE_ERROR) {
        error_ = true;
        last_number_ = KNumber::NaN;
        return;
    }

    last_number_ = input.tgamma();
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
            else
                qDebug() << "Something wrong in CalcEngine::SinDeg";
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
            else
                qDebug() << "Something wrong in CalcEngine::SinGrad";
            return;
        }
    }

    trunc_input = Gra2Rad(trunc_input);

    last_number_ = trunc_input.sin();
}

void CalcEngine::SinHyp(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)
            last_number_ = KNumber::NaN;
        if (input == KNumber::PosInfinity)
            last_number_ = KNumber::PosInfinity;
        if (input == KNumber::NegInfinity)
            last_number_ = KNumber::NegInfinity;
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
    last_number_ = KNumber(stats.count());
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
        if (input == KNumber::NaN)
            last_number_ = KNumber::NaN;
        if (input == KNumber::PosInfinity)
            last_number_ = KNumber::One;
        if (input == KNumber::NegInfinity)
            last_number_ = KNumber::NegOne;
        return;
    }

    last_number_ = input.tanh();
}

KNumber CalcEngine::evalOperation(const KNumber &arg1, Operation operation, const KNumber &arg2)
{
    if (!percent_mode_ || Operator[operation].prcnt_ptr == nullptr) {
        return (Operator[operation].arith_ptr)(arg1, arg2);
    } else {
        percent_mode_ = false;
        return (Operator[operation].prcnt_ptr)(arg1, arg2);
    }
}

void CalcEngine::enterOperation(const KNumber &number, Operation func, Repeat allow_repeat)
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

    if (KCalcSettings::repeatLastOperation()) {
        if (func != FUNC_EQUAL && func != FUNC_PERCENT) {
            last_operation_ = tmp_node.operation;
            repeat_mode_ = false;
        }

        if (func == FUNC_EQUAL || func == FUNC_PERCENT) {
            if (!repeat_mode_) {
                repeat_mode_ = last_operation_ != FUNC_EQUAL;
                last_repeat_number_ = number;
            } else if (allow_repeat == REPEAT_ALLOW) {
                Node repeat_node;
                repeat_node.operation = last_operation_;
                repeat_node.number = number;
                tmp_node.number = last_repeat_number_;
                stack_.push(repeat_node);
            }
        }
    }

    if (getOnlyUpdateOperation() && !stack_.isEmpty() && !(func == FUNC_EQUAL || func == FUNC_PERCENT))
        stack_.top().operation = func;
    else
        stack_.push(tmp_node);

    // The check for '=' or '%' is unnecessary; it is just a safety measure
    if (!((func == FUNC_EQUAL) || (func == FUNC_PERCENT)))
        setOnlyUpdateOperation(true);

    evalStack();
}

bool CalcEngine::evalStack()
{
    // this should never happen
    Q_ASSERT(!stack_.isEmpty());

    Node tmp_node = stack_.pop();

    while (!stack_.isEmpty()) {
        Node tmp_node2 = stack_.pop();
        if (Operator[tmp_node.operation].precedence <= Operator[tmp_node2.operation].precedence) {
            if (tmp_node2.operation == FUNC_BRACKET)
                continue;
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
    percent_mode_ = false;
    repeat_mode_ = false;
    last_operation_ = FUNC_EQUAL;
    error_ = false;
    last_number_ = KNumber::Zero;
    only_update_operation_ = false;

    stack_.clear();
}

void CalcEngine::setOnlyUpdateOperation(bool update)
{
    only_update_operation_ = update;
}

bool CalcEngine::getOnlyUpdateOperation() const
{
    return only_update_operation_;
}
