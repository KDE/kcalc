/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_core_p.h"
#include "knumber.h"
#include "stats.h"

KNumber CalcEngine_p::Deg2Rad(const KNumber &x)
{
    return x * (KNumber::Pi() / KNumber(180));
}

KNumber CalcEngine_p::Gra2Rad(const KNumber &x)
{
    return x * (KNumber::Pi() / KNumber(200));
}

KNumber CalcEngine_p::Rad2Deg(const KNumber &x)
{
    return x * (KNumber(180) / KNumber::Pi());
}

KNumber CalcEngine_p::Rad2Gra(const KNumber &x)
{
    return x * (KNumber(200) / KNumber::Pi());
}

KNumber CalcEngine_p::ExecOr(const KNumber &left_op, const KNumber &right_op)
{
    return left_op | right_op;
}

KNumber CalcEngine_p::ExecXor(const KNumber &left_op, const KNumber &right_op)
{
    return left_op ^ right_op;
}

KNumber CalcEngine_p::ExecAnd(const KNumber &left_op, const KNumber &right_op)
{
    return left_op & right_op;
}

KNumber CalcEngine_p::ExecLsh(const KNumber &left_op, const KNumber &right_op)
{
    return left_op << right_op;
}

KNumber CalcEngine_p::ExecRsh(const KNumber &left_op, const KNumber &right_op)
{
    return left_op >> right_op;
}

KNumber CalcEngine_p::ExecAdd(const KNumber &left_op, const KNumber &right_op)
{
    return left_op + right_op;
}

KNumber CalcEngine_p::ExecSubtract(const KNumber &left_op, const KNumber &right_op)
{
    return left_op - right_op;
}

KNumber CalcEngine_p::ExecMultiply(const KNumber &left_op, const KNumber &right_op)
{
    return left_op * right_op;
}

KNumber CalcEngine_p::ExecDivide(const KNumber &left_op, const KNumber &right_op)
{
    return left_op / right_op;
}

KNumber CalcEngine_p::ExecMod(const KNumber &left_op, const KNumber &right_op)
{
    return left_op % right_op;
}

KNumber CalcEngine_p::ExecIntDiv(const KNumber &left_op, const KNumber &right_op)
{
    return (left_op / right_op).integerPart();
}

KNumber CalcEngine_p::ExecBinom(const KNumber &left_op, const KNumber &right_op)
{
    return left_op.bin(right_op);
}

KNumber CalcEngine_p::ExecPower(const KNumber &left_op, const KNumber &right_op)
{
    return left_op.pow(right_op);
}

KNumber CalcEngine_p::ExecPwrRoot(const KNumber &left_op, const KNumber &right_op)
{
    return left_op.pow(KNumber::One / right_op);
}

KNumber CalcEngine_p::ExecAddP(const KNumber &left_op, const KNumber &right_op)
{
    return left_op * (KNumber::One + right_op / KNumber(100));
}

KNumber CalcEngine_p::ExecSubP(const KNumber &left_op, const KNumber &right_op)
{
    return left_op * (KNumber::One - right_op / KNumber(100));
}

KNumber CalcEngine_p::ExecMultiplyP(const KNumber &left_op, const KNumber &right_op)
{
    return left_op * right_op / KNumber(100);
}

KNumber CalcEngine_p::ExecDivideP(const KNumber &left_op, const KNumber &right_op)
{
    return left_op * KNumber(100) / right_op;
}

// move a number into the interval [0,360) by adding multiples of 360
KNumber CalcEngine_p::moveIntoDegInterval(const KNumber &num)
{
    KNumber tmp_num = num - (num / KNumber(360)).integerPart() * KNumber(360);
    if (tmp_num < KNumber::Zero)
        return tmp_num + KNumber(360);
    return tmp_num;
}

// move a number into the interval [0,400) by adding multiples of 400
KNumber CalcEngine_p::moveIntoGradInterval(const KNumber &num)
{
    KNumber tmp_num = num - (num / KNumber(400)).integerPart() * KNumber(400);
    if (tmp_num < KNumber::Zero)
        return tmp_num + KNumber(400);
    return tmp_num;
}

KNumber CalcEngine_p::ArcCosDeg(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR || input < -KNumber::One || input > KNumber::One) {
        return KNumber::NaN;
    }

    if (input.type() == KNumber::TYPE_INTEGER) {
        if (input == KNumber::One) {
            return KNumber::Zero;
        }
        if (input == -KNumber::One) {
            return KNumber(180);
        }
        if (input == KNumber::Zero) {
            return KNumber(90);
        }
    }
    return Rad2Deg(input.acos());
}

