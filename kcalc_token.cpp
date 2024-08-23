/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_token.h"
#include "kcalc_core_p.h"
#include "kcalc_debug.h"
#include <QDebug>

//------------------------------------------------------------------------------
// Name: KCalcToken
// Desc: constructor
//------------------------------------------------------------------------------
KCalcToken::KCalcToken()
{
    this->resetToken_();
}

KCalcToken::KCalcToken(const KNumber &kNumber)
    : KCalcToken()
{
    kNumber_ = kNumber;
    token_Code_ = KNUMBER;
    is_KNumber_ = true;
    token_Type_ = KNUMBER_TYPE;
}

KCalcToken::KCalcToken(const KNumber &kNumber, int stringIndex)
    : KCalcToken(kNumber)
{
    string_index_ = stringIndex;
}

KCalcToken::KCalcToken(TokenCode tokenCode)
    : KCalcToken()
{
    this->setToken_(tokenCode);
}

KCalcToken::KCalcToken(TokenCode tokenCode, int stringIndex)
    : KCalcToken(tokenCode)
{
    string_index_ = stringIndex;
}

void KCalcToken::updateToken(const KNumber &kNumber)
{
    this->resetToken_();
    kNumber_ = kNumber;
    token_Code_ = KNUMBER;
    is_KNumber_ = true;
    token_Type_ = KNUMBER_TYPE;
}

void KCalcToken::updateToken(TokenCode tokenCode)
{
    this->resetToken_();
    this->setToken_(tokenCode);
}

