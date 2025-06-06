/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_parser.h"
#include "kcalc_debug.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>

const QString KCalcParser::BINARY_DIGITS_PATTERN = QStringLiteral("[0-1]{1,}");
const QString KCalcParser::OCTAL_DIGITS_PATTERN = QStringLiteral("[0-7]{1,}");
const QString KCalcParser::HEX_DIGITS_PATTERN = QStringLiteral("[0-9A-Fa-f]{1,}");

const QString KCalcParser::DECIMAL_NUMBER_PATTERN = QLatin1String("(0*)((\\d*)[.,]?\\d+([e|E]([+-]?\\d+))?)");

const QRegularExpression KCalcParser::BINARY_NUMBER_DIGITS_REGEX = QRegularExpression(KCalcParser::BINARY_DIGITS_PATTERN);
const QRegularExpression KCalcParser::OCTAL_NUMBER_DIGITS_REGEX = QRegularExpression(KCalcParser::OCTAL_DIGITS_PATTERN);
const QRegularExpression KCalcParser::HEX_NUMBER_DIGITS_REGEX = QRegularExpression(KCalcParser::HEX_DIGITS_PATTERN);

const QRegularExpression KCalcParser::DECIMAL_NUMBER_REGEX = QRegularExpression(KCalcParser::DECIMAL_NUMBER_PATTERN);

//------------------------------------------------------------------------------
// Name: KCalcParser
// Desc: constructor
//------------------------------------------------------------------------------
KCalcParser::KCalcParser()
{
}

//------------------------------------------------------------------------------
// Name: ~KCalcParser
// Desc: destructor
//------------------------------------------------------------------------------
KCalcParser::~KCalcParser() = default;