KNumber CalcEngine_p::ArcCosRad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR || input < -KNumber::One || input > KNumber::One) {
        return KNumber::NaN;
    }
    return input.acos();
}

KNumber CalcEngine_p::ArcCosGrad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR || input < -KNumber::One || input > KNumber::One) {
        return KNumber::NaN;
    }
    if (input.type() == KNumber::TYPE_INTEGER) {
        if (input == KNumber::One) {
            return KNumber::Zero;
        }
        if (input == -KNumber::One) {
            return KNumber(200);
        }
        if (input == KNumber::Zero) {
            return KNumber(100);
        }
    }
    return Rad2Gra(input.acos());
}

KNumber CalcEngine_p::ArcSinDeg(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR || input < -KNumber::One || input > KNumber::One) {
        return KNumber::NaN;
    }
    if (input.type() == KNumber::TYPE_INTEGER) {
        if (input == KNumber::One) {
            return KNumber(90);
        }
        if (input == -KNumber::One) {
            return KNumber(-90);
        }
        if (input == KNumber::Zero) {
            return KNumber::Zero;
        }
    }
    return Rad2Deg(input.asin());
}

KNumber CalcEngine_p::ArcSinRad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR || input < -KNumber::One || input > KNumber::One) {
        return KNumber::NaN;
    }
    return input.asin();
}

KNumber CalcEngine_p::ArcSinGrad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR || input < -KNumber::One || input > KNumber::One) {
        return KNumber::NaN;
    }
    if (input.type() == KNumber::TYPE_INTEGER) {
        if (input == KNumber::One) {
            return KNumber(100);
        }
        if (input == -KNumber::One) {
            return KNumber(-100);
        }
        if (input == KNumber::Zero) {
            return KNumber::Zero;
        }
    }
    return Rad2Gra(input.asin());
}

KNumber CalcEngine_p::ArcTangensDeg(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)
            return KNumber::NaN;
        if (input == KNumber::PosInfinity)
            return KNumber(90);
        if (input == KNumber::NegInfinity)
            return KNumber(-90);
    }

    return Rad2Deg(input.atan());
}

KNumber CalcEngine_p::ArcTangensRad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)
            return KNumber::NaN;
        if (input == KNumber::PosInfinity)
            return KNumber::Pi() / KNumber(2);
        if (input == KNumber::NegInfinity)
            return -KNumber::Pi() / KNumber(2);
    }

    return input.atan();
}

KNumber CalcEngine_p::ArcTangensGrad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)
            return KNumber::NaN;
        if (input == KNumber::PosInfinity)
            return KNumber(100);
        if (input == KNumber::NegInfinity)
            return KNumber(-100);
    }

    return Rad2Gra(input.atan());
}

KNumber CalcEngine_p::AreaCosHyp(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)
            return KNumber::NaN;
        if (input == KNumber::PosInfinity)
            return KNumber::PosInfinity;
        if (input == KNumber::NegInfinity)
            return KNumber::NaN;
    }

    if (input < KNumber::One) {
        return KNumber::NaN;
    }
    if (input == KNumber::One) {
        return KNumber::Zero;
    }
    return input.acosh();
}

KNumber CalcEngine_p::AreaSinHyp(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)
            return KNumber::NaN;
        if (input == KNumber::PosInfinity)
            return KNumber::PosInfinity;
        if (input == KNumber::NegInfinity)
            return KNumber::NegInfinity;
    }

    if (input == KNumber::Zero) {
        return KNumber::Zero;
    }
    return input.asinh();
}

KNumber CalcEngine_p::AreaTangensHyp(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        return KNumber::NaN;
    }

    if (input < -KNumber::One || input > KNumber::One) {
        return KNumber::NaN;
    }
    if (input == KNumber::One) {
        return KNumber::PosInfinity;
    }
    if (input == -KNumber::One) {
        return KNumber::NegInfinity;
    }
    return input.atanh();
}

KNumber CalcEngine_p::Complement(const KNumber &input)
{
    if (input.type() != KNumber::TYPE_INTEGER) {
        return KNumber::NaN;
    }

    return ~input;
}