void KCalcToken::setToken_(TokenCode tokenCode)
{
    token_Code_ = tokenCode;

    switch (tokenCode) {
    case KNUMBER:
        is_KNumber_ = true;
        token_Type_ = KNUMBER_TYPE;
        break;
    case DECIMAL_POINT:
        break;
    case PLUS:
        set_Binary_Token_(CalcEngine_p::ExecAdd, ADD_PRIORITY_LEVEL);
        break;
    case MINUS:
        set_Binary_Token_(CalcEngine_p::ExecSubtract, SUB_PRIORITY_LEVEL);
        break;
    case DIVISION:
    case SLASH:
        set_Binary_Token_(CalcEngine_p::ExecDivide, DIV_PRIORITY_LEVEL);
        break;
    case MULTIPLICATION:
    case DOT:
    case ASTERISK:
        set_Binary_Token_(CalcEngine_p::ExecMultiply, MUL_PRIORITY_LEVEL);
        break;
    case PERCENTAGE:
        set_Left_Unary_Token_(CalcEngine_p::Percentage, PERCENTAGE_PRIORITY_LEVEL);
        break;
    case OPENING_PARENTHESIS:
        is_Opening_Parentheses_ = true;
        token_Type_ = OPENING_PARENTHESES_TYPE;
        break;
    case CLOSING_PARENTHESIS:
        is_Closing_Parentheses_ = true;
        token_Type_ = CLOSING_PARENTHESES_TYPE;
        break;
    case SQUARE:
        set_Left_Unary_Token_(CalcEngine_p::Square, SQUARE_PRIORITY_LEVEL);
        break;
    case CUBE:
        set_Left_Unary_Token_(CalcEngine_p::Cube, CUBE_PRIORITY_LEVEL);
        break;
    case SQUARE_ROOT:
        set_Right_UnaryToken_(CalcEngine_p::SquareRoot, SQUARE_ROOT_PRIORITY_LEVEL);
        break;
    case CUBIC_ROOT:
        set_Right_UnaryToken_(CalcEngine_p::CubeRoot, CUBIC_ROOT_PRIORITY_LEVEL);
        break;
    case SIN:
    case SIN_RAD:
        set_Right_UnaryToken_(CalcEngine_p::SinRad, SIN_PRIORITY_LEVEL);
        break;
    case COS:
    case COS_RAD:
        set_Right_UnaryToken_(CalcEngine_p::CosRad, COS_PRIORITY_LEVEL);
        break;
    case TAN:
    case TAN_RAD:
        set_Right_UnaryToken_(CalcEngine_p::TangensRad, TAN_PRIORITY_LEVEL);
        break;
    case SIN_DEG:
        set_Right_UnaryToken_(CalcEngine_p::SinDeg, SIN_PRIORITY_LEVEL);
        break;
    case COS_DEG:
        set_Right_UnaryToken_(CalcEngine_p::CosDeg, COS_PRIORITY_LEVEL);
        break;
    case TAN_DEG:
        set_Right_UnaryToken_(CalcEngine_p::TangensDeg, TAN_PRIORITY_LEVEL);
        break;
    case SIN_GRAD:
        set_Right_UnaryToken_(CalcEngine_p::SinGrad, SIN_PRIORITY_LEVEL);
        break;
    case COS_GRAD:
        set_Right_UnaryToken_(CalcEngine_p::CosGrad, COS_PRIORITY_LEVEL);
        break;
    case TAN_GRAD:
        set_Right_UnaryToken_(CalcEngine_p::TangensGrad, TAN_PRIORITY_LEVEL);
        break;
    case POLAR_RAD:
        set_Right_UnaryToken_(CalcEngine_p::PolarRad, POLAR_PRIORITY_LEVEL);
        break;
    case POLAR_DEG:
        set_Right_UnaryToken_(CalcEngine_p::PolarDeg, POLAR_PRIORITY_LEVEL);
        break;
    case POLAR_GRAD:
        set_Right_UnaryToken_(CalcEngine_p::PolarGrad, POLAR_PRIORITY_LEVEL);
        break;
    case ASIN:
    case ASIN_RAD:
        set_Right_UnaryToken_(CalcEngine_p::ArcSinRad, ASIN_PRIORITY_LEVEL);
        break;
    case ASIN_DEG:
        set_Right_UnaryToken_(CalcEngine_p::ArcSinDeg, ASIN_PRIORITY_LEVEL);
        break;
    case ASIN_GRAD:
        set_Right_UnaryToken_(CalcEngine_p::ArcSinGrad, ASIN_PRIORITY_LEVEL);
        break;
    case ACOS:
    case ACOS_RAD:
        set_Right_UnaryToken_(CalcEngine_p::ArcCosRad, ACOS_PRIORITY_LEVEL);
        break;
    case ACOS_DEG:
        set_Right_UnaryToken_(CalcEngine_p::ArcCosDeg, ACOS_PRIORITY_LEVEL);
        break;
    case ACOS_GRAD:
        set_Right_UnaryToken_(CalcEngine_p::ArcCosGrad, ACOS_PRIORITY_LEVEL);
        break;
    case ATAN:
    case ATAN_RAD:
        set_Right_UnaryToken_(CalcEngine_p::ArcTangensRad, ATAN_PRIORITY_LEVEL);
        break;
    case ATAN_DEG:
        set_Right_UnaryToken_(CalcEngine_p::ArcTangensDeg, ATAN_PRIORITY_LEVEL);
        break;
    case ATAN_GRAD:
        set_Right_UnaryToken_(CalcEngine_p::ArcTangensGrad, ATAN_PRIORITY_LEVEL);
        break;
    case SINH:
        set_Right_UnaryToken_(CalcEngine_p::SinHyp, SINH_PRIORITY_LEVEL);
        break;
    case COSH:
        set_Right_UnaryToken_(CalcEngine_p::CosHyp, COSH_PRIORITY_LEVEL);
        break;
    case TANH:
        set_Right_UnaryToken_(CalcEngine_p::TangensHyp, TANH_PRIORITY_LEVEL);
        break;
    case ASINH:
        set_Right_UnaryToken_(CalcEngine_p::AreaSinHyp, ASINH_PRIORITY_LEVEL);
        break;
    case ACOSH:
        set_Right_UnaryToken_(CalcEngine_p::AreaCosHyp, ACOSH_PRIORITY_LEVEL);
        break;
    case ATANH:
        set_Right_UnaryToken_(CalcEngine_p::AreaTangensHyp, ATANH_PRIORITY_LEVEL);
        break;
    case GAMMA:
        set_Right_UnaryToken_(CalcEngine_p::Gamma, GAMMA_PRIORITY_LEVEL);
        break;
    case FACTORIAL:
        set_Left_Unary_Token_(CalcEngine_p::Factorial, FACTORIAL_PRIORITY_LEVEL);
        break;
    case POWER:
        set_Binary_Token_(CalcEngine_p::ExecPower, POWER_PRIORITY_LEVEL);
        break;
    case POWER_ROOT:
        set_Binary_Token_(CalcEngine_p::ExecPwrRoot, POWER_ROOT_PRIORITY_LEVEL);
        break;
    case EXP:
        set_Right_UnaryToken_(CalcEngine_p::Exp, EXP_PRIORITY_LEVEL);
        break;
    case EXP_10:
        set_Right_UnaryToken_(CalcEngine_p::Exp10, EXP_10_PRIORITY_LEVEL);
        break;
    case LN:
        set_Right_UnaryToken_(CalcEngine_p::Ln, LN_PRIORITY_LEVEL);
        break;
    case LOG_10:
        set_Right_UnaryToken_(CalcEngine_p::Log10, LOG_10_PRIORITY_LEVEL);
        break;
    case INVERT_SIGN:
        set_Right_UnaryToken_(CalcEngine_p::InvertSign, INVERTSIGN_PRIORITY_LEVEL);
        break;
    case RECIPROCAL:
        set_Left_Unary_Token_(CalcEngine_p::Reciprocal, RECIPROCAL_ROOT_PRIORITY_LEVEL);
        break;
    case BINOMIAL:
        set_Binary_Token_(CalcEngine_p::ExecBinom, BINOMIAL_PRIORITY_LEVEL);
        break;
    case MODULO:
        set_Binary_Token_(CalcEngine_p::ExecMod, MODULO_PRIORITY_LEVEL);
        break;
    case INTEGER_DIVISION:
        set_Binary_Token_(CalcEngine_p::ExecIntDiv, INTEGER_DIVISION_PRIORITY_LEVEL);
        break;
    case AND:
        set_Binary_Token_(CalcEngine_p::ExecAnd, AND_PRIORITY_LEVEL);
        break;
    case OR:
        set_Binary_Token_(CalcEngine_p::ExecOr, OR_PRIORITY_LEVEL);
        break;
    case XOR:
        set_Binary_Token_(CalcEngine_p::ExecXor, XOR_PRIORITY_LEVEL);
        break;
    case ONE_S_COMPLEMENT:
        set_Right_UnaryToken_(CalcEngine_p::Complement, ONE_S_COMPLEMENT_PRIORITY_LEVEL);
        break;
    case RSH:
        set_Binary_Token_(CalcEngine_p::ExecRsh, RSH_STR_PRIORITY_LEVEL);
        break;
    case LSH:
        set_Binary_Token_(CalcEngine_p::ExecLsh, LSH_STR_PRIORITY_LEVEL);
        break;
    case ANS:
        is_KNumber_ = true;
        token_Type_ = KNUMBER_TYPE;
        break;
    case EQUAL:
        break;
    case INVALID_TOKEN:
        break;
    case PERMILLE:
    case TWO_S_COMPLEMENT:
    case DOUBLE_FACTORIAL:
    case DEGREE:
    case GRADIAN:
    case RADIAN:
    default:
        qCDebug(KCALC_LOG) << "Not implemented token ";
        break;
    }
}

