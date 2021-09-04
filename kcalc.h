/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 1996-2000 Bernd Johannes Wuebben <wuebben@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

class Constants;
class QButtonGroup;
class QLabel;
class KToggleAction;
class KCalcConstMenu;
class KCalcStatusBar;

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

#include "kcalc_button.h"
#include "kcalc_const_button.h"
#include "kcalc_core.h"

#include "ui_colors.h"
#include "ui_constants.h"
#include "ui_fonts.h"
#include "ui_general.h"
#include "ui_kcalc.h"

#include <array>
#include <kxmlguiwindow.h>

class General : public QWidget, public Ui::General
{
    Q_OBJECT
public:
    explicit General(QWidget *parent)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

class Fonts : public QWidget, public Ui::Fonts
{
    Q_OBJECT
public:
    explicit Fonts(QWidget *parent)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

class Constants : public QWidget, public Ui::Constants
{
    Q_OBJECT
public:
    explicit Constants(QWidget *parent)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

class Colors : public QWidget, public Ui::Colors
{
    Q_OBJECT
public:
    explicit Colors(QWidget *parent)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

class KCalculator : public KXmlGuiWindow, private Ui::KCalculator
{
    Q_OBJECT

public:
    explicit KCalculator(QWidget *parent = nullptr);
    ~KCalculator() override;

Q_SIGNALS:
    void switchMode(ButtonModeFlags, bool);
    void switchShowAccels(bool);

public:
    enum UpdateFlag { UPDATE_FROM_CORE = 1, UPDATE_STORE_RESULT = 2 };

    Q_DECLARE_FLAGS(UpdateFlags, UpdateFlag)

private:
    bool eventFilter(QObject *o, QEvent *e) override;
    bool event(QEvent *e) override;
    void updateGeometry();
    void setupMainActions();
    void setupKeys();
    void setupNumberKeys();
    void setupRightKeypad();
    void setupNumericKeypad();
    void setupLogicKeys();
    void setupScientificKeys();
    void setupStatisticKeys();
    void setupConstantsKeys();
    void setupMiscKeys();
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void setPrecision();
    void setAngle();
    void setBase();

    void resizeEvent(QResizeEvent* event) override;

    void updateDisplay(UpdateFlags flags);
    void updateHistoryWithFunction(CalcEngine::Operation);
    KCalcStatusBar *statusBar();

    // button sets
    void showMemButtons(bool toggled);
    void showStatButtons(bool toggled);
    void showScienceButtons(bool toggled);
    void showLogicButtons(bool toggled);

    KCalcConstMenu *createConstantsMenu();

protected Q_SLOTS:
    void changeButtonNames();
    void updateSettings();
    void setColors();
    void setFonts();
    void EnterEqual(CalcEngine::Repeat allow_repeat = CalcEngine::REPEAT_ALLOW);
    void showSettings();

    // Mode
    void slotSetSimpleMode();
    void slotSetScienceMode();
    void slotSetStatisticMode();
    void slotSetNumeralMode();

    void slotHistoryshow(bool toggled);
    void slotConstantsShow(bool toggled);
    void slotBitsetshow(bool toggled);
    void slotAngleSelected(QAbstractButton *button);
    void slotBaseSelected(QAbstractButton *button);
    void slotNumberclicked(QAbstractButton *button);
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
    void slotBackspaceclicked();

    void slotConstantToDisplay(const science_constant &const_chosen);
    void slotChooseScientificConst0(const science_constant &);
    void slotChooseScientificConst1(const science_constant &);
    void slotChooseScientificConst2(const science_constant &);
    void slotChooseScientificConst3(const science_constant &);
    void slotChooseScientificConst4(const science_constant &);
    void slotChooseScientificConst5(const science_constant &);

    void slotBitsetChanged(quint64);
    void slotUpdateBitset(const KNumber &);

    void slotBaseModeAmountChanged(const KNumber &number);

    void slotPaste();

private:
    enum StatusField { ShiftField = 0, BaseField, AngleField, MemField };

    enum AngleMode { DegMode = 0, RadMode, GradMode };

    enum BaseMode { BinMode = 2, OctMode = 8, DecMode = 10, HexMode = 16 };

private:
    bool shift_mode_ = false;
    bool hyp_mode_ = false;
    bool update_history_window_ = false;
    KNumber memory_num_;

    int angle_mode_; // angle modes for trigonometric values

    KCalcConstMenu *constants_menu_ = nullptr;

    Constants *constants_ = nullptr; // this is the dialog for configuring const buttons

    QButtonGroup *angle_choose_group_ = nullptr;
    QButtonGroup *base_choose_group_ = nullptr;
    // num_button_group_: 0-9 = digits, 0xA-0xF = hex-keys
    QButtonGroup *num_button_group_ = nullptr;

    QList<QAbstractButton *> logic_buttons_;
    QList<QAbstractButton *> scientific_buttons_;
    QList<QAbstractButton *> stat_buttons_;
    QList<QAbstractButton *> const_buttons_;

    std::array<QLabel *, 4> base_conversion_labels_;

    KToggleAction *action_history_show_ = nullptr;
    KToggleAction *action_bitset_show_ = nullptr;
    KToggleAction *action_constants_show_ = nullptr;

    KToggleAction *action_mode_simple_ = nullptr;
    KToggleAction *action_mode_science_ = nullptr;
    KToggleAction *action_mode_statistic_ = nullptr;
    KToggleAction *action_mode_numeral_ = nullptr;

    QList<QAbstractButton *> function_button_list_;
    QList<QAbstractButton *> stat_button_list_;
    QList<QAbstractButton *> mem_button_list_;
    QList<QAbstractButton *> operation_button_list_;

    CalcEngine core;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KCalculator::UpdateFlags)

