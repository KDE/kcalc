/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "kcalc_stats.h"
#include "knumber.h"

class CalcEngineP
{
public:
    static KNumber arcCosDeg(const KNumber &input);
    static KNumber arcCosRad(const KNumber &input);
    static KNumber arcCosGrad(const KNumber &input);
    static KNumber arcSinDeg(const KNumber &input);
    static KNumber arcSinRad(const KNumber &input);
    static KNumber arcSinGrad(const KNumber &input);
    static KNumber arcTangensDeg(const KNumber &input);
    static KNumber arcTangensRad(const KNumber &input);
    static KNumber arcTangensGrad(const KNumber &input);
    static KNumber areaCosHyp(const KNumber &input);
    static KNumber areaSinHyp(const KNumber &input);
    static KNumber areaTangensHyp(const KNumber &input);
    static KNumber complement(const KNumber &input);
    static KNumber cube(const KNumber &input);
    static KNumber cubeRoot(const KNumber &input);
    static KNumber exp(const KNumber &input);
    static KNumber exp10(const KNumber &input);
    static KNumber factorial(const KNumber &input);
    static KNumber gamma(const KNumber &input);
    static KNumber invertSign(const KNumber &input);
    static KNumber ln(const KNumber &input);
    static KNumber log10(const KNumber &input);
    static KNumber reciprocal(const KNumber &input);
    static KNumber sinDeg(const KNumber &input);
    static KNumber sinGrad(const KNumber &input);
    static KNumber sinRad(const KNumber &input);
    static KNumber sinHyp(const KNumber &input);
    static KNumber cosDeg(const KNumber &input);
    static KNumber cosRad(const KNumber &input);
    static KNumber cosGrad(const KNumber &input);
    static KNumber cosHyp(const KNumber &input);
    static KNumber tangensDeg(const KNumber &input);
    static KNumber tangensRad(const KNumber &input);
    static KNumber tangensGrad(const KNumber &input);
    static KNumber polarDeg(const KNumber &input);
    static KNumber polarRad(const KNumber &input);
    static KNumber polarGrad(const KNumber &input);
    static KNumber conjugate(const KNumber &input);
    static KNumber argumentDeg(const KNumber &input);
    static KNumber argumentGrad(const KNumber &input);
    static KNumber argumentRad(const KNumber &input);
    static KNumber real(const KNumber &input);
    static KNumber imaginary(const KNumber &input);
    static KNumber tangensHyp(const KNumber &input);
    static KNumber square(const KNumber &input);
    static KNumber squareRoot(const KNumber &input);
    static KNumber percentage(const KNumber &input);

    static KNumber execOr(const KNumber &leftOp, const KNumber &rightOp);
    static KNumber execXor(const KNumber &leftOp, const KNumber &rightOp);
    static KNumber execAnd(const KNumber &leftOp, const KNumber &rightOp);
    static KNumber execLsh(const KNumber &leftOp, const KNumber &rightOp);
    static KNumber execRsh(const KNumber &leftOp, const KNumber &rightOp);
    static KNumber execAdd(const KNumber &leftOp, const KNumber &rightOp);
    static KNumber execSubtract(const KNumber &leftOp, const KNumber &rightOp);
    static KNumber execMultiply(const KNumber &leftOp, const KNumber &rightOp);
    static KNumber execDivide(const KNumber &leftOp, const KNumber &rightOp);
    static KNumber execAddP(const KNumber &leftOp, const KNumber &rightOp);
    static KNumber execSubP(const KNumber &leftOp, const KNumber &rightOp);
    static KNumber execMultiplyP(const KNumber &leftOp, const KNumber &rightOp);
    static KNumber execDivideP(const KNumber &leftOp, const KNumber &rightOp);
    static KNumber execMod(const KNumber &leftOp, const KNumber &rightOp);
    static KNumber execIntDiv(const KNumber &leftOp, const KNumber &rightOp);
    static KNumber execBinom(const KNumber &leftOp, const KNumber &rightOp);
    static KNumber execPower(const KNumber &leftOp, const KNumber &rightOp);
    static KNumber execPwrRoot(const KNumber &leftOp, const KNumber &rightOp);

    static KNumber deg2Rad(const KNumber &x);
    static KNumber gra2Rad(const KNumber &x);
    static KNumber rad2Deg(const KNumber &x);
    static KNumber rad2Gra(const KNumber &x);
    static KNumber moveIntoDegInterval(const KNumber &num);
    static KNumber moveIntoGradInterval(const KNumber &num);

    static void statClearAll(const KNumber &input);
    static KNumber statCount(const KNumber &input);
    static void statDataNew(const KNumber &input);
    static void statDataDel(const KNumber &input);
    static KNumber statMean(const KNumber &input);
    static KNumber statMedian(const KNumber &input);
    static KNumber statStdDeviation(const KNumber &input);
    static KNumber statStdSample(const KNumber &input);
    static KNumber statSum(const KNumber &input);
    static KNumber statSumSquares(const KNumber &input);

private:
    static inline KStats m_stats;
};
