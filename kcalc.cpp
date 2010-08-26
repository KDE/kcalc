// -*- indent-tabs-mode: nil -*-
/*
    kCalculator, a simple scientific calculator for KDE

    Copyright (C) 1996-2000 Bernd Johannes Wuebben
                            wuebben@kde.org

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

#include "kcalc.h"

#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <QApplication>
#include <QCursor>
#include <QKeyEvent>
#include <QShortcut>
#include <QStyle>

#include <kaboutdata.h>
#include <kacceleratormanager.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kbuttongroup.h>
#include <kcmdlineargs.h>
#include <kcolorbutton.h>
#include <kcolormimedata.h>
#include <kconfig.h>
#include <kconfigdialog.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kmenu.h>
#include <kmenubar.h>
#include <knumvalidator.h>
#include <kpushbutton.h>
#include <kstatusbar.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <ktoolbar.h>
#include <kxmlguifactory.h>

#include "kcalcdisplay.h"
#include "kcalc_const_menu.h"
#include "version.h"
#include "kcalc_settings.h"
#include "kcalc_bitset.h"

class QActionGroup;

static const char description[] = I18N_NOOP("KDE Calculator");
static const char version[] = KCALCVERSION;

static const int maxprecision = 1000;

KCalculator::KCalculator(QWidget *parent)
        : KXmlGuiWindow(parent), shift_mode_(false), hyp_mode_(false),
        memory_num_(0.0), constants_(0), core()
{
    /* central widget to contain all the elements */
    QWidget *central = new QWidget(this);
    central->setLayoutDirection(Qt::LeftToRight);
    setCentralWidget(central);
    KAcceleratorManager::setNoAccel(central);

    // load science constants_ from xml-file
    KCalcConstMenu::init_consts();

    // setup interface (order is critical)

    setupUi(central);

    setupMainActions();

    setupStatusbar();

    createGUI();

    setupKeys();

    toolBar()->hide(); // hide by default

    // create button groups

    base_choose_group_ = new QButtonGroup(this);
    base_choose_group_->setExclusive(true);
    base_choose_group_->addButton(hexRadio, HexMode);
    base_choose_group_->addButton(decRadio, DecMode);
    base_choose_group_->addButton(octRadio, OctMode);
    base_choose_group_->addButton(binRadio, BinMode);
    connect(base_choose_group_, SIGNAL(buttonClicked(int)),
            SLOT(slotBaseSelected(int)));

    angle_choose_group_ =  new QButtonGroup(this);
    angle_choose_group_->setExclusive(true);
    angle_choose_group_->addButton(degRadio, DegMode);
    angle_choose_group_->addButton(radRadio, RadMode);
    angle_choose_group_->addButton(gradRadio, GradMode);
    connect(angle_choose_group_, SIGNAL(buttonClicked(int)),
            SLOT(slotAngleSelected(int)));

    // additional menu setup

    KCalcConstMenu *tmp_menu = new KCalcConstMenu(i18n("&Constants"), this);
    menuBar()->insertMenu((menuBar()->actions)()[2], tmp_menu);
    connect(tmp_menu, SIGNAL(triggeredConstant(science_constant const &)),
            this, SLOT(slotConstantToDisplay(science_constant const &)));

    // misc setup

    setColors();
    setFonts();

    // Show the result in the app's caption in taskbar (wishlist - bug #52858)
    if (KCalcSettings::captionResult() == true)
        connect(calc_display, SIGNAL(changedText(const QString &)),
                SLOT(setCaption(const QString &)));
    calc_display->changeSettings();
    setPrecision();

    updateGeometry();

    setFixedSize(minimumSize());

    updateDisplay(true);

    // misc settings
    KCalcSettings::EnumCalculatorMode::type calculatorMode = KCalcSettings::calculatorMode();
    switch (calculatorMode) {
    case KCalcSettings::EnumCalculatorMode::science:
        action_mode_science_->setChecked(true);
        break;
    case KCalcSettings::EnumCalculatorMode::statistics:
        action_mode_statistic_->setChecked(true);
        break;
    case KCalcSettings::EnumCalculatorMode::numeral:
        action_mode_numeral_->setChecked(true);
        break;
    case KCalcSettings::EnumCalculatorMode::simple:
    default:
        action_mode_simple_->setChecked(true);
    }

    setAngle();
    setBase();

    // connections

    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
            SLOT(setColors()));
    connect(KGlobalSettings::self(), SIGNAL(kdisplayFontChanged()),
            SLOT(setFonts()));

    calc_display->setFocus();
}

KCalculator::~KCalculator()
{
    KCalcSettings::self()->writeConfig();
}

void KCalculator::setupMainActions()
{
    // file menu
    KStandardAction::quit(this, SLOT(close()), actionCollection());

    // edit menu
    KStandardAction::undo(calc_display, SLOT(slotHistoryBack()), actionCollection());
    KStandardAction::redo(calc_display, SLOT(slotHistoryForward()), actionCollection());
    KStandardAction::cut(calc_display, SLOT(slotCut()), actionCollection());
    KStandardAction::copy(calc_display, SLOT(slotCopy()), actionCollection());
    KStandardAction::paste(calc_display, SLOT(slotPaste()), actionCollection());

    // mode menu
    QActionGroup *modeGroup = new QActionGroup(this);
    action_mode_simple_ = actionCollection()->add<KToggleAction>("mode_simple");
    action_mode_simple_->setActionGroup(modeGroup);
    action_mode_simple_->setText(i18n("Simple Mode"));
    connect(action_mode_simple_, SIGNAL(toggled(bool)), SLOT(slotSetSimpleMode()));
    action_mode_science_ = actionCollection()->add<KToggleAction>("mode_science");
    action_mode_science_->setActionGroup(modeGroup);
    action_mode_science_->setText(i18n("Science Mode"));
    connect(action_mode_science_, SIGNAL(toggled(bool)), SLOT(slotSetScienceMode()));
    action_mode_statistic_ = actionCollection()->add<KToggleAction>("mode_statistics");
    action_mode_statistic_->setActionGroup(modeGroup);
    action_mode_statistic_->setText(i18n("Statistic Mode"));
    connect(action_mode_statistic_, SIGNAL(toggled(bool)), SLOT(slotSetStatisticMode()));
    action_mode_numeral_ = actionCollection()->add<KToggleAction>("mode_numeral");
    action_mode_numeral_->setActionGroup(modeGroup);
    action_mode_numeral_->setText(i18n("Numeral System Mode"));
    connect(action_mode_numeral_, SIGNAL(toggled(bool)), SLOT(slotSetNumeralMode()));

    // settings menu
    action_constants_show_ = actionCollection()->add<KToggleAction>("show_constants");
    action_constants_show_->setText(i18n("&Constants Buttons"));
    action_constants_show_->setChecked(true);
    connect(action_constants_show_, SIGNAL(toggled(bool)),
            SLOT(slotConstantsShow(bool)));

    action_bitset_show_ = actionCollection()->add<KToggleAction>("show_bitset");
    action_bitset_show_->setText(i18n("Show B&it Edit"));
    action_bitset_show_->setChecked(true);
    connect(action_bitset_show_, SIGNAL(toggled(bool)),
            SLOT(slotBitsetshow(bool)));

    KStandardAction::preferences(this, SLOT(showSettings()), actionCollection());
    KStandardAction::keyBindings(guiFactory(), SLOT(configureShortcuts()),
                                 actionCollection());
}

void KCalculator::setupStatusbar()
{
    // Status bar contents
    statusBar()->insertPermanentFixedItem(" NORM ", ShiftField);
    statusBar()->setItemAlignment(ShiftField, Qt::AlignCenter);

    statusBar()->insertPermanentFixedItem(" HEX ", BaseField);
    statusBar()->setItemAlignment(BaseField, Qt::AlignCenter);

    statusBar()->insertPermanentFixedItem(" DEG ", AngleField);
    statusBar()->setItemAlignment(AngleField, Qt::AlignCenter);

    statusBar()->insertPermanentFixedItem(" \xa0\xa0 ", MemField);   // nbsp
    statusBar()->setItemAlignment(MemField, Qt::AlignCenter);
}

// additional setup for button keys

