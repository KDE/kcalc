/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 1996-2000 Bernd Johannes Wuebben <wuebben@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "kcalc_token.h"
#include "knumber.h"
#include "stats.h"

#include <QQueue>

class CalcEngine
{
public:
    CalcEngine();

    enum CALCULATION_RESULT { SUCCESS, EMPTY_INPUT, MATH_ERROR, SYNTAX_ERROR };

    int calculate(const QQueue<KCalcToken> tokenBuffer, int &errorIndex); // returs 0 on success
    KNumber getResult();

    KNumber lastOutput(bool &error) const;

public:
    void StatClearAll();
    void StatCount(const KNumber &input);
    void StatDataNew(const KNumber &input);
    void StatDataDel();
    void StatMean(const KNumber &input);
    void StatMedian(const KNumber &input);
    void StatStdDeviation(const KNumber &input);
    void StatStdSample(const KNumber &input);
    void StatSum(const KNumber &input);
    void StatSumSquares(const KNumber &input);

public:
    void Reset();

private:
    KStats stats;

    QList<KCalcToken> token_stack_;

    const KCalcToken multiplication_Token_ = KCalcToken(KCalcToken::TokenCode::MULTIPLICATION);

    int reduce_Stack_(bool toParentheses = true);
    int insert_Binary_Function_Token_In_Stack_(const KCalcToken &token);
    int insert_KNumber_Token_In_Stack_(const KCalcToken &token);

    void printStacks_();
    bool error_;

    KNumber buffer_result_;
};