void inline KCalcToken::set_Left_Unary_Token_(Unary_Function_Ptr function, int priorityLevel)
{
    unary_Function_Ptr_ = function;
    priority_level_ = priorityLevel;
    is_Left_Unary_Function_ = true;
    token_Type_ = LEFT_UNARY_FUNCTION_TYPE;
}

void inline KCalcToken::set_Right_UnaryToken_(Unary_Function_Ptr function, int priorityLevel)
{
    unary_Function_Ptr_ = function;
    priority_level_ = priorityLevel;
    is_Right_Unary_Function_ = true;
    token_Type_ = RIGHT_UNARY_FUNCTION_TYPE;
}

void inline KCalcToken::set_Binary_Token_(Binary_Function_Ptr function, int priorityLevel)
{
    binary_Function_Ptr_ = function;
    is_Binary_Function_ = true;
    token_Type_ = BINARY_FUNCTION_TYPE;
    priority_level_ = priorityLevel;
}

void KCalcToken::resetToken_()
{
    key_ = (nullptr);
    kNumber_ = KNumber::NaN;
    token_Code_ = INVALID_TOKEN;

    unary_Function_Ptr_ = nullptr;
    binary_Function_Ptr_ = nullptr;

    is_KNumber_ = false;
    is_Right_Unary_Function_ = false;
    is_Left_Unary_Function_ = false;
    is_Binary_Function_ = false;
    is_Opening_Parentheses_ = false;
    is_Closing_Parentheses_ = false;

    invert_Sign_First_Arg_ = false;
    invert_Sign_Second_Arg_ = false;

    priority_level_ = NO_LEVEL;

    string_index_ = -1;
}

