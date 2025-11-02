/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_core_p.h"

KNumber CalcEngineP::deg2Rad(const KNumber &x)
{
    return x * (KNumber::Pi() / KNumber(180));
}

KNumber CalcEngineP::gra2Rad(const KNumber &x)
{
    return x * (KNumber::Pi() / KNumber(200));
}

KNumber CalcEngineP::rad2Deg(const KNumber &x)
{
    return x * (KNumber(180) / KNumber::Pi());
}

KNumber CalcEngineP::rad2Gra(const KNumber &x)
{
    return x * (KNumber(200) / KNumber::Pi());
}

KNumber CalcEngineP::execOr(const KNumber &leftOp, const KNumber &rightOp)
{
    return leftOp | rightOp;
}

KNumber CalcEngineP::execXor(const KNumber &leftOp, const KNumber &rightOp)
{
    return leftOp ^ rightOp;
}

KNumber CalcEngineP::execAnd(const KNumber &leftOp, const KNumber &rightOp)
{
    return leftOp & rightOp;
}

KNumber CalcEngineP::execLsh(const KNumber &leftOp, const KNumber &rightOp)
{
    return leftOp << rightOp;
}

KNumber CalcEngineP::execRsh(const KNumber &leftOp, const KNumber &rightOp)
{
    return leftOp >> rightOp;
}

KNumber CalcEngineP::execAdd(const KNumber &leftOp, const KNumber &rightOp)
{
    return leftOp + rightOp;
}

KNumber CalcEngineP::execSubtract(const KNumber &leftOp, const KNumber &rightOp)
{
    return leftOp - rightOp;
}

KNumber CalcEngineP::execMultiply(const KNumber &leftOp, const KNumber &rightOp)
{
    return leftOp * rightOp;
}

KNumber CalcEngineP::execDivide(const KNumber &leftOp, const KNumber &rightOp)
{
    return leftOp / rightOp;
}

KNumber CalcEngineP::execMod(const KNumber &leftOp, const KNumber &rightOp)
{
    return leftOp % rightOp;
}

KNumber CalcEngineP::execIntDiv(const KNumber &leftOp, const KNumber &rightOp)
{
    return (leftOp / rightOp).integerPart();
}

KNumber CalcEngineP::execBinom(const KNumber &leftOp, const KNumber &rightOp)
{
    return leftOp.bin(rightOp);
}

KNumber CalcEngineP::execPower(const KNumber &leftOp, const KNumber &rightOp)
{
    return leftOp.pow(rightOp);
}

KNumber CalcEngineP::execPwrRoot(const KNumber &leftOp, const KNumber &rightOp)
{
    return leftOp.pow(KNumber::One / rightOp);
}

KNumber CalcEngineP::execAddP(const KNumber &leftOp, const KNumber &rightOp)
{
    return leftOp * (KNumber::One + rightOp / KNumber(100));
}

KNumber CalcEngineP::execSubP(const KNumber &leftOp, const KNumber &rightOp)
{
    return leftOp * (KNumber::One - rightOp / KNumber(100));
}

KNumber CalcEngineP::execMultiplyP(const KNumber &leftOp, const KNumber &rightOp)
{
    return leftOp * rightOp / KNumber(100);
}

KNumber CalcEngineP::execDivideP(const KNumber &leftOp, const KNumber &rightOp)
{
    return leftOp * KNumber(100) / rightOp;
}

// move a number into the interval [0,360) by adding multiples of 360
KNumber CalcEngineP::moveIntoDegInterval(const KNumber &num)
{
    KNumber tmpNum = num - (num / KNumber(360)).integerPart() * KNumber(360);
    if (tmpNum < KNumber::Zero) {
        return tmpNum + KNumber(360);
    }
    return tmpNum;
}

// move a number into the interval [0,400) by adding multiples of 400
KNumber CalcEngineP::moveIntoGradInterval(const KNumber &num)
{
    KNumber tmpNum = num - (num / KNumber(400)).integerPart() * KNumber(400);
    if (tmpNum < KNumber::Zero) {
        return tmpNum + KNumber(400);
    }
    return tmpNum;
}

KNumber CalcEngineP::arcCosDeg(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }

    if (input.type() == KNumber::TypeInteger) {
        if (input == KNumber::One) {
            return KNumber::Zero;
        }
        if (input == -KNumber::One) {
            return KNumber(180);
        }
        if (input == KNumber::Zero) {
            return KNumber(90);
        }
    }
    return rad2Deg(input.acos());
}

KNumber CalcEngineP::arcCosRad(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }
    return input.acos();
}

KNumber CalcEngineP::arcCosGrad(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }
    if (input.type() == KNumber::TypeInteger) {
        if (input == KNumber::One) {
            return KNumber::Zero;
        }
        if (input == -KNumber::One) {
            return KNumber(200);
        }
        if (input == KNumber::Zero) {
            return KNumber(100);
        }
    }
    return rad2Gra(input.acos());
}