//------------------------------------------------------------------------------
// Name: stringToToken
// Desc: tries to find a valid token starting at index
//------------------------------------------------------------------------------
KCalcToken::TokenCode KCalcParser::stringToToken(const QString &buffer, int &index, int base)
{
    qCDebug(KCALC_LOG) << "Converting string to Token ";

    int maxTokenLength = (index + 5 > buffer.size()) ? buffer.size() - index : 5;
    QString s = buffer.sliced(index, maxTokenLength);

    // TODO: change this to be switch table on the first char
    if (s.startsWith(SPACE_STR) || s.startsWith(THIN_SPACE_STR)) {
        index++;
        return KCalcToken::TokenCode::Stub;
    }

    if (s.startsWith(HEX_NUMBER_PREFIX_STR)) {
        QRegularExpressionMatch match;
        int numIndex = buffer.indexOf(HEX_NUMBER_DIGITS_REGEX, index + HEX_NUMBER_PREFIX_STR.length(), &match);
        if (numIndex == index + HEX_NUMBER_PREFIX_STR.length()) {
            token_KNumber_ = HEX_NUMBER_PREFIX_STR + match.captured();
            if (match.captured().size() > 16) {
                return KCalcToken::TokenCode::InvalidToken;
            }
            index += HEX_NUMBER_PREFIX_STR.length();
            index += match.captured().size();
            return KCalcToken::TokenCode::KNumber;
        } else {
            return KCalcToken::TokenCode::InvalidToken;
        }
    }

    if (s.startsWith(BINARY_NUMBER_PREFIX_STR)) {
        QRegularExpressionMatch match;
        int numIndex = buffer.indexOf(BINARY_NUMBER_DIGITS_REGEX, index + BINARY_NUMBER_PREFIX_STR.length(), &match);
        if (numIndex == index + BINARY_NUMBER_PREFIX_STR.length()) {
            token_KNumber_ = BINARY_NUMBER_PREFIX_STR + match.captured();
            if (match.captured().size() > 64) {
                return KCalcToken::TokenCode::InvalidToken;
            }
            index += BINARY_NUMBER_PREFIX_STR.length();
            index += match.captured().size();
            return KCalcToken::TokenCode::KNumber;
        } else {
            return KCalcToken::TokenCode::InvalidToken;
        }
    }

    if (s.startsWith(OCTAL_NUMBER_PREFIX_STR)) {
        QRegularExpressionMatch match;
        int numIndex = buffer.indexOf(OCTAL_NUMBER_DIGITS_REGEX, index + OCTAL_NUMBER_PREFIX_STR.length(), &match);
        if (numIndex == index + OCTAL_NUMBER_PREFIX_STR.length()) {
            token_KNumber_ = OCTAL_NUMBER_PREFIX_STR_C_STYLE + match.captured();
            if (match.captured().size() > 21) {
                return KCalcToken::TokenCode::InvalidToken;
            }
            index += OCTAL_NUMBER_PREFIX_STR.length();
            index += match.captured().size();
            return KCalcToken::TokenCode::KNumber;
        }
        // Code execution continues, since an entry could
        // start with a 0 but not be a valid octal number
        // being a number in base 10 or 16
    }

    if ((A_STR <= s.first(1) && s.first(1) <= F_STR) || (ZERO_STR <= s.first(1) && s.first(1) <= NINE_STR)
        || (s.first(1) == COMMA_STR || s.first(1) == POINT_STR)) {
        int numIndex = -1;
        QRegularExpressionMatch match;
        switch (base) {
        case 2:
            numIndex = buffer.indexOf(BINARY_NUMBER_DIGITS_REGEX, index, &match);
            token_KNumber_ = BINARY_NUMBER_PREFIX_STR;
            if (match.captured().size() > 64) {
                return KCalcToken::TokenCode::InvalidToken;
            }
            break;
        case 8:
            numIndex = buffer.indexOf(OCTAL_NUMBER_DIGITS_REGEX, index, &match);
            token_KNumber_ = OCTAL_NUMBER_PREFIX_STR_C_STYLE;
            if (match.captured().size() > 21) {
                return KCalcToken::TokenCode::InvalidToken;
            }
            break;
        case 10:
            numIndex = buffer.indexOf(DECIMAL_NUMBER_REGEX, index, &match);
            token_KNumber_.clear();
            break;
        case 16:
            numIndex = buffer.indexOf(HEX_NUMBER_DIGITS_REGEX, index, &match);
            token_KNumber_ = HEX_NUMBER_PREFIX_STR;
            if (match.captured().size() > 16) {
                return KCalcToken::TokenCode::InvalidToken;
            }
            break;
        default:
            break;
        }
        if (numIndex == index) {
            if (base == 10) {
                token_KNumber_ += match.captured(2);
            } else {
                token_KNumber_ += match.captured();
            }
            index += match.captured().size();
            return KCalcToken::TokenCode::KNumber;
        } else {
            return KCalcToken::TokenCode::InvalidToken;
        }
    }

    if (ANGLE_STR == s.first(1)) {
        index++;
        switch (trigonometric_Mode_) {
        case RADIANS:
            return KCalcToken::TokenCode::PolarRad;
        case GRADIANS:
            return KCalcToken::TokenCode::PolarGrad;
        case DEGREES:
            return KCalcToken::TokenCode::PolarDeg;
        default:
            Q_UNREACHABLE();
            return KCalcToken::TokenCode::InvalidToken;
        }
    }

    if (s.startsWith(PLUS_STR)) {
        index++;
        return KCalcToken::TokenCode::Plus;
    }
    if (s.startsWith(HYPHEN_MINUS_STR) || s.startsWith(MINUS_SIGN_STR)) {
        index++;
        return KCalcToken::TokenCode::Minus;
    }
    if (s.startsWith(EQUAL_STR)) {
        index++;
        return KCalcToken::TokenCode::Equal;
    }
    if (s.startsWith(SQUARE_STR)) {
        index++;
        return KCalcToken::TokenCode::Square;
    }
    if (s.startsWith(CUBE_STR)) {
        index++;
        return KCalcToken::TokenCode::Cube;
    }
    if (s.startsWith(POWER_STR)) {
        index++;
        return KCalcToken::TokenCode::Power;
    }
    if (s.startsWith(POWER_ROOT_STR)) {
        index++;
        return KCalcToken::TokenCode::PowerRoot;
    }
    if (s.startsWith(EXP_10_STR)) {
        index++;
        return KCalcToken::TokenCode::Exp10;
    }
    if (s.startsWith(PERCENTAGE_STR)) {
        index++;
        return KCalcToken::TokenCode::Percentage;
    }
    if (s.startsWith(PERMILLE_STR)) {
        index++;
        return KCalcToken::TokenCode::Permille;
    }
    if (s.startsWith(I_STR) && !s.startsWith(IM_STR)) {
        index++;
        token_KNumber_ = I_STR;
        return KCalcToken::TokenCode::KNumber;
    }
    if (s.startsWith(DIVISION_STR) || s.startsWith(SLASH_STR) || s.startsWith(DIVISION_SLASH_STR)) {
        index++;
        return KCalcToken::TokenCode::Division;
    }
    if (s.startsWith(GAMMA_STR)) {
        index++;
        return KCalcToken::TokenCode::Gamma;
    }
    if (s.startsWith(SQUARE_ROOT_STR)) {
        index++;
        return KCalcToken::TokenCode::SquareRoot;
    }
    if (s.startsWith(CUBIC_ROOT_STR)) {
        index++;
        return KCalcToken::TokenCode::CubicRoot;
    }
    if (s.startsWith(OPENING_PARENTHESIS_STR)) {
        index++;
        return KCalcToken::TokenCode::OpeningParenthesis;
    }
    if (s.startsWith(CLOSING_PARENTHESIS_STR)) {
        index++;
        return KCalcToken::TokenCode::ClosingParenthesis;
    }
    if (s.startsWith(INVERT_SIGN_STR)) {
        index++;
        return KCalcToken::TokenCode::InvertSign;
    }

    if (s.startsWith(MULTIPLICATION_STR) || s.startsWith(DOT_STR) || s.startsWith(ASTERISK_STR)) {
        index++;
        return KCalcToken::TokenCode::Multiplication;
    }

    if (s.startsWith(DEGREE_STR)) {
        index++;
        return KCalcToken::TokenCode::Degree;
    }
    if (s.startsWith(AND_STR)) {
        index++;
        return KCalcToken::TokenCode::And;
    }
    if (s.startsWith(OR_STR)) {
        index++;
        return KCalcToken::TokenCode::Or;
    }
    if (s.startsWith(XOR_STR)) {
        index++;
        return KCalcToken::TokenCode::Xor;
    }
    if (s.startsWith(XOR_LETTER_STR, Qt::CaseSensitivity::CaseInsensitive)) {
        index += 3;
        return KCalcToken::TokenCode::Xor;
    }

    if (s.startsWith(ASINH_STR)) {
        index += 5;
        return KCalcToken::TokenCode::Asinh;
    }
    if (s.startsWith(ACOSH_STR)) {
        index += 5;
        return KCalcToken::TokenCode::Acosh;
    }
    if (s.startsWith(ATANH_STR)) {
        index += 5;
        return KCalcToken::TokenCode::Atanh;
    }

    if (s.startsWith(ASIN_STR)) {
        index += 4;
        switch (trigonometric_Mode_) {
        case RADIANS:
            return KCalcToken::TokenCode::AsinRad;
        case GRADIANS:
            return KCalcToken::TokenCode::AsinGrad;
        case DEGREES:
            return KCalcToken::TokenCode::AsinDeg;
        default:
            break;
        }
    }
    if (s.startsWith(ACOS_STR)) {
        index += 4;
        switch (trigonometric_Mode_) {
        case RADIANS:
            return KCalcToken::TokenCode::AcosRad;
        case GRADIANS:
            return KCalcToken::TokenCode::AcosGrad;
        case DEGREES:
            return KCalcToken::TokenCode::AcosDeg;
        default:
            break;
        }
    }
    if (s.startsWith(ATAN_STR)) {
        index += 4;
        switch (trigonometric_Mode_) {
        case RADIANS:
            return KCalcToken::TokenCode::AtanRad;
        case GRADIANS:
            return KCalcToken::TokenCode::AtanGrad;
        case DEGREES:
            return KCalcToken::TokenCode::AtanDeg;
        default:
            break;
        }
    }

    if (s.startsWith(SINH_STR)) {
        index += 4;
        return KCalcToken::TokenCode::Sinh;
    }
    if (s.startsWith(COSH_STR)) {
        index += 4;
        return KCalcToken::TokenCode::Cosh;
    }
    if (s.startsWith(TANH_STR)) {
        index += 4;
        return KCalcToken::TokenCode::Tanh;
    }
    if (s.startsWith(CONJ_STR)) {
        index += 4;
        return KCalcToken::TokenCode::Conj;
    }

    if (s.startsWith(SIN_STR)) {
        index += 3;
        switch (trigonometric_Mode_) {
        case RADIANS:
            return KCalcToken::TokenCode::SinRad;
        case GRADIANS:
            return KCalcToken::TokenCode::SinGrad;
        case DEGREES:
            return KCalcToken::TokenCode::SinDeg;
        default:
            break;
        }
    }
    if (s.startsWith(COS_STR)) {
        index += 3;
        switch (trigonometric_Mode_) {
        case RADIANS:
            return KCalcToken::TokenCode::CosRad;
        case GRADIANS:
            return KCalcToken::TokenCode::CosGrad;
        case DEGREES:
            return KCalcToken::TokenCode::CosDeg;
        default:
            break;
        }
    }
    if (s.startsWith(TAN_STR)) {
        index += 3;
        switch (trigonometric_Mode_) {
        case RADIANS:
            return KCalcToken::TokenCode::TanRad;
        case GRADIANS:
            return KCalcToken::TokenCode::TanGrad;
        case DEGREES:
            return KCalcToken::TokenCode::TanDeg;
        default:
            break;
        }
    }

    if (s.startsWith(ARG_STR)) {
        index += 3;
        switch (trigonometric_Mode_) {
        case RADIANS:
            return KCalcToken::TokenCode::ArgRad;
        case GRADIANS:
            return KCalcToken::TokenCode::ArgGrad;
        case DEGREES:
            return KCalcToken::TokenCode::ArgDeg;
        default:
            break;
        }
    }

    if (s.startsWith(EXP_STR)) {
        index += 3;
        return KCalcToken::TokenCode::Exp;
    }

    if (s.startsWith(LOG_10_STR)) {
        index += 3;
        return KCalcToken::TokenCode::Log10;
    }

    if (s.startsWith(ANS_STR)) {
        index += 3;
        return KCalcToken::TokenCode::Ans;
    }

    if (s.startsWith(BINOMIAL_STR)) {
        index += 3;
        return KCalcToken::TokenCode::Binomial;
    }

    if (s.startsWith(MODULO_STR)) {
        index += 3;
        return KCalcToken::TokenCode::Modulo;
    }

    if (s.startsWith(INTEGER_DIVISION_STR)) {
        index += 3;
        return KCalcToken::TokenCode::IntegerDivision;
    }

    if (s.startsWith(GRADIAN_STR)) {
        index += 3;
        return KCalcToken::TokenCode::Gradian;
    }

    if (s.startsWith(RADIAN_STR)) {
        index += 3;
        return KCalcToken::TokenCode::Radian;
    }

    if (s.startsWith(RSH_STR)) {
        index += 2;
        return KCalcToken::TokenCode::Rsh;
    }
    if (s.startsWith(LSH_STR)) {
        index += 2;
        return KCalcToken::TokenCode::Lsh;
    }
    if (s.startsWith(TWO_S_COMP_STR)) {
        index += 2;
        return KCalcToken::TokenCode::TwosComplement;
    }

    if (s.startsWith(RECIPROCAL_STR)) {
        index += 2;
        return KCalcToken::TokenCode::Reciprocal;
    }
    if (s.startsWith(LN_STR)) {
        index += 2;
        return KCalcToken::TokenCode::Ln;
    }
    if (s.startsWith(RE_STR)) {
        index += 2;
        return KCalcToken::TokenCode::Re;
    }
    if (s.startsWith(IM_STR)) {
        index += 2;
        return KCalcToken::TokenCode::Im;
    }

    if (index + 2 <= buffer.size()) {
        s = buffer.sliced(index, 2);
        if (constantSymbolToValue_(s)) {
            index += 2;
            return KCalcToken::TokenCode::KNumber;
        }
    }
    if (s.startsWith(ONE_S_COMP_STR)) {
        index++;
        return KCalcToken::TokenCode::OnesComplement;
    }
    if (s.startsWith(FACTORIAL_STR)) {
        index++;
        return KCalcToken::TokenCode::Factorial;
    }
    if (s.startsWith(DOUBLE_FACTORIAL_STR)) {
        index++;
        return KCalcToken::TokenCode::DoubleFactorial;
    }

    s = buffer.sliced(index, 1);

    if ((A_LOWER_CASE_STR <= s.first(1) && s.first(1) <= F_LOWER_CASE_STR)) {
        if (m_numeralMode && base == 16) {
            QRegularExpressionMatch match;
            int numIndex = buffer.indexOf(HEX_NUMBER_DIGITS_REGEX, index, &match);
            if (match.captured().size() > 16 || numIndex != index) {
                return KCalcToken::TokenCode::InvalidToken;
            }
            token_KNumber_ = HEX_NUMBER_PREFIX_STR + match.captured();
            index += match.captured().size();
            return KCalcToken::TokenCode::KNumber;
        }
    }

    if (constantSymbolToValue_(s)) {
        index++;
        return KCalcToken::TokenCode::KNumber;
    }

    parsing_Result_ = INVALID_TOKEN;
    return KCalcToken::TokenCode::InvalidToken;
}

