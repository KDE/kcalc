/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2003-2005 Klaus Niederkrueger <kniederk@math.uni-koeln.de>
    SPDX-FileCopyrightText: 1996-2000 Bernd Johannes Wuebben <wuebben@kde.org>
    SPDX-FileCopyrightText: 1995 Martin Bartlett

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_core.h"
#include "kcalc_debug.h"

#include <QDebug>

CalcEngine::CalcEngine()
    : m_errorGlobal(false)
    , m_bufferResult(KNumber::Zero)
{
}

KNumber CalcEngine::lastOutput(bool &error) const
{
    error = m_errorGlobal;
    return m_bufferResult;
}

void CalcEngine::statClearAll()
{
    stats.clearAll();
}

void CalcEngine::statCount(const KNumber &input)
{
    Q_UNUSED(input);
    m_bufferResult = KNumber(stats.count());
}

void CalcEngine::statDataNew(const KNumber &input)
{
    stats.enterData(input);
    m_bufferResult = KNumber(stats.count());
}

void CalcEngine::statDataDel()
{
    stats.clearLast();
    m_bufferResult = KNumber(stats.count());
}

void CalcEngine::statMean(const KNumber &input)
{
    Q_UNUSED(input);
    m_bufferResult = stats.mean();

    m_errorGlobal = stats.error();
}

void CalcEngine::statMedian(const KNumber &input)
{
    Q_UNUSED(input);
    m_bufferResult = stats.median();

    m_errorGlobal = stats.error();
}

void CalcEngine::statStdDeviation(const KNumber &input)
{
    Q_UNUSED(input);
    m_bufferResult = stats.std();

    m_errorGlobal = stats.error();
}

void CalcEngine::statStdSample(const KNumber &input)
{
    Q_UNUSED(input);
    m_bufferResult = stats.sampleStd();

    m_errorGlobal = stats.error();
}

void CalcEngine::statSum(const KNumber &input)
{
    Q_UNUSED(input);
    m_bufferResult = stats.sum();
}

void CalcEngine::statSumSquares(const KNumber &input)
{
    Q_UNUSED(input);
    m_bufferResult = stats.sumOfSquares();

    m_errorGlobal = stats.error();
}

void CalcEngine::reset()
{
    m_errorGlobal = false;
    m_bufferResult = KNumber::Zero;
    statClearAll();
}

