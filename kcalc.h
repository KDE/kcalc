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
        UpdateFromCore = 1,
        UpdateStoreResult = 2,
        UpdateMathError = 4,
        UpdateSyntaxError = 8,
        UpdateMalformedExpression = 16,
        UpdateClear = 32,
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

    void slotConstantToDisplay(const ScienceConstant &constChosen);
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

    int commitInput();
    void commitResult(bool toHistory = true);
    int loadConstants(const QString &filePath);

    bool m_numeralSystemMode = false;
    bool m_shiftMode = false;
    bool m_hypMode = false;
    bool m_updateHistoryWindow = false;
    QQueue<KCalcToken> m_tokenQueue;
    KNumber m_memoryNum;

    int m_angleMode{}; // angle modes for trigonometric values
    int m_baseMode;

    KCalcParser::ParsingResult m_ParsingResult;
    bool m_parsingFailure{};
    bool m_calculationFailure{};
    CalcEngine::ResultCode m_calculationResultCode;
    int m_inputErrorIndex{};
    int m_calculationErrorTokenIndex{};
    void inline handleParsingError();
    void inline handleCalculationError();

    KCalcConstMenu *m_constantsMenu = nullptr;

    Constants *m_constants = nullptr; // this is the dialog for configuring const buttons

    QButtonGroup *m_angleChooseGroup = nullptr;
    // num_button_group_: 0-9 = digits, 0xA-0xF = hex-keys
    QButtonGroup *m_numButtonGroup = nullptr;

    QList<QAbstractButton *> m_logicButtons;
    QList<QAbstractButton *> m_scientificButtons;
    QList<QAbstractButton *> m_statButtons;
    QList<QAbstractButton *> m_constButtons;

    KToggleAction *m_actionHistoryShow = nullptr;
    KToggleAction *m_actionBitsetShow = nullptr;
    KToggleAction *m_actionConstantsShow = nullptr;

    KToggleAction *m_actionModeSimple = nullptr;
    KToggleAction *m_actionModeScience = nullptr;
    KToggleAction *m_actionModeStatistic = nullptr;
    KToggleAction *m_actionModeNumeral = nullptr;

    QList<QAbstractButton *> m_functionButtonList;
    QList<QAbstractButton *> m_statButtonList;
    QList<QAbstractButton *> m_memButtonList;
    QList<QAbstractButton *> m_operationButtonList;

    QFont m_baseFont;
    bool m_isStillInLaunch = true; // necessary for startup at minimum size

    CalcEngine m_core;
    KCalcParser m_parser;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KCalculator::UpdateFlags)
