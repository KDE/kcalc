/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 1996-2000 Bernd Johannes Wuebben <wuebben@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "kcalc_stats.h"
#include "kcalc_token.h"
#include "knumber.h"

#include <QQueue>

class CalcEngine
{
public:
    CalcEngine();

    enum ResultCode {
        Success,
        EmptyInput,
        MathError,
        MissingLeftUnaryArg,
        MissingRightUnaryArg,
        MissingRightBinaryArg,
        IncompleteInput,
        SyntaxError
    };

    ResultCode calculate(const QQueue<KCalcToken> &tokenBuffer, int &errorIndex); // returs 0 on success
    KNumber getResult();

    KNumber lastOutput(bool &error) const;

public:
    void statClearAll();
    void statCount(const KNumber &input);
    void statDataNew(const KNumber &input);
    void statDataDel();
    void statMean(const KNumber &input);
    void statMedian(const KNumber &input);
    void statStdDeviation(const KNumber &input);
    void statStdSample(const KNumber &input);
    void statSum(const KNumber &input);
    void statSumSquares(const KNumber &input);

public:
    void reset();

private:
    KStats stats;

    QList<KCalcToken> m_tokenStack;

    const KCalcToken m_multiplicationToken = KCalcToken(KCalcToken::TokenCode::Multiplication);

    int reduceStack(bool toParentheses = true);
    int insertBinaryFunctionTokenInStack(const KCalcToken &token);
    int insertKNumberTokenInStack(const KCalcToken &token);
    int insertPercentageTokenInStack();

    void printStacks();
    bool m_errorGlobal;

    KNumber m_bufferResult;
};
