/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_token.h"
#include "kcalc_debug.h"

//------------------------------------------------------------------------------
// Name: KCalcToken
// Desc: constructor
//------------------------------------------------------------------------------
KCalcToken::KCalcToken()
{
    this->resetToken();
}

KCalcToken::KCalcToken(const class KNumber &kNumber)
    : KCalcToken()
{
    this->m_kNumber = kNumber;
    m_tokenCode = Knumber;
    m_isKNumber = true;
    m_tokenType = KNumberType;
}

KCalcToken::KCalcToken(const class KNumber &kNumber, int stringIndex)
    : KCalcToken(kNumber)
{
    m_stringIndex = stringIndex;
}

KCalcToken::KCalcToken(TokenCode tokenCode)
    : KCalcToken()
{
    this->setToken(tokenCode);
}

KCalcToken::KCalcToken(TokenCode tokenCode, int stringIndex)
    : KCalcToken(tokenCode)
{
    m_stringIndex = stringIndex;
}

void KCalcToken::updateToken(const class KNumber &kNumber)
{
    this->resetToken();
    this->m_kNumber = kNumber;
    m_tokenCode = Knumber;
    m_isKNumber = true;
    m_tokenType = KNumberType;
}

void KCalcToken::updateToken(TokenCode tokenCode)
{
    this->resetToken();
    this->setToken(tokenCode);
}

