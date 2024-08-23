/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "kcalc_stats.h"
#include "knumber.h"

class CalcEngine_p
{
public:
    static KNumber ArcCosDeg(const KNumber &input);
    static KNumber ArcCosRad(const KNumber &input);
    static KNumber ArcCosGrad(const KNumber &input);
    static KNumber ArcSinDeg(const KNumber &input);
    static KNumber ArcSinRad(const KNumber &input);
    static KNumber ArcSinGrad(const KNumber &input);
    static KNumber ArcTangensDeg(const KNumber &input);
    static KNumber ArcTangensRad(const KNumber &input);
    static KNumber ArcTangensGrad(const KNumber &input);
    static KNumber AreaCosHyp(const KNumber &input);
    static KNumber AreaSinHyp(const KNumber &input);
    static KNumber AreaTangensHyp(const KNumber &input);
    static KNumber Complement(const KNumber &input);
    static KNumber Cube(const KNumber &input);
    static KNumber CubeRoot(const KNumber &input);
    static KNumber Exp(const KNumber &input);
    static KNumber Exp10(const KNumber &input);
    static KNumber Factorial(const KNumber &input);
    static KNumber Gamma(const KNumber &input);
    static KNumber InvertSign(const KNumber &input);
    static KNumber Ln(const KNumber &input);
    static KNumber Log10(const KNumber &input);
    static KNumber Reciprocal(const KNumber &input);
    static KNumber SinDeg(const KNumber &input);
    static KNumber SinGrad(const KNumber &input);
    static KNumber SinRad(const KNumber &input);
    static KNumber SinHyp(const KNumber &input);
    static KNumber CosDeg(const KNumber &input);
    static KNumber CosRad(const KNumber &input);
    static KNumber CosGrad(const KNumber &input);
    static KNumber CosHyp(const KNumber &input);
    static KNumber TangensDeg(const KNumber &input);
    static KNumber TangensRad(const KNumber &input);
    static KNumber TangensGrad(const KNumber &input);
    static KNumber PolarDeg(const KNumber &input);
    static KNumber PolarRad(const KNumber &input);
    static KNumber PolarGrad(const KNumber &input);
    static KNumber TangensHyp(const KNumber &input);
    static KNumber Square(const KNumber &input);
    static KNumber SquareRoot(const KNumber &input);
    static KNumber Percentage(const KNumber &input);

    static KNumber ExecOr(const KNumber &left_op, const KNumber &right_op);
    static KNumber ExecXor(const KNumber &left_op, const KNumber &right_op);
    static KNumber ExecAnd(const KNumber &left_op, const KNumber &right_op);
    static KNumber ExecLsh(const KNumber &left_op, const KNumber &right_op);
    static KNumber ExecRsh(const KNumber &left_op, const KNumber &right_op);
    static KNumber ExecAdd(const KNumber &left_op, const KNumber &right_op);
    static KNumber ExecSubtract(const KNumber &left_op, const KNumber &right_op);
    static KNumber ExecMultiply(const KNumber &left_op, const KNumber &right_op);
    static KNumber ExecDivide(const KNumber &left_op, const KNumber &right_op);
    static KNumber ExecAddP(const KNumber &left_op, const KNumber &right_op);
    static KNumber ExecSubP(const KNumber &left_op, const KNumber &right_op);
    static KNumber ExecMultiplyP(const KNumber &left_op, const KNumber &right_op);
    static KNumber ExecDivideP(const KNumber &left_op, const KNumber &right_op);
    static KNumber ExecMod(const KNumber &left_op, const KNumber &right_op);
    static KNumber ExecIntDiv(const KNumber &left_op, const KNumber &right_op);
    static KNumber ExecBinom(const KNumber &left_op, const KNumber &right_op);
    static KNumber ExecPower(const KNumber &left_op, const KNumber &right_op);
    static KNumber ExecPwrRoot(const KNumber &left_op, const KNumber &right_op);

    static KNumber Deg2Rad(const KNumber &x);
    static KNumber Gra2Rad(const KNumber &x);
    static KNumber Rad2Deg(const KNumber &x);
    static KNumber Rad2Gra(const KNumber &x);
    static KNumber moveIntoDegInterval(const KNumber &num);
    static KNumber moveIntoGradInterval(const KNumber &num);

    static void StatClearAll(const KNumber &input);
    static KNumber StatCount(const KNumber &input);
    static void StatDataNew(const KNumber &input);
    static void StatDataDel(const KNumber &input);
    static KNumber StatMean(const KNumber &input);
    static KNumber StatMedian(const KNumber &input);
    static KNumber StatStdDeviation(const KNumber &input);
    static KNumber StatStdSample(const KNumber &input);
    static KNumber StatSum(const KNumber &input);
    static KNumber StatSumSquares(const KNumber &input);

private:
    static inline KStats stats;
};