void KCalculator::setupKeys()
{
    // NOTE: all alphanumeric shorts set in ui file

    // numbers

    num_button_group_ = new QButtonGroup(this);
    connect(num_button_group_, SIGNAL(buttonClicked(int)),
            SLOT(slotNumberclicked(int)));

    num_button_group_->addButton(pb0, 0);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pb0, SLOT(slotSetAccelDisplayMode(bool)));
    num_button_group_->addButton(pb1, 1);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pb1, SLOT(slotSetAccelDisplayMode(bool)));
    num_button_group_->addButton(pb2, 2);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pb2, SLOT(slotSetAccelDisplayMode(bool)));
    num_button_group_->addButton(pb3, 3);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pb3, SLOT(slotSetAccelDisplayMode(bool)));
    num_button_group_->addButton(pb4, 4);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pb4, SLOT(slotSetAccelDisplayMode(bool)));
    num_button_group_->addButton(pb5, 5);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pb5, SLOT(slotSetAccelDisplayMode(bool)));
    num_button_group_->addButton(pb6, 6);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pb6, SLOT(slotSetAccelDisplayMode(bool)));
    num_button_group_->addButton(pb7, 7);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pb7, SLOT(slotSetAccelDisplayMode(bool)));
    num_button_group_->addButton(pb8, 8);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pb8, SLOT(slotSetAccelDisplayMode(bool)));
    num_button_group_->addButton(pb9, 9);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pb9, SLOT(slotSetAccelDisplayMode(bool)));
    num_button_group_->addButton(pbA, 0xA);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbA, SLOT(slotSetAccelDisplayMode(bool)));
    num_button_group_->addButton(pbB, 0xB);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbB, SLOT(slotSetAccelDisplayMode(bool)));
    num_button_group_->addButton(pbC, 0xC);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbC, SLOT(slotSetAccelDisplayMode(bool)));
    num_button_group_->addButton(pbD, 0xD);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbD, SLOT(slotSetAccelDisplayMode(bool)));
    num_button_group_->addButton(pbE, 0xE);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbE, SLOT(slotSetAccelDisplayMode(bool)));
    num_button_group_->addButton(pbF, 0xF);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbF, SLOT(slotSetAccelDisplayMode(bool)));

    // right keypad

    connect(pbShift, SIGNAL(toggled(bool)),
            SLOT(slotShifttoggled(bool)));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbShift, SLOT(slotSetAccelDisplayMode(bool)));

    pbClear->setShortcut(QKeySequence(Qt::Key_Escape));
    new QShortcut(Qt::Key_PageUp, pbClear, SLOT(animateClick()));
    connect(pbClear, SIGNAL(clicked()),
            SLOT(slotClearclicked()));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbClear, SLOT(slotSetAccelDisplayMode(bool)));

    pbAllClear->setShortcut(QKeySequence(Qt::Key_Delete));
    new QShortcut(Qt::Key_PageDown, pbAllClear, SLOT(animateClick()));
    connect(pbAllClear, SIGNAL(clicked()),
            SLOT(slotAllClearclicked()));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbAllClear, SLOT(slotSetAccelDisplayMode(bool)));

    pbParenOpen->setShortcut(QKeySequence(Qt::Key_ParenLeft));
    connect(pbParenOpen, SIGNAL(clicked()),
            SLOT(slotParenOpenclicked()));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbParenOpen, SLOT(slotSetAccelDisplayMode(bool)));

    pbParenClose->setShortcut(QKeySequence(Qt::Key_ParenRight));
    connect(pbParenClose, SIGNAL(clicked()),
            SLOT(slotParenCloseclicked()));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbParenClose, SLOT(slotSetAccelDisplayMode(bool)));

    pbMemRecall->setDisabled(true);   // nothing in memory at start
    connect(pbMemRecall, SIGNAL(clicked()),
            SLOT(slotMemRecallclicked()));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbMemRecall, SLOT(slotSetAccelDisplayMode(bool)));

    connect(pbMemClear, SIGNAL(clicked()),
            SLOT(slotMemClearclicked()));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbMemClear, SLOT(slotSetAccelDisplayMode(bool)));

    pbMemPlusMinus->addMode(ModeNormal, i18nc("Add display to memory", "M+"), i18n("Add display to memory"));
    pbMemPlusMinus->addMode(ModeShift, i18nc("Subtract from memory", "M−"), i18n("Subtract from memory"));
    connect(pbMemPlusMinus, SIGNAL(clicked()),
            SLOT(slotMemPlusMinusclicked()));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbMemPlusMinus, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbMemPlusMinus, SLOT(slotSetMode(ButtonModeFlags, bool)));

    connect(pbMemStore, SIGNAL(clicked()),
            SLOT(slotMemStoreclicked()));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbMemStore, SLOT(slotSetAccelDisplayMode(bool)));

    pbPercent->setShortcut(QKeySequence(Qt::Key_Percent));
    connect(pbPercent, SIGNAL(clicked()),
            SLOT(slotPercentclicked()));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbPercent, SLOT(slotSetAccelDisplayMode(bool)));

    pbPlusMinus->setShortcut(QKeySequence(Qt::Key_Backslash));
    connect(pbPlusMinus, SIGNAL(clicked()),
            SLOT(slotPlusMinusclicked()));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbPlusMinus, SLOT(slotSetAccelDisplayMode(bool)));

    // numeric keypad

    pbCube->addMode(ModeNormal, i18nc("Third power", "x<sup>3</sup>"), i18n("Third power"));
    pbCube->addMode(ModeShift, "<sup>3</sup>&radic;x", i18n("Cube root"));
    connect(pbCube, SIGNAL(clicked()),
            SLOT(slotCubeclicked()));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbCube, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbCube, SLOT(slotSetMode(ButtonModeFlags, bool)));

    pbDivision->setShortcut(QKeySequence(Qt::Key_Slash));
    new QShortcut(Qt::Key_division, pbDivision, SLOT(animateClick()));
    connect(pbDivision, SIGNAL(clicked()),
            SLOT(slotDivisionclicked()));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbDivision, SLOT(slotSetAccelDisplayMode(bool)));

    pbMultiplication->setShortcut(QKeySequence(Qt::Key_Asterisk));
    new QShortcut(Qt::Key_X, pbMultiplication, SLOT(animateClick()));
    new QShortcut(Qt::Key_multiply, pbMultiplication, SLOT(animateClick()));
    connect(pbMultiplication, SIGNAL(clicked()),
            SLOT(slotMultiplicationclicked()));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbMultiplication, SLOT(slotSetAccelDisplayMode(bool)));

    pbMinus->setShortcut(QKeySequence(Qt::Key_Minus));
    connect(pbMinus, SIGNAL(clicked()),
            SLOT(slotMinusclicked()));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbMinus, SLOT(slotSetAccelDisplayMode(bool)));

    pbPlus->setShortcut(QKeySequence(Qt::Key_Plus));
    connect(pbPlus, SIGNAL(clicked()),
            SLOT(slotPlusclicked()));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbPlus, SLOT(slotSetAccelDisplayMode(bool)));

    pbPeriod->setText(KGlobal::locale()->decimalSymbol());
    pbPeriod->setShortcut(KGlobal::locale()->decimalSymbol());
    if (KGlobal::locale()->decimalSymbol() == ".")
        new QShortcut(Qt::Key_Comma, pbPeriod, SLOT(animateClick()));
    else if (KGlobal::locale()->decimalSymbol() == ",")
        new QShortcut(Qt::Key_Period, pbPeriod, SLOT(animateClick()));
    connect(pbPeriod, SIGNAL(clicked()),
            SLOT(slotPeriodclicked()));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbPeriod, SLOT(slotSetAccelDisplayMode(bool)));

    pbEqual->setShortcut(QKeySequence(Qt::Key_Enter));
    new QShortcut(Qt::Key_Equal, pbEqual, SLOT(animateClick()));
    new QShortcut(Qt::Key_Return, pbEqual, SLOT(animateClick()));
    connect(pbEqual, SIGNAL(clicked()),
            SLOT(slotEqualclicked()));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbEqual, SLOT(slotSetAccelDisplayMode(bool)));

    // logic keys

    logic_buttons_.append(pbAND);
    logic_buttons_.append(pbOR);
    logic_buttons_.append(pbXOR);
    logic_buttons_.append(pbLsh);
    logic_buttons_.append(pbRsh);
    logic_buttons_.append(pbCmp);

    pbAND->setShortcut(QKeySequence(Qt::Key_Ampersand));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbAND, SLOT(slotSetAccelDisplayMode(bool)));
    connect(pbAND, SIGNAL(clicked()), SLOT(slotANDclicked()));

    pbOR->setShortcut(QKeySequence(Qt::Key_Bar));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbOR, SLOT(slotSetAccelDisplayMode(bool)));
    connect(pbOR, SIGNAL(clicked()), SLOT(slotORclicked()));

    connect(this, SIGNAL(switchShowAccels(bool)),
            pbXOR, SLOT(slotSetAccelDisplayMode(bool)));
    connect(pbXOR, SIGNAL(clicked()), SLOT(slotXORclicked()));

    pbLsh->setShortcut(QKeySequence(Qt::Key_Less));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbLsh, SLOT(slotSetAccelDisplayMode(bool)));
    connect(pbLsh, SIGNAL(clicked()),
            SLOT(slotLeftShiftclicked()));

    pbRsh->setShortcut(QKeySequence(Qt::Key_Greater));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbRsh, SLOT(slotSetAccelDisplayMode(bool)));
    connect(pbRsh, SIGNAL(clicked()),
            SLOT(slotRightShiftclicked()));

    pbCmp->setShortcut(QKeySequence(Qt::Key_AsciiTilde));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbCmp, SLOT(slotSetAccelDisplayMode(bool)));
    connect(pbCmp, SIGNAL(clicked()), SLOT(slotNegateclicked()));

    // scientific keys

    scientific_buttons_.append(pbHyp);
    scientific_buttons_.append(pbSin);
    scientific_buttons_.append(pbCos);
    scientific_buttons_.append(pbTan);
    scientific_buttons_.append(pbLog);
    scientific_buttons_.append(pbLn);

    connect(this, SIGNAL(switchShowAccels(bool)),
            pbHyp, SLOT(slotSetAccelDisplayMode(bool)));
    connect(pbHyp, SIGNAL(toggled(bool)), SLOT(slotHyptoggled(bool)));

    pbSin->addMode(ModeNormal, i18nc("Sine", "Sin"), i18n("Sine"));
    pbSin->addMode(ModeShift, i18nc("Arc sine", "Asin"), i18n("Arc sine"));
    pbSin->addMode(ModeHyperbolic, i18nc("Hyperbolic sine", "Sinh"), i18n("Hyperbolic sine"));
    pbSin->addMode(ButtonModeFlags(ModeShift | ModeHyperbolic),
                   i18nc("Inverse hyperbolic sine", "Asinh"), i18n("Inverse hyperbolic sine"));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbSin, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbSin, SLOT(slotSetMode(ButtonModeFlags, bool)));
    connect(pbSin, SIGNAL(clicked()), SLOT(slotSinclicked()));

    pbCos->addMode(ModeNormal, i18nc("Cosine", "Cos"), i18n("Cosine"));
    pbCos->addMode(ModeShift, i18nc("Arc cosine", "Acos"), i18n("Arc cosine"));
    pbCos->addMode(ModeHyperbolic, i18nc("Hyperbolic cosine", "Cosh"), i18n("Hyperbolic cosine"));
    pbCos->addMode(ButtonModeFlags(ModeShift | ModeHyperbolic),
                   i18nc("Inverse hyperbolic cosine", "Acosh"), i18n("Inverse hyperbolic cosine"));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbCos, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbCos, SLOT(slotSetMode(ButtonModeFlags, bool)));
    connect(pbCos, SIGNAL(clicked()), SLOT(slotCosclicked()));

    pbTan->addMode(ModeNormal, i18nc("Tangent", "Tan"), i18n("Tangent"));
    pbTan->addMode(ModeShift, i18nc("Arc tangent", "Atan"), i18n("Arc tangent"));
    pbTan->addMode(ModeHyperbolic, i18nc("Hyperbolic tangent", "Tanh"), i18n("Hyperbolic tangent"));
    pbTan->addMode(ButtonModeFlags(ModeShift | ModeHyperbolic),
                   i18nc("Inverse hyperbolic tangent", "Atanh"), i18n("Inverse hyperbolic tangent"));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbTan, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbTan, SLOT(slotSetMode(ButtonModeFlags, bool)));
    connect(pbTan, SIGNAL(clicked()), SLOT(slotTanclicked()));

    pbLog->addMode(ModeNormal, i18nc("Logarithm to base 10", "Log"), i18n("Logarithm to base 10"));
    pbLog->addMode(ModeShift, i18nc("10 to the power of x", "10<sup>x</sup>"), i18n("10 to the power of x"));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbLog, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbLog, SLOT(slotSetMode(ButtonModeFlags, bool)));
    connect(pbLog, SIGNAL(clicked()), SLOT(slotLogclicked()));
    pbLn->addMode(ModeNormal, i18nc("Natural log", "Ln"), i18n("Natural log"));
    pbLn->addMode(ModeShift, i18nc("Exponential function", "e<sup>x</sup>"), i18n("Exponential function"));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbLn, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbLn, SLOT(slotSetMode(ButtonModeFlags, bool)));
    connect(pbLn, SIGNAL(clicked()), SLOT(slotLnclicked()));

    // statistic buttons

    stat_buttons_.append(pbNData);
    stat_buttons_.append(pbMean);
    stat_buttons_.append(pbSd);
    stat_buttons_.append(pbMed);
    stat_buttons_.append(pbDat);
    stat_buttons_.append(pbCSt);

    pbNData->addMode(ModeNormal, i18nc("Number of data entered", "N"), i18n("Number of data entered"));
    pbNData->addMode(ModeShift, QString::fromUtf8("\xce\xa3")
                     + 'x', i18n("Sum of all data items"));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbNData, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbNData, SLOT(slotSetMode(ButtonModeFlags, bool)));
    connect(pbNData, SIGNAL(clicked()), SLOT(slotStatNumclicked()));

    pbMean->addMode(ModeNormal, i18nc("Mean", "Mea"), i18n("Mean"));
    pbMean->addMode(ModeShift, QString::fromUtf8("\xce\xa3")
                    + "x<sup>2</sup>",
                    i18n("Sum of all data items squared"));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbMean, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbMean, SLOT(slotSetMode(ButtonModeFlags, bool)));
    connect(pbMean, SIGNAL(clicked()), SLOT(slotStatMeanclicked()));

    pbSd->addMode(ModeNormal, QString::fromUtf8("σ", -1) + "<sub>N</sub>",
                  i18n("Standard deviation"));
    pbSd->addMode(ModeShift, QString::fromUtf8("σ", -1) + "<sub>N-1</sub>",
                  i18n("Sample standard deviation"));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbSd, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbSd, SLOT(slotSetMode(ButtonModeFlags, bool)));
    connect(pbSd, SIGNAL(clicked()), SLOT(slotStatStdDevclicked()));

    connect(this, SIGNAL(switchShowAccels(bool)),
            pbMed, SLOT(slotSetAccelDisplayMode(bool)));
    connect(pbMed, SIGNAL(clicked()), SLOT(slotStatMedianclicked()));

    pbDat->addMode(ModeNormal, i18nc("Enter data", "Dat"), i18n("Enter data"));
    pbDat->addMode(ModeShift, i18nc("Delete last data item", "CDat"), i18n("Delete last data item"));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbDat, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbDat, SLOT(slotSetMode(ButtonModeFlags, bool)));
    connect(pbDat, SIGNAL(clicked()), SLOT(slotStatDataInputclicked()));

    connect(this, SIGNAL(switchShowAccels(bool)),
            pbCSt, SLOT(slotSetAccelDisplayMode(bool)));
    connect(pbCSt, SIGNAL(clicked()), SLOT(slotStatClearDataclicked()));

    // constants_ keys

    const_buttons_.append(pbC1);
    const_buttons_.append(pbC2);
    const_buttons_.append(pbC3);
    const_buttons_.append(pbC4);
    const_buttons_.append(pbC5);
    const_buttons_.append(pbC6);

    pbC1->setButtonNumber(0);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbC1, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbC1, SLOT(slotSetMode(ButtonModeFlags, bool)));
    connect(pbC1, SIGNAL(clicked(int)), this, SLOT(slotConstclicked(int)));

    pbC2->setButtonNumber(1);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbC2, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbC2, SLOT(slotSetMode(ButtonModeFlags, bool)));
    connect(pbC2, SIGNAL(clicked(int)), this, SLOT(slotConstclicked(int)));

    pbC3->setButtonNumber(2);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbC3, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbC3, SLOT(slotSetMode(ButtonModeFlags, bool)));
    connect(pbC3, SIGNAL(clicked(int)), this, SLOT(slotConstclicked(int)));

    pbC4->setButtonNumber(3);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbC4, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbC4, SLOT(slotSetMode(ButtonModeFlags, bool)));
    connect(pbC4, SIGNAL(clicked(int)), this, SLOT(slotConstclicked(int)));

    pbC5->setButtonNumber(4);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbC5, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbC5, SLOT(slotSetMode(ButtonModeFlags, bool)));
    connect(pbC5, SIGNAL(clicked(int)), this, SLOT(slotConstclicked(int)));

    pbC6->setButtonNumber(5);
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbC6, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbC6, SLOT(slotSetMode(ButtonModeFlags, bool)));
    connect(pbC6, SIGNAL(clicked(int)), this, SLOT(slotConstclicked(int)));

    changeButtonNames();

    // misc buttons

    pbMod->addMode(ModeNormal, i18nc("Modulo", "Mod"), i18n("Modulo"));
    pbMod->addMode(ModeShift, i18nc("Integer division", "IntDiv"), i18n("Integer division"));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbMod, SLOT(slotSetMode(ButtonModeFlags, bool)));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbMod, SLOT(slotSetAccelDisplayMode(bool)));
    pbMod->setShortcut(QKeySequence(Qt::Key_Colon));
    connect(pbMod, SIGNAL(clicked()), SLOT(slotModclicked()));

    pbReci->addMode(ModeNormal, i18nc("Reciprocal", "1/x"), i18n("Reciprocal"));
    pbReci->addMode(ModeShift, i18nc("n Choose m", "nCm"), i18n("n Choose m"));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbReci, SLOT(slotSetMode(ButtonModeFlags, bool)));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbReci, SLOT(slotSetAccelDisplayMode(bool)));
    connect(pbReci, SIGNAL(clicked()), SLOT(slotReciclicked()));

    pbFactorial->addMode(ModeNormal, i18nc("Factorial", "x!"), i18n("Factorial"));
    pbFactorial->setShortcut(QKeySequence(Qt::Key_Exclam));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbFactorial, SLOT(slotSetAccelDisplayMode(bool)));
    connect(pbFactorial, SIGNAL(clicked()), SLOT(slotFactorialclicked()));

    pbSquare->addMode(ModeNormal, i18nc("Square", "x<sup>2</sup>"), i18n("Square"));
    pbSquare->addMode(ModeShift, "&radic;x", i18n("Square root"));
    pbSquare->setShortcut(QKeySequence(Qt::Key_BracketLeft));
    new QShortcut(Qt::Key_twosuperior, pbSquare, SLOT(animateClick()));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbSquare, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbSquare, SLOT(slotSetMode(ButtonModeFlags, bool)));
    connect(pbSquare, SIGNAL(clicked()), SLOT(slotSquareclicked()));

    pbPower->addMode(ModeNormal, i18nc("x to the power of y", "x<sup>y</sup>"), i18n("x to the power of y"));
    pbPower->addMode(ModeShift, i18nc("x to the power of 1/y", "x<sup>1/y</sup>"), i18n("x to the power of 1/y"));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbPower, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
            pbPower, SLOT(slotSetMode(ButtonModeFlags, bool)));
    pbPower->setShortcut(QKeySequence(Qt::Key_AsciiCircum));
    connect(pbPower, SIGNAL(clicked()), SLOT(slotPowerclicked()));

    pbEE->addMode(ModeNormal, "x<small>" "\xb7" "10</small><sup>y</sup>",
                  i18n("Exponent"));
    connect(this, SIGNAL(switchShowAccels(bool)),
            pbEE, SLOT(slotSetAccelDisplayMode(bool)));
    connect(pbEE, SIGNAL(clicked()), SLOT(slotEEclicked()));

    // other button lists

    function_button_list_.append(pbHyp);
    function_button_list_.append(pbShift);
    function_button_list_.append(pbEE);
    function_button_list_.append(pbSin);
    function_button_list_.append(pbPlusMinus);
    function_button_list_.append(pbCos);
    function_button_list_.append(pbReci);
    function_button_list_.append(pbTan);
    function_button_list_.append(pbFactorial);
    function_button_list_.append(pbLog);
    function_button_list_.append(pbSquare);
    function_button_list_.append(pbLn);
    function_button_list_.append(pbPower);

    mem_button_list_.append(pbMemRecall);
    mem_button_list_.append(pbMemPlusMinus);
    mem_button_list_.append(pbMemStore);
    mem_button_list_.append(pbMemClear);
    mem_button_list_.append(pbClear);
    mem_button_list_.append(pbAllClear);

    operation_button_list_.append(pbMultiplication);
    operation_button_list_.append(pbParenOpen);
    operation_button_list_.append(pbParenClose);
    operation_button_list_.append(pbAND);
    operation_button_list_.append(pbDivision);
    operation_button_list_.append(pbOR);
    operation_button_list_.append(pbXOR);
    operation_button_list_.append(pbPlus);
    operation_button_list_.append(pbMinus);
    operation_button_list_.append(pbLsh);
    operation_button_list_.append(pbRsh);
    operation_button_list_.append(pbPeriod);
    operation_button_list_.append(pbEqual);
    operation_button_list_.append(pbPercent);
    operation_button_list_.append(pbCmp);
    operation_button_list_.append(pbMod);
    operation_button_list_.append(pbCube);
}