void KCalcToken::setToken(TokenCode tokenCode)
{
    this->m_tokenCode = tokenCode;

    switch (tokenCode) {
    case Knumber:
        m_isKNumber = true;
        m_tokenType = KNumberType;
        break;
    case DecimalPoint:
        break;
    case Plus:
        setBinaryToken(CalcEngineP::execAdd, ADD_PRIORITY_LEVEL);
        break;
    case Minus:
        setBinaryToken(CalcEngineP::execSubtract, SUB_PRIORITY_LEVEL);
        break;
    case Division:
    case Slash:
        setBinaryToken(CalcEngineP::execDivide, DIV_PRIORITY_LEVEL);
        break;
    case Multiplication:
    case Dot:
    case Asterisk:
        setBinaryToken(CalcEngineP::execMultiply, MUL_PRIORITY_LEVEL);
        break;
    case Percentage:
        setLeftUnaryToken(CalcEngineP::percentage, PERCENTAGE_PRIORITY_LEVEL);
        break;
    case OpeningParenthesis:
        m_isOpeningParentheses = true;
        m_tokenType = OpeningParenthesesType;
        break;
    case ClosingParenthesis:
        m_isClosingParentheses = true;
        m_tokenType = ClosingParenthesesType;
        break;
    case Square:
        setLeftUnaryToken(CalcEngineP::square, SQUARE_PRIORITY_LEVEL);
        break;
    case Cube:
        setLeftUnaryToken(CalcEngineP::cube, CUBE_PRIORITY_LEVEL);
        break;
    case SquareRoot:
        setRightUnaryToken(CalcEngineP::squareRoot, SQUARE_ROOT_PRIORITY_LEVEL);
        break;
    case CubicRoot:
        setRightUnaryToken(CalcEngineP::cubeRoot, CUBIC_ROOT_PRIORITY_LEVEL);
        break;
    case Sin:
    case SinRad:
        setRightUnaryToken(CalcEngineP::sinRad, SIN_PRIORITY_LEVEL);
        break;
    case Cos:
    case CosRad:
        setRightUnaryToken(CalcEngineP::cosRad, COS_PRIORITY_LEVEL);
        break;
    case Tan:
    case TanRad:
        setRightUnaryToken(CalcEngineP::tangensRad, TAN_PRIORITY_LEVEL);
        break;
    case SinDeg:
        setRightUnaryToken(CalcEngineP::sinDeg, SIN_PRIORITY_LEVEL);
        break;
    case CosDeg:
        setRightUnaryToken(CalcEngineP::cosDeg, COS_PRIORITY_LEVEL);
        break;
    case TanDeg:
        setRightUnaryToken(CalcEngineP::tangensDeg, TAN_PRIORITY_LEVEL);
        break;
    case SinGrad:
        setRightUnaryToken(CalcEngineP::sinGrad, SIN_PRIORITY_LEVEL);
        break;
    case CosGrad:
        setRightUnaryToken(CalcEngineP::cosGrad, COS_PRIORITY_LEVEL);
        break;
    case TanGrad:
        setRightUnaryToken(CalcEngineP::tangensGrad, TAN_PRIORITY_LEVEL);
        break;
    case PolarRad:
        setRightUnaryToken(CalcEngineP::polarRad, POLAR_PRIORITY_LEVEL);
        break;
    case PolarDeg:
        setRightUnaryToken(CalcEngineP::polarDeg, POLAR_PRIORITY_LEVEL);
        break;
    case PolarGrad:
        setRightUnaryToken(CalcEngineP::polarGrad, POLAR_PRIORITY_LEVEL);
        break;
    case ArgRad:
        setRightUnaryToken(CalcEngineP::argumentRad, TAN_PRIORITY_LEVEL);
        break;
    case ArgDeg:
        setRightUnaryToken(CalcEngineP::argumentDeg, TAN_PRIORITY_LEVEL);
        break;
    case ArgGrad:
        setRightUnaryToken(CalcEngineP::argumentGrad, TAN_PRIORITY_LEVEL);
        break;
    case Re:
        setRightUnaryToken(CalcEngineP::real, TAN_PRIORITY_LEVEL);
        break;
    case Im:
        setRightUnaryToken(CalcEngineP::imaginary, TAN_PRIORITY_LEVEL);
        break;
    case Conj:
        setRightUnaryToken(CalcEngineP::conjugate, TAN_PRIORITY_LEVEL);
        break;
    case Asin:
    case AsinRad:
        setRightUnaryToken(CalcEngineP::arcSinRad, ASIN_PRIORITY_LEVEL);
        break;
    case AsinDeg:
        setRightUnaryToken(CalcEngineP::arcSinDeg, ASIN_PRIORITY_LEVEL);
        break;
    case AsinGrad:
        setRightUnaryToken(CalcEngineP::arcSinGrad, ASIN_PRIORITY_LEVEL);
        break;
    case Acos:
    case AcosRad:
        setRightUnaryToken(CalcEngineP::arcCosRad, ACOS_PRIORITY_LEVEL);
        break;
    case AcosDeg:
        setRightUnaryToken(CalcEngineP::arcCosDeg, ACOS_PRIORITY_LEVEL);
        break;
    case AcosGrad:
        setRightUnaryToken(CalcEngineP::arcCosGrad, ACOS_PRIORITY_LEVEL);
        break;
    case Atan:
    case AtanRad:
        setRightUnaryToken(CalcEngineP::arcTangensRad, ATAN_PRIORITY_LEVEL);
        break;
    case AtanDeg:
        setRightUnaryToken(CalcEngineP::arcTangensDeg, ATAN_PRIORITY_LEVEL);
        break;
    case AtanGrad:
        setRightUnaryToken(CalcEngineP::arcTangensGrad, ATAN_PRIORITY_LEVEL);
        break;
    case Sinh:
        setRightUnaryToken(CalcEngineP::sinHyp, SINH_PRIORITY_LEVEL);
        break;
    case Cosh:
        setRightUnaryToken(CalcEngineP::cosHyp, COSH_PRIORITY_LEVEL);
        break;
    case Tanh:
        setRightUnaryToken(CalcEngineP::tangensHyp, TANH_PRIORITY_LEVEL);
        break;
    case Asinh:
        setRightUnaryToken(CalcEngineP::areaSinHyp, ASINH_PRIORITY_LEVEL);
        break;
    case Acosh:
        setRightUnaryToken(CalcEngineP::areaCosHyp, ACOSH_PRIORITY_LEVEL);
        break;
    case Atanh:
        setRightUnaryToken(CalcEngineP::areaTangensHyp, ATANH_PRIORITY_LEVEL);
        break;
    case Gamma:
        setRightUnaryToken(CalcEngineP::gamma, GAMMA_PRIORITY_LEVEL);
        break;
    case Factorial:
        setLeftUnaryToken(CalcEngineP::factorial, FACTORIAL_PRIORITY_LEVEL);
        break;
    case Power:
        setBinaryToken(CalcEngineP::execPower, POWER_PRIORITY_LEVEL);
        break;
    case PowerRoot:
        setBinaryToken(CalcEngineP::execPwrRoot, POWER_ROOT_PRIORITY_LEVEL);
        break;
    case Exp:
        setRightUnaryToken(CalcEngineP::exp, EXP_PRIORITY_LEVEL);
        break;
    case Exp10:
        setRightUnaryToken(CalcEngineP::exp10, EXP_10_PRIORITY_LEVEL);
        break;
    case Ln:
        setRightUnaryToken(CalcEngineP::ln, LN_PRIORITY_LEVEL);
        break;
    case Log10:
        setRightUnaryToken(CalcEngineP::log10, LOG_10_PRIORITY_LEVEL);
        break;
    case InvertSign:
        setRightUnaryToken(CalcEngineP::invertSign, INVERTSIGN_PRIORITY_LEVEL);
        break;
    case Reciprocal:
        setLeftUnaryToken(CalcEngineP::reciprocal, RECIPROCAL_ROOT_PRIORITY_LEVEL);
        break;
    case Binomial:
        setBinaryToken(CalcEngineP::execBinom, BINOMIAL_PRIORITY_LEVEL);
        break;
    case Modulo:
        setBinaryToken(CalcEngineP::execMod, MODULO_PRIORITY_LEVEL);
        break;
    case IntegerDivision:
        setBinaryToken(CalcEngineP::execIntDiv, INTEGER_DIVISION_PRIORITY_LEVEL);
        break;
    case And:
        setBinaryToken(CalcEngineP::execAnd, AND_PRIORITY_LEVEL);
        break;
    case Or:
        setBinaryToken(CalcEngineP::execOr, OR_PRIORITY_LEVEL);
        break;
    case Xor:
        setBinaryToken(CalcEngineP::execXor, XOR_PRIORITY_LEVEL);
        break;
    case OnesComplement:
        setRightUnaryToken(CalcEngineP::complement, ONE_S_COMPLEMENT_PRIORITY_LEVEL);
        break;
    case Rsh:
        setBinaryToken(CalcEngineP::execRsh, RSH_STR_PRIORITY_LEVEL);
        break;
    case Lsh:
        setBinaryToken(CalcEngineP::execLsh, LSH_STR_PRIORITY_LEVEL);
        break;
    case Ans:
        m_isKNumber = true;
        m_tokenType = KNumberType;
        break;
    case Equal:
        break;
    case InvalidToken:
        break;
    case Permille:
    case TwosComplement:
    case DoubleFactorial:
    case Degree:
    case Gradian:
    case Radian:
    default:
        qCDebug(KCALC_LOG) << "Not implemented token ";
        break;
    }
}

