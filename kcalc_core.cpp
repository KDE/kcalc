/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2003-2005 Klaus Niederkrueger <kniederk@math.uni-koeln.de>
    SPDX-FileCopyrightText: 1996-2000 Bernd Johannes Wuebben <wuebben@kde.org>
    SPDX-FileCopyrightText: 1995 Martin Bartlett

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_core.h"
#include "kcalc_debug.h"
#include "kcalc_stats.h"
#include "kcalc_token.h"
#include "knumber.h"
#include <QQueue>

#include <QDebug>

CalcEngine::CalcEngine()
    : buffer_result_(KNumber::Zero)
{
    error_ = false;
}

KNumber CalcEngine::lastOutput(bool &error) const
{
    error = error_;
    return buffer_result_;
}

void CalcEngine::StatClearAll()
{
    stats.clearAll();
}

void CalcEngine::StatCount(const KNumber &input)
{
    Q_UNUSED(input);
    buffer_result_ = KNumber(stats.count());
}

void CalcEngine::StatDataNew(const KNumber &input)
{
    stats.enterData(input);
    buffer_result_ = KNumber(stats.count());
}

void CalcEngine::StatDataDel()
{
    stats.clearLast();
    buffer_result_ = KNumber(stats.count());
}

void CalcEngine::StatMean(const KNumber &input)
{
    Q_UNUSED(input);
    buffer_result_ = stats.mean();

    error_ = stats.error();
}

void CalcEngine::StatMedian(const KNumber &input)
{
    Q_UNUSED(input);
    buffer_result_ = stats.median();

    error_ = stats.error();
}

void CalcEngine::StatStdDeviation(const KNumber &input)
{
    Q_UNUSED(input);
    buffer_result_ = stats.std();

    error_ = stats.error();
}

void CalcEngine::StatStdSample(const KNumber &input)
{
    Q_UNUSED(input);
    buffer_result_ = stats.sample_std();

    error_ = stats.error();
}

void CalcEngine::StatSum(const KNumber &input)
{
    Q_UNUSED(input);
    buffer_result_ = stats.sum();
}

void CalcEngine::StatSumSquares(const KNumber &input)
{
    Q_UNUSED(input);
    buffer_result_ = stats.sum_of_squares();

    error_ = stats.error();
}

void CalcEngine::Reset()
{
    error_ = false;
    buffer_result_ = KNumber::Zero;
    StatClearAll();
}

