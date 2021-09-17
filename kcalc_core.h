/*
SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>

SPDX-FileCopyrightText: 1996-2000 Bernd Johannes Wuebben <wuebben@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "knumber.h"
#include "stats.h"
#include <QStack>

class CalcEngine
{
public:
    // operations that can be stored in calculation stack
    enum Operation {
        FUNC_EQUAL,
        FUNC_PERCENT,
        FUNC_BRACKET,
        FUNC_OR,
        FUNC_XOR,
        FUNC_AND,
        FUNC_LSH,
        FUNC_RSH,
        FUNC_ADD,
        FUNC_SUBTRACT,
        FUNC_MULTIPLY,
        FUNC_DIVIDE,
        FUNC_MOD,
        FUNC_INTDIV,
        FUNC_BINOM,
        FUNC_POWER,
        FUNC_PWR_ROOT
    };

    enum Repeat { REPEAT_ALLOW, REPEAT_PREVENT };

    CalcEngine();

    KNumber lastOutput(bool &error) const;

    void enterOperation(const KNumber &num, Operation func, Repeat allow_repeat = REPEAT_ALLOW);

    void ArcCosDeg(const KNumber &input);
    void ArcCosRad(const KNumber &input);
    void ArcCosGrad(const KNumber &input);
    void ArcSinDeg(const KNumber &input);
    void ArcSinRad(const KNumber &input);
    void ArcSinGrad(const KNumber &input);
    void ArcTangensDeg(const KNumber &input);
    void ArcTangensRad(const KNumber &input);
    void ArcTangensGrad(const KNumber &input);
    void AreaCosHyp(const KNumber &input);
    void AreaSinHyp(const KNumber &input);
    void AreaTangensHyp(const KNumber &input);
    void Complement(const KNumber &input);
    void CosDeg(const KNumber &input);
    void CosRad(const KNumber &input);
    void CosGrad(const KNumber &input);
    void CosHyp(const KNumber &input);
    void Cube(const KNumber &input);
    void CubeRoot(const KNumber &input);
    void Exp(const KNumber &input);
    void Exp10(const KNumber &input);
    void Factorial(const KNumber &input);
    void Gamma(const KNumber &input);
    void InvertSign(const KNumber &input);
    void Ln(const KNumber &input);
    void Log10(const KNumber &input);
    void ParenClose(KNumber input);
    void ParenOpen(const KNumber &input);
    void Reciprocal(const KNumber &input);
    void SinDeg(const KNumber &input);
    void SinGrad(const KNumber &input);
    void SinRad(const KNumber &input);
    void SinHyp(const KNumber &input);
    void Square(const KNumber &input);
    void SquareRoot(const KNumber &input);
    void StatClearAll(const KNumber &input);
    void StatCount(const KNumber &input);
    void StatDataNew(const KNumber &input);
    void StatDataDel(const KNumber &input);
    void StatMean(const KNumber &input);
    void StatMedian(const KNumber &input);
    void StatStdDeviation(const KNumber &input);
    void StatStdSample(const KNumber &input);
    void StatSum(const KNumber &input);
    void StatSumSquares(const KNumber &input);
    void TangensDeg(const KNumber &input);
    void TangensRad(const KNumber &input);
    void TangensGrad(const KNumber &input);
    void TangensHyp(const KNumber &input);

    void Reset();
    void setOnlyUpdateOperation(bool update);
    bool getOnlyUpdateOperation() const;

private:
    KStats stats;

    struct Node {
        KNumber number;
        Operation operation;
    };

    // Stack holds all operations and numbers that have not yet been
    // processed, e.g. user types "2+3*", the calculation can not be
    // executed, because "*" has a higher precedence than "+", so we
    // need to wait for the next number.
    //
    // In the stack this would be stored as ((2,+),(3,*),...)
    //
    // "enterOperation": If the introduced Operation has lower priority
    // than the preceding operations in the stack, then we can start to
    // evaluate the stack (with "evalStack"). Otherwise we append the new
    // Operation and number to the stack.
    //
    // E.g. "2*3+" evaluates to "6+", but "2+3*" can not be evaluated
    // yet.
    //
    // We also take care of brackets, by writing a marker "FUNC_BRACKET"
    // into the stack, each time the user opens one.  When a bracket is
    // closed, everything in the stack is evaluated until the first
    // marker "FUNC_BRACKET" found.
    QStack<Node> stack_;

    KNumber last_number_;

    Operation last_operation_;
    KNumber last_repeat_number_;
    bool only_update_operation_;

    bool percent_mode_;
    bool repeat_mode_;

    bool evalStack();

    KNumber evalOperation(const KNumber &arg1, Operation operation, const KNumber &arg2);
};