void KCalculator::updateGeometry()
{
    KCalcButton *button;
    QSize em = pbAND->fontMetrics().size(0, "M");
    int margin =
        QApplication::style()->pixelMetric(QStyle::PM_ButtonMargin, 0, 0);
    margin = qMax(qMin(margin / 2, 3), 3);

    // left pad

    foreach(QObject *obj, leftPad->children()) {
        button = qobject_cast<KCalcButton*>(obj);
        if (button) {
            button->setFixedWidth(em.width()*4 + margin*2);
            button->installEventFilter(this);
        }
    }

    // right pad

    foreach(QObject *obj, rightPad->children()) {
        button = qobject_cast<KCalcButton*>(obj);
        if (button) {
            button->setFixedWidth(em.width()*3 + margin*2);
            button->installEventFilter(this);
        }
    }

    // numeric pad

    foreach(QObject *obj, numericPad->children()) {
        button = qobject_cast<KCalcButton*>(obj);
        if (button) {
            if (button != pb0) {   // let pb0 expand freely
                button->setFixedWidth(em.width()*3 + margin*2);
            }
            button->installEventFilter(this);
        }
    }
}

void KCalculator::slotConstantToDisplay(struct science_constant const &const_chosen)
{
    calc_display->setAmount(const_chosen.value);

    updateDisplay(false);
}

