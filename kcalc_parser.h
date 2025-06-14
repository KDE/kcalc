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
#include <QHash>
#include <QLocale>
#include <QQueue>
#include <QString>

class KCalcParser
{
public:
    KCalcParser();
    ~KCalcParser();

    enum ParsingResult {
        Success,
        SuccessSingleKNumber,
        Empty,
        InvalidToken,
    };

    enum TrigonometricMode {
        Degrees,
        Radians,
        Gradians
    };

    int loadConstants(const QDomDocument &doc);

    void setTrigonometricMode(int mode);
    int getTrigonometricMode();

    void setNumeralMode(bool numeralMode);
    bool getNumeralMode() const;

    ParsingResult getParsingResult();

    ParsingResult stringToTokenQueue(const QString &buffer, int base, QQueue<KCalcToken> &tokenQueue, int &errorIndex);
    /*TODO: int stringToHTMLString(const QString& buffer, QString& htmlText);*/

    const QString tokenToString(KCalcToken::TokenCode tokenCode);

private:
    KCalcToken::TokenCode stringToToken(const QString &string, int &index, int base = 10);

    static const inline QString spaceStr = QStringLiteral(" ");
    static const inline QString thinSpaceStr = QStringLiteral(" ");

    static const inline QString zeroStr = QStringLiteral("0");
    static const inline QString nineStr = QStringLiteral("9");
    static const inline QString decimalPointStr = QLocale().decimalPoint();
    static const inline QString commaStr = QLatin1String(",");
    static const inline QString pointStr = QLatin1String(".");

    static const inline QString aStr = QStringLiteral("A");
    static const inline QString fStr = QStringLiteral("F");
    static const inline QString aLowerCaseStr = QStringLiteral("a");
    static const inline QString fLowerCaseStr = QStringLiteral("f");
    static const inline QString bStr = QStringLiteral("b");
    static const inline QString xStr = QStringLiteral("x");

    static const inline QString angleStr = QStringLiteral("∠");

    static const inline QString binaryNumberPrefixStr = QStringLiteral("0b");
    static const inline QString octalNumberPrefixStr = QStringLiteral("0o");
    static const inline QString octalNumberPrefixStrCStyle = QStringLiteral("0");
    static const inline QString hexNumberPrefixStr = QStringLiteral("0x");

    static const QString binaryDigitsPattern;
    static const QString octalDigitsPattern;
    static const QString hexDigitsPattern;

    static const QRegularExpression binaryNumberDigitsRegex;
    static const QRegularExpression octalNumberDigitsRegex;
    static const QRegularExpression hexNumberDigitsRegex;

    static const QString decimalNumberPattern;
    static const QRegularExpression decimalNumberRegex;

    static const inline QString eStr = QStringLiteral("e");
    static const inline QString piStr = QStringLiteral("π");
    static const inline QString phiStr = QStringLiteral("ϕ");
    static const inline QString iStr = QStringLiteral("i");
    static const inline QString posInfinityStr = QStringLiteral("∞");
    static const inline QString negInfinityStr = QStringLiteral("-∞");

    static const inline QString vacuumPermitivityStr = QStringLiteral("ε₀");
    static const inline QString vacuumPermeabilityStr = QStringLiteral("μ₀");
    static const inline QString vacuumImpedanceStr = QStringLiteral("Z₀");

    static const inline QString plancksConstantStr = QStringLiteral("ℎ");
    static const inline QString plancksOver2PiStr = QStringLiteral("ℏ");

    static const inline QString plusStr = QStringLiteral("+");
    static const inline QString hyphenMinusStr = QStringLiteral("-");
    static const inline QString minusSignStr = QStringLiteral("−");
    static const inline QString multiplicationStr = QStringLiteral("×");
    static const inline QString divisionStr = QStringLiteral("÷");
    static const inline QString divisionSlashStr = QStringLiteral("∕");
    static const inline QString slashStr = QStringLiteral("/");
    static const inline QString dotStr = QStringLiteral("⋅");
    static const inline QString asteriskStr = QStringLiteral("*");
    static const inline QString percentageStr = QStringLiteral("%");
    static const inline QString permilleStr = QStringLiteral("‰");

    static const inline QString binomialStr = QStringLiteral("nCm");
    static const inline QString moduloStr = QStringLiteral("mod");
    static const inline QString integerDivisionStr = QStringLiteral("div");

    static const inline QString openingParenthesisStr = QStringLiteral("(");
    static const inline QString closingParenthesisStr = QStringLiteral(")");

    static const inline QString squareStr = QStringLiteral("²");
    static const inline QString cubeStr = QStringLiteral("³");
    static const inline QString reciprocalStr = QStringLiteral("⁻¹");

    static const inline QString log10Str = QStringLiteral("log");
    static const inline QString lnStr = QStringLiteral("ln");
    static const inline QString expStr = QStringLiteral("exp");
    static const inline QString exp10Str = QStringLiteral("⏨");
    static const inline QString powerStr = QStringLiteral("^");
    static const inline QString powerRootStr = QStringLiteral("⌄");
    static const inline QString squareRootStr = QStringLiteral("√");
    static const inline QString cubicRootStr = QStringLiteral("∛");

    static const inline QString factorialStr = QStringLiteral("!");
    static const inline QString doubleFactorialStr = QStringLiteral("‼");
    static const inline QString gammaStr = QStringLiteral("Γ");
    static const inline QString invertSignStr = QStringLiteral("–");

    static const inline QString degreeStr = QStringLiteral("°");
    static const inline QString gradianStr = QStringLiteral("gon");
    static const inline QString radianStr = QStringLiteral("rad");
    static const inline QString sinStr = QStringLiteral("sin");
    static const inline QString cosStr = QStringLiteral("cos");
    static const inline QString tanStr = QStringLiteral("tan");
    static const inline QString asinStr = QStringLiteral("asin");
    static const inline QString acosStr = QStringLiteral("acos");
    static const inline QString atanStr = QStringLiteral("atan");

    static const inline QString sinhStr = QStringLiteral("sinh");
    static const inline QString coshStr = QStringLiteral("cosh");
    static const inline QString tanhStr = QStringLiteral("tanh");
    static const inline QString asinhStr = QStringLiteral("asinh");
    static const inline QString acoshStr = QStringLiteral("acosh");
    static const inline QString atanhStr = QStringLiteral("atanh");

    static const inline QString conjStr = QStringLiteral("conj");
    static const inline QString argStr = QStringLiteral("arg");
    static const inline QString reStr = QStringLiteral("re");
    static const inline QString imStr = QStringLiteral("im");

    static const inline QString andStr = QStringLiteral("&");
    static const inline QString orStr = QStringLiteral("|");
    static const inline QString xorStr = QStringLiteral("⊕");
    static const inline QString xorLetterStr = QStringLiteral("XOR");
    static const inline QString rshStr = QStringLiteral(">>");
    static const inline QString lshStr = QStringLiteral("<<");
    static const inline QString onesCompStr = QStringLiteral("~");
    static const inline QString twosCompStr = QStringLiteral("~₂");

    static const inline QString equalStr = QStringLiteral("=");
    static const inline QString ansStr = QStringLiteral("ans");

    static const inline QString errorStr = QStringLiteral("error");

private:
    QHash<QString, QString> m_constants;
    bool constantSymbolToValue(const QString &constantSymbol);

    QString m_tokenKNumber;
    bool m_inputHasConstants = false;
    int m_trigonometricMode = Degrees;
    bool m_numeralMode = false;
    ParsingResult m_parsingResult = Empty;
};