KNumber CalcEngine_p::CosDeg(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        return KNumber::NaN;
    }

    KNumber trunc_input = moveIntoDegInterval(input);

    if (trunc_input.type() == KNumber::TYPE_INTEGER) {
        KNumber mult = trunc_input / KNumber(90);
        if (mult.type() == KNumber::TYPE_INTEGER) {
            if (mult == KNumber::Zero)
                return KNumber::One;
            else if (mult == KNumber::One)
                return KNumber::Zero;
            else if (mult == KNumber(2))
                return KNumber::NegOne;
            else if (mult == KNumber(3))
                return KNumber::Zero;
            else
                return KNumber::NaN;
        }
    }

    trunc_input = Deg2Rad(trunc_input);
    return trunc_input.cos();
}

KNumber CalcEngine_p::CosRad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        return KNumber::NaN;
    }

    return input.cos();
}

KNumber CalcEngine_p::CosGrad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        return KNumber::NaN;
    }
    KNumber trunc_input = moveIntoGradInterval(input);
    if (trunc_input.type() == KNumber::TYPE_INTEGER) {
        KNumber mult = trunc_input / KNumber(100);
        if (mult.type() == KNumber::TYPE_INTEGER) {
            if (mult == KNumber::Zero)
                return KNumber::One;
            else if (mult == KNumber::One)
                return KNumber::Zero;
            else if (mult == KNumber(2))
                return KNumber::NegOne;
            else if (mult == KNumber(3))
                return KNumber::Zero;
            else
                return KNumber::NaN;
        }
    }
    trunc_input = Gra2Rad(trunc_input);

    return trunc_input.cos();
}

KNumber CalcEngine_p::CosHyp(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)
            return KNumber::NaN;
        if (input == KNumber::PosInfinity)
            return KNumber::PosInfinity;
        if (input == KNumber::NegInfinity)
            return KNumber::PosInfinity;
        return KNumber::NaN;
    }

    return input.cosh();
}

KNumber CalcEngine_p::Cube(const KNumber &input)
{
    return input * input * input;
}

KNumber CalcEngine_p::CubeRoot(const KNumber &input)
{
    return input.cbrt();
}

KNumber CalcEngine_p::Exp(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)
            return KNumber::NaN;
        if (input == KNumber::PosInfinity)
            return KNumber::PosInfinity;
        if (input == KNumber::NegInfinity)
            return KNumber::Zero;
        return KNumber::NaN;
    }
    return KNumber::Euler().pow(input);
}

KNumber CalcEngine_p::Exp10(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)
            return KNumber::NaN;
        if (input == KNumber::PosInfinity)
            return KNumber::PosInfinity;
        if (input == KNumber::NegInfinity)
            return KNumber::Zero;
        return KNumber::NaN;
    }
    return KNumber(10).pow(input);
}

KNumber CalcEngine_p::Factorial(const KNumber &input)
{
    if (input == KNumber::PosInfinity)
        return KNumber::PosInfinity;
    if (input < KNumber::Zero || input.type() == KNumber::TYPE_ERROR) {
        return KNumber::NaN;
    }

    return input.integerPart().factorial();
}

KNumber CalcEngine_p::Gamma(const KNumber &input)
{
    if (input == KNumber::PosInfinity)
        return KNumber::PosInfinity;
    if (input < KNumber::Zero || input.type() == KNumber::TYPE_ERROR) {
        return KNumber::NaN;
    }

    return input.tgamma();
}

KNumber CalcEngine_p::InvertSign(const KNumber &input)
{
    return -input;
}

KNumber CalcEngine_p::Ln(const KNumber &input)
{
    if (input < KNumber::Zero)
        return KNumber::NaN;
    else if (input == KNumber::Zero)
        return KNumber::NegInfinity;
    else if (input == KNumber::One)
        return KNumber::Zero;
    else {
        return input.ln();
    }
}

KNumber CalcEngine_p::Log10(const KNumber &input)
{
    if (input < KNumber::Zero)
        return KNumber::NaN;
    else if (input == KNumber::Zero)
        return KNumber::NegInfinity;
    else if (input == KNumber::One)
        return KNumber::Zero;
    else {
        return input.log10();
    }
}

KNumber CalcEngine_p::Reciprocal(const KNumber &input)
{
    return KNumber::One / input;
}