CalcEngine::ResultCode CalcEngine::calculate(const QQueue<KCalcToken> tokenBuffer, int &errorIndex)
{
    token_stack_.clear();
    int token_index = 0;
    int buffer_size = tokenBuffer.size();
    qCDebug(KCALC_LOG) << "Token buffer size: " << buffer_size;

    KCalcToken const *tokenFunction;
    KCalcToken const *tokenFirstArg;
    KNumber result;
    KCalcToken::TokenType tokenType;

    while (token_index < buffer_size) {
        qCDebug(KCALC_LOG) << "Processing token queue at: " << token_index;
        KCalcToken::TokenCode tokenCode = tokenBuffer.at(token_index).getTokenCode();

        if (tokenCode == KCalcToken::TokenCode::EQUAL) {
            token_index++;
            continue;
        }

        tokenType = tokenBuffer.at(token_index).getTokenType();
        switch (tokenType) {
        case KCalcToken::TokenType::KNUMBER_TYPE:
            if (tokenCode == KCalcToken::TokenCode::ANS) {
                insert_KNumber_Token_In_Stack_(KCalcToken(buffer_result_));
            } else {
                insert_KNumber_Token_In_Stack_(tokenBuffer.at(token_index));
            }
            token_index++;
            break;
        case KCalcToken::TokenType::RIGHT_UNARY_FUNCTION_TYPE:
            if (token_index + 1 >= buffer_size) {
                errorIndex = token_index;
                return MISIING_RIGHT_UNARY_ARG;
            }
            if (!token_stack_.isEmpty()) {
                if (token_stack_.last().isKNumber()) {
                    insert_Binary_Function_Token_In_Stack_(multiplication_Token_);
                }
            }

            token_stack_.push_back(tokenBuffer.at(token_index));
            token_index++;
            break;
        case KCalcToken::TokenType::LEFT_UNARY_FUNCTION_TYPE:
            if (token_stack_.isEmpty()) {
                errorIndex = token_index;
                return MISIING_LEFT_UNARY_ARG;
            }
            if (!token_stack_.last().isKNumber()) {
                errorIndex = token_index;
                return MISIING_LEFT_UNARY_ARG;
            }

            if (token_stack_.size() > 1) {
                while (token_stack_.at(token_stack_.size() - 2).isRightUnaryFunction()) {
                    tokenFunction = &token_stack_.at(token_stack_.size() - 2);
                    tokenFirstArg = &token_stack_.last();
                    KNumber result = tokenFunction->evaluate(tokenFirstArg->getKNumber());
                    token_stack_.pop_back();
                    token_stack_.pop_back();
                    insert_KNumber_Token_In_Stack_(KCalcToken(result));
                    if (token_stack_.size() == 1) {
                        break;
                    }
                }
            }

            tokenFunction = &tokenBuffer.at(token_index);
            tokenFirstArg = &token_stack_.last();
            result = tokenFunction->evaluate(tokenFirstArg->getKNumber());
            token_stack_.pop_back();
            insert_KNumber_Token_In_Stack_(KCalcToken(result));
            token_index++;
            break;
        case KCalcToken::TokenType::BINARY_FUNCTION_TYPE:
            if (token_index + 1 >= buffer_size) {
                errorIndex = token_index;
                return MISIING_RIGHT_BINARY_ARG;
            }

            if (token_stack_.isEmpty()) {
                switch (tokenCode) {
                case KCalcToken::TokenCode::PLUS:
                case KCalcToken::TokenCode::MINUS:
                    token_stack_.push_back(KCalcToken(KNumber::Zero));
                    break;
                default:
                    errorIndex = token_index;
                    return SYNTAX_ERROR;
                    break;
                }
            }

            switch (token_stack_.last().getTokenType()) {
            case KCalcToken::TokenType::BINARY_FUNCTION_TYPE:
                if (tokenCode == KCalcToken::TokenCode::PLUS) {
                    token_index++;
                    continue;
                } else if (tokenCode == KCalcToken::TokenCode::MINUS) {
                    token_stack_.last().invertSignSecondArg();
                    token_index++;
                    continue;
                } else {
                    errorIndex = token_index;
                    return SYNTAX_ERROR;
                }
                break;
            case KCalcToken::TokenType::RIGHT_UNARY_FUNCTION_TYPE:
                if (tokenCode == KCalcToken::TokenCode::MINUS) {
                    token_stack_.last().invertSignFirstArg();
                    token_index++;
                    continue;
                } else if (tokenCode == KCalcToken::TokenCode::PLUS) {
                    token_index++;
                    continue;
                }
                break;
            case KCalcToken::TokenType::OPENING_PARENTHESES_TYPE:
                if (tokenCode == KCalcToken::TokenCode::PLUS || tokenCode == KCalcToken::TokenCode::MINUS) {
                    token_stack_.push_back(KCalcToken(KNumber::Zero));
                } else {
                    errorIndex = token_index;
                    return SYNTAX_ERROR;
                }
                break;
            default:
                break;
            }

            insert_Binary_Function_Token_In_Stack_(tokenBuffer.at(token_index));
            token_index++;
            break;
        case KCalcToken::TokenType::OPENING_PARENTHESES_TYPE:
            if (!token_stack_.isEmpty()) {
                switch (token_stack_.last().getTokenType()) {
                case KCalcToken::TokenType::KNUMBER_TYPE:
                case KCalcToken::TokenType::LEFT_UNARY_FUNCTION_TYPE:
                case KCalcToken::TokenType::CLOSING_PARENTHESES_TYPE:
                    insert_Binary_Function_Token_In_Stack_(multiplication_Token_);
                    break;
                default:
                    break;
                }
            }
            token_stack_.push_back(tokenBuffer.at(token_index));
            token_index++;
            break;
        case KCalcToken::TokenType::CLOSING_PARENTHESES_TYPE:
            if (token_stack_.isEmpty()) {
                errorIndex = token_index;
                return SYNTAX_ERROR;
            }
            switch (token_stack_.last().getTokenType()) {
            case KCalcToken::TokenType::BINARY_FUNCTION_TYPE:
            case KCalcToken::TokenType::RIGHT_UNARY_FUNCTION_TYPE:
                errorIndex = token_index;
                return SYNTAX_ERROR;
                break;
            default:
                reduce_Stack_();
                break;
            }
            token_index++;
            break;
        default:
            break;
        }
    }

    qCDebug(KCALC_LOG) << "Done processing token list";
    // printStacks_();

    reduce_Stack_(/*toParentheses =*/false);

    qCDebug(KCALC_LOG) << "Done reducing final token stack";

    if (token_stack_.isEmpty()) {
        buffer_result_ = KNumber::Zero;
        return EMPTY_INPUT;
    } else if (token_stack_.last().isKNumber() && token_stack_.last().getKNumber().type() == KNumber::Type::TYPE_ERROR) {
        error_ = true;
        buffer_result_ = token_stack_.last().getKNumber();
        token_stack_.clear();
        return MATH_ERROR;
    } else if (token_stack_.size() > 2) {
        error_ = true;
        buffer_result_ = KNumber::NaN;
        token_stack_.clear();
        return SYNTAX_ERROR;
    } else if (token_stack_.size() == 2 && token_stack_.last().isBinaryFunction() && token_stack_.at(0).isKNumber()) {
        error_ = true;
        if (token_stack_.at(0).getKNumber().type() == KNumber::Type::TYPE_ERROR) {
            token_stack_.clear();
            return MATH_ERROR;
        } else {
            token_stack_.clear();
            return MISIING_RIGHT_BINARY_ARG;
        }
    } else if (!token_stack_.last().isKNumber()) {
        error_ = true;
        token_stack_.clear();
        return SYNTAX_ERROR;
    } else {
        buffer_result_ = token_stack_.last().getKNumber();
        token_stack_.clear();
    }

    qCDebug(KCALC_LOG) << "Result: " << buffer_result_.toQString(12, -1);

    error_ = false;
    return SUCCESS;
}