KNumber CalcEngineP::arcSinDeg(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }
    if (input.type() == KNumber::TypeInteger) {
        if (input == KNumber::One) {
            return KNumber(90);
        }
        if (input == -KNumber::One) {
            return KNumber(-90);
        }
        if (input == KNumber::Zero) {
            return KNumber::Zero;
        }
    }
    return rad2Deg(input.asin());
}

KNumber CalcEngineP::arcSinRad(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }
    return input.asin();
}

KNumber CalcEngineP::arcSinGrad(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }
    if (input.type() == KNumber::TypeInteger) {
        if (input == KNumber::One) {
            return KNumber(100);
        }
        if (input == -KNumber::One) {
            return KNumber(-100);
        }
        if (input == KNumber::Zero) {
            return KNumber::Zero;
        }
    }
    return rad2Gra(input.asin());
}

KNumber CalcEngineP::arcTangensDeg(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        if (input == KNumber::NaN) {
            return KNumber::NaN;
        }
        if (input == KNumber::PosInfinity) {
            return KNumber(90);
        }
        if (input == KNumber::NegInfinity) {
            return KNumber(-90);
        }
    }

    return rad2Deg(input.atan());
}

KNumber CalcEngineP::arcTangensRad(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        if (input == KNumber::NaN) {
            return KNumber::NaN;
        }
        if (input == KNumber::PosInfinity) {
            return KNumber::Pi() / KNumber(2);
        }
        if (input == KNumber::NegInfinity) {
            return -KNumber::Pi() / KNumber(2);
        }
    }

    return input.atan();
}

KNumber CalcEngineP::arcTangensGrad(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        if (input == KNumber::NaN) {
            return KNumber::NaN;
        }
        if (input == KNumber::PosInfinity) {
            return KNumber(100);
        }
        if (input == KNumber::NegInfinity) {
            return KNumber(-100);
        }
    }

    return rad2Gra(input.atan());
}

KNumber CalcEngineP::areaCosHyp(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        if (input == KNumber::NaN) {
            return KNumber::NaN;
        }
        if (input == KNumber::PosInfinity) {
            return KNumber::PosInfinity;
        }
        if (input == KNumber::NegInfinity) {
            return KNumber::NaN;
        }
    }

    if (input == KNumber::One) {
        return KNumber::Zero;
    }
    return input.acosh();
}

KNumber CalcEngineP::areaSinHyp(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        if (input == KNumber::NaN) {
            return KNumber::NaN;
        }
        if (input == KNumber::PosInfinity) {
            return KNumber::PosInfinity;
        }
        if (input == KNumber::NegInfinity) {
            return KNumber::NegInfinity;
        }
    }

    if (input == KNumber::Zero) {
        return KNumber::Zero;
    }
    return input.asinh();
}

KNumber CalcEngineP::areaTangensHyp(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }

    if (input == KNumber::One) {
        return KNumber::PosInfinity;
    }
    if (input == -KNumber::One) {
        return KNumber::NegInfinity;
    }
    return input.atanh();
}

KNumber CalcEngineP::complement(const KNumber &input)
{
    if (input.type() != KNumber::TypeInteger) {
        return KNumber::NaN;
    }

    return ~input;
}

KNumber CalcEngineP::cosDeg(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }

    KNumber truncInput = moveIntoDegInterval(input);

    if (truncInput.type() == KNumber::TypeInteger) {
        KNumber mult = truncInput / KNumber(90);
        if (mult.type() == KNumber::TypeInteger) {
            if (mult == KNumber::Zero) {
                return KNumber::One;
            } else if (mult == KNumber::One) {
                return KNumber::Zero;
            } else if (mult == KNumber(2)) {
                return KNumber::NegOne;
            } else if (mult == KNumber(3)) {
                return KNumber::Zero;
            } else {
                return KNumber::NaN;
            }
        }
    }

    truncInput = deg2Rad(truncInput);
    return truncInput.cos();
}

KNumber CalcEngineP::cosRad(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }

    return input.cos();
}

KNumber CalcEngineP::cosGrad(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }
    KNumber truncInput = moveIntoGradInterval(input);
    if (truncInput.type() == KNumber::TypeInteger) {
        KNumber mult = truncInput / KNumber(100);
        if (mult.type() == KNumber::TypeInteger) {
            if (mult == KNumber::Zero) {
                return KNumber::One;
            } else if (mult == KNumber::One) {
                return KNumber::Zero;
            } else if (mult == KNumber(2)) {
                return KNumber::NegOne;
            } else if (mult == KNumber(3)) {
                return KNumber::Zero;
            } else {
                return KNumber::NaN;
            }
        }
    }
    truncInput = gra2Rad(truncInput);

    return truncInput.cos();
}

