/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "kcalc_core_p.h"
#include "kcalc_priority_levels_p.h"
#include "knumber.h"
#include <QDebug>

class KCalcToken
{
public:
    KCalcToken();
    ~KCalcToken();

    enum TokenCode {
        Knumber,
        DecimalPoint,
        Plus,
        Minus,
        InvertSign,
        Division,
        Slash,
        Multiplication,
        Dot,
        Asterisk,
        Percentage,
        Permille,
        OpeningParenthesis,
        ClosingParenthesis,
        Square,
        Cube,
        SquareRoot,
        CubicRoot,
        Degree,
        Gradian,
        Radian,
        Sin,
        SinRad,
        SinDeg,
        SinGrad,
        Cos,
        CosRad,
        CosDeg,
        CosGrad,
        Tan,
        TanRad,
        TanDeg,
        TanGrad,
        Asin,
        AsinRad,
        AsinDeg,
        AsinGrad,
        Acos,
        AcosRad,
        AcosDeg,
        AcosGrad,
        Atan,
        AtanRad,
        AtanDeg,
        AtanGrad,
        PolarRad,
        PolarDeg,
        PolarGrad,
        ArgRad,
        ArgDeg,
        ArgGrad,
        Sinh,
        Cosh,
        Tanh,
        Asinh,
        Acosh,
        Atanh,
        E,
        Pi,
        Phi,
        I,
        Polar,
        Re,
        Im,
        Conj,
        PosInfinity,
        NegInfinity,
        VacuumPermitivity,
        VacuumPermeability,
        VacuumImpedance,
        PlanckSConstant,
        PlanckOver2Pi,
        Exp,
        Exp10,
        Power,
        PowerRoot,
        Factorial,
        DoubleFactorial,
        Gamma,
        Ln,
        Log10,
        Reciprocal,
        Binomial,
        Modulo,
        IntegerDivision,
        And,
        Or,
        Xor,
        OnesComplement,
        TwosComplement,
        Rsh,
        Lsh,
        Equal,
        Ans,
        InvalidToken,
        Stub
    };

    enum TokenType {
        KNumberType,
        RightUnaryFunctionType,
        LeftUnaryFunctionType,
        BinaryFunctionType,
        OpeningParenthesesType,
        ClosingParenthesesType,
        NotInitialized
    };

    KCalcToken(TokenCode tokenCode);
    KCalcToken(TokenCode tokenCode, int stringIndex);

    KCalcToken(const KNumber &kNumber);
    KCalcToken(const KNumber &kNumber, int stringIndex);

    bool isKNumber() const;
    bool isRightUnaryFunction() const;
    bool isLeftUnaryFunction() const;
    bool isBinaryFunction() const;
    bool isOpeningParentheses() const;
    bool isClosingParentheses() const;

    TokenCode getTokenCode() const;
    TokenType getTokenType() const;
    int getPriorityLevel() const;
    int getStringIndex() const;

    const KNumber getKNumber() const;
    KNumber evaluate(const KNumber &kNumber) const;
    KNumber evaluate(const KNumber &kNumberLeft, const KNumber &kNumberRight) const;

    void invertSignFirstArg();
    void invertSignSecondArg();

    bool isFirstArgInverted() const;
    bool isSecondArgInverted() const;

    void updateToken(const KNumber &kNumber);
    void updateToken(TokenCode tokenCode);

private:
    typedef KNumber (*Unary_Function_Ptr)(const KNumber &);
    typedef KNumber (*Binary_Function_Ptr)(const KNumber &, const KNumber &);

    KNumber *m_key;
    KNumber m_kNumber;
    TokenCode m_tokenCode;
    TokenType m_tokenType;
    Unary_Function_Ptr m_unaryFunctionPtr;
    Binary_Function_Ptr m_binaryFunctionPtr;

    bool m_isKNumber;
    bool m_isRightUnaryFunction;
    bool m_isLeftUnaryFunction;
    bool m_isBinaryFunction;
    bool m_isOpeningParentheses;
    bool m_isClosingParentheses;

    bool m_invertSignFirstArg;
    bool m_invertSignSecondArg;

    int m_priorityLevel;
    int m_stringIndex;

    void setToken(TokenCode tokenCode);
    void inline setLeftUnaryToken(Unary_Function_Ptr function, int priorityLevel);
    void inline setRightUnaryToken(Unary_Function_Ptr function, int priorityLevel);
    void inline setBinaryToken(Binary_Function_Ptr function, int priorityLevel);
    void resetToken();
};
