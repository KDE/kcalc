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

#ifndef KCALC_H_
#define KCALC_H_

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
    void setupMainActions();
    void setupStatusbar();
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
    void slotEEclicked();
    void slotShifttoggled(bool myboolean);
    void slotMemRecallclicked();
    void slotMemStoreclicked();
    void slotSinclicked();
    void slotPlusMinusclicked();
    void slotMemPlusMinusclicked();
    void slotCosclicked();
    void slotReciclicked();
    void slotTanclicked();
    void slotFactorialclicked();
    void slotLogclicked();
    void slotSquareclicked();
    void slotCubeclicked();
    void slotLnclicked();
    void slotPowerclicked();
    void slotMemClearclicked();
    void slotClearclicked();
    void slotAllClearclicked();
    void slotParenOpenclicked();
    void slotParenCloseclicked();
    void slotANDclicked();
    void slotMultiplicationclicked();
    void slotDivisionclicked();
    void slotORclicked();
    void slotXORclicked();
    void slotPlusclicked();
    void slotMinusclicked();
    void slotLeftShiftclicked();
    void slotRightShiftclicked();
    void slotPeriodclicked();
    void slotEqualclicked();
    void slotPercentclicked();
    void slotNegateclicked();
    void slotModclicked();
    void slotStatNumclicked();
    void slotStatMeanclicked();
    void slotStatStdDevclicked();
    void slotStatMedianclicked();
    void slotStatDataInputclicked();
    void slotStatClearDataclicked();
    void slotHyptoggled(bool flag);
    void slotConstclicked(int);

    void slotConstantToDisplay(const science_constant &const_chosen);
    void slotChooseScientificConst0(const science_constant &);
    void slotChooseScientificConst1(const science_constant &);
    void slotChooseScientificConst2(const science_constant &);
    void slotChooseScientificConst3(const science_constant &);
    void slotChooseScientificConst4(const science_constant &);
    void slotChooseScientificConst5(const science_constant &);

    void slotBitsetChanged(quint64);
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
    bool shift_mode_;
    bool hyp_mode_;
    KNumber memory_num_;

    int angle_mode_; // angle modes for trigonometric values

    Constants*          constants_; // this is the dialog for configuring const buttons

    QButtonGroup* angle_choose_group_;
    QButtonGroup*  base_choose_group_;
    // num_button_group_: 0-9 = digits, 0xA-0xF = hex-keys
    QButtonGroup*  num_button_group_;

    QList<QAbstractButton*> logic_buttons_;
    QList<QAbstractButton*> scientific_buttons_;
    QList<QAbstractButton*> stat_buttons_;
    QList<QAbstractButton*> const_buttons_;

    KToggleAction *action_bitset_show_;
    KToggleAction *action_constants_show_;

    KToggleAction *action_mode_simple_;
    KToggleAction *action_mode_science_;
    KToggleAction *action_mode_statistic_;
    KToggleAction *action_mode_numeral_;

    QList<QAbstractButton*> function_button_list_;
    QList<QAbstractButton*> stat_button_list_;
    QList<QAbstractButton*> mem_button_list_;
    QList<QAbstractButton*> operation_button_list_;

    CalcEngine core;
};

#endif  // KCALC_H_
