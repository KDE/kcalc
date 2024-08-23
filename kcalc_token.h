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
        KNUMBER,
        DECIMAL_POINT,
        PLUS,
        MINUS,
        INVERT_SIGN,
        DIVISION,
        SLASH,
        MULTIPLICATION,
        DOT,
        ASTERISK,
        PERCENTAGE,
        PERMILLE,
        OPENING_PARENTHESIS,
        CLOSING_PARENTHESIS,
        SQUARE,
        CUBE,
        SQUARE_ROOT,
        CUBIC_ROOT,
        DEGREE,
        GRADIAN,
        RADIAN,
        SIN,
        SIN_RAD,
        SIN_DEG,
        SIN_GRAD,
        COS,
        COS_RAD,
        COS_DEG,
        COS_GRAD,
        TAN,
        TAN_RAD,
        TAN_DEG,
        TAN_GRAD,
        ASIN,
        ASIN_RAD,
        ASIN_DEG,
        ASIN_GRAD,
        ACOS,
        ACOS_RAD,
        ACOS_DEG,
        ACOS_GRAD,
        ATAN,
        ATAN_RAD,
        ATAN_DEG,
        ATAN_GRAD,
        POLAR_RAD,
        POLAR_DEG,
        POLAR_GRAD,
        SINH,
        COSH,
        TANH,
        ASINH,
        ACOSH,
        ATANH,
        E,
        PI,
        PHI,
        I,
        POLAR,
        POS_INFINITY,
        NEG_INFINITY,
        VACUUM_PERMITIVITY,
        VACUUM_PERMEABILITY,
        VACUUM_IMPEDANCE,
        PLANCK_S_CONSTANT,
        PLANCK_OVER_2PI,
        EXP,
        EXP_10,
        POWER,
        POWER_ROOT,
        FACTORIAL,
        DOUBLE_FACTORIAL,
        GAMMA,
        LN,
        LOG_10,
        RECIPROCAL,
        BINOMIAL,
        MODULO,
        INTEGER_DIVISION,
        AND,
        OR,
        XOR,
        ONE_S_COMPLEMENT,
        TWO_S_COMPLEMENT,
        RSH,
        LSH,
        EQUAL,
        ANS,
        INVALID_TOKEN,
        STUB
    };

    enum TokenType {
        KNUMBER_TYPE,
        RIGHT_UNARY_FUNCTION_TYPE,
        LEFT_UNARY_FUNCTION_TYPE,
        BINARY_FUNCTION_TYPE,
        OPENING_PARENTHESES_TYPE,
        CLOSING_PARENTHESES_TYPE,
        NOT_INITIALIZED
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

    const KNumber *key_;
    KNumber kNumber_;
    TokenCode token_Code_;
    TokenType token_Type_;
    Unary_Function_Ptr unary_Function_Ptr_;
    Binary_Function_Ptr binary_Function_Ptr_;

    bool is_KNumber_;
    bool is_Right_Unary_Function_;
    bool is_Left_Unary_Function_;
    bool is_Binary_Function_;
    bool is_Opening_Parentheses_;
    bool is_Closing_Parentheses_;

    bool invert_Sign_First_Arg_;
    bool invert_Sign_Second_Arg_;

    int priority_level_;
    int string_index_;

    void setToken_(TokenCode tokenCode);
    void inline set_Left_Unary_Token_(Unary_Function_Ptr function, int priorityLevel);
    void inline set_Right_UnaryToken_(Unary_Function_Ptr function, int priorityLevel);
    void inline set_Binary_Token_(Binary_Function_Ptr function, int priorityLevel);
    void resetToken_();
};