void KCalculator::slotBaseSelected(int base)
{
    int current_base;

    // set display & statusbar (if item exist in statusbar)
    switch (base) {
    case BinMode:
        current_base = calc_display->setBase(NumBase(2));
        statusBar()->changeItem("BIN", BaseField);
        calc_display->setStatusText(BaseField, "Bin");
        break;
    case OctMode:
        current_base = calc_display->setBase(NumBase(8));
        statusBar()->changeItem("OCT", BaseField);
        calc_display->setStatusText(BaseField, "Oct");
        break;
    case DecMode:
        current_base = calc_display->setBase(NumBase(10));
        statusBar()->changeItem("DEC", BaseField);
        calc_display->setStatusText(BaseField, "Dec");
        break;
    case HexMode:
        current_base = calc_display->setBase(NumBase(16));
        statusBar()->changeItem("HEX", BaseField);
        calc_display->setStatusText(BaseField, "Hex");
        break;
    default:
        statusBar()->changeItem("Error", BaseField);
        calc_display->setStatusText(BaseField, "Error");
        return;
    }

    // Enable the buttons not available in this base
    for (int i = 0; i < current_base; i++)
        (num_button_group_->buttons()[i])->setEnabled(true);

    // Disable the buttons not available in this base
    for (int i = current_base; i < 16; i++)
        (num_button_group_->buttons()[i])->setEnabled(false);

    // Only enable the decimal point in decimal
    pbPeriod->setEnabled(current_base == NB_DECIMAL);
    // Only enable the x*10^y button in decimal
    pbEE->setEnabled(current_base == NB_DECIMAL);

    // Disable buttons that make only sense with floating point numbers
    if (current_base != NB_DECIMAL)  {
        foreach(QAbstractButton *btn, scientific_buttons_) {
            btn->setEnabled(false);
        }
    } else {
        foreach(QAbstractButton *btn, scientific_buttons_) {
            btn->setEnabled(true);
        }
    }
    KCalcSettings::setBaseMode(base);
}