bool KCalcToken::isKNumber() const
{
    return is_KNumber_;
}

bool KCalcToken::isRightUnaryFunction() const
{
    return is_Right_Unary_Function_;
}

bool KCalcToken::isLeftUnaryFunction() const
{
    return is_Left_Unary_Function_;
}

bool KCalcToken::isBinaryFunction() const
{
    return is_Binary_Function_;
}

bool KCalcToken::isOpeningParentheses() const
{
    return is_Opening_Parentheses_;
}

bool KCalcToken::isClosingParentheses() const
{
    return is_Closing_Parentheses_;
}

const KNumber KCalcToken::getKNumber() const
{
    return kNumber_;
}

KCalcToken::TokenCode KCalcToken::getTokenCode() const
{
    return token_Code_;
}

KCalcToken::TokenType KCalcToken::getTokenType() const
{
    return token_Type_;
}

int KCalcToken::getPriorityLevel() const
{
    return priority_level_;
}

int KCalcToken::getStringIndex() const
{
    return string_index_;
}

KNumber KCalcToken::evaluate(const KNumber &kNumber) const
{
    if (unary_Function_Ptr_ == nullptr) {
        return KNumber::NaN; // safety code
    } else {
        if (invert_Sign_First_Arg_) {
            return unary_Function_Ptr_(-kNumber);
        } else {
            return unary_Function_Ptr_(kNumber);
        }
    }
}

KNumber KCalcToken::evaluate(const KNumber &kNumberLeft, const KNumber &kNumberRight) const
{
    if (binary_Function_Ptr_ == nullptr) {
        return KNumber::NaN; // safety code
    } else {
        if (!invert_Sign_First_Arg_ && !invert_Sign_Second_Arg_) {
            return binary_Function_Ptr_(kNumberLeft, kNumberRight);
        } else if (invert_Sign_First_Arg_ && !invert_Sign_Second_Arg_) {
            return binary_Function_Ptr_(-kNumberLeft, kNumberRight);
        } else if (!invert_Sign_First_Arg_ && invert_Sign_Second_Arg_) {
            return binary_Function_Ptr_(kNumberLeft, -kNumberRight);
        } else {
            return binary_Function_Ptr_(-kNumberLeft, -kNumberRight);
        }
    }
}

void KCalcToken::invertSignFirstArg()
{
    invert_Sign_First_Arg_ = !invert_Sign_First_Arg_;
}

void KCalcToken::invertSignSecondArg()
{
    invert_Sign_Second_Arg_ = !invert_Sign_Second_Arg_;
}

bool KCalcToken::isFirstArgInverted() const
{
    return invert_Sign_First_Arg_;
}

bool KCalcToken::isSecondArgInverted() const
{
    return invert_Sign_Second_Arg_;
}

//------------------------------------------------------------------------------
// Name: ~KCalcToken
// Desc: destructor
//------------------------------------------------------------------------------
KCalcToken::~KCalcToken() = default;