CalcEngine::ResultCode CalcEngine::calculate(const QQueue<KCalcToken> &tokenBuffer, int &errorIndex)
{
    m_tokenStack.clear();
    int tokenIndex = 0;
    int bufferSize = tokenBuffer.size();
    qCDebug(KCALC_LOG) << "Token buffer size: " << bufferSize;

    KCalcToken const *tokenFunction = nullptr;
    KCalcToken const *tokenFirstArg = nullptr;
    KNumber result;
    KCalcToken::TokenType tokenType;

    while (tokenIndex < bufferSize) {
        qCDebug(KCALC_LOG) << "Processing token queue at: " << tokenIndex;
        KCalcToken::TokenCode tokenCode = tokenBuffer.at(tokenIndex).getTokenCode();

        if (tokenCode == KCalcToken::TokenCode::Equal) {
            tokenIndex++;
            continue;
        }

        tokenType = tokenBuffer.at(tokenIndex).getTokenType();
        switch (tokenType) {
        case KCalcToken::TokenType::KNumberType:
            if (tokenCode == KCalcToken::TokenCode::Ans) {
                insertKNumberTokenInStack(KCalcToken(m_bufferResult));
            } else {
                insertKNumberTokenInStack(tokenBuffer.at(tokenIndex));
            }
            tokenIndex++;
            break;
        case KCalcToken::TokenType::RightUnaryFunctionType:
            if (tokenIndex + 1 >= bufferSize) {
                errorIndex = tokenIndex;
                return ResultCode::MissingRightUnaryArg;
            }
            if (!m_tokenStack.isEmpty()) {
                if (m_tokenStack.last().isKNumber()) {
                    insertBinaryFunctionTokenInStack(m_multiplicationToken);
                }
            }

            m_tokenStack.push_back(tokenBuffer.at(tokenIndex));
            tokenIndex++;
            break;
        case KCalcToken::TokenType::LeftUnaryFunctionType:
            if (m_tokenStack.isEmpty()) {
                errorIndex = tokenIndex;
                return ResultCode::MissingLeftUnaryArg;
            }
            if (!m_tokenStack.last().isKNumber()) {
                errorIndex = tokenIndex;
                return ResultCode::MissingLeftUnaryArg;
            }

            if (m_tokenStack.size() > 1) {
                while (m_tokenStack.at(m_tokenStack.size() - 2).isRightUnaryFunction()) {
                    tokenFunction = &m_tokenStack.at(m_tokenStack.size() - 2);
                    tokenFirstArg = &m_tokenStack.last();
                    KNumber result = tokenFunction->evaluate(tokenFirstArg->getKNumber());
                    m_tokenStack.pop_back();
                    m_tokenStack.pop_back();
                    insertKNumberTokenInStack(KCalcToken(result));
                    if (m_tokenStack.size() == 1) {
                        break;
                    }
                }
            }

            if (tokenCode == KCalcToken::TokenCode::Percentage) {
                insertPercentageTokenInStack();
            }

            tokenFunction = &tokenBuffer.at(tokenIndex);
            tokenFirstArg = &m_tokenStack.last();
            result = tokenFunction->evaluate(tokenFirstArg->getKNumber());
            m_tokenStack.pop_back();
            insertKNumberTokenInStack(KCalcToken(result));
            tokenIndex++;
            break;
        case KCalcToken::TokenType::BinaryFunctionType:
            if (tokenIndex + 1 >= bufferSize) {
                errorIndex = tokenIndex;
                return ResultCode::MissingRightBinaryArg;
            }

            if (m_tokenStack.isEmpty()) {
                switch (tokenCode) {
                case KCalcToken::TokenCode::Plus:
                case KCalcToken::TokenCode::Minus:
                    m_tokenStack.push_back(KCalcToken(KNumber::Zero));
                    break;
                default:
                    errorIndex = tokenIndex;
                    return ResultCode::SyntaxError;
                    break;
                }
            }

            switch (m_tokenStack.last().getTokenType()) {
            case KCalcToken::TokenType::BinaryFunctionType:
                if (tokenCode == KCalcToken::TokenCode::Plus) {
                    tokenIndex++;
                    continue;
                } else if (tokenCode == KCalcToken::TokenCode::Minus) {
                    m_tokenStack.last().invertSignSecondArg();
                    tokenIndex++;
                    continue;
                } else {
                    errorIndex = tokenIndex;
                    return ResultCode::SyntaxError;
                }
                break;
            case KCalcToken::TokenType::RightUnaryFunctionType:
                if (tokenCode == KCalcToken::TokenCode::Minus) {
                    m_tokenStack.last().invertSignFirstArg();
                    tokenIndex++;
                    continue;
                } else if (tokenCode == KCalcToken::TokenCode::Plus) {
                    tokenIndex++;
                    continue;
                }
                break;
            case KCalcToken::TokenType::OpeningParenthesesType:
                if (tokenCode == KCalcToken::TokenCode::Plus || tokenCode == KCalcToken::TokenCode::Minus) {
                    m_tokenStack.push_back(KCalcToken(KNumber::Zero));
                } else {
                    errorIndex = tokenIndex;
                    return ResultCode::SyntaxError;
                }
                break;
            default:
                break;
            }

            insertBinaryFunctionTokenInStack(tokenBuffer.at(tokenIndex));
            tokenIndex++;
            break;
        case KCalcToken::TokenType::OpeningParenthesesType:
            if (!m_tokenStack.isEmpty()) {
                switch (m_tokenStack.last().getTokenType()) {
                case KCalcToken::TokenType::KNumberType:
                case KCalcToken::TokenType::LeftUnaryFunctionType:
                case KCalcToken::TokenType::ClosingParenthesesType:
                    insertBinaryFunctionTokenInStack(m_multiplicationToken);
                    break;
                default:
                    break;
                }
            }
            m_tokenStack.push_back(tokenBuffer.at(tokenIndex));
            tokenIndex++;
            break;
        case KCalcToken::TokenType::ClosingParenthesesType:
            if (m_tokenStack.isEmpty()) {
                errorIndex = tokenIndex;
                return ResultCode::SyntaxError;
            }
            switch (m_tokenStack.last().getTokenType()) {
            case KCalcToken::TokenType::BinaryFunctionType:
            case KCalcToken::TokenType::RightUnaryFunctionType:
                errorIndex = tokenIndex;
                return ResultCode::SyntaxError;
                break;
            default:
                reduceStack();
                break;
            }
            tokenIndex++;
            break;
        default:
            break;
        }
    }

    qCDebug(KCALC_LOG) << "Done processing token list";
    // printStacks_();

    reduceStack(/*toParentheses =*/false);

    qCDebug(KCALC_LOG) << "Done reducing final token stack";

    if (m_tokenStack.isEmpty()) {
        m_bufferResult = KNumber::Zero;
        errorIndex = bufferSize - 1;
        return ResultCode::EmptyInput;
    } else if (m_tokenStack.last().isKNumber() && m_tokenStack.last().getKNumber().type() == KNumber::Type::TypeError) {
        m_errorGlobal = true;
        errorIndex = bufferSize - 1;
        m_bufferResult = m_tokenStack.last().getKNumber();
        m_tokenStack.clear();
        return ResultCode::MathError;
    } else if (m_tokenStack.size() > 2) {
        m_errorGlobal = true;
        errorIndex = bufferSize - 1;
        m_bufferResult = KNumber::NaN;
        m_tokenStack.clear();
        return ResultCode::SyntaxError;
    } else if (m_tokenStack.size() == 2 && m_tokenStack.last().isBinaryFunction() && m_tokenStack.at(0).isKNumber()) {
        m_errorGlobal = true;
        errorIndex = bufferSize - 1;
        if (m_tokenStack.at(0).getKNumber().type() == KNumber::Type::TypeError) {
            m_tokenStack.clear();
            return ResultCode::MathError;
        } else {
            m_tokenStack.clear();
            return ResultCode::MissingRightBinaryArg;
        }
    } else if (!m_tokenStack.last().isKNumber()) {
        m_errorGlobal = true;
        errorIndex = bufferSize - 1;
        m_tokenStack.clear();
        return ResultCode::SyntaxError;
    } else {
        m_bufferResult = m_tokenStack.last().getKNumber();
        m_tokenStack.clear();
    }

    qCDebug(KCALC_LOG) << "Result: " << m_bufferResult.toQString(12, -1);

    m_errorGlobal = false;
    return ResultCode::Success;
}