int CalcEngine::insert_KNumber_Token_In_Stack_(const KCalcToken &token)
{
    qCDebug(KCALC_LOG) << "Inserting KNumber Token in stack";
    // printStacks_();
    KCalcToken const *tokenFunction;
    KCalcToken const *tokenFirstArg;

    KCalcToken tokenToInsert = token;

    while (!token_stack_.isEmpty()) {
        if (token_stack_.last().isKNumber()) {
            insert_Binary_Function_Token_In_Stack_(multiplication_Token_);
            break;
        }
        if (token_stack_.last().isBinaryFunction() || token_stack_.last().isOpeningParentheses()) {
            break;
        }
        if (token_stack_.last().isLeftUnaryFunction()) {
            // never executed //
            break;
        }
        while (token_stack_.last().isRightUnaryFunction()) {
            tokenFunction = &token_stack_.last();
            tokenFirstArg = &tokenToInsert;
            KNumber result = tokenFunction->evaluate(tokenFirstArg->getKNumber());
            token_stack_.pop_back();
            tokenToInsert = KCalcToken(result);

            if (token_stack_.isEmpty()) {
                break;
            }
        }
    }
    token_stack_.push_back(tokenToInsert);
    return 0;
}

int CalcEngine::insert_Binary_Function_Token_In_Stack_(const KCalcToken &token)
{
    qCDebug(KCALC_LOG) << "Insert Binary Function Token in stack";
    // printStacks_();
    KCalcToken const *tokenFunction;
    KCalcToken const *tokenFirstArg;
    KCalcToken const *tokenSecondArg;

    if (token_stack_.isEmpty()) {
        return -1;
    }

    if (token_stack_.size() <= 2) {
        token_stack_.push_back(token);
        return 0;
    }

    if (token_stack_.at(token_stack_.size() - 2).isRightUnaryFunction() && token_stack_.last().isKNumber()) {
        tokenFunction = &token_stack_.at(token_stack_.size() - 2);
        tokenFirstArg = &token_stack_.last();
        KNumber result = tokenFunction->evaluate(tokenFirstArg->getKNumber());
        token_stack_.pop_back();
        token_stack_.pop_back();
        insert_KNumber_Token_In_Stack_(KCalcToken(result));
        if (token_stack_.size() <= 2) {
            token_stack_.push_back(token);
            return 0;
        } // else continue inserting
    }

    if (token_stack_.at(token_stack_.size() - 1).isKNumber() && token_stack_.at(token_stack_.size() - 2).isBinaryFunction()
        && token_stack_.at(token_stack_.size() - 3).isKNumber()) {
        if (token_stack_.size() > 3) {
            if (token_stack_.at(token_stack_.size() - 4).isRightUnaryFunction()) {
                token_stack_.push_back(token);
                return 0;
            }
        }
        if (token_stack_.at(token_stack_.size() - 2).getPriorityLevel() >= token.getPriorityLevel()) {
            tokenFunction = &token_stack_.at(token_stack_.size() - 2);
            tokenFirstArg = &token_stack_.at(token_stack_.size() - 3);
            tokenSecondArg = &token_stack_.at(token_stack_.size() - 1);
            KNumber result = tokenFunction->evaluate(tokenFirstArg->getKNumber(), tokenSecondArg->getKNumber());
            token_stack_.pop_back();
            token_stack_.pop_back();
            token_stack_.last().updateToken(result);
            token_stack_.push_back(token);
            return 0;
        } else {
            token_stack_.push_back(token);
            return 0;
        }
    }

    token_stack_.push_back(token);

    return 0;
}