void inline KCalcToken::setLeftUnaryToken(Unary_Function_Ptr function, int priorityLevel)
{
    m_unaryFunctionPtr = function;
    m_priorityLevel = priorityLevel;
    m_isLeftUnaryFunction = true;
    m_tokenType = LeftUnaryFunctionType;
}

void inline KCalcToken::setRightUnaryToken(Unary_Function_Ptr function, int priorityLevel)
{
    m_unaryFunctionPtr = function;
    m_priorityLevel = priorityLevel;
    m_isRightUnaryFunction = true;
    m_tokenType = RightUnaryFunctionType;
}

void inline KCalcToken::setBinaryToken(Binary_Function_Ptr function, int priorityLevel)
{
    m_binaryFunctionPtr = function;
    m_priorityLevel = priorityLevel;
    m_isBinaryFunction = true;
    m_tokenType = BinaryFunctionType;
}

void KCalcToken::resetToken()
{
    m_key = nullptr;
    m_kNumber = KNumber::NaN;
    m_tokenCode = InvalidToken;

    m_unaryFunctionPtr = nullptr;
    m_binaryFunctionPtr = nullptr;

    m_isKNumber = false;
    m_isRightUnaryFunction = false;
    m_isLeftUnaryFunction = false;
    m_isBinaryFunction = false;
    m_isOpeningParentheses = false;
    m_isClosingParentheses = false;

    m_invertSignFirstArg = false;
    m_invertSignSecondArg = false;

    m_priorityLevel = NO_LEVEL;

    m_stringIndex = -1;
}