int CalcEngine::insertKNumberTokenInStack(const KCalcToken &token)
{
    qCDebug(KCALC_LOG) << "Inserting KNumber Token in stack";
    // printStacks_();
    KCalcToken const *tokenFunction = nullptr;
    KCalcToken const *tokenFirstArg = nullptr;

    KCalcToken tokenToInsert = token;

    while (!m_tokenStack.isEmpty()) {
        if (m_tokenStack.last().isKNumber()) {
            insertBinaryFunctionTokenInStack(m_multiplicationToken);
            break;
        }
        if (m_tokenStack.last().isBinaryFunction() || m_tokenStack.last().isOpeningParentheses()) {
            break;
        }
        if (m_tokenStack.last().isLeftUnaryFunction()) {
            // never executed //
            break;
        }
        while (m_tokenStack.last().isRightUnaryFunction()) {
            tokenFunction = &m_tokenStack.last();
            tokenFirstArg = &tokenToInsert;
            KNumber result = tokenFunction->evaluate(tokenFirstArg->getKNumber());
            m_tokenStack.pop_back();
            tokenToInsert = KCalcToken(result);

            if (m_tokenStack.isEmpty()) {
                break;
            }
        }
    }
    m_tokenStack.push_back(tokenToInsert);
    return 0;
}

