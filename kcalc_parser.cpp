/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_parser.h"
#include "kcalc_debug.h"
#include "kcalc_token.h"
#include "knumber/knumber.h"

#include <QDebug>
#include <QDomDocument>
#include <QLocale>
#include <QQueue>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QString>

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
        return KCalcToken::TokenCode::STUB;
    }

    if (s.startsWith(HEX_NUMBER_PREFIX_STR)) {
        QRegularExpressionMatch match;
        int numIndex = buffer.indexOf(HEX_NUMBER_DIGITS_REGEX, index + HEX_NUMBER_PREFIX_STR.length(), &match);
        if (numIndex == index + HEX_NUMBER_PREFIX_STR.length()) {
            token_KNumber_ = HEX_NUMBER_PREFIX_STR + match.captured();
            if (match.captured().size() > 16) {
                return KCalcToken::TokenCode::INVALID_TOKEN;
            }
            index += HEX_NUMBER_PREFIX_STR.length();
            index += match.captured().size();
            return KCalcToken::TokenCode::KNUMBER;
        } else {
            return KCalcToken::TokenCode::INVALID_TOKEN;
        }
    }

    if (s.startsWith(BINARY_NUMBER_PREFIX_STR)) {
        QRegularExpressionMatch match;
        int numIndex = buffer.indexOf(BINARY_NUMBER_DIGITS_REGEX, index + BINARY_NUMBER_PREFIX_STR.length(), &match);
        if (numIndex == index + BINARY_NUMBER_PREFIX_STR.length()) {
            token_KNumber_ = BINARY_NUMBER_PREFIX_STR + match.captured();
            if (match.captured().size() > 64) {
                return KCalcToken::TokenCode::INVALID_TOKEN;
            }
            index += BINARY_NUMBER_PREFIX_STR.length();
            index += match.captured().size();
            return KCalcToken::TokenCode::KNUMBER;
        } else {
            return KCalcToken::TokenCode::INVALID_TOKEN;
        }
    }

    if (s.startsWith(OCTAL_NUMBER_PREFIX_STR)) {
        QRegularExpressionMatch match;
        int numIndex = buffer.indexOf(OCTAL_NUMBER_DIGITS_REGEX, index + OCTAL_NUMBER_PREFIX_STR.length(), &match);
        if (numIndex == index + OCTAL_NUMBER_PREFIX_STR.length()) {
            token_KNumber_ = OCTAL_NUMBER_PREFIX_STR_C_STYLE + match.captured();
            if (match.captured().size() > 21) {
                return KCalcToken::TokenCode::INVALID_TOKEN;
            }
            index += OCTAL_NUMBER_PREFIX_STR.length();
            index += match.captured().size();
            return KCalcToken::TokenCode::KNUMBER;
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
                return KCalcToken::TokenCode::INVALID_TOKEN;
            }
            break;
        case 8:
            numIndex = buffer.indexOf(OCTAL_NUMBER_DIGITS_REGEX, index, &match);
            token_KNumber_ = OCTAL_NUMBER_PREFIX_STR_C_STYLE;
            if (match.captured().size() > 21) {
                return KCalcToken::TokenCode::INVALID_TOKEN;
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
                return KCalcToken::TokenCode::INVALID_TOKEN;
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
            return KCalcToken::TokenCode::KNUMBER;
        } else {
            return KCalcToken::TokenCode::INVALID_TOKEN;
        }
    }

    if (s.startsWith(PLUS_STR)) {
        index++;
        return KCalcToken::TokenCode::PLUS;
    }
    if (s.startsWith(HYPHEN_MINUS_STR) || s.startsWith(MINUS_SIGN_STR)) {
        index++;
        return KCalcToken::TokenCode::MINUS;
    }
    if (s.startsWith(EQUAL_STR)) {
        index++;
        return KCalcToken::TokenCode::EQUAL;
    }
    if (s.startsWith(SQUARE_STR)) {
        index++;
        return KCalcToken::TokenCode::SQUARE;
    }
    if (s.startsWith(CUBE_STR)) {
        index++;
        return KCalcToken::TokenCode::CUBE;
    }
    if (s.startsWith(POWER_STR)) {
        index++;
        return KCalcToken::TokenCode::POWER;
    }
    if (s.startsWith(POWER_ROOT_STR)) {
        index++;
        return KCalcToken::TokenCode::POWER_ROOT;
    }
    if (s.startsWith(EXP_10_STR)) {
        index++;
        return KCalcToken::TokenCode::EXP_10;
    }
    if (s.startsWith(PERCENTAGE_STR)) {
        index++;
        return KCalcToken::TokenCode::PERCENTAGE;
    }
    if (s.startsWith(PERMILLE_STR)) {
        index++;
        return KCalcToken::TokenCode::PERMILLE;
    }
    if (s.startsWith(DIVISION_STR) || s.startsWith(SLASH_STR) || s.startsWith(DIVISION_SLASH_STR)) {
        index++;
        return KCalcToken::TokenCode::DIVISION;
    }
    if (s.startsWith(GAMMA_STR)) {
        index++;
        return KCalcToken::TokenCode::GAMMA;
    }
    if (s.startsWith(SQUARE_ROOT_STR)) {
        index++;
        return KCalcToken::TokenCode::SQUARE_ROOT;
    }
    if (s.startsWith(CUBIC_ROOT_STR)) {
        index++;
        return KCalcToken::TokenCode::CUBIC_ROOT;
    }
    if (s.startsWith(OPENING_PARENTHESIS_STR)) {
        index++;
        return KCalcToken::TokenCode::OPENING_PARENTHESIS;
    }
    if (s.startsWith(CLOSING_PARENTHESIS_STR)) {
        index++;
        return KCalcToken::TokenCode::CLOSING_PARENTHESIS;
    }
    if (s.startsWith(INVERT_SIGN_STR)) {
        index++;
        return KCalcToken::TokenCode::INVERT_SIGN;
    }

    if (s.startsWith(MULTIPLICATION_STR) || s.startsWith(DOT_STR) || s.startsWith(ASTERISK_STR)) {
        index++;
        return KCalcToken::TokenCode::MULTIPLICATION;
    }

    if (s.startsWith(DEGREE_STR)) {
        index++;
        return KCalcToken::TokenCode::DEGREE;
    }
    if (s.startsWith(AND_STR)) {
        index++;
        return KCalcToken::TokenCode::AND;
    }
    if (s.startsWith(OR_STR)) {
        index++;
        return KCalcToken::TokenCode::OR;
    }
    if (s.startsWith(XOR_STR)) {
        index++;
        return KCalcToken::TokenCode::XOR;
    }
    if (s.startsWith(XOR_LETTER_STR, Qt::CaseSensitivity::CaseInsensitive)) {
        index += 3;
        return KCalcToken::TokenCode::XOR;
    }

    if (s.startsWith(ASINH_STR)) {
        index += 5;
        return KCalcToken::TokenCode::ASINH;
    }
    if (s.startsWith(ACOSH_STR)) {
        index += 5;
        return KCalcToken::TokenCode::ACOSH;
    }
    if (s.startsWith(ATANH_STR)) {
        index += 5;
        return KCalcToken::TokenCode::ATANH;
    }

    if (s.startsWith(ASIN_STR)) {
        index += 4;
        switch (trigonometric_Mode_) {
        case RADIANS:
            return KCalcToken::TokenCode::ASIN_RAD;
            break;
        case GRADIANS:
            return KCalcToken::TokenCode::ASIN_GRAD;
            break;
        case DEGREES:
            return KCalcToken::TokenCode::ASIN_DEG;
            break;
        default:
            break;
        }
    }
    if (s.startsWith(ACOS_STR)) {
        index += 4;
        switch (trigonometric_Mode_) {
        case RADIANS:
            return KCalcToken::TokenCode::ACOS_RAD;
            break;
        case GRADIANS:
            return KCalcToken::TokenCode::ACOS_GRAD;
            break;
        case DEGREES:
            return KCalcToken::TokenCode::ACOS_DEG;
            break;
        default:
            break;
        }
    }
    if (s.startsWith(ATAN_STR)) {
        index += 4;
        switch (trigonometric_Mode_) {
        case RADIANS:
            return KCalcToken::TokenCode::ATAN_RAD;
            break;
        case GRADIANS:
            return KCalcToken::TokenCode::ATAN_GRAD;
            break;
        case DEGREES:
            return KCalcToken::TokenCode::ATAN_DEG;
            break;
        default:
            break;
        }
    }

    if (s.startsWith(SINH_STR)) {
        index += 4;
        return KCalcToken::TokenCode::SINH;
    }
    if (s.startsWith(COSH_STR)) {
        index += 4;
        return KCalcToken::TokenCode::COSH;
    }
    if (s.startsWith(TANH_STR)) {
        index += 4;
        return KCalcToken::TokenCode::TANH;
    }

    if (s.startsWith(SIN_STR)) {
        index += 3;
        switch (trigonometric_Mode_) {
        case RADIANS:
            return KCalcToken::TokenCode::SIN_RAD;
            break;
        case GRADIANS:
            return KCalcToken::TokenCode::SIN_GRAD;
            break;
        case DEGREES:
            return KCalcToken::TokenCode::SIN_DEG;
            break;
        default:
            break;
        }
    }
    if (s.startsWith(COS_STR)) {
        index += 3;
        switch (trigonometric_Mode_) {
        case RADIANS:
            return KCalcToken::TokenCode::COS_RAD;
            break;
        case GRADIANS:
            return KCalcToken::TokenCode::COS_GRAD;
            break;
        case DEGREES:
            return KCalcToken::TokenCode::COS_DEG;
            break;
        default:
            break;
        }
    }
    if (s.startsWith(TAN_STR)) {
        index += 3;
        switch (trigonometric_Mode_) {
        case RADIANS:
            return KCalcToken::TokenCode::TAN_RAD;
            break;
        case GRADIANS:
            return KCalcToken::TokenCode::TAN_GRAD;
            break;
        case DEGREES:
            return KCalcToken::TokenCode::TAN_DEG;
            break;
        default:
            break;
        }
    }

    if (s.startsWith(EXP_STR)) {
        index += 3;
        return KCalcToken::TokenCode::EXP;
    }

    if (s.startsWith(LOG_10_STR)) {
        index += 3;
        return KCalcToken::TokenCode::LOG_10;
    }

    if (s.startsWith(ANS_STR)) {
        index += 3;
        return KCalcToken::TokenCode::ANS;
    }

    if (s.startsWith(BINOMIAL_STR)) {
        index += 3;
        return KCalcToken::TokenCode::BINOMIAL;
    }

    if (s.startsWith(MODULO_STR)) {
        index += 3;
        return KCalcToken::TokenCode::MODULO;
    }

    if (s.startsWith(INTEGER_DIVISION_STR)) {
        index += 3;
        return KCalcToken::TokenCode::INTEGER_DIVISION;
    }

    if (s.startsWith(GRADIAN_STR)) {
        index += 3;
        return KCalcToken::TokenCode::GRADIAN;
    }

    if (s.startsWith(RADIAN_STR)) {
        index += 3;
        return KCalcToken::TokenCode::RADIAN;
    }

    if (s.startsWith(RSH_STR)) {
        index += 2;
        return KCalcToken::TokenCode::RSH;
    }
    if (s.startsWith(LSH_STR)) {
        index += 2;
        return KCalcToken::TokenCode::LSH;
    }
    if (s.startsWith(TWO_S_COMP_STR)) {
        index += 2;
        return KCalcToken::TokenCode::TWO_S_COMPLEMENT;
    }

    if (s.startsWith(RECIPROCAL_STR)) {
        index += 2;
        return KCalcToken::TokenCode::RECIPROCAL;
    }
    if (s.startsWith(LN_STR)) {
        index += 2;
        return KCalcToken::TokenCode::LN;
    }

    if (index + 2 <= buffer.size()) {
        s = buffer.sliced(index, 2);
        if (constantSymbolToValue_(s)) {
            index += 2;
            return KCalcToken::TokenCode::KNUMBER;
        }
    }
    if (s.startsWith(ONE_S_COMP_STR)) {
        index++;
        return KCalcToken::TokenCode::ONE_S_COMPLEMENT;
    }
    if (s.startsWith(FACTORIAL_STR)) {
        index++;
        return KCalcToken::TokenCode::FACTORIAL;
    }
    if (s.startsWith(DOUBLE_FACTORIAL_STR)) {
        index++;
        return KCalcToken::TokenCode::DOUBLE_FACTORIAL;
    }

    s = buffer.sliced(index, 1);

    if ((A_LOWER_CASE_STR <= s.first(1) && s.first(1) <= F_LOWER_CASE_STR)) {
        if (m_numeralMode && base == 16) {
            QRegularExpressionMatch match;
            int numIndex = buffer.indexOf(HEX_NUMBER_DIGITS_REGEX, index, &match);
            if (match.captured().size() > 16 || numIndex != index) {
                return KCalcToken::TokenCode::INVALID_TOKEN;
            }
            token_KNumber_ = HEX_NUMBER_PREFIX_STR + match.captured();
            index += match.captured().size();
            return KCalcToken::TokenCode::KNUMBER;
        }
    }

    if (constantSymbolToValue_(s)) {
        index++;
        return KCalcToken::TokenCode::KNUMBER;
    }

    parsing_Result_ = INVALID_TOKEN;
    return KCalcToken::TokenCode::INVALID_TOKEN;
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

    KCalcToken::TokenCode tokenCode = KCalcToken::TokenCode::INVALID_TOKEN;

    if (buffer_size == 0) {
        errorIndex = -1;
        parsing_Result_ = EMPTY;
        return ParsingResult::EMPTY;
    }

    while (buffer_index < buffer_size) {
        tokenCode = KCalcToken::TokenCode::INVALID_TOKEN;
        KNumber operand;

        tokenCode = stringToToken(buffer, buffer_index, base);

        if (tokenCode == KCalcToken::TokenCode::INVALID_TOKEN) {
            parsing_Result_ = INVALID_TOKEN;
            errorIndex = buffer_index; // this indicates where the error was found
            return ParsingResult::INVALID_TOKEN; // in the input string
        }

        if (tokenCode == KCalcToken::TokenCode::STUB) {
            continue;
        }

        if (tokenCode == KCalcToken::TokenCode::KNUMBER) {
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
        if (!m_inputHasConstants && tokenQueue.first().getTokenCode() == KCalcToken::TokenCode::MINUS && tokenQueue.at(1).isKNumber()) {
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
    case KCalcToken::TokenCode::DECIMAL_POINT:
        return DECIMAL_POINT_STR;
        break;
    case KCalcToken::TokenCode::PLUS:
        return PLUS_STR;
        break;
    case KCalcToken::TokenCode::MINUS:
        return HYPHEN_MINUS_STR;
        break;
    case KCalcToken::TokenCode::DIVISION:
        return DIVISION_STR;
        break;
    case KCalcToken::TokenCode::SLASH:
        return SLASH_STR;
        break;
    case KCalcToken::TokenCode::MULTIPLICATION:
        return MULTIPLICATION_STR;
        break;
    case KCalcToken::TokenCode::DOT:
        return DOT_STR;
        break;
    case KCalcToken::TokenCode::ASTERISK:
        return ASTERISK_STR;
        break;
    case KCalcToken::TokenCode::PERCENTAGE:
        return PERCENTAGE_STR;
        break;
    case KCalcToken::TokenCode::OPENING_PARENTHESIS:
        return OPENING_PARENTHESIS_STR;
        break;
    case KCalcToken::TokenCode::CLOSING_PARENTHESIS:
        return CLOSING_PARENTHESIS_STR;
        break;
    case KCalcToken::TokenCode::SQUARE:
        return SQUARE_STR;
        break;
    case KCalcToken::TokenCode::CUBE:
        return CUBE_STR;
        break;
    case KCalcToken::TokenCode::SQUARE_ROOT:
        return SQUARE_ROOT_STR;
        break;
    case KCalcToken::TokenCode::CUBIC_ROOT:
        return CUBIC_ROOT_STR;
        break;
    case KCalcToken::TokenCode::DEGREE:
        return DEGREE_STR;
        break;
    case KCalcToken::TokenCode::GRADIAN:
        return GRADIAN_STR;
        break;
    case KCalcToken::TokenCode::RADIAN:
        return RADIAN_STR;
        break;
    case KCalcToken::TokenCode::SIN:
    case KCalcToken::TokenCode::SIN_RAD:
    case KCalcToken::TokenCode::SIN_GRAD:
    case KCalcToken::TokenCode::SIN_DEG:
        return SIN_STR;
        break;
    case KCalcToken::TokenCode::COS:
    case KCalcToken::TokenCode::COS_RAD:
    case KCalcToken::TokenCode::COS_GRAD:
    case KCalcToken::TokenCode::COS_DEG:
        return COS_STR;
        break;
    case KCalcToken::TokenCode::TAN:
    case KCalcToken::TokenCode::TAN_RAD:
    case KCalcToken::TokenCode::TAN_GRAD:
    case KCalcToken::TokenCode::TAN_DEG:
        return TAN_STR;
        break;
    case KCalcToken::TokenCode::ASIN:
    case KCalcToken::TokenCode::ASIN_RAD:
    case KCalcToken::TokenCode::ASIN_DEG:
    case KCalcToken::TokenCode::ASIN_GRAD:
        return ASIN_STR;
        break;
    case KCalcToken::TokenCode::ACOS:
    case KCalcToken::TokenCode::ACOS_RAD:
    case KCalcToken::TokenCode::ACOS_DEG:
    case KCalcToken::TokenCode::ACOS_GRAD:
        return ACOS_STR;
        break;
    case KCalcToken::TokenCode::ATAN:
    case KCalcToken::TokenCode::ATAN_RAD:
    case KCalcToken::TokenCode::ATAN_DEG:
    case KCalcToken::TokenCode::ATAN_GRAD:
        return ATAN_STR;
        break;
    case KCalcToken::TokenCode::SINH:
        return SINH_STR;
        break;
    case KCalcToken::TokenCode::COSH:
        return COSH_STR;
        break;
    case KCalcToken::TokenCode::TANH:
        return TANH_STR;
        break;
    case KCalcToken::TokenCode::ASINH:
        return ASINH_STR;
        break;
    case KCalcToken::TokenCode::ACOSH:
        return ACOSH_STR;
        break;
    case KCalcToken::TokenCode::ATANH:
        return ATANH_STR;
        break;
    case KCalcToken::TokenCode::E:
        return E_STR;
        break;
    case KCalcToken::TokenCode::PI:
        return PI_STR;
        break;
    case KCalcToken::TokenCode::PHI:
        return PHI_STR;
        break;
    case KCalcToken::TokenCode::I:
        return I_STR;
        break;
    case KCalcToken::TokenCode::POS_INFINITY:
        return POS_INFINITY_STR;
        break;
    case KCalcToken::TokenCode::NEG_INFINITY:
        return NEG_INFINITY_STR;
        break;
    case KCalcToken::TokenCode::VACUUM_PERMITIVITY:
        return VACUUM_PERMITIVITY_STR;
        break;
    case KCalcToken::TokenCode::VACUUM_PERMEABILITY:
        return VACUUM_PERMEABILITY_STR;
        break;
    case KCalcToken::TokenCode::VACUUM_IMPEDANCE:
        return VACUUM_IMPEDANCE_STR;
        break;
    case KCalcToken::TokenCode::PLANCK_S_CONSTANT:
        return PLANCK_S_CONSTANT_STR;
        break;
    case KCalcToken::TokenCode::PLANCK_OVER_2PI:
        return PLANCK_S_OVER_2PI_STR;
        break;
    case KCalcToken::TokenCode::EXP:
        return EXP_STR;
        break;
    case KCalcToken::TokenCode::EXP_10:
        return EXP_10_STR;
        break;
    case KCalcToken::TokenCode::POWER:
        return POWER_STR;
        break;
    case KCalcToken::TokenCode::POWER_ROOT:
        return POWER_ROOT_STR;
        break;
    case KCalcToken::TokenCode::FACTORIAL:
        return FACTORIAL_STR;
        break;
    case KCalcToken::TokenCode::DOUBLE_FACTORIAL:
        return DOUBLE_FACTORIAL_STR;
        break;
    case KCalcToken::TokenCode::GAMMA:
        return GAMMA_STR;
        break;
    case KCalcToken::TokenCode::INVERT_SIGN:
        return INVERT_SIGN_STR;
        break;
    case KCalcToken::TokenCode::LN:
        return LN_STR;
        break;
    case KCalcToken::TokenCode::LOG_10:
        return LOG_10_STR;
        break;
    case KCalcToken::TokenCode::RECIPROCAL:
        return RECIPROCAL_STR;
        break;
    case KCalcToken::TokenCode::BINOMIAL:
        return THIN_SPACE_STR + BINOMIAL_STR + THIN_SPACE_STR;
        break;
    case KCalcToken::TokenCode::MODULO:
        return THIN_SPACE_STR + MODULO_STR + THIN_SPACE_STR;
        break;
    case KCalcToken::TokenCode::INTEGER_DIVISION:
        return THIN_SPACE_STR + INTEGER_DIVISION_STR + THIN_SPACE_STR;
        break;
    case KCalcToken::TokenCode::AND:
        return THIN_SPACE_STR + AND_STR + THIN_SPACE_STR;
        break;
    case KCalcToken::TokenCode::OR:
        return THIN_SPACE_STR + OR_STR + THIN_SPACE_STR;
        break;
    case KCalcToken::TokenCode::XOR:
        return THIN_SPACE_STR + XOR_STR + THIN_SPACE_STR;
        break;
    case KCalcToken::TokenCode::ONE_S_COMPLEMENT:
        return ONE_S_COMP_STR;
        break;
    case KCalcToken::TokenCode::TWO_S_COMPLEMENT:
        return TWO_S_COMP_STR;
        break;
    case KCalcToken::TokenCode::RSH:
        return THIN_SPACE_STR + RSH_STR + THIN_SPACE_STR;
        break;
    case KCalcToken::TokenCode::LSH:
        return THIN_SPACE_STR + LSH_STR + THIN_SPACE_STR;
        break;
    case KCalcToken::TokenCode::ANS:
        return THIN_SPACE_STR + ANS_STR + THIN_SPACE_STR;
        break;
    case KCalcToken::TokenCode::EQUAL:
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
    constant_ constant;

    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull() && e.tagName() == QLatin1String("constant")) {
            constant.value = e.attributeNode(QStringLiteral("value")).value();
            constant.symbol = e.attributeNode(QStringLiteral("symbol")).value();

            constants_.append(constant);
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
    QList<constant_>::const_iterator i;
    for (i = constants_.constBegin(); i != constants_.constEnd(); ++i) {
        if (i->symbol == constantSymbol) {
            token_KNumber_ = i->value;
            m_inputHasConstants = true;
            return true;
        }
    }

    return false;
}