//------------------------------------------------------------------------------
// Name: stringToTokenQueue
// Desc: parses string into the token Queue
//------------------------------------------------------------------------------
KCalcParser::ParsingResult KCalcParser::stringToTokenQueue(const QString &buffer, int base, QQueue<KCalcToken> &tokenQueue, int &errorIndex)
{
    tokenQueue.clear();
    m_inputHasConstants = false;
    int buffer_index = 0;
    int buffer_size = buffer.size();
    qCDebug(KCALC_LOG) << "Parsing string to TokenQueue";
    qCDebug(KCALC_LOG) << "Buffer string to parse: " << buffer;

    KCalcToken::TokenCode tokenCode = KCalcToken::TokenCode::InvalidToken;

    if (buffer_size == 0) {
        errorIndex = -1;
        parsing_Result_ = EMPTY;
        return ParsingResult::EMPTY;
    }

    while (buffer_index < buffer_size) {
        tokenCode = KCalcToken::TokenCode::InvalidToken;
        KNumber operand;

        tokenCode = stringToToken(buffer, buffer_index, base);

        if (tokenCode == KCalcToken::TokenCode::InvalidToken) {
            parsing_Result_ = INVALID_TOKEN;
            errorIndex = buffer_index; // this indicates where the error was found
            return ParsingResult::INVALID_TOKEN; // in the input string
        }

        if (tokenCode == KCalcToken::TokenCode::Stub) {
            continue;
        }

        if (tokenCode == KCalcToken::TokenCode::KNumber) {
            qCDebug(KCALC_LOG) << "String KNumber converted: " << token_KNumber_;
            token_KNumber_.replace(COMMA_STR, KNumber::decimalSeparator());
            token_KNumber_.replace(POINT_STR, KNumber::decimalSeparator());
            operand = KNumber(token_KNumber_);

            tokenQueue.enqueue(KCalcToken(operand, buffer_index));
        } else {
            tokenQueue.enqueue(KCalcToken(tokenCode, buffer_index));
            qCDebug(KCALC_LOG) << "KCalcToken converted with code: " << tokenCode;
        }

        qCDebug(KCALC_LOG) << "Parsing index after this previous step: " << buffer_index;
    }

    qCDebug(KCALC_LOG) << "Parsing done, index after parsing: " << buffer_index;

    if (tokenQueue.length() > 2) {
        return ParsingResult::SUCCESS;
    } else if (tokenQueue.length() == 2) {
        if (!m_inputHasConstants && tokenQueue.first().getTokenCode() == KCalcToken::TokenCode::Minus && tokenQueue.at(1).isKNumber()) {
            return ParsingResult::SUCCESS_SINGLE_KNUMBER;
        } else {
            return ParsingResult::SUCCESS;
        }
    } else if (tokenQueue.length() == 1) {
        if (!m_inputHasConstants && tokenQueue.first().isKNumber()) {
            return ParsingResult::SUCCESS_SINGLE_KNUMBER;
        } else {
            return ParsingResult::SUCCESS;
        }
    } else {
        return ParsingResult::EMPTY;
    }
}