KNumber CalcEngineP::cosHyp(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        if (input == KNumber::NaN) {
            return KNumber::NaN;
        }
        if (input == KNumber::PosInfinity) {
            return KNumber::PosInfinity;
        }
        if (input == KNumber::NegInfinity) {
            return KNumber::PosInfinity;
        }
        return KNumber::NaN;
    }

    return input.cosh();
}

KNumber CalcEngineP::cube(const KNumber &input)
{
    return input * input * input;
}

KNumber CalcEngineP::cubeRoot(const KNumber &input)
{
    return input.cbrt();
}

KNumber CalcEngineP::exp(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        if (input == KNumber::NaN) {
            return KNumber::NaN;
        }
        if (input == KNumber::PosInfinity) {
            return KNumber::PosInfinity;
        }
        if (input == KNumber::NegInfinity) {
            return KNumber::Zero;
        }
        return KNumber::NaN;
    }
    return KNumber::Euler().pow(input);
}

KNumber CalcEngineP::exp10(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        if (input == KNumber::NaN) {
            return KNumber::NaN;
        }
        if (input == KNumber::PosInfinity) {
            return KNumber::PosInfinity;
        }
        if (input == KNumber::NegInfinity) {
            return KNumber::Zero;
        }
        return KNumber::NaN;
    }
    return KNumber(10).pow(input);
}

KNumber CalcEngineP::factorial(const KNumber &input)
{
    if (input == KNumber::PosInfinity) {
        return KNumber::PosInfinity;
    }
    if ((input.type() == KNumber::TypeInteger && input < KNumber::Zero) || input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }

    return input.factorial();
}

KNumber CalcEngineP::gamma(const KNumber &input)
{
    if (input == KNumber::PosInfinity) {
        return KNumber::PosInfinity;
    }
    if ((input.type() == KNumber::TypeInteger && input <= KNumber::Zero) || input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }

    return input.tgamma();
}

KNumber CalcEngineP::invertSign(const KNumber &input)
{
    return -input;
}

KNumber CalcEngineP::ln(const KNumber &input)
{
    if (input == KNumber::One) {
        return KNumber::Zero;
    } else {
        return input.ln();
    }
}

KNumber CalcEngineP::log10(const KNumber &input)
{
    if (input == KNumber::One) {
        return KNumber::Zero;
    } else {
        return input.log10();
    }
}

KNumber CalcEngineP::reciprocal(const KNumber &input)
{
    return KNumber::One / input;
}

KNumber CalcEngineP::sinDeg(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }

    KNumber truncInput = moveIntoDegInterval(input);
    if (truncInput.type() == KNumber::TypeInteger) {
        KNumber mult = truncInput / KNumber(90);
        if (mult.type() == KNumber::TypeInteger) {
            if (mult == KNumber::Zero) {
                return KNumber::Zero;
            } else if (mult == KNumber::One) {
                return KNumber::One;
            } else if (mult == KNumber(2)) {
                return KNumber::Zero;
            } else if (mult == KNumber(3)) {
                return KNumber::NegOne;
            } else {
                return KNumber::NaN;
            }
        }
    }
    truncInput = deg2Rad(truncInput);

    return truncInput.sin();
}

KNumber CalcEngineP::sinRad(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }

    return input.sin();
}

KNumber CalcEngineP::sinGrad(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }

    KNumber truncInput = moveIntoGradInterval(input);
    if (truncInput.type() == KNumber::TypeInteger) {
        KNumber mult = truncInput / KNumber(100);
        if (mult.type() == KNumber::TypeInteger) {
            if (mult == KNumber::Zero) {
                return KNumber::Zero;
            } else if (mult == KNumber::One) {
                return KNumber::One;
            } else if (mult == KNumber(2)) {
                return KNumber::Zero;
            } else if (mult == KNumber(3)) {
                return KNumber::NegOne;
            } else {
                return KNumber::NaN;
            }
        }
    }

    truncInput = gra2Rad(truncInput);

    return truncInput.sin();
}

KNumber CalcEngineP::sinHyp(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        if (input == KNumber::NaN) {
            return KNumber::NaN;
        }
        if (input == KNumber::PosInfinity) {
            return KNumber::PosInfinity;
        }
        if (input == KNumber::NegInfinity) {
            return KNumber::NegInfinity;
        }
        return KNumber::NaN;
    }

    return input.sinh();
}

KNumber CalcEngineP::square(const KNumber &input)
{
    return input * input;
}

KNumber CalcEngineP::squareRoot(const KNumber &input)
{
    return input.sqrt();
}

KNumber CalcEngineP::percentage(const KNumber &input)
{
    return execDivide(input, KNumber::OneHundred);
}