void KCalculator::keyPressEvent(QKeyEvent *e)
{
    if (((e->modifiers() & Qt::NoModifier) == 0) ||
            (e->modifiers() & Qt::ShiftModifier)) {
        switch (e->key()) {
        case Qt::Key_Backspace:
            calc_display->deleteLastDigit();
            break;
        }
    }

    if (e->key() == Qt::Key_Control)
        emit switchShowAccels(true);
}

void KCalculator::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Control)
        emit switchShowAccels(false);
}

void KCalculator::slotAngleSelected(int mode)
{
    angle_mode_ = mode;
    switch (mode) {
    case DegMode:
        statusBar()->changeItem("DEG", AngleField);
        calc_display->setStatusText(AngleField, "Deg");
        break;
    case RadMode:
        statusBar()->changeItem("RAD", AngleField);
        calc_display->setStatusText(AngleField, "Rad");
        break;
    case GradMode:
        statusBar()->changeItem("GRA", AngleField);
        calc_display->setStatusText(AngleField, "Gra");
        break;
    default: // we shouldn't ever end up here
        angle_mode_ = RadMode;
    }
    KCalcSettings::setAngleMode(angle_mode_);
}

void KCalculator::slotEEclicked()
{
    calc_display->newCharacter('e');
}

void KCalculator::slotShifttoggled(bool flag)
{
    shift_mode_ = flag;

    emit switchMode(ModeShift, flag);

    if (shift_mode_) {
        statusBar()->changeItem(i18nc("Second button functions are active", "SHIFT"), ShiftField);
        calc_display->setStatusText(ShiftField, i18n("Shift"));
    } else {
        statusBar()->changeItem(i18nc("Normal button functions are active", "NORM"), ShiftField);
        calc_display->setStatusText(ShiftField, QString());
    }
}

void KCalculator::slotHyptoggled(bool flag)
{
    // toggle between hyperbolic and standart trig functions
    hyp_mode_ = flag;

    emit switchMode(ModeHyperbolic, flag);
}



void KCalculator::slotMemRecallclicked()
{
    // temp. work-around
    calc_display->sendEvent(KCalcDisplay::EventReset);

    calc_display->setAmount(memory_num_);
    updateDisplay(false);
}

void KCalculator::slotMemStoreclicked()
{
    EnterEqual();

    memory_num_ = calc_display->getAmount();
    calc_display->setStatusText(MemField, "M");
    statusBar()->changeItem("M", MemField);
    pbMemRecall->setEnabled(true);
}

void KCalculator::slotNumberclicked(int number_clicked)
{
    calc_display->enterDigit(number_clicked);
}

void KCalculator::slotSinclicked()
{
    if (hyp_mode_) {
        // sinh or arsinh
        if (!shift_mode_)
            core.SinHyp(calc_display->getAmount());
        else
            core.AreaSinHyp(calc_display->getAmount());
    } else {
        // sine or arcsine
        if (!shift_mode_)
            switch (angle_mode_) {
            case DegMode:
                core.SinDeg(calc_display->getAmount());
                break;
            case RadMode:
                core.SinRad(calc_display->getAmount());
                break;
            case GradMode:
                core.SinGrad(calc_display->getAmount());
                break;
            }
        else
            switch (angle_mode_) {
            case DegMode:
                core.ArcSinDeg(calc_display->getAmount());
                break;
            case RadMode:
                core.ArcSinRad(calc_display->getAmount());
                break;
            case GradMode:
                core.ArcSinGrad(calc_display->getAmount());
                break;
            }
    }

    updateDisplay(true);
}

void KCalculator::slotPlusMinusclicked()
{
    // display can only change sign, when in input mode, otherwise we
    // need the core to do this.
    if (!calc_display->sendEvent(KCalcDisplay::EventChangeSign)) {
        core.InvertSign(calc_display->getAmount());
        updateDisplay(true);
    }
}

void KCalculator::slotMemPlusMinusclicked()
{
    bool tmp_shift_mode = shift_mode_; // store this, because next command deletes shift_mode_
    EnterEqual(); // finish calculation so far, to store result into MEM

    if (!tmp_shift_mode) memory_num_ += calc_display->getAmount();
    else    memory_num_ -= calc_display->getAmount();

    pbShift->setChecked(false);
    statusBar()->changeItem(i18n("M"), MemField);
    calc_display->setStatusText(MemField, i18n("M"));
    pbMemRecall->setEnabled(true);
}

void KCalculator::slotCosclicked()
{
    if (hyp_mode_) {
        // cosh or arcosh
        if (!shift_mode_)
            core.CosHyp(calc_display->getAmount());
        else
            core.AreaCosHyp(calc_display->getAmount());
    } else {
        // cosine or arccosine
        if (!shift_mode_)
            switch (angle_mode_) {
            case DegMode:
                core.CosDeg(calc_display->getAmount());
                break;
            case RadMode:
                core.CosRad(calc_display->getAmount());
                break;
            case GradMode:
                core.CosGrad(calc_display->getAmount());
                break;
            }
        else
            switch (angle_mode_) {
            case DegMode:
                core.ArcCosDeg(calc_display->getAmount());
                break;
            case RadMode:
                core.ArcCosRad(calc_display->getAmount());
                break;
            case GradMode:
                core.ArcCosGrad(calc_display->getAmount());
                break;
            }
    }

    updateDisplay(true);
}

void KCalculator::slotReciclicked()
{
    if (shift_mode_) {
        core.enterOperation(calc_display->getAmount(),
                            CalcEngine::FUNC_BINOM);
    }  else {
        core.Reciprocal(calc_display->getAmount());
        updateDisplay(true);
        return;
    }
    // temp. work-around
    KNumber tmp_num = calc_display->getAmount();
    calc_display->sendEvent(KCalcDisplay::EventReset);
    calc_display->setAmount(tmp_num);
    updateDisplay(false);

}

void KCalculator::slotTanclicked()
{
    if (hyp_mode_) {
        // tanh or artanh
        if (!shift_mode_)
            core.TangensHyp(calc_display->getAmount());
        else
            core.AreaTangensHyp(calc_display->getAmount());
    } else {
        // tan or arctan
        if (!shift_mode_)
            switch (angle_mode_) {
            case DegMode:
                core.TangensDeg(calc_display->getAmount());
                break;
            case RadMode:
                core.TangensRad(calc_display->getAmount());
                break;
            case GradMode:
                core.TangensGrad(calc_display->getAmount());
                break;
            }
        else
            switch (angle_mode_) {
            case DegMode:
                core.ArcTangensDeg(calc_display->getAmount());
                break;
            case RadMode:
                core.ArcTangensRad(calc_display->getAmount());
                break;
            case GradMode:
                core.ArcTangensGrad(calc_display->getAmount());
                break;
            }
    }

    updateDisplay(true);
}

void KCalculator::slotFactorialclicked()
{
    // Set WaitCursor, as this operation may take looooong
    // time and UI frezes with large numbers. User needs some
    // visual feedback.
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    core.Factorial(calc_display->getAmount());
    QApplication::restoreOverrideCursor();
    updateDisplay(true);
}

void KCalculator::slotLogclicked()
{
    if (!shift_mode_)
        core.Log10(calc_display->getAmount());
    else
        core.Exp10(calc_display->getAmount());

    updateDisplay(true);
}


void KCalculator::slotSquareclicked()
{
    if (!shift_mode_)
        core.Square(calc_display->getAmount());
    else
        core.SquareRoot(calc_display->getAmount());

    updateDisplay(true);
}


