// -*- indent-tabs-mode: nil -*-
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

#ifndef KCALC_H
#define KCALC_H

class Constants;
class QButtonGroup;
class KToggleAction;

/*
  Kcalc basically consist of a class for the GUI (here), a class for
  the display (dlabel.h), and one for the mathematics core
  (kcalc_core.h).

  When for example '+' is pressed, one sends the contents of the
  Display and the '+' to the core via "core.Plus(DISPLAY_AMOUNT)".
  This only updates the core. To bring the changes to the display,
  use afterwards "UpdateDisplay(true)".

  "UpdateDisplay(true)" means that the amount to be displayed should
  be taken from the core (get the result of some operation that was
  performed), "UpdateDisplay(false)" has already the information, what
  to be display (e.g. user is typing in a number).  Note that in the
  last case the core does not know the number typed in until some
  operation button is pressed, e.g. "core.Plus(display_number)".
 */

#include "config-kcalc.h"

// IMPORTANT this has to come after config-kcalc.h - DF: why?
#include "kcalc_core.h"
#include "kcalc_button.h"
#include "kcalc_const_button.h"

#include "ui_kcalc.h"
#include "ui_general.h"
#include "ui_fonts.h"
#include "ui_constants.h"
#include "ui_colors.h"

#include <kxmlguiwindow.h>

class  General: public QWidget, public Ui::General
{
public:
    General(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class  Fonts: public QWidget, public Ui::Fonts
{
public:
    Fonts(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class Constants : public QWidget, public Ui::Constants
{
public:
    Constants(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class Colors : public QWidget, public Ui::Colors
{
public:
    Colors(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};


class KCalculator : public KXmlGuiWindow, private Ui::KCalculator
{
    Q_OBJECT

public:
    KCalculator(QWidget *parent = 0);
    ~KCalculator();

signals:
    void switchShift(bool);
    void switchMode(ButtonModeFlags, bool);
    void switchShowAccels(bool);

private:
    virtual bool eventFilter(QObject *o, QEvent *e);
    void updateGeometry();
    void setupMainActions(void);
    void setupStatusbar(void);
    void setupKeys();
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void setPrecision();
    void setAngle();
    void setBase();

    void updateDisplay(bool get_amount_from_core = false,
                       bool store_result_in_history = false);
    // button sets
    void showStatButtons(bool toggled);
    void showScienceButtons(bool toggled);
    void showLogicButtons(bool toggled);

protected slots:
    void changeButtonNames();
    void updateSettings();
    void setColors();
    void setFonts();
    void EnterEqual();
    void showSettings();

    // Mode
    void slotSetSimpleMode();
    void slotSetScienceMode();
    void slotSetStatisticMode();
    void slotSetNumeralMode();

    void slotConstantsShow(bool toggled);
    void slotBitsetshow(bool toggled);
    void slotAngleSelected(int mode);
    void slotBaseSelected(int base);
    void slotNumberclicked(int number_clicked);
    void slotEEclicked(void);
    void slotShifttoggled(bool myboolean);
    void slotMemRecallclicked(void);
    void slotMemStoreclicked(void);
    void slotSinclicked(void);
    void slotPlusMinusclicked(void);
    void slotMemPlusMinusclicked(void);
    void slotCosclicked(void);
    void slotReciclicked(void);
    void slotTanclicked(void);
    void slotFactorialclicked(void);
    void slotLogclicked(void);
    void slotSquareclicked(void);
    void slotCubeclicked(void);
    void slotLnclicked(void);
    void slotPowerclicked(void);
    void slotMemClearclicked(void);
    void slotClearclicked(void);
    void slotAllClearclicked(void);
    void slotParenOpenclicked(void);
    void slotParenCloseclicked(void);
    void slotANDclicked(void);
    void slotMultiplicationclicked(void);
    void slotDivisionclicked(void);
    void slotORclicked(void);
    void slotXORclicked(void);
    void slotPlusclicked(void);
    void slotMinusclicked(void);
    void slotLeftShiftclicked(void);
    void slotRightShiftclicked(void);
    void slotPeriodclicked(void);
    void slotEqualclicked(void);
    void slotPercentclicked(void);
    void slotNegateclicked(void);
    void slotModclicked(void);
    void slotStatNumclicked(void);
    void slotStatMeanclicked(void);
    void slotStatStdDevclicked(void);
    void slotStatMedianclicked(void);
    void slotStatDataInputclicked(void);
    void slotStatClearDataclicked(void);
    void slotHyptoggled(bool flag);
    void slotConstclicked(int);

    void slotConstantToDisplay(struct science_constant const &const_chosen);
    void slotChooseScientificConst0(struct science_constant const &);
    void slotChooseScientificConst1(struct science_constant const &);
    void slotChooseScientificConst2(struct science_constant const &);
    void slotChooseScientificConst3(struct science_constant const &);
    void slotChooseScientificConst4(struct science_constant const &);
    void slotChooseScientificConst5(struct science_constant const &);

    void slotBitsetChanged(unsigned long long);
    void slotUpdateBitset(const KNumber &);

private:
    enum StatusField {
        ShiftField = 0,
        BaseField,
        AngleField,
        MemField
    };
    enum AngleMode {
        DegMode = 0,
        RadMode,
        GradMode
    };
    enum BaseMode {
        BinMode = 2,
        OctMode = 8,
        DecMode = 10,
        HexMode = 16
    };

private:
    bool shift_mode;
    bool hyp_mode;
    KNumber memory_num;
    KNumber setvalue;

    int _angle_mode; // angle modes for trigonometric values

    Constants*          constants; // this is the dialog for configuring const buttons

    QButtonGroup* AngleChooseGroup;
    QButtonGroup*  BaseChooseGroup;
    // NumButtonGroup: 0-9 = digits, 0xA-0xF = hex-keys
    QButtonGroup*  NumButtonGroup;

    QList<QAbstractButton*> logicButtons;
    QList<QAbstractButton*> scientificButtons;
    QList<QAbstractButton*> statButtons;
    QList<QAbstractButton*> constButtons;

    KToggleAction *actionBitsetshow;
    KToggleAction *actionConstantsShow;

    KToggleAction *actionModeSimple;
    KToggleAction *actionModeScience;
    KToggleAction *actionModeStatistic;
    KToggleAction *actionModeNumeral;

    QList<QAbstractButton*> mFunctionButtonList;
    QList<QAbstractButton*> mStatButtonList;
    QList<QAbstractButton*> mMemButtonList;
    QList<QAbstractButton*> mOperationButtonList;

    CalcEngine core;
};

#endif  // KCALC_H