//------------------------------------------------------------------------------
// Name: insert_percentage_Token_In_Stack_
// Desc: rearranges the stack accordingly in order to insert a percentage token
//------------------------------------------------------------------------------
int CalcEngine::insertPercentageTokenInStack()
{
    qCDebug(KCALC_LOG) << "Inserting Percentage Token in stack";
    // printStacks_();

    if (m_tokenStack.size() < 2) {
        return 0;
    } else if (m_tokenStack.last().isKNumber() && m_tokenStack.at(m_tokenStack.size() - 3).isKNumber()
               && m_tokenStack.at(m_tokenStack.size() - 2).isBinaryFunction()) {
        KCalcToken::TokenCode binaryToken = m_tokenStack.at(m_tokenStack.size() - 2).getTokenCode();
        if (binaryToken == KCalcToken::TokenCode::Plus || binaryToken == KCalcToken::TokenCode::Minus) {
            // recover tokens
            KCalcToken percentage = m_tokenStack.last();
            m_tokenStack.pop_back();
            KCalcToken operation = m_tokenStack.last();
            m_tokenStack.pop_back();
            // reduce stack
            reduceStack();
            // //insert tokens
            m_tokenStack.push_back(operation);
            m_tokenStack.push_back(percentage);
            m_tokenStack.push_back(m_multiplicationToken);
            m_tokenStack.push_back(m_tokenStack.at(m_tokenStack.size() - 4));
        } else {
            return 0;
        }
    } else {
        return 0;
    }
    return 0;
}

int CalcEngine::insertBinaryFunctionTokenInStack(const KCalcToken &token)
{
    qCDebug(KCALC_LOG) << "Insert Binary Function Token in stack";
    // printStacks_();
    KCalcToken const *tokenFunction = nullptr;
    KCalcToken const *tokenFirstArg = nullptr;
    KCalcToken const *tokenSecondArg = nullptr;

    if (m_tokenStack.isEmpty()) {
        return -1;
    }

    if (m_tokenStack.size() <= 2) {
        m_tokenStack.push_back(token);
        return 0;
    }

    if (m_tokenStack.at(m_tokenStack.size() - 2).isRightUnaryFunction() && m_tokenStack.last().isKNumber()) {
        tokenFunction = &m_tokenStack.at(m_tokenStack.size() - 2);
        tokenFirstArg = &m_tokenStack.last();
        KNumber result = tokenFunction->evaluate(tokenFirstArg->getKNumber());
        m_tokenStack.pop_back();
        m_tokenStack.pop_back();
        insertKNumberTokenInStack(KCalcToken(result));
        if (m_tokenStack.size() <= 2) {
            m_tokenStack.push_back(token);
            return 0;
        } // else continue inserting
    }

    while (m_tokenStack.at(m_tokenStack.size() - 1).isKNumber() && m_tokenStack.at(m_tokenStack.size() - 2).isBinaryFunction()
           && m_tokenStack.at(m_tokenStack.size() - 3).isKNumber()) {
        if (m_tokenStack.size() > 3) {
            if (m_tokenStack.at(m_tokenStack.size() - 4).isRightUnaryFunction()) {
                m_tokenStack.push_back(token);
                return 0;
            }
        }
        if (m_tokenStack.at(m_tokenStack.size() - 2).getPriorityLevel() >= token.getPriorityLevel()) {
            tokenFunction = &m_tokenStack.at(m_tokenStack.size() - 2);
            tokenFirstArg = &m_tokenStack.at(m_tokenStack.size() - 3);
            tokenSecondArg = &m_tokenStack.at(m_tokenStack.size() - 1);
            KNumber result = tokenFunction->evaluate(tokenFirstArg->getKNumber(), tokenSecondArg->getKNumber());
            m_tokenStack.pop_back();
            m_tokenStack.pop_back();
            m_tokenStack.last().updateToken(result);
            if (m_tokenStack.size() < 3) {
                m_tokenStack.push_back(token);
                return 0;
            }
            continue;
        } else {
            m_tokenStack.push_back(token);
            return 0;
        }
    }

    m_tokenStack.push_back(token);

    return 0;
}