void KCalculator::slotCubeclicked()
{
    if (!shift_mode_)
        core.Cube(calc_display->getAmount());
    else
        core.CubeRoot(calc_display->getAmount());

    updateDisplay(true);
}


void KCalculator::slotLnclicked()
{
    if (!shift_mode_)
        core.Ln(calc_display->getAmount());
    else
        core.Exp(calc_display->getAmount());

    updateDisplay(true);
}

void KCalculator::slotPowerclicked()
{
    if (shift_mode_) {
        core.enterOperation(calc_display->getAmount(),
                            CalcEngine::FUNC_PWR_ROOT);
        pbShift->setChecked(false);
    } else {
        core.enterOperation(calc_display->getAmount(),
                            CalcEngine::FUNC_POWER);
    }
    // temp. work-around
    KNumber tmp_num = calc_display->getAmount();
    calc_display->sendEvent(KCalcDisplay::EventReset);
    calc_display->setAmount(tmp_num);
    updateDisplay(false);
}

void KCalculator::slotMemClearclicked()
{
    memory_num_  = 0;
    statusBar()->changeItem(" \xa0\xa0 ", MemField);
    calc_display->setStatusText(MemField, QString());
    pbMemRecall->setDisabled(true);
}

void KCalculator::slotClearclicked()
{
    calc_display->sendEvent(KCalcDisplay::EventClear);
}

void KCalculator::slotAllClearclicked()
{
    core.Reset();
    calc_display->sendEvent(KCalcDisplay::EventReset);

    updateDisplay(true);
}

void KCalculator::slotParenOpenclicked()
{
    core.ParenOpen(calc_display->getAmount());

    // What behavior, if e.g.: "12(6*6)"??
    //updateDisplay(true);
}

void KCalculator::slotParenCloseclicked()
{
    core.ParenClose(calc_display->getAmount());

    updateDisplay(true);
}

void KCalculator::slotANDclicked()
{
    core.enterOperation(calc_display->getAmount(),
                        CalcEngine::FUNC_AND);

    updateDisplay(true);
}

void KCalculator::slotMultiplicationclicked()
{
    core.enterOperation(calc_display->getAmount(),
                        CalcEngine::FUNC_MULTIPLY);

    updateDisplay(true);
}

void KCalculator::slotDivisionclicked()
{
    core.enterOperation(calc_display->getAmount(),
                        CalcEngine::FUNC_DIVIDE);

    updateDisplay(true);
}

void KCalculator::slotORclicked()
{
    core.enterOperation(calc_display->getAmount(),
                        CalcEngine::FUNC_OR);

    updateDisplay(true);
}

void KCalculator::slotXORclicked()
{
    core.enterOperation(calc_display->getAmount(),
                        CalcEngine::FUNC_XOR);

    updateDisplay(true);
}

void KCalculator::slotPlusclicked()
{
    core.enterOperation(calc_display->getAmount(),
                        CalcEngine::FUNC_ADD);

    updateDisplay(true);
}

void KCalculator::slotMinusclicked()
{
    core.enterOperation(calc_display->getAmount(),
                        CalcEngine::FUNC_SUBTRACT);

    updateDisplay(true);
}

void KCalculator::slotLeftShiftclicked()
{
    core.enterOperation(calc_display->getAmount(),
                        CalcEngine::FUNC_LSH);

    updateDisplay(true);
}

void KCalculator::slotRightShiftclicked()
{
    core.enterOperation(calc_display->getAmount(),
                        CalcEngine::FUNC_RSH);

    updateDisplay(true);
}

void KCalculator::slotPeriodclicked()
{
    calc_display->newCharacter('.');
}

void KCalculator::EnterEqual()
{
    core.enterOperation(calc_display->getAmount(),
                        CalcEngine::FUNC_EQUAL);

    updateDisplay(true, true);
}

void KCalculator::slotEqualclicked()
{
    EnterEqual();
}

void KCalculator::slotPercentclicked()
{
    core.enterOperation(calc_display->getAmount(),
                        CalcEngine::FUNC_PERCENT);

    updateDisplay(true);
}

void KCalculator::slotNegateclicked()
{
    core.Complement(calc_display->getAmount());

    updateDisplay(true);
}

void KCalculator::slotModclicked()
{
    if (shift_mode_)
        core.enterOperation(calc_display->getAmount(),
                            CalcEngine::FUNC_INTDIV);
    else
        core.enterOperation(calc_display->getAmount(),
                            CalcEngine::FUNC_MOD);

    updateDisplay(true);
}

void KCalculator::slotStatNumclicked()
{
    if (!shift_mode_) {
        core.StatCount(0);
    } else {
        pbShift->setChecked(false);
        core.StatSum(0);
    }

    updateDisplay(true);
}

void KCalculator::slotStatMeanclicked()
{
    if (!shift_mode_)
        core.StatMean(0);
    else {
        pbShift->setChecked(false);
        core.StatSumSquares(0);
    }

    updateDisplay(true);
}

void KCalculator::slotStatStdDevclicked()
{
    if (shift_mode_) {
        // std (n-1)
        core.StatStdDeviation(0);
        pbShift->setChecked(false);
    } else {
        // std (n)
        core.StatStdSample(0);
    }

    updateDisplay(true);
}

void KCalculator::slotStatMedianclicked()
{
    if (!shift_mode_) {
        // std (n-1)
        core.StatMedian(0);
    } else {
        // std (n)
        core.StatMedian(0);
        pbShift->setChecked(false);
    }
    // it seems two different modes should be implemented, but...?
    updateDisplay(true);
}

void KCalculator::slotStatDataInputclicked()
{
    if (!shift_mode_) {
        core.StatDataNew(calc_display->getAmount());
    } else {
        pbShift->setChecked(false);
        core.StatDataDel(0);
        statusBar()->showMessage(i18n("Last stat item erased"), 3000);
    }

    updateDisplay(true);
}

void KCalculator::slotStatClearDataclicked()
{
    if (!shift_mode_) {
        core.StatClearAll(0);
        statusBar()->showMessage(i18n("Stat mem cleared"), 3000);
    } else {
        pbShift->setChecked(false);
        updateDisplay(false);
    }
}

void KCalculator::slotConstclicked(int button)
{
    KCalcConstButton *btn = qobject_cast<KCalcConstButton*>(const_buttons_[button]);
    if (!btn) return;

    if (!shift_mode_) {
        // set the display to the configured value of constant button
        calc_display->setAmount(btn->constant());
    } else {
        pbShift->setChecked(false);
        KCalcSettings::setValueConstant(button, calc_display->text());
        // below set new tooltip
        btn->setLabelAndTooltip();
        // work around: after storing a number, pressing a digit should start
        // a new number
        calc_display->setAmount(calc_display->getAmount());
    }

    updateDisplay(false);
}

