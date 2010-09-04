/*

    KCalc, a scientific calculator for the X window system using the
    Qt widget libraries, available at no cost at http://www.troll.no

    Copyright (C) 1996 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KCALC_CORE_H_
#define KCALC_CORE_H_

#include <QStack>
#include "stats.h"
#include "knumber.h"

#define  POS_ZERO  1e-19L  /* What we consider zero is */
#define  NEG_ZERO -1e-19L  /* anything between these two */


typedef KNumber(*Arith)(const KNumber &, const KNumber &);
typedef KNumber(*Prcnt)(const KNumber &, const KNumber &);

struct operator_data {
    int precedence;  // priority of operators in " enum Operation"
    Arith arith_ptr;
    Prcnt prcnt_ptr;
};

class CalcEngine
{
public:
    // operations that can be stored in calculation stack
    enum Operation {
        FUNC_EQUAL,
        FUNC_PERCENT,
        FUNC_BRACKET,
        FUNC_OR,
        FUNC_XOR,
        FUNC_AND,
        FUNC_LSH,
        FUNC_RSH,
        FUNC_ADD,
        FUNC_SUBTRACT,
        FUNC_MULTIPLY,
        FUNC_DIVIDE,
        FUNC_MOD,
        FUNC_INTDIV,
        FUNC_BINOM,
        FUNC_POWER,
        FUNC_PWR_ROOT
    };

    CalcEngine();

    KNumber lastOutput(bool &error) const;

    void enterOperation(KNumber num, Operation func);


    void ArcCosDeg(const KNumber &input);
    void ArcCosRad(const KNumber &input);
    void ArcCosGrad(const KNumber &input);
    void ArcSinDeg(const KNumber &input);
    void ArcSinRad(const KNumber &input);
    void ArcSinGrad(const KNumber &input);
    void ArcTangensDeg(const KNumber &input);
    void ArcTangensRad(const KNumber &input);
    void ArcTangensGrad(const KNumber &input);
    void AreaCosHyp(const KNumber &input);
    void AreaSinHyp(const KNumber &input);
    void AreaTangensHyp(const KNumber &input);
    void Complement(const KNumber &input);
    void CosDeg(const KNumber &input);
    void CosRad(const KNumber &input);
    void CosGrad(const KNumber &input);
    void CosHyp(const KNumber &input);
    void Cube(const KNumber &input);
    void CubeRoot(const KNumber &input);
    void Exp(const KNumber &input);
    void Exp10(const KNumber &input);
    void Factorial(const KNumber &input);
    void InvertSign(const KNumber &input);
    void Ln(const KNumber &input);
    void Log10(const KNumber &input);
    void ParenClose(KNumber input);
    void ParenOpen(const KNumber &input);
    void Reciprocal(const KNumber &input);
    void SinDeg(const KNumber &input);
    void SinGrad(const KNumber &input);
    void SinRad(const KNumber &input);
    void SinHyp(const KNumber &input);
    void Square(const KNumber &input);
    void SquareRoot(const KNumber &input);
    void StatClearAll(const KNumber &input);
    void StatCount(const KNumber &input);
    void StatDataNew(const KNumber &input);
    void StatDataDel(const KNumber &input);
    void StatMean(const KNumber &input);
    void StatMedian(const KNumber &input);
    void StatStdDeviation(const KNumber &input);
    void StatStdSample(const KNumber &input);
    void StatSum(const KNumber &input);
    void StatSumSquares(const KNumber &input);
    void TangensDeg(const KNumber &input);
    void TangensRad(const KNumber &input);
    void TangensGrad(const KNumber &input);
    void TangensHyp(const KNumber &input);

    void Reset();
private:
    KStats stats;

    typedef struct {
        KNumber number;
        Operation operation;
    } Node;

    // Stack holds all operations and numbers that have not yet been
    // processed, e.g. user types "2+3*", the calculation can not be
    // executed, because "*" has a higher precedence than "+", so we
    // need to wait for the next number.
    //
    // In the stack this would be stored as ((2,+),(3,*),...)
    //
    // "enterOperation": If the introduced Operation has lower priority
    // than the preceding operations in the stack, then we can start to
    // evaluate the stack (with "evalStack"). Otherwise we append the new
    // Operation and number to the stack.
    //
    // E.g. "2*3+" evaluates to "6+", but "2+3*" can not be evaluated
    // yet.
    //
    // We also take care of brackets, by writing a marker "FUNC_BRACKET"
    // into the stack, each time the user opens one.  When a bracket is
    // closed, everything in the stack is evaluated until the first
    // marker "FUNC_BRACKET" found.
    QStack<Node> stack_;

    KNumber last_number_;

    bool percent_mode_;


    static const struct operator_data Operator[];

    bool evalStack();

    KNumber evalOperation(const KNumber &arg1, Operation operation, const KNumber &arg2);

    KNumber Deg2Rad(const KNumber &x) const {
        return KNumber(2) * KNumber::Pi() / KNumber(360) * x;
    }
    KNumber Gra2Rad(const KNumber &x) const {
        return KNumber(2)*KNumber::Pi() / KNumber(400) * x;
    }
    KNumber Rad2Deg(const KNumber &x) const {
        return KNumber(360) / (KNumber(2) * KNumber::Pi()) * x;
    }
    KNumber Rad2Gra(const KNumber &x) const {
        return KNumber(400) / (KNumber(2)*KNumber::Pi()) * x;
    }

};


#endif  // KCALC_CORE_H_