int CalcEngine::reduce_Stack_(bool toParentheses /*= true*/)
{
    KCalcToken const *tokenFunction;
    KCalcToken const *tokenFirstArg;
    KCalcToken const *tokenSecondArg;

    while (token_stack_.size() > 1) {
        qCDebug(KCALC_LOG) << "Reducing at stack size: " << token_stack_.size();
        // printStacks_();
        if (token_stack_.last().isOpeningParentheses()) {
            token_stack_.pop_back();
            if (toParentheses) {
                return 0;
            } else {
                continue;
            }
        }

        if (token_stack_.last().isKNumber() && token_stack_.at(token_stack_.size() - 2).isOpeningParentheses()) {
            KNumber result = token_stack_.last().getKNumber();
            token_stack_.pop_back();
            token_stack_.pop_back();
            insert_KNumber_Token_In_Stack_(KCalcToken(result));
            if (toParentheses) {
                return 0;
            } else {
                continue;
            }
        }

        if (token_stack_.last().isKNumber() && token_stack_.at(token_stack_.size() - 2).isRightUnaryFunction()) {
            tokenFunction = &token_stack_.at(token_stack_.size() - 2);
            tokenFirstArg = &token_stack_.last();
            KNumber result = tokenFunction->evaluate(tokenFirstArg->getKNumber());
            token_stack_.pop_back();
            token_stack_.pop_back();
            insert_KNumber_Token_In_Stack_(KCalcToken(result));
            continue;
        }

        if (token_stack_.last().isLeftUnaryFunction() && token_stack_.at(token_stack_.size() - 2).isKNumber()) {
            // never executed
            tokenFunction = &token_stack_.last();
            tokenFirstArg = &token_stack_.at(token_stack_.size() - 2);
            KNumber result = tokenFunction->evaluate(tokenFirstArg->getKNumber());
            token_stack_.pop_back();
            token_stack_.last().updateToken(result);
            continue;
        }

        if (token_stack_.size() > 3) {
            if (token_stack_.at(token_stack_.size() - 3).isKNumber() && token_stack_.at(token_stack_.size() - 4).isRightUnaryFunction()) {
                tokenFunction = &token_stack_.at(token_stack_.size() - 4);
                tokenFirstArg = &token_stack_.at(token_stack_.size() - 3);

                KNumber result = tokenFunction->evaluate(tokenFirstArg->getKNumber());

                token_stack_.removeAt(token_stack_.size() - 4);
                token_stack_.replace(token_stack_.size() - 3, KCalcToken(result));
                continue;
            }

            if (token_stack_.at(token_stack_.size() - 1).isKNumber() && token_stack_.at(token_stack_.size() - 2).isBinaryFunction()
                && token_stack_.at(token_stack_.size() - 3).isBinaryFunction() && token_stack_.at(token_stack_.size() - 4).isKNumber()) {
                if (token_stack_.at(token_stack_.size() - 2).getTokenCode() != KCalcToken::TokenCode::MINUS) {
                    return -1;
                }

                tokenFunction = &token_stack_.at(token_stack_.size() - 3);
                tokenFirstArg = &token_stack_.at(token_stack_.size() - 4);
                tokenSecondArg = &token_stack_.at(token_stack_.size() - 1);
                KNumber result = tokenFunction->evaluate(tokenFirstArg->getKNumber(), -tokenSecondArg->getKNumber());

                token_stack_.pop_back();
                token_stack_.pop_back();
                token_stack_.pop_back();
                token_stack_.last().updateToken(result);
                continue;
            }
            if (token_stack_.at(token_stack_.size() - 1).isKNumber() && token_stack_.at(token_stack_.size() - 2).isBinaryFunction()
                && token_stack_.at(token_stack_.size() - 3).isKNumber() && token_stack_.at(token_stack_.size() - 4).isBinaryFunction()) {
                if (token_stack_.at(token_stack_.size() - 2).getTokenCode() == KCalcToken::TokenCode::MINUS
                    || token_stack_.at(token_stack_.size() - 2).getTokenCode() == KCalcToken::TokenCode::PLUS) {
                    if (token_stack_.at(token_stack_.size() - 4).getTokenCode() == KCalcToken::TokenCode::MINUS) {
                        tokenSecondArg = &token_stack_.at(token_stack_.size() - 1);
                        tokenFunction = &token_stack_.at(token_stack_.size() - 2);
                        tokenFirstArg = &token_stack_.at(token_stack_.size() - 3);
                        KNumber result;
                        if (tokenFunction->isSecondArgInverted()) {
                            result = tokenFunction->evaluate(tokenFirstArg->getKNumber(), tokenSecondArg->getKNumber());
                        } else {
                            result = tokenFunction->evaluate(-tokenFirstArg->getKNumber(), tokenSecondArg->getKNumber());
                        }
                        token_stack_.pop_back();
                        token_stack_.pop_back();
                        token_stack_.pop_back();
                        token_stack_.last().updateToken(KCalcToken::TokenCode::PLUS);
                        token_stack_.push_back(KCalcToken(result));
                        continue;
                    }
                }
            }
        }

        if (token_stack_.size() > 2) {
            if (token_stack_.last().isKNumber() && token_stack_.at(token_stack_.size() - 2).isBinaryFunction()
                && token_stack_.at(token_stack_.size() - 3).isKNumber()) {
                tokenSecondArg = &token_stack_.last();
                tokenFunction = &token_stack_.at(token_stack_.size() - 2);
                tokenFirstArg = &token_stack_.at(token_stack_.size() - 3);

                KNumber result = tokenFunction->evaluate(tokenFirstArg->getKNumber(), tokenSecondArg->getKNumber());

                token_stack_.pop_back();
                token_stack_.pop_back();
                token_stack_.last().updateToken(result);
                continue;
            }
            if (token_stack_.at(token_stack_.size() - 1).isKNumber() && token_stack_.at(token_stack_.size() - 2).isBinaryFunction()
                && token_stack_.at(token_stack_.size() - 3).isBinaryFunction()) {
                if (token_stack_.at(token_stack_.size() - 2).getTokenCode() != KCalcToken::TokenCode::MINUS) {
                    return -1;
                }
                KNumber result = -token_stack_.last().getKNumber();
                token_stack_.pop_back();
                token_stack_.last().updateToken(result);
                continue;
            }
        }

        if (token_stack_.at(token_stack_.size() - 1).isKNumber() && token_stack_.at(token_stack_.size() - 2).isBinaryFunction()) {
            if (token_stack_.at(token_stack_.size() - 2).getTokenCode() != KCalcToken::TokenCode::MINUS) {
                return -1;
            }
            KNumber result = -token_stack_.last().getKNumber();
            token_stack_.pop_back();
            token_stack_.last().updateToken(result);
            continue;
        }

        qCDebug(KCALC_LOG) << "Error at stack size = " << token_stack_.size();
        return -1;
    }
    return 0;
}

void CalcEngine::printStacks_()
{
    int tokenStaskSize = token_stack_.size();

    qCDebug(KCALC_LOG) << "Printing current stack:";

    for (int i = 0; i < tokenStaskSize; i++) {
        if (token_stack_.at(i).isKNumber()) {
            qCDebug(KCALC_LOG) << "TokenStack at:" << i << " is KNumber   = " << (token_stack_.at(i).getKNumber()).toQString();

        } else {
            qCDebug(KCALC_LOG) << "TokenStack at:" << i << " is TokenCode = " << (token_stack_.at(i).getTokenCode());
        }
    }

    qCDebug(KCALC_LOG) << "Print current stack done";
}

KNumber CalcEngine::getResult()
{
    return buffer_result_;
}