void CalcEngineP::statClearAll(const KNumber &input)
{
    Q_UNUSED(input);
    m_stats.clearAll();
}

KNumber CalcEngineP::statCount(const KNumber &input)
{
    Q_UNUSED(input);
    return KNumber(m_stats.count());
}

void CalcEngineP::statDataNew(const KNumber &input)
{
    m_stats.enterData(input);
}

void CalcEngineP::statDataDel(const KNumber &input)
{
    Q_UNUSED(input);
    m_stats.clearLast();
}

KNumber CalcEngineP::statMean(const KNumber &input)
{
    Q_UNUSED(input);
    return m_stats.mean();
}

KNumber CalcEngineP::statMedian(const KNumber &input)
{
    Q_UNUSED(input);
    return m_stats.median();
}

KNumber CalcEngineP::statStdDeviation(const KNumber &input)
{
    Q_UNUSED(input);
    return m_stats.std();
}

KNumber CalcEngineP::statStdSample(const KNumber &input)
{
    Q_UNUSED(input);
    return m_stats.sampleStd();
}

KNumber CalcEngineP::statSum(const KNumber &input)
{
    Q_UNUSED(input);
    return m_stats.sum();
}

KNumber CalcEngineP::statSumSquares(const KNumber &input)
{
    Q_UNUSED(input);
    return m_stats.sumOfSquares();
}

KNumber CalcEngineP::tangensDeg(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }

    KNumber arg1 = sinDeg(input);
    KNumber arg2 = cosDeg(input);

    return arg1 / arg2;
}

KNumber CalcEngineP::tangensRad(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }

    KNumber arg1 = sinRad(input);
    KNumber arg2 = cosRad(input);

    return arg1 / arg2;
}

KNumber CalcEngineP::tangensGrad(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }

    KNumber arg1 = sinGrad(input);
    KNumber arg2 = cosGrad(input);

    return arg1 / arg2;
}

KNumber CalcEngineP::polarDeg(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }

    KNumber arg1 = cosDeg(input);
    KNumber arg2 = sinDeg(input);

    return arg1 + arg2 * KNumber(QStringLiteral("i"));
}

KNumber CalcEngineP::polarRad(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }

    KNumber arg1 = cosRad(input);
    KNumber arg2 = sinRad(input);

    return arg1 + arg2 * KNumber(QStringLiteral("i"));
}

KNumber CalcEngineP::polarGrad(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        return KNumber::NaN;
    }

    KNumber arg1 = cosGrad(input);
    KNumber arg2 = sinGrad(input);

    return arg1 + arg2 * KNumber(QStringLiteral("i"));
}

KNumber CalcEngineP::tangensHyp(const KNumber &input)
{
    if (input.type() == KNumber::TypeError) {
        if (input == KNumber::NaN) {
            return KNumber::NaN;
        }
        if (input == KNumber::PosInfinity) {
            return KNumber::One;
        }
        if (input == KNumber::NegInfinity) {
            return KNumber::NegOne;
        }
        return KNumber::NaN;
    }

    return input.tanh();
}

KNumber CalcEngineP::conjugate(const KNumber &input)
{
    return input.conj();
}

KNumber CalcEngineP::argumentDeg(const KNumber &input)
{
    if (input.realPart() == KNumber::Zero) {
        return (input.imaginaryPart() > KNumber::Zero) ? KNumber(90) : KNumber(-90);
    } else if (input.imaginaryPart() == KNumber::Zero) {
        return (input.realPart() >= KNumber::Zero) ? KNumber::Zero : KNumber(180);
    } else {
        return rad2Deg(input.arg());
    }
}

KNumber CalcEngineP::argumentGrad(const KNumber &input)
{
    if (input.realPart() == KNumber::Zero) {
        return (input.imaginaryPart() > KNumber::Zero) ? KNumber(100) : KNumber(-100);
    } else if (input.imaginaryPart() == KNumber::Zero) {
        return (input.realPart() >= KNumber::Zero) ? KNumber::Zero : KNumber(200);
    } else {
        return rad2Gra(input.arg());
    }
}

KNumber CalcEngineP::argumentRad(const KNumber &input)
{
    if (input.realPart() == KNumber::Zero) {
        return (input.imaginaryPart() > KNumber::Zero) ? KNumber::Pi() / KNumber(2) : -KNumber::Pi() / KNumber(2);
    } else if (input.imaginaryPart() == KNumber::Zero) {
        return (input.realPart() >= KNumber::Zero) ? KNumber::Zero : KNumber::Pi();
    } else {
        return input.arg();
    }
}

KNumber CalcEngineP::real(const KNumber &input)
{
    return input.realPart();
}

KNumber CalcEngineP::imaginary(const KNumber &input)
{
    return input.imaginaryPart();
}