void KCalculator::showSettings()
{
    // Check if there is already a dialog and if so bring
    // it to the foreground.
    if (KConfigDialog::showDialog("settings"))
        return;

    // Create a new dialog with the same name as the above checking code.
    KConfigDialog *dialog = new KConfigDialog(this, "settings", KCalcSettings::self());
    dialog->showButtonSeparator(true);

    // general settings

    General *general = new General(0);
    general->kcfg_Precision->setMaximum(maxprecision);
    dialog->addPage(general, i18n("General"), "accessories-calculator", i18n("General Settings"));

    // font settings

    Fonts *fonts = new Fonts(0);
    dialog->addPage(fonts, i18n("Font"), "preferences-desktop-font", i18n("Select Display Font"));

    // color settings

    Colors *color = new Colors(0);
    dialog->addPage(color, i18n("Colors"), "format-fill-color", i18n("Button & Display Colors"));

    // constant settings

    if (!constants_)
        constants_ = new Constants(0);

    KCalcConstMenu *tmp_menu = new KCalcConstMenu(this);
    connect(tmp_menu,
            SIGNAL(triggeredConstant(science_constant const &)),
            this,
            SLOT(slotChooseScientificConst0(science_constant const &)));
    constants_->pushButton0->setMenu(tmp_menu);

    tmp_menu = new KCalcConstMenu(this);
    connect(tmp_menu,
            SIGNAL(triggeredConstant(science_constant const &)),
            this,
            SLOT(slotChooseScientificConst1(science_constant const &)));
    constants_->pushButton1->setMenu(tmp_menu);

    tmp_menu = new KCalcConstMenu(this);
    connect(tmp_menu,
            SIGNAL(triggeredConstant(science_constant const &)),
            this,
            SLOT(slotChooseScientificConst2(science_constant const &)));
    constants_->pushButton2->setMenu(tmp_menu);

    tmp_menu = new KCalcConstMenu(this);
    connect(tmp_menu,
            SIGNAL(triggeredConstant(science_constant const &)),
            this,
            SLOT(slotChooseScientificConst3(science_constant const &)));
    constants_->pushButton3->setMenu(tmp_menu);

    tmp_menu = new KCalcConstMenu(this);
    connect(tmp_menu,
            SIGNAL(triggeredConstant(science_constant const &)),
            this,
            SLOT(slotChooseScientificConst4(science_constant const &)));
    constants_->pushButton4->setMenu(tmp_menu);

    tmp_menu = new KCalcConstMenu(this);
    connect(tmp_menu,
            SIGNAL(triggeredConstant(science_constant const &)),
            this,
            SLOT(slotChooseScientificConst5(science_constant const &)));
    constants_->pushButton5->setMenu(tmp_menu);

    dialog->addPage(constants_, i18n("Constants"), "preferences-kcalc-constants_", i18n("Define Constants"));

    // When the user clicks OK or Apply we want to update our settings.
    connect(dialog, SIGNAL(settingsChanged(const QString &)), SLOT(updateSettings()));

    // Display the dialog.
    dialog->show();
}


// these 6 slots are just a quick hack, instead of setting the
// TextEdit fields in the configuration dialog, we are setting the
// Settingvalues themselves!!
void KCalculator::slotChooseScientificConst0(struct science_constant const & chosen_const)
{
    constants_->kcfg_valueConstant0->setText(chosen_const.value);

    constants_->kcfg_nameConstant0->setText(chosen_const.label);
}

void KCalculator::slotChooseScientificConst1(struct science_constant const & chosen_const)
{
    constants_->kcfg_valueConstant1->setText(chosen_const.value);

    constants_->kcfg_nameConstant1->setText(chosen_const.label);
}

void KCalculator::slotChooseScientificConst2(struct science_constant const & chosen_const)
{
    constants_->kcfg_valueConstant2->setText(chosen_const.value);

    constants_->kcfg_nameConstant2->setText(chosen_const.label);
}

void KCalculator::slotChooseScientificConst3(struct science_constant const & chosen_const)
{
    constants_->kcfg_valueConstant3->setText(chosen_const.value);

    constants_->kcfg_nameConstant3->setText(chosen_const.label);
}

void KCalculator::slotChooseScientificConst4(struct science_constant const & chosen_const)
{
    constants_->kcfg_valueConstant4->setText(chosen_const.value);

    constants_->kcfg_nameConstant4->setText(chosen_const.label);
}

void KCalculator::slotChooseScientificConst5(struct science_constant const & chosen_const)
{
    constants_->kcfg_valueConstant5->setText(chosen_const.value);

    constants_->kcfg_nameConstant5->setText(chosen_const.label);
}

void KCalculator::slotSetSimpleMode()
{
    action_constants_show_->setChecked(false);
    action_constants_show_->setEnabled(false);
    action_bitset_show_->setChecked(false);
    action_bitset_show_->setEnabled(false);
    showScienceButtons(false);
    showStatButtons(false);
    showLogicButtons(false);
    KCalcSettings::setCalculatorMode(KCalcSettings::EnumCalculatorMode::simple);
}

void KCalculator::slotSetScienceMode()
{
    action_constants_show_->setEnabled(true);
    action_constants_show_->setChecked(KCalcSettings::showConstants());
    action_bitset_show_->setChecked(false);
    action_bitset_show_->setEnabled(false);
    showScienceButtons(true);
    showStatButtons(false);
    showLogicButtons(false);
    KCalcSettings::setCalculatorMode(KCalcSettings::EnumCalculatorMode::science);
}

void KCalculator::slotSetStatisticMode()
{
    action_constants_show_->setEnabled(true);
    action_constants_show_->setChecked(KCalcSettings::showConstants());
    action_bitset_show_->setChecked(false);
    action_bitset_show_->setEnabled(false);
    showScienceButtons(true);
    showStatButtons(true);
    showLogicButtons(false);
    KCalcSettings::setCalculatorMode(KCalcSettings::EnumCalculatorMode::statistics);
}

void KCalculator::slotSetNumeralMode()
{
    action_constants_show_->setChecked(false);
    action_constants_show_->setEnabled(false);
    action_bitset_show_->setEnabled(true);
    action_bitset_show_->setChecked(KCalcSettings::showBitset());
    showScienceButtons(false);
    showStatButtons(false);
    showLogicButtons(true);
    KCalcSettings::setCalculatorMode(KCalcSettings::EnumCalculatorMode::numeral);
}

void KCalculator::showStatButtons(bool toggled)
{
    if (toggled) {
        foreach(QAbstractButton *btn, stat_buttons_) {
            btn->show();
        }
    } else {
        foreach(QAbstractButton *btn, stat_buttons_) {
            btn->hide();
        }
    }
}

void KCalculator::showScienceButtons(bool toggled)
{
    if (toggled) {
        foreach(QAbstractButton* btn, scientific_buttons_) {
            btn->show();
        }
        foreach(QAbstractButton* btn, angle_choose_group_->buttons()) {
            btn->show();
        }
        statusBar()->setItemFixed(AngleField, -1);
        setAngle();
    } else {
        foreach(QAbstractButton* btn, scientific_buttons_) {
            btn->hide();
        }
        foreach(QAbstractButton* btn, angle_choose_group_->buttons()) {
            btn->hide();
        }
        statusBar()->changeItem(QString(), AngleField);
        statusBar()->setItemFixed(AngleField, 0);
        calc_display->setStatusText(AngleField, QString());
    }
}

void KCalculator::showLogicButtons(bool toggled)
{
    if (toggled) {
        mBitset->setEnabled(true);
        connect(mBitset, SIGNAL(valueChanged(unsigned long long)),
                this, SLOT(slotBitsetChanged(unsigned long long)));
        connect(calc_display, SIGNAL(changedAmount(const KNumber &)),
                SLOT(slotUpdateBitset(const KNumber &)));
        foreach(QAbstractButton* btn, logic_buttons_) {
            btn->show();
        }
        statusBar()->setItemFixed(BaseField, -1);
        setBase();

        foreach(QAbstractButton *btn, base_choose_group_->buttons()) {
            btn->show();
        }
        for (int i = 10; i < 16; i++)
            (num_button_group_->button(i))->show();
    } else {
        mBitset->setEnabled(false);
        disconnect(mBitset, SIGNAL(valueChanged(unsigned long long)),
                   this, SLOT(slotBitsetChanged(unsigned long long)));
        disconnect(calc_display, SIGNAL(changedAmount(const KNumber &)),
                   this, SLOT(slotUpdateBitset(const KNumber &)));
        foreach(QAbstractButton* btn, logic_buttons_) {
            btn->hide();
        }
        // Hide Hex-Buttons, but first switch back to decimal
        decRadio->animateClick(0);
        foreach(QAbstractButton *btn, base_choose_group_->buttons()) {
            btn->hide();
        }
        statusBar()->changeItem(QString(), BaseField);
        statusBar()->setItemFixed(BaseField, 0);
        calc_display->setStatusText(BaseField, QString());
        for (int i = 10; i < 16; i++)
            (num_button_group_->button(i))->hide();
    }
}

void KCalculator::slotConstantsShow(bool toggled)
{
    if (toggled) {
        foreach(QAbstractButton *btn, const_buttons_) {
            btn->show();
        }
    } else {
        foreach(QAbstractButton *btn, const_buttons_) {
            btn->hide();
        }
    }
    KCalcSettings::setShowConstants(toggled);
}

void KCalculator::slotBitsetshow(bool toggled)
{
    mBitset->setVisible(toggled);
    KCalcSettings::setShowBitset(toggled);
}

