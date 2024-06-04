/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "kcalc_token.h"
#include "knumber/knumber.h"

#include <QChar>
#include <QDebug>
#include <QDomDocument>
#include <QList>
#include <QLocale>
#include <QQueue>
#include <QString>

class KCalcParser
{
public:
    KCalcParser();
    ~KCalcParser();

    enum ParsingResult {
        SUCCESS,
        SUCCESS_SINGLE_KNUMBER,
        EMPTY,
        INVALID_TOKEN,
    };

    enum TrigonometricMode { DEGREES, RADIANS, GRADIANS };

    int loadConstants(const QDomDocument &doc);

    void setTrigonometricMode(int mode);
    int getTrigonometricMode();

    ParsingResult getParsingResult();

    ParsingResult stringToTokenQueue(const QString &buffer, int base, QQueue<KCalcToken> &tokenQueue, int &errorIndex);
    /*TODO: int stringToHTMLString(const QString& buffer, QString& htmlText);*/

    const QString TokenToString(KCalcToken::TokenCode tokenCode);

private:
    KCalcToken::TokenCode stringToToken(const QString &string, int &index, int base = 10);

    static const inline QString SPACE_STR = QStringLiteral(" ");
    static const inline QString THIN_SPACE_STR = QStringLiteral(" ");

    static const inline QString ZERO_STR = QStringLiteral("0");
    static const inline QString NINE_STR = QStringLiteral("9");
    static const inline QString DECIMAL_POINT_STR = QLocale().decimalPoint();
    static const inline QString COMMA_STR = QLatin1String(",");
    static const inline QString POINT_STR = QLatin1String(".");

    static const inline QString A_STR = QStringLiteral("A");
    static const inline QString F_STR = QStringLiteral("F");
    static const inline QString B_STR = QStringLiteral("b");
    static const inline QString X_STR = QStringLiteral("x");

    static const inline QString BINARY_NUMBER_PREFIX_STR = QStringLiteral("0b");
    static const inline QString OCTAL_NUMBER_PREFIX_STR = QStringLiteral("0");
    static const inline QString HEX_NUMBER_PREFIX_STR = QStringLiteral("0x");

    static const inline QString E_STR = QStringLiteral("e");
    static const inline QString PI_STR = QStringLiteral("π");
    static const inline QString PHI_STR = QStringLiteral("ϕ");
    static const inline QString I_STR = QStringLiteral("i");
    static const inline QString POS_INFINITY_STR = QStringLiteral("∞");
    static const inline QString NEG_INFINITY_STR = QStringLiteral("-∞");

    static const inline QString VACUUM_PERMITIVITY_STR = QStringLiteral("ε₀");
    static const inline QString VACUUM_PERMEABILITY_STR = QStringLiteral("μ₀");
    static const inline QString VACUUM_IMPEDANCE_STR = QStringLiteral("Z₀");

    static const inline QString PLANCK_S_CONSTANT_STR = QStringLiteral("ℎ");
    static const inline QString PLANCK_S_OVER_2PI_STR = QStringLiteral("ℏ");

    static const inline QString PLUS_STR = QStringLiteral("+");
    static const inline QString HYPHEN_MINUS_STR = QStringLiteral("-");
    static const inline QString MINUS_SIGN_STR = QStringLiteral("−");
    static const inline QString MULTIPLICATION_STR = QStringLiteral("×");
    static const inline QString DIVISION_STR = QStringLiteral("÷");
    static const inline QString DIVISION_SLASH_STR = QStringLiteral("∕");
    static const inline QString SLASH_STR = QStringLiteral("/");
    static const inline QString DOT_STR = QStringLiteral("⋅");
    static const inline QString ASTERISK_STR = QStringLiteral("*");
    static const inline QString PERCENTAGE_STR = QStringLiteral("%");
    static const inline QString PERMILLE_STR = QStringLiteral("‰");

    static const inline QString BINOMIAL_STR = QStringLiteral("nCm");
    static const inline QString MODULO_STR = QStringLiteral("mod");
    static const inline QString INTEGER_DIVISION_STR = QStringLiteral("div");

    static const inline QString OPENING_PARENTHESIS_STR = QStringLiteral("(");
    static const inline QString CLOSING_PARENTHESIS_STR = QStringLiteral(")");

    static const inline QString SQUARE_STR = QStringLiteral("²");
    static const inline QString CUBE_STR = QStringLiteral("³");
    static const inline QString RECIPROCAL_STR = QStringLiteral("⁻¹");

    static const inline QString LOG_10_STR = QStringLiteral("log");
    static const inline QString LN_STR = QStringLiteral("ln");
    static const inline QString EXP_STR = QStringLiteral("exp");
    static const inline QString EXP_10_STR = QStringLiteral("⏨");
    static const inline QString POWER_STR = QStringLiteral("^");
    static const inline QString POWER_ROOT_STR = QStringLiteral("⌄");
    static const inline QString SQUARE_ROOT_STR = QStringLiteral("√");
    static const inline QString CUBIC_ROOT_STR = QStringLiteral("∛");

    static const inline QString FACTORIAL_STR = QStringLiteral("!");
    static const inline QString DOUBLE_FACTORIAL_STR = QStringLiteral("‼");
    static const inline QString GAMMA_STR = QStringLiteral("Γ");
    static const inline QString INVERT_SIGN_STR = QStringLiteral("–");

    static const inline QString DEGREE_STR = QStringLiteral("°");
    static const inline QString GRADIAN_STR = QStringLiteral("gon");
    static const inline QString RADIAN_STR = QStringLiteral("rad");
    static const inline QString SIN_STR = QStringLiteral("sin");
    static const inline QString COS_STR = QStringLiteral("cos");
    static const inline QString TAN_STR = QStringLiteral("tan");
    static const inline QString ASIN_STR = QStringLiteral("asin");
    static const inline QString ACOS_STR = QStringLiteral("acos");
    static const inline QString ATAN_STR = QStringLiteral("atan");

    static const inline QString SINH_STR = QStringLiteral("sinh");
    static const inline QString COSH_STR = QStringLiteral("cosh");
    static const inline QString TANH_STR = QStringLiteral("tanh");
    static const inline QString ASINH_STR = QStringLiteral("asinh");
    static const inline QString ACOSH_STR = QStringLiteral("acosh");
    static const inline QString ATANH_STR = QStringLiteral("atanh");

    static const inline QString AND_STR = QStringLiteral("&");
    static const inline QString OR_STR = QStringLiteral("|");
    static const inline QString XOR_STR = QStringLiteral("⊕");
    static const inline QString XOR_LETTER_STR = QStringLiteral("XOR");
    static const inline QString RSH_STR = QStringLiteral(">>");
    static const inline QString LSH_STR = QStringLiteral("<<");
    static const inline QString ONE_S_COMP_STR = QStringLiteral("~");
    static const inline QString TWO_S_COMP_STR = QStringLiteral("~₂");

    static const inline QString EQUAL_STR = QStringLiteral("=");
    static const inline QString ANS_STR = QStringLiteral("ans");

    static const inline QString ERROR_STR = QStringLiteral("error");

private:
    typedef struct {
        QString symbol;
        QString value;
    } constant_;

    QList<constant_> constants_;
    bool constantSymbolToValue_(const QString &constantSymbol);

    QString token_KNumber_;
    bool m_inputHasConstants = false;
    int trigonometric_Mode_ = DEGREES;
    ParsingResult parsing_Result_ = EMPTY;
};