int CalcEngine::reduceStack(bool toParentheses /*= true*/)
{
    KCalcToken const *tokenFunction = nullptr;
    KCalcToken const *tokenFirstArg = nullptr;
    KCalcToken const *tokenSecondArg = nullptr;

    while (m_tokenStack.size() > 1) {
        qCDebug(KCALC_LOG) << "Reducing at stack size: " << m_tokenStack.size();
        // printStacks_();
        if (m_tokenStack.last().isOpeningParentheses()) {
            m_tokenStack.pop_back();
            if (toParentheses) {
                return 0;
            } else {
                continue;
            }
        }

        if (m_tokenStack.last().isKNumber() && m_tokenStack.at(m_tokenStack.size() - 2).isOpeningParentheses()) {
            KNumber result = m_tokenStack.last().getKNumber();
            m_tokenStack.pop_back();
            m_tokenStack.pop_back();
            insertKNumberTokenInStack(KCalcToken(result));
            if (toParentheses) {
                return 0;
            } else {
                continue;
            }
        }

        if (m_tokenStack.last().isKNumber() && m_tokenStack.at(m_tokenStack.size() - 2).isRightUnaryFunction()) {
            tokenFunction = &m_tokenStack.at(m_tokenStack.size() - 2);
            tokenFirstArg = &m_tokenStack.last();
            KNumber result = tokenFunction->evaluate(tokenFirstArg->getKNumber());
            m_tokenStack.pop_back();
            m_tokenStack.pop_back();
            insertKNumberTokenInStack(KCalcToken(result));
            continue;
        }

        if (m_tokenStack.last().isLeftUnaryFunction() && m_tokenStack.at(m_tokenStack.size() - 2).isKNumber()) {
            // never executed
            tokenFunction = &m_tokenStack.last();
            tokenFirstArg = &m_tokenStack.at(m_tokenStack.size() - 2);
            KNumber result = tokenFunction->evaluate(tokenFirstArg->getKNumber());
            m_tokenStack.pop_back();
            m_tokenStack.last().updateToken(result);
            continue;
        }

        if (m_tokenStack.size() > 3) {
            if (m_tokenStack.at(m_tokenStack.size() - 3).isKNumber() && m_tokenStack.at(m_tokenStack.size() - 4).isRightUnaryFunction()) {
                tokenFunction = &m_tokenStack.at(m_tokenStack.size() - 4);
                tokenFirstArg = &m_tokenStack.at(m_tokenStack.size() - 3);

                KNumber result = tokenFunction->evaluate(tokenFirstArg->getKNumber());

                m_tokenStack.removeAt(m_tokenStack.size() - 4);
                m_tokenStack.replace(m_tokenStack.size() - 3, KCalcToken(result));
                continue;
            }

            if (m_tokenStack.at(m_tokenStack.size() - 1).isKNumber() && m_tokenStack.at(m_tokenStack.size() - 2).isBinaryFunction()
                && m_tokenStack.at(m_tokenStack.size() - 3).isBinaryFunction() && m_tokenStack.at(m_tokenStack.size() - 4).isKNumber()) {
                if (m_tokenStack.at(m_tokenStack.size() - 2).getTokenCode() != KCalcToken::TokenCode::Minus) {
                    return -1;
                }

                tokenFunction = &m_tokenStack.at(m_tokenStack.size() - 3);
                tokenFirstArg = &m_tokenStack.at(m_tokenStack.size() - 4);
                tokenSecondArg = &m_tokenStack.at(m_tokenStack.size() - 1);
                KNumber result = tokenFunction->evaluate(tokenFirstArg->getKNumber(), -tokenSecondArg->getKNumber());

                m_tokenStack.pop_back();
                m_tokenStack.pop_back();
                m_tokenStack.pop_back();
                m_tokenStack.last().updateToken(result);
                continue;
            }
            if (m_tokenStack.at(m_tokenStack.size() - 1).isKNumber() && m_tokenStack.at(m_tokenStack.size() - 2).isBinaryFunction()
                && m_tokenStack.at(m_tokenStack.size() - 3).isKNumber() && m_tokenStack.at(m_tokenStack.size() - 4).isBinaryFunction()) {
                if (m_tokenStack.at(m_tokenStack.size() - 2).getTokenCode() == KCalcToken::TokenCode::Minus
                    || m_tokenStack.at(m_tokenStack.size() - 2).getTokenCode() == KCalcToken::TokenCode::Plus) {
                    if (m_tokenStack.at(m_tokenStack.size() - 4).getTokenCode() == KCalcToken::TokenCode::Minus) {
                        tokenSecondArg = &m_tokenStack.at(m_tokenStack.size() - 1);
                        tokenFunction = &m_tokenStack.at(m_tokenStack.size() - 2);
                        tokenFirstArg = &m_tokenStack.at(m_tokenStack.size() - 3);
                        KNumber result;
                        if (tokenFunction->isSecondArgInverted()) {
                            result = tokenFunction->evaluate(tokenFirstArg->getKNumber(), tokenSecondArg->getKNumber());
                        } else {
                            result = tokenFunction->evaluate(-tokenFirstArg->getKNumber(), tokenSecondArg->getKNumber());
                        }
                        m_tokenStack.pop_back();
                        m_tokenStack.pop_back();
                        m_tokenStack.pop_back();
                        m_tokenStack.last().updateToken(KCalcToken::TokenCode::Plus);
                        m_tokenStack.push_back(KCalcToken(result));
                        continue;
                    }
                }
            }
        }

        if (m_tokenStack.size() > 2) {
            if (m_tokenStack.last().isKNumber() && m_tokenStack.at(m_tokenStack.size() - 2).isBinaryFunction()
                && m_tokenStack.at(m_tokenStack.size() - 3).isKNumber()) {
                tokenSecondArg = &m_tokenStack.last();
                tokenFunction = &m_tokenStack.at(m_tokenStack.size() - 2);
                tokenFirstArg = &m_tokenStack.at(m_tokenStack.size() - 3);

                KNumber result = tokenFunction->evaluate(tokenFirstArg->getKNumber(), tokenSecondArg->getKNumber());

                m_tokenStack.pop_back();
                m_tokenStack.pop_back();
                m_tokenStack.last().updateToken(result);
                continue;
            }
            if (m_tokenStack.at(m_tokenStack.size() - 1).isKNumber() && m_tokenStack.at(m_tokenStack.size() - 2).isBinaryFunction()
                && m_tokenStack.at(m_tokenStack.size() - 3).isBinaryFunction()) {
                if (m_tokenStack.at(m_tokenStack.size() - 2).getTokenCode() != KCalcToken::TokenCode::Minus) {
                    return -1;
                }
                KNumber result = -m_tokenStack.last().getKNumber();
                m_tokenStack.pop_back();
                m_tokenStack.last().updateToken(result);
                continue;
            }
        }

        if (m_tokenStack.at(m_tokenStack.size() - 1).isKNumber() && m_tokenStack.at(m_tokenStack.size() - 2).isBinaryFunction()) {
            if (m_tokenStack.at(m_tokenStack.size() - 2).getTokenCode() != KCalcToken::TokenCode::Minus) {
                return -1;
            }
            KNumber result = -m_tokenStack.last().getKNumber();
            m_tokenStack.pop_back();
            m_tokenStack.last().updateToken(result);
            continue;
        }

        qCDebug(KCALC_LOG) << "Error at stack size = " << m_tokenStack.size();
        return -1;
    }
    return 0;
}

void CalcEngine::printStacks()
{
    int tokenStaskSize = m_tokenStack.size();

    qCDebug(KCALC_LOG) << "Printing current stack:";

    for (int i = 0; i < tokenStaskSize; i++) {
        if (m_tokenStack.at(i).isKNumber()) {
            qCDebug(KCALC_LOG) << "TokenStack at:" << i << " is KNumber   = " << (m_tokenStack.at(i).getKNumber()).toQString();

        } else {
            qCDebug(KCALC_LOG) << "TokenStack at:" << i << " is TokenCode = " << (m_tokenStack.at(i).getTokenCode());
        }
    }

    qCDebug(KCALC_LOG) << "Print current stack done";
}

KNumber CalcEngine::getResult()
{
    return m_bufferResult;
}