KNumber CalcEngine_p::SinDeg(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        return KNumber::NaN;
    }

    KNumber trunc_input = moveIntoDegInterval(input);
    if (trunc_input.type() == KNumber::TYPE_INTEGER) {
        KNumber mult = trunc_input / KNumber(90);
        if (mult.type() == KNumber::TYPE_INTEGER) {
            if (mult == KNumber::Zero)
                return KNumber::Zero;
            else if (mult == KNumber::One)
                return KNumber::One;
            else if (mult == KNumber(2))
                return KNumber::Zero;
            else if (mult == KNumber(3))
                return KNumber::NegOne;
            else
                return KNumber::NaN;
        }
    }
    trunc_input = Deg2Rad(trunc_input);

    return trunc_input.sin();
}

KNumber CalcEngine_p::SinRad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        return KNumber::NaN;
    }

    return input.sin();
}

KNumber CalcEngine_p::SinGrad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        return KNumber::NaN;
    }

    KNumber trunc_input = moveIntoGradInterval(input);
    if (trunc_input.type() == KNumber::TYPE_INTEGER) {
        KNumber mult = trunc_input / KNumber(100);
        if (mult.type() == KNumber::TYPE_INTEGER) {
            if (mult == KNumber::Zero)
                return KNumber::Zero;
            else if (mult == KNumber::One)
                return KNumber::One;
            else if (mult == KNumber(2))
                return KNumber::Zero;
            else if (mult == KNumber(3))
                return KNumber::NegOne;
            else
                return KNumber::NaN;
        }
    }

    trunc_input = Gra2Rad(trunc_input);

    return trunc_input.sin();
}

KNumber CalcEngine_p::SinHyp(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)
            return KNumber::NaN;
        if (input == KNumber::PosInfinity)
            return KNumber::PosInfinity;
        if (input == KNumber::NegInfinity)
            return KNumber::NegInfinity;
        return KNumber::NaN;
    }

    return input.sinh();
}

KNumber CalcEngine_p::Square(const KNumber &input)
{
    return input * input;
}

KNumber CalcEngine_p::SquareRoot(const KNumber &input)
{
    return input.sqrt();
}

KNumber CalcEngine_p::Percentage(const KNumber &input)
{
    return ExecDivide(input, KNumber::OneHundred);
}

void CalcEngine_p::StatClearAll(const KNumber &input)
{
    Q_UNUSED(input);
    stats.clearAll();
}

KNumber CalcEngine_p::StatCount(const KNumber &input)
{
    Q_UNUSED(input);
    return KNumber(stats.count());
}

void CalcEngine_p::StatDataNew(const KNumber &input)
{
    stats.enterData(input);
}

void CalcEngine_p::StatDataDel(const KNumber &input)
{
    Q_UNUSED(input);
    stats.clearLast();
}

KNumber CalcEngine_p::StatMean(const KNumber &input)
{
    Q_UNUSED(input);
    return stats.mean();
}

KNumber CalcEngine_p::StatMedian(const KNumber &input)
{
    Q_UNUSED(input);
    return stats.median();
}

KNumber CalcEngine_p::StatStdDeviation(const KNumber &input)
{
    Q_UNUSED(input);
    return stats.std();
}

KNumber CalcEngine_p::StatStdSample(const KNumber &input)
{
    Q_UNUSED(input);
    return stats.sample_std();
}

KNumber CalcEngine_p::StatSum(const KNumber &input)
{
    Q_UNUSED(input);
    return stats.sum();
}

KNumber CalcEngine_p::StatSumSquares(const KNumber &input)
{
    Q_UNUSED(input);
    return stats.sum_of_squares();
}

KNumber CalcEngine_p::TangensDeg(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        return KNumber::NaN;
    }

    KNumber arg1 = SinDeg(input);
    KNumber arg2 = CosDeg(input);

    return arg1 / arg2;
}

KNumber CalcEngine_p::TangensRad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        return KNumber::NaN;
    }

    KNumber arg1 = SinRad(input);
    KNumber arg2 = CosRad(input);

    return arg1 / arg2;
}

KNumber CalcEngine_p::TangensGrad(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        return KNumber::NaN;
    }

    KNumber arg1 = SinGrad(input);
    KNumber arg2 = CosGrad(input);

    return arg1 / arg2;
}

KNumber CalcEngine_p::TangensHyp(const KNumber &input)
{
    if (input.type() == KNumber::TYPE_ERROR) {
        if (input == KNumber::NaN)
            return KNumber::NaN;
        if (input == KNumber::PosInfinity)
            return KNumber::One;
        if (input == KNumber::NegInfinity)
            return KNumber::NegOne;
        return KNumber::NaN;
    }

    return input.tanh();
}
