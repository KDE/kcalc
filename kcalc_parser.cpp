/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_parser.h"
#include "kcalc_debug.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>

const QString KCalcParser::binaryDigitsPattern = QStringLiteral("[0-1]{1,}");
const QString KCalcParser::octalDigitsPattern = QStringLiteral("[0-7]{1,}");
const QString KCalcParser::hexDigitsPattern = QStringLiteral("[0-9A-Fa-f]{1,}");

const QString KCalcParser::decimalNumberPattern = QLatin1String("(0*)((\\d*)[.,]?\\d+([e|E]([+-]?\\d+))?)");

const QRegularExpression KCalcParser::binaryNumberDigitsRegex = QRegularExpression(KCalcParser::binaryDigitsPattern);
const QRegularExpression KCalcParser::octalNumberDigitsRegex = QRegularExpression(KCalcParser::octalDigitsPattern);
const QRegularExpression KCalcParser::hexNumberDigitsRegex = QRegularExpression(KCalcParser::hexDigitsPattern);

const QRegularExpression KCalcParser::decimalNumberRegex = QRegularExpression(KCalcParser::decimalNumberPattern);

//------------------------------------------------------------------------------
// Name: KCalcParser
// Desc: constructor
//------------------------------------------------------------------------------
KCalcParser::KCalcParser() = default;

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
    if (s.startsWith(spaceStr) || s.startsWith(thinSpaceStr)) {
        index++;
        return KCalcToken::TokenCode::Stub;
    }

    if (s.startsWith(hexNumberPrefixStr)) {
        QRegularExpressionMatch match;
        int numIndex = buffer.indexOf(hexNumberDigitsRegex, index + hexNumberPrefixStr.length(), &match);
        if (numIndex == index + hexNumberPrefixStr.length()) {
            m_tokenKNumber = hexNumberPrefixStr + match.captured();
            if (match.captured().size() > 16) {
                return KCalcToken::TokenCode::InvalidToken;
            }
            index += hexNumberPrefixStr.length();
            index += match.captured().size();
            return KCalcToken::TokenCode::KNumber;
        } else {
            return KCalcToken::TokenCode::InvalidToken;
        }
    }

    if (s.startsWith(binaryNumberPrefixStr)) {
        QRegularExpressionMatch match;
        int numIndex = buffer.indexOf(binaryNumberDigitsRegex, index + binaryNumberPrefixStr.length(), &match);
        if (numIndex == index + binaryNumberPrefixStr.length()) {
            m_tokenKNumber = binaryNumberPrefixStr + match.captured();
            if (match.captured().size() > 64) {
                return KCalcToken::TokenCode::InvalidToken;
            }
            index += binaryNumberPrefixStr.length();
            index += match.captured().size();
            return KCalcToken::TokenCode::KNumber;
        } else {
            return KCalcToken::TokenCode::InvalidToken;
        }
    }

    if (s.startsWith(octalNumberPrefixStr)) {
        QRegularExpressionMatch match;
        int numIndex = buffer.indexOf(octalNumberDigitsRegex, index + octalNumberPrefixStr.length(), &match);
        if (numIndex == index + octalNumberPrefixStr.length()) {
            m_tokenKNumber = octalNumberPrefixStrCStyle + match.captured();
            if (match.captured().size() > 21) {
                return KCalcToken::TokenCode::InvalidToken;
            }
            index += octalNumberPrefixStr.length();
            index += match.captured().size();
            return KCalcToken::TokenCode::KNumber;
        }
        // Code execution continues, since an entry could
        // start with a 0 but not be a valid octal number
        // being a number in base 10 or 16
    }

    if ((aStr <= s.first(1) && s.first(1) <= fStr) || (zeroStr <= s.first(1) && s.first(1) <= nineStr) || (s.first(1) == commaStr || s.first(1) == pointStr)) {
        int numIndex = -1;
        QRegularExpressionMatch match;
        switch (base) {
        case 2:
            numIndex = buffer.indexOf(binaryNumberDigitsRegex, index, &match);
            m_tokenKNumber = binaryNumberPrefixStr;
            if (match.captured().size() > 64) {
                return KCalcToken::TokenCode::InvalidToken;
            }
            break;
        case 8:
            numIndex = buffer.indexOf(octalNumberDigitsRegex, index, &match);
            m_tokenKNumber = octalNumberPrefixStrCStyle;
            if (match.captured().size() > 21) {
                return KCalcToken::TokenCode::InvalidToken;
            }
            break;
        case 10:
            numIndex = buffer.indexOf(decimalNumberRegex, index, &match);
            m_tokenKNumber.clear();
            break;
        case 16:
            numIndex = buffer.indexOf(hexNumberDigitsRegex, index, &match);
            m_tokenKNumber = hexNumberPrefixStr;
            if (match.captured().size() > 16) {
                return KCalcToken::TokenCode::InvalidToken;
            }
            break;
        default:
            break;
        }
        if (numIndex == index) {
            if (base == 10) {
                m_tokenKNumber += match.captured(2);
            } else {
                m_tokenKNumber += match.captured();
            }
            index += match.captured().size();
            return KCalcToken::TokenCode::KNumber;
        } else {
            return KCalcToken::TokenCode::InvalidToken;
        }
    }

    if (angleStr == s.first(1)) {
        index++;
        switch (m_trigonometricMode) {
        case Radians:
            return KCalcToken::TokenCode::PolarRad;
        case Gradians:
            return KCalcToken::TokenCode::PolarGrad;
        case Degrees:
            return KCalcToken::TokenCode::PolarDeg;
        default:
            Q_UNREACHABLE();
            return KCalcToken::TokenCode::InvalidToken;
        }
    }

    if (s.startsWith(plusStr)) {
        index++;
        return KCalcToken::TokenCode::Plus;
    }
    if (s.startsWith(hyphenMinusStr) || s.startsWith(minusSignStr)) {
        index++;
        return KCalcToken::TokenCode::Minus;
    }
    if (s.startsWith(equalStr)) {
        index++;
        return KCalcToken::TokenCode::Equal;
    }
    if (s.startsWith(squareStr)) {
        index++;
        return KCalcToken::TokenCode::Square;
    }
    if (s.startsWith(cubeStr)) {
        index++;
        return KCalcToken::TokenCode::Cube;
    }
    if (s.startsWith(powerStr)) {
        index++;
        return KCalcToken::TokenCode::Power;
    }
    if (s.startsWith(powerRootStr)) {
        index++;
        return KCalcToken::TokenCode::PowerRoot;
    }
    if (s.startsWith(exp10Str)) {
        index++;
        return KCalcToken::TokenCode::Exp10;
    }
    if (s.startsWith(percentageStr)) {
        index++;
        return KCalcToken::TokenCode::Percentage;
    }
    if (s.startsWith(permilleStr)) {
        index++;
        return KCalcToken::TokenCode::Permille;
    }
    if (s.startsWith(iStr) && !s.startsWith(imStr)) {
        index++;
        m_tokenKNumber = iStr;
        return KCalcToken::TokenCode::KNumber;
    }
    if (s.startsWith(divisionStr) || s.startsWith(slashStr) || s.startsWith(divisionSlashStr)) {
        index++;
        return KCalcToken::TokenCode::Division;
    }
    if (s.startsWith(gammaStr)) {
        index++;
        return KCalcToken::TokenCode::Gamma;
    }
    if (s.startsWith(squareRootStr)) {
        index++;
        return KCalcToken::TokenCode::SquareRoot;
    }
    if (s.startsWith(cubicRootStr)) {
        index++;
        return KCalcToken::TokenCode::CubicRoot;
    }
    if (s.startsWith(openingParenthesisStr)) {
        index++;
        return KCalcToken::TokenCode::OpeningParenthesis;
    }
    if (s.startsWith(closingParenthesisStr)) {
        index++;
        return KCalcToken::TokenCode::ClosingParenthesis;
    }
    if (s.startsWith(invertSignStr)) {
        index++;
        return KCalcToken::TokenCode::InvertSign;
    }

    if (s.startsWith(multiplicationStr) || s.startsWith(dotStr) || s.startsWith(asteriskStr)) {
        index++;
        return KCalcToken::TokenCode::Multiplication;
    }

    if (s.startsWith(degreeStr)) {
        index++;
        return KCalcToken::TokenCode::Degree;
    }
    if (s.startsWith(andStr)) {
        index++;
        return KCalcToken::TokenCode::And;
    }
    if (s.startsWith(orStr)) {
        index++;
        return KCalcToken::TokenCode::Or;
    }
    if (s.startsWith(xorStr)) {
        index++;
        return KCalcToken::TokenCode::Xor;
    }
    if (s.startsWith(xorLetterStr, Qt::CaseSensitivity::CaseInsensitive)) {
        index += 3;
        return KCalcToken::TokenCode::Xor;
    }

    if (s.startsWith(asinhStr)) {
        index += 5;
        return KCalcToken::TokenCode::Asinh;
    }
    if (s.startsWith(acoshStr)) {
        index += 5;
        return KCalcToken::TokenCode::Acosh;
    }
    if (s.startsWith(atanhStr)) {
        index += 5;
        return KCalcToken::TokenCode::Atanh;
    }

    if (s.startsWith(asinStr)) {
        index += 4;
        switch (m_trigonometricMode) {
        case Radians:
            return KCalcToken::TokenCode::AsinRad;
        case Gradians:
            return KCalcToken::TokenCode::AsinGrad;
        case Degrees:
            return KCalcToken::TokenCode::AsinDeg;
        default:
            break;
        }
    }
    if (s.startsWith(acosStr)) {
        index += 4;
        switch (m_trigonometricMode) {
        case Radians:
            return KCalcToken::TokenCode::AcosRad;
        case Gradians:
            return KCalcToken::TokenCode::AcosGrad;
        case Degrees:
            return KCalcToken::TokenCode::AcosDeg;
        default:
            break;
        }
    }
    if (s.startsWith(atanStr)) {
        index += 4;
        switch (m_trigonometricMode) {
        case Radians:
            return KCalcToken::TokenCode::AtanRad;
        case Gradians:
            return KCalcToken::TokenCode::AtanGrad;
        case Degrees:
            return KCalcToken::TokenCode::AtanDeg;
        default:
            break;
        }
    }

    if (s.startsWith(sinhStr)) {
        index += 4;
        return KCalcToken::TokenCode::Sinh;
    }
    if (s.startsWith(coshStr)) {
        index += 4;
        return KCalcToken::TokenCode::Cosh;
    }
    if (s.startsWith(tanhStr)) {
        index += 4;
        return KCalcToken::TokenCode::Tanh;
    }
    if (s.startsWith(conjStr)) {
        index += 4;
        return KCalcToken::TokenCode::Conj;
    }

    if (s.startsWith(sinStr)) {
        index += 3;
        switch (m_trigonometricMode) {
        case Radians:
            return KCalcToken::TokenCode::SinRad;
        case Gradians:
            return KCalcToken::TokenCode::SinGrad;
        case Degrees:
            return KCalcToken::TokenCode::SinDeg;
        default:
            break;
        }
    }
    if (s.startsWith(cosStr)) {
        index += 3;
        switch (m_trigonometricMode) {
        case Radians:
            return KCalcToken::TokenCode::CosRad;
        case Gradians:
            return KCalcToken::TokenCode::CosGrad;
        case Degrees:
            return KCalcToken::TokenCode::CosDeg;
        default:
            break;
        }
    }
    if (s.startsWith(tanStr)) {
        index += 3;
        switch (m_trigonometricMode) {
        case Radians:
            return KCalcToken::TokenCode::TanRad;
        case Gradians:
            return KCalcToken::TokenCode::TanGrad;
        case Degrees:
            return KCalcToken::TokenCode::TanDeg;
        default:
            break;
        }
    }

    if (s.startsWith(argStr)) {
        index += 3;
        switch (m_trigonometricMode) {
        case Radians:
            return KCalcToken::TokenCode::ArgRad;
        case Gradians:
            return KCalcToken::TokenCode::ArgGrad;
        case Degrees:
            return KCalcToken::TokenCode::ArgDeg;
        default:
            break;
        }
    }

    if (s.startsWith(expStr)) {
        index += 3;
        return KCalcToken::TokenCode::Exp;
    }

    if (s.startsWith(log10Str)) {
        index += 3;
        return KCalcToken::TokenCode::Log10;
    }

    if (s.startsWith(ansStr)) {
        index += 3;
        return KCalcToken::TokenCode::Ans;
    }

    if (s.startsWith(binomialStr)) {
        index += 3;
        return KCalcToken::TokenCode::Binomial;
    }

    if (s.startsWith(moduloStr)) {
        index += 3;
        return KCalcToken::TokenCode::Modulo;
    }

    if (s.startsWith(integerDivisionStr)) {
        index += 3;
        return KCalcToken::TokenCode::IntegerDivision;
    }

    if (s.startsWith(gradianStr)) {
        index += 3;
        return KCalcToken::TokenCode::Gradian;
    }

    if (s.startsWith(radianStr)) {
        index += 3;
        return KCalcToken::TokenCode::Radian;
    }

    if (s.startsWith(rshStr)) {
        index += 2;
        return KCalcToken::TokenCode::Rsh;
    }
    if (s.startsWith(lshStr)) {
        index += 2;
        return KCalcToken::TokenCode::Lsh;
    }
    if (s.startsWith(twosCompStr)) {
        index += 2;
        return KCalcToken::TokenCode::TwosComplement;
    }

    if (s.startsWith(reciprocalStr)) {
        index += 2;
        return KCalcToken::TokenCode::Reciprocal;
    }
    if (s.startsWith(lnStr)) {
        index += 2;
        return KCalcToken::TokenCode::Ln;
    }
    if (s.startsWith(reStr)) {
        index += 2;
        return KCalcToken::TokenCode::Re;
    }
    if (s.startsWith(imStr)) {
        index += 2;
        return KCalcToken::TokenCode::Im;
    }

    if (index + 2 <= buffer.size()) {
        s = buffer.sliced(index, 2);
        if (constantSymbolToValue(s)) {
            index += 2;
            return KCalcToken::TokenCode::KNumber;
        }
    }
    if (s.startsWith(onesCompStr)) {
        index++;
        return KCalcToken::TokenCode::OnesComplement;
    }
    if (s.startsWith(factorialStr)) {
        index++;
        return KCalcToken::TokenCode::Factorial;
    }
    if (s.startsWith(doubleFactorialStr)) {
        index++;
        return KCalcToken::TokenCode::DoubleFactorial;
    }

    s = buffer.sliced(index, 1);

    if ((aLowerCaseStr <= s.first(1) && s.first(1) <= fLowerCaseStr)) {
        if (m_numeralMode && base == 16) {
            QRegularExpressionMatch match;
            int numIndex = buffer.indexOf(hexNumberDigitsRegex, index, &match);
            if (match.captured().size() > 16 || numIndex != index) {
                return KCalcToken::TokenCode::InvalidToken;
            }
            m_tokenKNumber = hexNumberPrefixStr + match.captured();
            index += match.captured().size();
            return KCalcToken::TokenCode::KNumber;
        }
    }

    if (constantSymbolToValue(s)) {
        index++;
        return KCalcToken::TokenCode::KNumber;
    }

    m_parsingResult = InvalidToken;
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
    int bufferIndex = 0;
    int bufferSize = buffer.size();
    qCDebug(KCALC_LOG) << "Parsing string to TokenQueue";
    qCDebug(KCALC_LOG) << "Buffer string to parse: " << buffer;

    KCalcToken::TokenCode tokenCode = KCalcToken::TokenCode::InvalidToken;

    if (bufferSize == 0) {
        errorIndex = -1;
        m_parsingResult = Empty;
        return ParsingResult::Empty;
    }

    while (bufferIndex < bufferSize) {
        tokenCode = KCalcToken::TokenCode::InvalidToken;
        KNumber operand;

        tokenCode = stringToToken(buffer, bufferIndex, base);

        if (tokenCode == KCalcToken::TokenCode::InvalidToken) {
            m_parsingResult = InvalidToken;
            errorIndex = bufferIndex; // this indicates where the error was found
            return ParsingResult::InvalidToken; // in the input string
        }

        if (tokenCode == KCalcToken::TokenCode::Stub) {
            continue;
        }

        if (tokenCode == KCalcToken::TokenCode::KNumber) {
            qCDebug(KCALC_LOG) << "String KNumber converted: " << m_tokenKNumber;
            m_tokenKNumber.replace(commaStr, KNumber::decimalSeparator());
            m_tokenKNumber.replace(pointStr, KNumber::decimalSeparator());
            operand = KNumber(m_tokenKNumber);

            tokenQueue.enqueue(KCalcToken(operand, bufferIndex));
        } else {
            tokenQueue.enqueue(KCalcToken(tokenCode, bufferIndex));
            qCDebug(KCALC_LOG) << "KCalcToken converted with code: " << tokenCode;
        }

        qCDebug(KCALC_LOG) << "Parsing index after this previous step: " << bufferIndex;
    }

    qCDebug(KCALC_LOG) << "Parsing done, index after parsing: " << bufferIndex;

    if (tokenQueue.length() > 2) {
        return ParsingResult::Success;
    } else if (tokenQueue.length() == 2) {
        if (!m_inputHasConstants && tokenQueue.first().getTokenCode() == KCalcToken::TokenCode::Minus && tokenQueue.at(1).isKNumber()) {
            return ParsingResult::SuccessSingleKNumber;
        } else {
            return ParsingResult::Success;
        }
    } else if (tokenQueue.length() == 1) {
        if (!m_inputHasConstants && tokenQueue.first().isKNumber()) {
            return ParsingResult::SuccessSingleKNumber;
        } else {
            return ParsingResult::Success;
        }
    } else {
        return ParsingResult::Empty;
    }
}