bool KCalcToken::isKNumber() const
{
    return m_isKNumber;
}

bool KCalcToken::isRightUnaryFunction() const
{
    return m_isRightUnaryFunction;
}

bool KCalcToken::isLeftUnaryFunction() const
{
    return m_isLeftUnaryFunction;
}

bool KCalcToken::isBinaryFunction() const
{
    return m_isBinaryFunction;
}

bool KCalcToken::isOpeningParentheses() const
{
    return m_isOpeningParentheses;
}

bool KCalcToken::isClosingParentheses() const
{
    return m_isClosingParentheses;
}

const KNumber KCalcToken::getKNumber() const
{
    return m_kNumber;
}

KCalcToken::TokenCode KCalcToken::getTokenCode() const
{
    return m_tokenCode;
}

KCalcToken::TokenType KCalcToken::getTokenType() const
{
    return m_tokenType;
}

int KCalcToken::getPriorityLevel() const
{
    return m_priorityLevel;
}

int KCalcToken::getStringIndex() const
{
    return m_stringIndex;
}

KNumber KCalcToken::evaluate(const class KNumber &kNumber) const
{
    if (m_unaryFunctionPtr == nullptr) {
        return KNumber::NaN; // safety code
    } else {
        if (m_invertSignFirstArg) {
            return m_unaryFunctionPtr(-kNumber);
        } else {
            return m_unaryFunctionPtr(kNumber);
        }
    }
}

KNumber KCalcToken::evaluate(const class KNumber &kNumberLeft, const class KNumber &kNumberRight) const
{
    if (m_binaryFunctionPtr == nullptr) {
        return KNumber::NaN; // safety code
    } else {
        if (!m_invertSignFirstArg && !m_invertSignSecondArg) {
            return m_binaryFunctionPtr(kNumberLeft, kNumberRight);
        } else if (m_invertSignFirstArg && !m_invertSignSecondArg) {
            return m_binaryFunctionPtr(-kNumberLeft, kNumberRight);
        } else if (!m_invertSignFirstArg && m_invertSignSecondArg) {
            return m_binaryFunctionPtr(kNumberLeft, -kNumberRight);
        } else {
            return m_binaryFunctionPtr(-kNumberLeft, -kNumberRight);
        }
    }
}

void KCalcToken::invertSignFirstArg()
{
    m_invertSignFirstArg = !m_invertSignFirstArg;
}

void KCalcToken::invertSignSecondArg()
{
    m_invertSignSecondArg = !m_invertSignSecondArg;
}

bool KCalcToken::isFirstArgInverted() const
{
    return m_invertSignFirstArg;
}

bool KCalcToken::isSecondArgInverted() const
{
    return m_invertSignSecondArg;
}

//------------------------------------------------------------------------------
// Name: ~KCalcToken
// Desc: destructor
//------------------------------------------------------------------------------
KCalcToken::~KCalcToken() = default;