//------------------------------------------------------------------------------
// Name: setTrigonometricMode
// Desc:
//------------------------------------------------------------------------------
void KCalcParser::setTrigonometricMode(int mode)
{
    trigonometric_Mode_ = mode;
}

//------------------------------------------------------------------------------
// Name: getTrigonometricMode
// Desc:
//------------------------------------------------------------------------------
int KCalcParser::getTrigonometricMode()
{
    return trigonometric_Mode_;
}

void KCalcParser::setNumeralMode(bool numeralMode)
{
    m_numeralMode = numeralMode;
}

bool KCalcParser::getNumeralMode() const
{
    return m_numeralMode;
}

//------------------------------------------------------------------------------
// Name: getParsingResult
// Desc:
//------------------------------------------------------------------------------
KCalcParser::ParsingResult KCalcParser::getParsingResult()
{
    return parsing_Result_;
}

//------------------------------------------------------------------------------
// Name: TokenToString
// Desc:
//------------------------------------------------------------------------------
const QString KCalcParser::TokenToString(KCalcToken::TokenCode tokenCode)
{
    switch (tokenCode) {
    case KCalcToken::TokenCode::DecimalPoint:
        return DECIMAL_POINT_STR;
        break;
    case KCalcToken::TokenCode::Plus:
        return PLUS_STR;
        break;
    case KCalcToken::TokenCode::Minus:
        return HYPHEN_MINUS_STR;
        break;
    case KCalcToken::TokenCode::Division:
        return DIVISION_STR;
        break;
    case KCalcToken::TokenCode::Slash:
        return SLASH_STR;
        break;
    case KCalcToken::TokenCode::Multiplication:
        return MULTIPLICATION_STR;
        break;
    case KCalcToken::TokenCode::Dot:
        return DOT_STR;
        break;
    case KCalcToken::TokenCode::Asterisk:
        return ASTERISK_STR;
        break;
    case KCalcToken::TokenCode::Percentage:
        return PERCENTAGE_STR;
        break;
    case KCalcToken::TokenCode::OpeningParenthesis:
        return OPENING_PARENTHESIS_STR;
        break;
    case KCalcToken::TokenCode::ClosingParenthesis:
        return CLOSING_PARENTHESIS_STR;
        break;
    case KCalcToken::TokenCode::Square:
        return SQUARE_STR;
        break;
    case KCalcToken::TokenCode::Cube:
        return CUBE_STR;
        break;
    case KCalcToken::TokenCode::SquareRoot:
        return SQUARE_ROOT_STR;
        break;
    case KCalcToken::TokenCode::CubicRoot:
        return CUBIC_ROOT_STR;
        break;
    case KCalcToken::TokenCode::Degree:
        return DEGREE_STR;
        break;
    case KCalcToken::TokenCode::Gradian:
        return GRADIAN_STR;
        break;
    case KCalcToken::TokenCode::Radian:
        return RADIAN_STR;
        break;
    case KCalcToken::TokenCode::Sin:
    case KCalcToken::TokenCode::SinRad:
    case KCalcToken::TokenCode::SinGrad:
    case KCalcToken::TokenCode::SinDeg:
        return SIN_STR;
        break;
    case KCalcToken::TokenCode::Cos:
    case KCalcToken::TokenCode::CosRad:
    case KCalcToken::TokenCode::CosGrad:
    case KCalcToken::TokenCode::CosDeg:
        return COS_STR;
        break;
    case KCalcToken::TokenCode::Tan:
    case KCalcToken::TokenCode::TanRad:
    case KCalcToken::TokenCode::TanGrad:
    case KCalcToken::TokenCode::TanDeg:
        return TAN_STR;
        break;
    case KCalcToken::TokenCode::Asin:
    case KCalcToken::TokenCode::AsinRad:
    case KCalcToken::TokenCode::AsinDeg:
    case KCalcToken::TokenCode::AsinGrad:
        return ASIN_STR;
        break;
    case KCalcToken::TokenCode::Acos:
    case KCalcToken::TokenCode::AcosRad:
    case KCalcToken::TokenCode::AcosDeg:
    case KCalcToken::TokenCode::AcosGrad:
        return ACOS_STR;
        break;
    case KCalcToken::TokenCode::Atan:
    case KCalcToken::TokenCode::AtanRad:
    case KCalcToken::TokenCode::AtanDeg:
    case KCalcToken::TokenCode::AtanGrad:
        return ATAN_STR;
        break;
    case KCalcToken::TokenCode::Sinh:
        return SINH_STR;
        break;
    case KCalcToken::TokenCode::Cosh:
        return COSH_STR;
        break;
    case KCalcToken::TokenCode::Tanh:
        return TANH_STR;
        break;
    case KCalcToken::TokenCode::Asinh:
        return ASINH_STR;
        break;
    case KCalcToken::TokenCode::Acosh:
        return ACOSH_STR;
        break;
    case KCalcToken::TokenCode::Atanh:
        return ATANH_STR;
        break;
    case KCalcToken::TokenCode::E:
        return E_STR;
        break;
    case KCalcToken::TokenCode::Pi:
        return PI_STR;
        break;
    case KCalcToken::TokenCode::Phi:
        return PHI_STR;
        break;
    case KCalcToken::TokenCode::I:
        return I_STR;
        break;
    case KCalcToken::TokenCode::Polar:
        return ANGLE_STR;
        break;
    case KCalcToken::TokenCode::PosInfinity:
        return POS_INFINITY_STR;
        break;
    case KCalcToken::TokenCode::NegInfinity:
        return NEG_INFINITY_STR;
        break;
    case KCalcToken::TokenCode::VacuumPermitivity:
        return VACUUM_PERMITIVITY_STR;
        break;
    case KCalcToken::TokenCode::VacuumPermeability:
        return VACUUM_PERMEABILITY_STR;
        break;
    case KCalcToken::TokenCode::VacuumImpedance:
        return VACUUM_IMPEDANCE_STR;
        break;
    case KCalcToken::TokenCode::PlanckSConstant:
        return PLANCK_S_CONSTANT_STR;
        break;
    case KCalcToken::TokenCode::PlanckOver2Pi:
        return PLANCK_S_OVER_2PI_STR;
        break;
    case KCalcToken::TokenCode::Exp:
        return EXP_STR;
        break;
    case KCalcToken::TokenCode::Exp10:
        return EXP_10_STR;
        break;
    case KCalcToken::TokenCode::Power:
        return POWER_STR;
        break;
    case KCalcToken::TokenCode::PowerRoot:
        return POWER_ROOT_STR;
        break;
    case KCalcToken::TokenCode::Factorial:
        return FACTORIAL_STR;
        break;
    case KCalcToken::TokenCode::DoubleFactorial:
        return DOUBLE_FACTORIAL_STR;
        break;
    case KCalcToken::TokenCode::Gamma:
        return GAMMA_STR;
        break;
    case KCalcToken::TokenCode::InvertSign:
        return INVERT_SIGN_STR;
        break;
    case KCalcToken::TokenCode::Ln:
        return LN_STR;
        break;
    case KCalcToken::TokenCode::Log10:
        return LOG_10_STR;
        break;
    case KCalcToken::TokenCode::Reciprocal:
        return RECIPROCAL_STR;
        break;
    case KCalcToken::TokenCode::Binomial:
        return THIN_SPACE_STR + BINOMIAL_STR + THIN_SPACE_STR;
        break;
    case KCalcToken::TokenCode::Modulo:
        return THIN_SPACE_STR + MODULO_STR + THIN_SPACE_STR;
        break;
    case KCalcToken::TokenCode::IntegerDivision:
        return THIN_SPACE_STR + INTEGER_DIVISION_STR + THIN_SPACE_STR;
        break;
    case KCalcToken::TokenCode::And:
        return THIN_SPACE_STR + AND_STR + THIN_SPACE_STR;
        break;
    case KCalcToken::TokenCode::Or:
        return THIN_SPACE_STR + OR_STR + THIN_SPACE_STR;
        break;
    case KCalcToken::TokenCode::Xor:
        return THIN_SPACE_STR + XOR_STR + THIN_SPACE_STR;
        break;
    case KCalcToken::TokenCode::OnesComplement:
        return ONE_S_COMP_STR;
        break;
    case KCalcToken::TokenCode::TwosComplement:
        return TWO_S_COMP_STR;
        break;
    case KCalcToken::TokenCode::Rsh:
        return THIN_SPACE_STR + RSH_STR + THIN_SPACE_STR;
        break;
    case KCalcToken::TokenCode::Lsh:
        return THIN_SPACE_STR + LSH_STR + THIN_SPACE_STR;
        break;
    case KCalcToken::TokenCode::Ans:
        return THIN_SPACE_STR + ANS_STR + THIN_SPACE_STR;
        break;
    case KCalcToken::TokenCode::Equal:
        return EQUAL_STR;
        break;
    default:
        break;
    }
    return ERROR_STR;
}

int KCalcParser::loadConstants(const QDomDocument &doc)
{
    QDomNode n = doc.documentElement().firstChild();

    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull() && e.tagName() == QLatin1String("constant")) {
            const QString value = e.attributeNode(QStringLiteral("value")).value();
            const QString symbol = e.attributeNode(QStringLiteral("symbol")).value();

            constants_.insert(symbol, value);
        }
        n = n.nextSibling();
    }

    return 0;
}

//------------------------------------------------------------------------------
// Name: constantSymbolToValue_
// Desc: tries to find a given constant in the stored constants list, returns
//       true if found, loads value in token_KNumber_
//------------------------------------------------------------------------------
bool KCalcParser::constantSymbolToValue_(const QString &constantSymbol)
{
    if (constants_.contains(constantSymbol)) {
        token_KNumber_ = constants_.value(constantSymbol);
        m_inputHasConstants = true;
        return true;
    }

    return false;
}