//------------------------------------------------------------------------------
// Name: setTrigonometricMode
// Desc:
//------------------------------------------------------------------------------
void KCalcParser::setTrigonometricMode(int mode)
{
    m_trigonometricMode = mode;
}

//------------------------------------------------------------------------------
// Name: getTrigonometricMode
// Desc:
//------------------------------------------------------------------------------
int KCalcParser::getTrigonometricMode()
{
    return m_trigonometricMode;
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
    return m_parsingResult;
}

//------------------------------------------------------------------------------
// Name: TokenToString
// Desc:
//------------------------------------------------------------------------------
const QString KCalcParser::tokenToString(KCalcToken::TokenCode tokenCode)
{
    switch (tokenCode) {
    case KCalcToken::TokenCode::DecimalPoint:
        return decimalPointStr;
        break;
    case KCalcToken::TokenCode::Plus:
        return plusStr;
        break;
    case KCalcToken::TokenCode::Minus:
        return hyphenMinusStr;
        break;
    case KCalcToken::TokenCode::Division:
        return divisionStr;
        break;
    case KCalcToken::TokenCode::Slash:
        return slashStr;
        break;
    case KCalcToken::TokenCode::Multiplication:
        return multiplicationStr;
        break;
    case KCalcToken::TokenCode::Dot:
        return dotStr;
        break;
    case KCalcToken::TokenCode::Asterisk:
        return asteriskStr;
        break;
    case KCalcToken::TokenCode::Percentage:
        return percentageStr;
        break;
    case KCalcToken::TokenCode::OpeningParenthesis:
        return openingParenthesisStr;
        break;
    case KCalcToken::TokenCode::ClosingParenthesis:
        return closingParenthesisStr;
        break;
    case KCalcToken::TokenCode::Square:
        return squareStr;
        break;
    case KCalcToken::TokenCode::Cube:
        return cubeStr;
        break;
    case KCalcToken::TokenCode::SquareRoot:
        return squareRootStr;
        break;
    case KCalcToken::TokenCode::CubicRoot:
        return cubicRootStr;
        break;
    case KCalcToken::TokenCode::Degree:
        return degreeStr;
        break;
    case KCalcToken::TokenCode::Gradian:
        return gradianStr;
        break;
    case KCalcToken::TokenCode::Radian:
        return radianStr;
        break;
    case KCalcToken::TokenCode::Sin:
    case KCalcToken::TokenCode::SinRad:
    case KCalcToken::TokenCode::SinGrad:
    case KCalcToken::TokenCode::SinDeg:
        return sinStr;
        break;
    case KCalcToken::TokenCode::Cos:
    case KCalcToken::TokenCode::CosRad:
    case KCalcToken::TokenCode::CosGrad:
    case KCalcToken::TokenCode::CosDeg:
        return cosStr;
        break;
    case KCalcToken::TokenCode::Tan:
    case KCalcToken::TokenCode::TanRad:
    case KCalcToken::TokenCode::TanGrad:
    case KCalcToken::TokenCode::TanDeg:
        return tanStr;
        break;
    case KCalcToken::TokenCode::Asin:
    case KCalcToken::TokenCode::AsinRad:
    case KCalcToken::TokenCode::AsinDeg:
    case KCalcToken::TokenCode::AsinGrad:
        return asinStr;
        break;
    case KCalcToken::TokenCode::Acos:
    case KCalcToken::TokenCode::AcosRad:
    case KCalcToken::TokenCode::AcosDeg:
    case KCalcToken::TokenCode::AcosGrad:
        return acosStr;
        break;
    case KCalcToken::TokenCode::Atan:
    case KCalcToken::TokenCode::AtanRad:
    case KCalcToken::TokenCode::AtanDeg:
    case KCalcToken::TokenCode::AtanGrad:
        return atanStr;
        break;
    case KCalcToken::TokenCode::Sinh:
        return sinhStr;
        break;
    case KCalcToken::TokenCode::Cosh:
        return coshStr;
        break;
    case KCalcToken::TokenCode::Tanh:
        return tanhStr;
        break;
    case KCalcToken::TokenCode::Asinh:
        return asinhStr;
        break;
    case KCalcToken::TokenCode::Acosh:
        return acoshStr;
        break;
    case KCalcToken::TokenCode::Atanh:
        return atanhStr;
        break;
    case KCalcToken::TokenCode::E:
        return eStr;
        break;
    case KCalcToken::TokenCode::Pi:
        return piStr;
        break;
    case KCalcToken::TokenCode::Phi:
        return phiStr;
        break;
    case KCalcToken::TokenCode::I:
        return iStr;
        break;
    case KCalcToken::TokenCode::Polar:
        return angleStr;
        break;
    case KCalcToken::TokenCode::PosInfinity:
        return posInfinityStr;
        break;
    case KCalcToken::TokenCode::NegInfinity:
        return negInfinityStr;
        break;
    case KCalcToken::TokenCode::VacuumPermitivity:
        return vacuumPermitivityStr;
        break;
    case KCalcToken::TokenCode::VacuumPermeability:
        return vacuumPermeabilityStr;
        break;
    case KCalcToken::TokenCode::VacuumImpedance:
        return vacuumImpedanceStr;
        break;
    case KCalcToken::TokenCode::PlanckSConstant:
        return plancksConstantStr;
        break;
    case KCalcToken::TokenCode::PlanckOver2Pi:
        return plancksOver2PiStr;
        break;
    case KCalcToken::TokenCode::Exp:
        return expStr;
        break;
    case KCalcToken::TokenCode::Exp10:
        return exp10Str;
        break;
    case KCalcToken::TokenCode::Power:
        return powerStr;
        break;
    case KCalcToken::TokenCode::PowerRoot:
        return powerRootStr;
        break;
    case KCalcToken::TokenCode::Factorial:
        return factorialStr;
        break;
    case KCalcToken::TokenCode::DoubleFactorial:
        return doubleFactorialStr;
        break;
    case KCalcToken::TokenCode::Gamma:
        return gammaStr;
        break;
    case KCalcToken::TokenCode::InvertSign:
        return invertSignStr;
        break;
    case KCalcToken::TokenCode::Ln:
        return lnStr;
        break;
    case KCalcToken::TokenCode::Log10:
        return log10Str;
        break;
    case KCalcToken::TokenCode::Reciprocal:
        return reciprocalStr;
        break;
    case KCalcToken::TokenCode::Binomial:
        return thinSpaceStr + binomialStr + thinSpaceStr;
        break;
    case KCalcToken::TokenCode::Modulo:
        return thinSpaceStr + moduloStr + thinSpaceStr;
        break;
    case KCalcToken::TokenCode::IntegerDivision:
        return thinSpaceStr + integerDivisionStr + thinSpaceStr;
        break;
    case KCalcToken::TokenCode::And:
        return thinSpaceStr + andStr + thinSpaceStr;
        break;
    case KCalcToken::TokenCode::Or:
        return thinSpaceStr + orStr + thinSpaceStr;
        break;
    case KCalcToken::TokenCode::Xor:
        return thinSpaceStr + xorStr + thinSpaceStr;
        break;
    case KCalcToken::TokenCode::OnesComplement:
        return onesCompStr;
        break;
    case KCalcToken::TokenCode::TwosComplement:
        return twosCompStr;
        break;
    case KCalcToken::TokenCode::Rsh:
        return thinSpaceStr + rshStr + thinSpaceStr;
        break;
    case KCalcToken::TokenCode::Lsh:
        return thinSpaceStr + lshStr + thinSpaceStr;
        break;
    case KCalcToken::TokenCode::Ans:
        return thinSpaceStr + ansStr + thinSpaceStr;
        break;
    case KCalcToken::TokenCode::Equal:
        return equalStr;
        break;
    default:
        break;
    }
    return errorStr;
}

int KCalcParser::loadConstants(const QDomDocument &doc)
{
    QDomNode n = doc.documentElement().firstChild();

    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull() && e.tagName() == QLatin1String("constant")) {
            const QString value = e.attributeNode(QStringLiteral("value")).value();
            const QString symbol = e.attributeNode(QStringLiteral("symbol")).value();

            m_constants.insert(symbol, value);
        }
        n = n.nextSibling();
    }

    return 0;
}

//------------------------------------------------------------------------------
// Name: constantSymbolToValue_
// Desc: tries to find a given constant in the stored constants list, returns
//       true if found, loads value in tokenKNumber
//------------------------------------------------------------------------------
bool KCalcParser::constantSymbolToValue(const QString &constantSymbol)
{
    if (m_constants.contains(constantSymbol)) {
        m_tokenKNumber = m_constants.value(constantSymbol);
        m_inputHasConstants = true;
        return true;
    }

    return false;
}