// This function is for setting the constant names configured in the
// kcalc settings menu. If the user doesn't enter a name for the
// constant C1 to C6 is used.
void KCalculator::changeButtonNames()
{
    KCalcConstButton *constbtn;
    foreach(QAbstractButton *btn, const_buttons_) {
        constbtn = qobject_cast<KCalcConstButton*>(btn);
        if (constbtn) constbtn->setLabelAndTooltip();
    }
}

void KCalculator::slotBitsetChanged(unsigned long long value)
{
    // note: sets display to *unsigned* value
    calc_display->setAmount(value);
    updateDisplay(false);
}

void KCalculator::slotUpdateBitset(const KNumber &nr)
{
    mBitset->setValue(static_cast<quint64>(nr));
}

void KCalculator::updateSettings()
{
    changeButtonNames();
    setColors();
    setFonts();
    setPrecision();
    // Show the result in the app's caption in taskbar (wishlist - bug #52858)
    disconnect(calc_display, SIGNAL(changedText(const QString &)),
               this, 0);
    if (KCalcSettings::captionResult()) {
        connect(calc_display,
                SIGNAL(changedText(const QString &)),
                SLOT(setCaption(const QString &)));
    } else {
        setCaption(QString());
    }
    calc_display->changeSettings();

    updateGeometry();
}

void KCalculator::updateDisplay(bool get_amount_from_core,
                                bool store_result_in_history)
{
    if (get_amount_from_core) {
        calc_display->updateFromCore(core, store_result_in_history);
    } else {
        calc_display->update();
    }

    pbShift->setChecked(false);

}

void KCalculator::setColors()
{
    calc_display->changeSettings();

    QString sheet = "KPushButton { background-color: %1 }";

    QColor numPal(KCalcSettings::numberButtonsColor());
    for (int i = 0; i < 10; i++) {
        (num_button_group_->button(i))->setStyleSheet(sheet.arg(numPal.name()));
    }

    QColor funcPal(KCalcSettings::functionButtonsColor());
    foreach(QAbstractButton *btn, function_button_list_) {
        btn->setStyleSheet(sheet.arg(funcPal.name()));
    }

    QColor statPal(KCalcSettings::statButtonsColor());
    foreach(QAbstractButton *btn, stat_buttons_) {
        btn->setStyleSheet(sheet.arg(statPal.name()));
    }

    QColor hexPal(KCalcSettings::hexButtonsColor());
    for (int i = 10; i < 16; i++) {
        (num_button_group_->button(i))->setStyleSheet(sheet.arg(hexPal.name()));
    }

    QColor memPal(KCalcSettings::memoryButtonsColor());
    foreach(QAbstractButton *btn, mem_button_list_) {
        btn->setStyleSheet(sheet.arg(memPal.name()));
    }

    QColor opPal(KCalcSettings::operationButtonsColor());
    foreach(QAbstractButton *btn, operation_button_list_) {
        btn->setStyleSheet(sheet.arg(opPal.name()));
    }
}

void KCalculator::setFonts()
{
    KCalcButton *button;
    foreach(QObject *obj, leftPad->children()) {
        button = qobject_cast<KCalcButton*>(obj);
        if (button) {
            button->setFont(KCalcSettings::buttonFont());
        }
    }
    foreach(QObject *obj, numericPad->children()) {
        button = qobject_cast<KCalcButton*>(obj);
        if (button) {
            button->setFont(KCalcSettings::buttonFont());
        }
    }
    foreach(QObject *obj, rightPad->children()) {
        button = qobject_cast<KCalcButton*>(obj);
        if (button) {
            button->setFont(KCalcSettings::buttonFont());
        }
    }
    updateGeometry();
}

void KCalculator::setPrecision()
{
    KNumber:: setDefaultFloatPrecision(KCalcSettings::precision());
    updateDisplay(false);
}

void KCalculator::setAngle()
{
    QAbstractButton *btn;
    btn = angle_choose_group_->button(KCalcSettings::angleMode());
    if (btn) btn->animateClick(0);
}

void KCalculator::setBase()
{
    QAbstractButton *btn;
    btn = base_choose_group_->button(KCalcSettings::baseMode());
    if (btn) btn->animateClick(0);
}

bool KCalculator::eventFilter(QObject *o, QEvent *e)
{
    switch (e->type()) {
    case QEvent::DragEnter: {
        QDragEnterEvent *ev = (QDragEnterEvent *)e;
        ev->setAccepted(KColorMimeData::canDecode(ev->mimeData()));
        return true;
    }
    case QEvent::DragLeave: {
        return true;
    }
    case QEvent::Drop: {
        KCalcButton* calcButton = qobject_cast<KCalcButton *>(o);
        if (!calcButton)
            return false;

        QDropEvent *ev = (QDropEvent *)e;
        QColor c = KColorMimeData::fromMimeData(ev->mimeData());
        if (c.isValid()) {
            QString cn = c.name();
            QString sheet = "background-color: %1";

            QList<QAbstractButton*> *list;
            int num_but;
            if ((num_but = num_button_group_->buttons().indexOf(calcButton)) != -1) {
                // Was it hex-button or normal digit??
                if (num_but < 10)
                    for (int i = 0; i < 10; i++)
                        (num_button_group_->buttons()[i])->setStyleSheet(sheet.arg(cn));
                else
                    for (int i = 10; i < 16; i++)
                        (num_button_group_->buttons()[i])->setStyleSheet(sheet.arg(cn));

                return true;
            } else if (function_button_list_.contains(calcButton)) {
                list = &function_button_list_;
            } else if (stat_button_list_.contains(calcButton)) {
                list = &stat_button_list_;
            } else if (mem_button_list_.contains(calcButton)) {
                list = &mem_button_list_;
            } else if (operation_button_list_.contains(calcButton)) {
                list = &operation_button_list_;
            } else
                return false;

            for (int i = 0; i < list->size(); i++)
                list->at(i)->setStyleSheet(sheet.arg(cn));
        }

        return true;
    }
    default:
        return KXmlGuiWindow::eventFilter(o, e);
    }
}




////////////////////////////////////////////////////////////////
// Include the meta-object code for classes in this file
//

#include "kcalc.moc"


extern "C" KDE_EXPORT int kdemain(int argc, char *argv[])
{
    KAboutData aboutData("kcalc",
                         0,
                         ki18n("KCalc"),
                         version,
                         ki18n(description),
                         KAboutData::License_GPL,
                         ki18n("(c) 2000-2008, The KDE Team\n"
                               "(c) 2003-2005, Klaus Niederkr" "\xc3\xbc" "ger\n"
                               "(c) 1996-2000, Bernd Johannes Wuebben"),
                         KLocalizedString(),
                         "http://utils.kde.org/projects/kcalc");

    /* Klaus Niederkrueger */
    aboutData.addAuthor(ki18n("Klaus Niederkr" "\xc3\xbc" "ger"), KLocalizedString(), "kniederk@math.uni-koeln.de");
    aboutData.addAuthor(ki18n("Bernd Johannes Wuebben"), KLocalizedString(), "wuebben@kde.org");
    aboutData.addAuthor(ki18n("Evan Teran"), KLocalizedString(), "eteran@alum.rit.edu");
    aboutData.addAuthor(ki18n("Espen Sand"), KLocalizedString(), "espen@kde.org");
    aboutData.addAuthor(ki18n("Chris Howells"), KLocalizedString(), "howells@kde.org");
    aboutData.addAuthor(ki18n("Aaron J. Seigo"), KLocalizedString(), "aseigo@olympusproject.org");
    aboutData.addAuthor(ki18n("Charles Samuels"), KLocalizedString(), "charles@altair.dhs.org");
    /* Rene Merou */
    aboutData.addAuthor(ki18n("Ren" "\xc3\xa9" " M" "\xc3\xa9"  "rou"), KLocalizedString(), "ochominutosdearco@yahoo.es");
    aboutData.addAuthor(ki18n("Michel Marti"), KLocalizedString(), "mma@objectxp.com");
    aboutData.addAuthor(ki18n("David Johnson"), ki18n("Maintainer"), "david@usermode.org");

    aboutData.setProgramIconName("accessories-calculator");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app;

    // force system locale to "C" internally [bug 159168]
    setlocale(LC_NUMERIC, "C");

    KCalculator *calc = new KCalculator(0);
    app.setTopWidget(calc);

    calc->show();

    return app.exec();
}

