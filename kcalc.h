/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 1996-2000 Bernd Johannes Wuebben <wuebben@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

class Constants;
class QButtonGroup;
class KToggleAction;
class KCalcConstMenu;
class KCalcStatusBar;

/*
 * Kcalc consist of a class for the GUI (here), and a few
 * implementation classes:
 *
 * - KNumber: holds a given number, integer or float, it implements
 *   all operations for a given KNumber (cos, ln, +, & and such).
 * - KCalcToken: class to hold "tokens" such as operations (+,-,×,..),
 *   KNumbers, parentheses or functions (ln, cos, ...) to be processed
 *   by the engine.
 * - KCalcParser: once an input string (such as "5cos(4.6)+1") can be committed
 *   for calculation, this class converts it to their respective KCalcToken
 *   objects and puts them into a Queue.
 * - CalcEngine: is able to process a given KCalToken Queue to produce a final
 *   result, based on precedence levels of each operation.
 *
 * Once the user has finished entering the desired expression to calculate
 * (signaling this by clicking "equal"), the string hold by KCalcInputDisplay is
 * sent to KCalcParser, the resulting KCalToken Queue is then sent to CalcEngine
 * and finally, the result is shown in KCalcDisplay.
 */

#include "kcalc_button.h"
#include "kcalc_const_button.h"
#include "kcalc_core.h"
#include "kcalc_parser.h"
#include "kcalc_token.h"

#include "ui_colors.h"
#include "ui_constants.h"
#include "ui_fonts.h"
#include "ui_general.h"
#include "ui_kcalc.h"

#include <QQueue>
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
    enum UpdateFlag {
        UPDATE_FROM_CORE = 1,
        UPDATE_STORE_RESULT = 2,
        UPDATE_MATH_ERROR = 4,
        UPDATE_SYNTAX_ERROR = 8,
        UPDATE_MALFORMED_EXPRESSION = 16,
        UPDATE_CLEAR = 32,
    };

    Q_DECLARE_FLAGS(UpdateFlags, UpdateFlag)

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    bool eventFilter(QObject *o, QEvent *e) override;
    bool event(QEvent *e) override;
    void updateGeometry();
    void setupMainActions();
    void setupDisplay();
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

    void setBaseFont(const QFont &font);
    const QFont &baseFont() const;
    bool isMinimumSize();
    void forceResizeEvent();
    void setLeftPadLayoutActive(bool active);
    void setBitsetLayoutActive(bool active);

    void updateDisplay(UpdateFlags flags);
    void insertNumericToInputDisplay(KCalcToken::TokenCode token);
    void insertFunctionToInputDisplay(KCalcToken::TokenCode token);
    void insertToInputDisplay(KCalcToken::TokenCode token);
    void insertToInputDisplay(const QString &token);
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
    void updateResultDisplay();
    void showSettings();

    // Mode
    void slotSetSimpleMode();
    void slotSetScienceMode();
    void slotSetStatisticMode();
    void slotSetNumeralMode();

    void slotHistoryshow(bool toggled);
    void slotConstantsShow(bool toggled);
    void slotBitsetshow(bool toggled);
    void slotAngleSelected(QAbstractButton *button, bool checked);
    void slotBaseSelected();
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
    void slotSqrtclicked();
    void slotLnclicked();
    void slotIclicked();
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
    void slotInputChanged();
    void slotClearResult();

    void slotConstantToDisplay(const ScienceConstant &const_chosen);
    void slotChooseScientificConst0(const ScienceConstant &);
    void slotChooseScientificConst1(const ScienceConstant &);
    void slotChooseScientificConst2(const ScienceConstant &);
    void slotChooseScientificConst3(const ScienceConstant &);
    void slotChooseScientificConst4(const ScienceConstant &);
    void slotChooseScientificConst5(const ScienceConstant &);

    void slotBitsetChanged(qint64 value);
    void slotUpdateBitset(const KNumber &);

    void slotBaseModeAmountChanged(const KNumber &number);
    void slotClearBaseModeAmount();

    void slotPaste();

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
    int commit_Input_();
    void commit_Result_(bool toHistory = true);
    int load_Constants_(const QString &filePath);

    bool numeral_system_mode_ = false;
    bool shift_mode_ = false;
    bool hyp_mode_ = false;
    bool update_history_window_ = false;
    QQueue<KCalcToken> token_Queue_;
    KNumber memory_num_;

    int angle_mode_; // angle modes for trigonometric values
    int base_mode_;

    KCalcParser::ParsingResult m_parsingResult;
    bool parsing_failure_;
    bool calculation_failure_;
    CalcEngine::ResultCode calculation_result_code_;
    int input_error_index_;
    int calculation_error_token_index_;
    void inline handle_Parsing_Error_();
    void inline handle_Calculation_Error_();

    KCalcConstMenu *constants_menu_ = nullptr;

    Constants *constants_ = nullptr; // this is the dialog for configuring const buttons

    QButtonGroup *angle_choose_group_ = nullptr;
    // num_button_group_: 0-9 = digits, 0xA-0xF = hex-keys
    QButtonGroup *num_button_group_ = nullptr;

    QList<QAbstractButton *> logic_buttons_;
    QList<QAbstractButton *> scientific_buttons_;
    QList<QAbstractButton *> stat_buttons_;
    QList<QAbstractButton *> const_buttons_;

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

    QFont baseFont_;
    bool is_still_in_launch_ = true; // necessary for startup at minimum size

    CalcEngine core;
    KCalcParser parser;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KCalculator::UpdateFlags)
