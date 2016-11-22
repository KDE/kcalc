/*
Copyright (C) 2001 - 2013 Evan Teran
                          evan.teran@gmail.com

Copyright (C) 2006        Michel Marti
                          mma@objectxp.com

Copyright (C) 1996 - 2000 Bernd Johannes Wuebben
                          wuebben@kde.org

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kcalc_version.h"
#include "kcalc.h"

#include <clocale>

#include <QApplication>
#include <QCommandLineParser>
#include <QCursor>
#include <QKeyEvent>
#include <QMenuBar>
#include <QShortcut>
#include <QStyle>

#include <KAboutData>
#include <KAcceleratorManager>
#include <KActionCollection>
#include <KColorMimeData>
#include <KConfigDialog>
#include <KStandardAction>
#include <KToggleAction>
#include <KToolBar>
#include <kxmlguifactory.h>
#include <Kdelibs4ConfigMigrator>

#include "kcalc_bitset.h"
#include "kcalc_const_menu.h"
#include "kcalc_settings.h"
#include "kcalc_statusbar.h"
#include "kcalcdisplay.h"

namespace {
const char description[] = I18N_NOOP("KDE Calculator");
const int maxprecision   = 1000;
}


//------------------------------------------------------------------------------
// Name: KCalculator
// Desc: constructor
//------------------------------------------------------------------------------
KCalculator::KCalculator(QWidget *parent) :
		KXmlGuiWindow(parent),
		shift_mode_(false),
		hyp_mode_(false),
		memory_num_(0.0),
		constants_menu_(0),
		constants_(0),
		core() {

	// central widget to contain all the elements
	QWidget *const central = new QWidget(this);
	central->setLayoutDirection(Qt::LeftToRight);
	setCentralWidget(central);
	KAcceleratorManager::setNoAccel(central);

	// load science constants_ from xml-file
	KCalcConstMenu::init_consts();

	// setup interface (order is critical)
	setupUi(central);
	setupMainActions();
	setStatusBar(new KCalcStatusBar(this));
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
	connect(base_choose_group_, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &KCalculator::slotBaseSelected);

	angle_choose_group_ =  new QButtonGroup(this);
	angle_choose_group_->setExclusive(true);
	angle_choose_group_->addButton(degRadio, DegMode);
	angle_choose_group_->addButton(radRadio, RadMode);
	angle_choose_group_->addButton(gradRadio, GradMode);
	connect(angle_choose_group_, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &KCalculator::slotAngleSelected);

	// additional menu setup
	constants_menu_ = createConstantsMenu();
	menuBar()->insertMenu((menuBar()->actions)()[2], constants_menu_);

	// misc setup
	setColors();
	setFonts();

	// Show the result in the app's caption in taskbar (wishlist - bug #52858)
	if (KCalcSettings::captionResult() == true) {
                connect(calc_display, &KCalcDisplay::changedText, this, &KCalculator::setWindowTitle);
	}

	calc_display->changeSettings();
	setPrecision();

	updateGeometry();

	layout()->setSizeConstraint(QLayout::SetFixedSize);

	updateDisplay(UPDATE_FROM_CORE);

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

	calc_display->setFocus();
}

//------------------------------------------------------------------------------
// Name: ~KCalculator
// Desc: deconstructor
//------------------------------------------------------------------------------
KCalculator::~KCalculator() {

	KCalcSettings::self()->save();
}

//------------------------------------------------------------------------------
// Name: setupMainActions
// Desc: connects all of the basic actions
//------------------------------------------------------------------------------
void KCalculator::setupMainActions() {

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

	action_mode_simple_ = actionCollection()->add<KToggleAction>(QLatin1String("mode_simple"));
	action_mode_simple_->setActionGroup(modeGroup);
	action_mode_simple_->setText(i18n("Simple Mode"));
	connect(action_mode_simple_, &KToggleAction::toggled, this, &KCalculator::slotSetSimpleMode);

	action_mode_science_ = actionCollection()->add<KToggleAction>(QLatin1String("mode_science"));
	action_mode_science_->setActionGroup(modeGroup);
	action_mode_science_->setText(i18n("Science Mode"));
	connect(action_mode_science_, &KToggleAction::toggled, this, &KCalculator::slotSetScienceMode);

	action_mode_statistic_ = actionCollection()->add<KToggleAction>(QLatin1String("mode_statistics"));
	action_mode_statistic_->setActionGroup(modeGroup);
	action_mode_statistic_->setText(i18n("Statistic Mode"));
	connect(action_mode_statistic_, &KToggleAction::toggled, this, &KCalculator::slotSetStatisticMode);

	action_mode_numeral_ = actionCollection()->add<KToggleAction>(QLatin1String("mode_numeral"));
	action_mode_numeral_->setActionGroup(modeGroup);
	action_mode_numeral_->setText(i18n("Numeral System Mode"));
	connect(action_mode_numeral_, &KToggleAction::toggled, this, &KCalculator::slotSetNumeralMode);

	// settings menu
	action_constants_show_ = actionCollection()->add<KToggleAction>(QLatin1String("show_constants"));
	action_constants_show_->setText(i18n("Constants &Buttons"));
	action_constants_show_->setChecked(true);
	connect(action_constants_show_, &KToggleAction::toggled, this, &KCalculator::slotConstantsShow);

	action_bitset_show_ = actionCollection()->add<KToggleAction>(QLatin1String("show_bitset"));
	action_bitset_show_->setText(i18n("Show B&it Edit"));
	action_bitset_show_->setChecked(true);
	connect(action_bitset_show_, &KToggleAction::toggled, this, &KCalculator::slotBitsetshow);

	KStandardAction::preferences(this, SLOT(showSettings()), actionCollection());
	KStandardAction::keyBindings(guiFactory(), SLOT(configureShortcuts()), actionCollection());
}

//------------------------------------------------------------------------------
// Name: createConstantsMenu
// Desc: creates and returns a pointer to the constant menu
//------------------------------------------------------------------------------
KCalcConstMenu *KCalculator::createConstantsMenu() {

	KCalcConstMenu *const menu = new KCalcConstMenu(i18n("&Constants"), this);
	connect(menu, &KCalcConstMenu::triggeredConstant, this, &KCalculator::slotConstantToDisplay);
	return menu;
}

//------------------------------------------------------------------------------
// Name: statusBar
// Desc: returns a pointer to the status bar
//------------------------------------------------------------------------------
KCalcStatusBar *KCalculator::statusBar() {

	return static_cast<KCalcStatusBar *>(KXmlGuiWindow::statusBar());
}

//------------------------------------------------------------------------------
// Name: setupNumberKeys
// Desc: sets up number keys and related shortcuts
//------------------------------------------------------------------------------
void KCalculator::setupNumberKeys() {

    num_button_group_ = new QButtonGroup(this);
    connect(num_button_group_, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &KCalculator::slotNumberclicked);

    num_button_group_->addButton(pb0, 0);
    num_button_group_->addButton(pb1, 1);
    num_button_group_->addButton(pb2, 2);
    num_button_group_->addButton(pb3, 3);
    num_button_group_->addButton(pb4, 4);
    num_button_group_->addButton(pb5, 5);
    num_button_group_->addButton(pb6, 6);
    num_button_group_->addButton(pb7, 7);
    num_button_group_->addButton(pb8, 8);
    num_button_group_->addButton(pb9, 9);
    num_button_group_->addButton(pbA, 0xA);
    num_button_group_->addButton(pbB, 0xB);
    num_button_group_->addButton(pbC, 0xC);
    num_button_group_->addButton(pbD, 0xD);
    num_button_group_->addButton(pbE, 0xE);
    num_button_group_->addButton(pbF, 0xF);
    connect(this, &KCalculator::switchShowAccels, pb0, &KCalcButton::slotSetAccelDisplayMode);
    connect(this, &KCalculator::switchShowAccels, pb1, &KCalcButton::slotSetAccelDisplayMode);
    connect(this, &KCalculator::switchShowAccels, pb2, &KCalcButton::slotSetAccelDisplayMode);
    connect(this, &KCalculator::switchShowAccels, pb3, &KCalcButton::slotSetAccelDisplayMode);
    connect(this, &KCalculator::switchShowAccels, pb4, &KCalcButton::slotSetAccelDisplayMode);
    connect(this, &KCalculator::switchShowAccels, pb5, &KCalcButton::slotSetAccelDisplayMode);
    connect(this, &KCalculator::switchShowAccels, pb6, &KCalcButton::slotSetAccelDisplayMode);
    connect(this, &KCalculator::switchShowAccels, pb7, &KCalcButton::slotSetAccelDisplayMode);
    connect(this, &KCalculator::switchShowAccels, pb8, &KCalcButton::slotSetAccelDisplayMode);
    connect(this, &KCalculator::switchShowAccels, pb9, &KCalcButton::slotSetAccelDisplayMode);
    connect(this, &KCalculator::switchShowAccels, pbA, &KCalcButton::slotSetAccelDisplayMode);
    connect(this, &KCalculator::switchShowAccels, pbB, &KCalcButton::slotSetAccelDisplayMode);
    connect(this, &KCalculator::switchShowAccels, pbC, &KCalcButton::slotSetAccelDisplayMode);
    connect(this, &KCalculator::switchShowAccels, pbD, &KCalcButton::slotSetAccelDisplayMode);
    connect(this, &KCalculator::switchShowAccels, pbE, &KCalcButton::slotSetAccelDisplayMode);
    connect(this, &KCalculator::switchShowAccels, pbF, &KCalcButton::slotSetAccelDisplayMode);
}

//------------------------------------------------------------------------------
// Name: setupRightKeypad
// Desc: sets up right keypad keys and related shortcuts
//------------------------------------------------------------------------------
void KCalculator::setupRightKeypad() {

	connect(pbShift, &KCalcButton::toggled, this, &KCalculator::slotShifttoggled);
	connect(this, &KCalculator::switchShowAccels, pbShift, &KCalcButton::slotSetAccelDisplayMode);

	pbBackspace->setShortcut(QKeySequence(Qt::Key_Backspace));
	new QShortcut(Qt::Key_PageUp, pbBackspace, SLOT(animateClick()));
	connect(pbBackspace, &KCalcButton::clicked, this, &KCalculator::slotBackspaceclicked);
	connect(this, &KCalculator::switchShowAccels, pbBackspace, &KCalcButton::slotSetAccelDisplayMode);

	pbClear->setShortcut(QKeySequence(Qt::Key_Escape));
	new QShortcut(Qt::Key_PageUp, pbClear, SLOT(animateClick()));
	connect(pbClear, &KCalcButton::clicked, this, &KCalculator::slotClearclicked);
	connect(this, &KCalculator::switchShowAccels, pbClear, &KCalcButton::slotSetAccelDisplayMode);

	pbAllClear->setShortcut(QKeySequence(Qt::Key_Delete));
	new QShortcut(Qt::Key_PageDown, pbAllClear, SLOT(animateClick()));
	connect(pbAllClear, &KCalcButton::clicked, this, &KCalculator::slotAllClearclicked);
	connect(this, &KCalculator::switchShowAccels, pbAllClear, &KCalcButton::slotSetAccelDisplayMode);

	pbParenOpen->setShortcut(QKeySequence(Qt::Key_ParenLeft));
	connect(pbParenOpen, &KCalcButton::clicked, this, &KCalculator::slotParenOpenclicked);
	connect(this, &KCalculator::switchShowAccels, pbParenOpen, &KCalcButton::slotSetAccelDisplayMode);

	pbParenClose->setShortcut(QKeySequence(Qt::Key_ParenRight));
	connect(pbParenClose, &KCalcButton::clicked, this, &KCalculator::slotParenCloseclicked);
	connect(this, &KCalculator::switchShowAccels, pbParenClose, &KCalcButton::slotSetAccelDisplayMode);

	pbMemRecall->setDisabled(true);   // nothing in memory at start
	connect(pbMemRecall, &KCalcButton::clicked, this, &KCalculator::slotMemRecallclicked);
	connect(this, &KCalculator::switchShowAccels, pbMemRecall, &KCalcButton::slotSetAccelDisplayMode);

	connect(pbMemClear, &KCalcButton::clicked, this, &KCalculator::slotMemClearclicked);
	connect(this, &KCalculator::switchShowAccels, pbMemClear, &KCalcButton::slotSetAccelDisplayMode);

	pbMemPlusMinus->addMode(ModeNormal, i18nc("Add display to memory", "M+"), i18n("Add display to memory"));
	pbMemPlusMinus->addMode(ModeShift, i18nc("Subtract from memory", "M\xe2\x88\x92"), i18n("Subtract from memory"));
	connect(pbMemPlusMinus, &KCalcButton::clicked, this, &KCalculator::slotMemPlusMinusclicked);
	connect(this, &KCalculator::switchShowAccels, pbMemPlusMinus, &KCalcButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbMemPlusMinus, &KCalcButton::slotSetMode);

	connect(pbMemStore, &KCalcButton::clicked, this, &KCalculator::slotMemStoreclicked);
	connect(this, &KCalculator::switchShowAccels, pbMemStore, &KCalcButton::slotSetAccelDisplayMode);

	pbPercent->setShortcut(QKeySequence(Qt::Key_Percent));
	connect(pbPercent, &KCalcButton::clicked, this, &KCalculator::slotPercentclicked);
	connect(this, &KCalculator::switchShowAccels, pbPercent, &KCalcButton::slotSetAccelDisplayMode);

	pbPlusMinus->setShortcut(QKeySequence(Qt::Key_Backslash));
	connect(pbPlusMinus, &KCalcButton::clicked, this, &KCalculator::slotPlusMinusclicked);
	connect(this, &KCalculator::switchShowAccels, pbPlusMinus, &KCalcButton::slotSetAccelDisplayMode);
}

//------------------------------------------------------------------------------
// Name: setupNumericKeypad
// Desc: sets up numeric keys and related shortcuts
//------------------------------------------------------------------------------
void KCalculator::setupNumericKeypad() {

	pbCube->addMode(ModeNormal, i18nc("Third power", "x<sup>3</sup>"), i18n("Third power"));
	pbCube->addMode(ModeShift, QLatin1String("<sup>3</sup>&radic;x"), i18n("Cube root"));
	connect(pbCube, &KCalcButton::clicked, this, &KCalculator::slotCubeclicked);
	connect(this, &KCalculator::switchShowAccels, pbCube, &KCalcButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbCube, &KCalcButton::slotSetMode);

	pbDivision->setShortcut(QKeySequence(Qt::Key_Slash));
	new QShortcut(Qt::Key_division, pbDivision, SLOT(animateClick()));
	connect(pbDivision, &KCalcButton::clicked, this, &KCalculator::slotDivisionclicked);
	connect(this, &KCalculator::switchShowAccels, pbDivision, &KCalcButton::slotSetAccelDisplayMode);

	pbMultiplication->setShortcut(QKeySequence(Qt::Key_Asterisk));
	new QShortcut(Qt::Key_X, pbMultiplication, SLOT(animateClick()));
	new QShortcut(Qt::Key_multiply, pbMultiplication, SLOT(animateClick()));
	connect(pbMultiplication, &KCalcButton::clicked, this, &KCalculator::slotMultiplicationclicked);
	connect(this, &KCalculator::switchShowAccels, pbMultiplication, &KCalcButton::slotSetAccelDisplayMode);

	pbMinus->setShortcut(QKeySequence(Qt::Key_Minus));
	connect(pbMinus, &KCalcButton::clicked, this, &KCalculator::slotMinusclicked);
	connect(this, &KCalculator::switchShowAccels, pbMinus, &KCalcButton::slotSetAccelDisplayMode);

	pbPlus->setShortcut(QKeySequence(Qt::Key_Plus));
	connect(pbPlus, &KCalcButton::clicked, this, &KCalculator::slotPlusclicked);
	connect(this, &KCalculator::switchShowAccels, pbPlus, &KCalcButton::slotSetAccelDisplayMode);

    pbPeriod->setText(QString(QLocale().decimalPoint()));
    pbPeriod->setShortcut(QString(QLocale().decimalPoint()));

	// TODO: is this needed? the above line look slike it should do the right thing?
	/*
	if (KGlobal::locale()->decimalSymbol() == QLatin1String(".")) {
		new QShortcut(Qt::Key_Comma, pbPeriod, SLOT(animateClick()));
	} else if (KGlobal::locale()->decimalSymbol() == QLatin1String(",")) {
		new QShortcut(Qt::Key_Period, pbPeriod, SLOT(animateClick()));
	}
	*/

	connect(pbPeriod, &KCalcButton::clicked, this, &KCalculator::slotPeriodclicked);
	connect(this, &KCalculator::switchShowAccels, pbPeriod, &KCalcButton::slotSetAccelDisplayMode);

	pbEqual->setShortcut(QKeySequence(Qt::Key_Enter));
	new QShortcut(Qt::Key_Equal, pbEqual, SLOT(animateClick()));
	new QShortcut(Qt::Key_Return, pbEqual, SLOT(animateClick()));
	connect(pbEqual, &KCalcButton::clicked, this, &KCalculator::slotEqualclicked);
	connect(this, &KCalculator::switchShowAccels, pbEqual, &KCalcButton::slotSetAccelDisplayMode);
}

//------------------------------------------------------------------------------
// Name: setupLogicKeys
// Desc: sets up logic keys and related shortcuts
//------------------------------------------------------------------------------
void KCalculator::setupLogicKeys() {

	logic_buttons_.append(pbAND);
	logic_buttons_.append(pbOR);
	logic_buttons_.append(pbXOR);
	logic_buttons_.append(pbLsh);
	logic_buttons_.append(pbRsh);
	logic_buttons_.append(pbCmp);

	pbAND->setShortcut(QKeySequence(Qt::Key_Ampersand));
	connect(this, &KCalculator::switchShowAccels, pbAND, &KCalcButton::slotSetAccelDisplayMode);
	connect(pbAND, &KCalcButton::clicked, this, &KCalculator::slotANDclicked);

	pbOR->setShortcut(QKeySequence(Qt::Key_Bar));
	connect(this, &KCalculator::switchShowAccels, pbOR, &KCalcButton::slotSetAccelDisplayMode);
	connect(pbOR, &KCalcButton::clicked, this, &KCalculator::slotORclicked);

	connect(this, &KCalculator::switchShowAccels, pbXOR, &KCalcButton::slotSetAccelDisplayMode);
	connect(pbXOR, &KCalcButton::clicked, this, &KCalculator::slotXORclicked);

	pbLsh->setShortcut(QKeySequence(Qt::Key_Less));
	connect(this, &KCalculator::switchShowAccels, pbLsh, &KCalcButton::slotSetAccelDisplayMode);
	connect(pbLsh, &KCalcButton::clicked, this, &KCalculator::slotLeftShiftclicked);

	pbRsh->setShortcut(QKeySequence(Qt::Key_Greater));
	connect(this, &KCalculator::switchShowAccels, pbRsh, &KCalcButton::slotSetAccelDisplayMode);
	connect(pbRsh, &KCalcButton::clicked, this, &KCalculator::slotRightShiftclicked);

	pbCmp->setShortcut(QKeySequence(Qt::Key_AsciiTilde));
	connect(this, &KCalculator::switchShowAccels, pbCmp, &KCalcButton::slotSetAccelDisplayMode);
	connect(pbCmp, &KCalcButton::clicked, this, &KCalculator::slotNegateclicked);
}

//------------------------------------------------------------------------------
// Name: setupLogicKeys
// Desc: sets up scientific keys and related shortcuts
//------------------------------------------------------------------------------
void KCalculator::setupScientificKeys() {

	scientific_buttons_.append(pbHyp);
	scientific_buttons_.append(pbSin);
	scientific_buttons_.append(pbCos);
	scientific_buttons_.append(pbTan);
	scientific_buttons_.append(pbLog);
	scientific_buttons_.append(pbLn);

	connect(this, &KCalculator::switchShowAccels, pbHyp, &KCalcButton::slotSetAccelDisplayMode);
	connect(pbHyp, &KCalcButton::toggled, this, &KCalculator::slotHyptoggled);

	pbSin->addMode(ModeNormal, i18nc("Sine", "Sin"), i18n("Sine"));
	pbSin->addMode(ModeShift, i18nc("Arc sine", "Asin"), i18n("Arc sine"));
	pbSin->addMode(ModeHyperbolic, i18nc("Hyperbolic sine", "Sinh"), i18n("Hyperbolic sine"));
	pbSin->addMode(ButtonModeFlags(ModeShift | ModeHyperbolic), i18nc("Inverse hyperbolic sine", "Asinh"), i18n("Inverse hyperbolic sine"));
	connect(this, &KCalculator::switchShowAccels, pbSin, &KCalcButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbSin, &KCalcButton::slotSetMode);
	connect(pbSin, &KCalcButton::clicked, this, &KCalculator::slotSinclicked);

	pbCos->addMode(ModeNormal, i18nc("Cosine", "Cos"), i18n("Cosine"));
	pbCos->addMode(ModeShift, i18nc("Arc cosine", "Acos"), i18n("Arc cosine"));
	pbCos->addMode(ModeHyperbolic, i18nc("Hyperbolic cosine", "Cosh"), i18n("Hyperbolic cosine"));
	pbCos->addMode(ButtonModeFlags(ModeShift | ModeHyperbolic), i18nc("Inverse hyperbolic cosine", "Acosh"), i18n("Inverse hyperbolic cosine"));
	connect(this, &KCalculator::switchShowAccels, pbCos, &KCalcButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbCos, &KCalcButton::slotSetMode);
	connect(pbCos, &KCalcButton::clicked, this, &KCalculator::slotCosclicked);

	pbTan->addMode(ModeNormal, i18nc("Tangent", "Tan"), i18n("Tangent"));
	pbTan->addMode(ModeShift, i18nc("Arc tangent", "Atan"), i18n("Arc tangent"));
	pbTan->addMode(ModeHyperbolic, i18nc("Hyperbolic tangent", "Tanh"), i18n("Hyperbolic tangent"));
	pbTan->addMode(ButtonModeFlags(ModeShift | ModeHyperbolic), i18nc("Inverse hyperbolic tangent", "Atanh"), i18n("Inverse hyperbolic tangent"));
	connect(this, &KCalculator::switchShowAccels, pbTan, &KCalcButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbTan, &KCalcButton::slotSetMode);
	connect(pbTan, &KCalcButton::clicked, this, &KCalculator::slotTanclicked);

	pbLog->addMode(ModeNormal, i18nc("Logarithm to base 10", "Log"), i18n("Logarithm to base 10"));
	pbLog->addMode(ModeShift, i18nc("10 to the power of x", "10<sup>x</sup>"), i18n("10 to the power of x"));
	connect(this, &KCalculator::switchShowAccels, pbLog, &KCalcButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbLog, &KCalcButton::slotSetMode);
	connect(pbLog, &KCalcButton::clicked, this, &KCalculator::slotLogclicked);
	pbLn->addMode(ModeNormal, i18nc("Natural log", "Ln"), i18n("Natural log"));
	pbLn->addMode(ModeShift, i18nc("Exponential function", "e<sup>x</sup>"), i18n("Exponential function"));
	connect(this, &KCalculator::switchShowAccels, pbLn, &KCalcButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbLn, &KCalcButton::slotSetMode);
	connect(pbLn, &KCalcButton::clicked, this, &KCalculator::slotLnclicked);
}

//------------------------------------------------------------------------------
// Name: setupStatisticKeys
// Desc: sets up statistical keys and related shortcuts
//------------------------------------------------------------------------------
void KCalculator::setupStatisticKeys() {

	stat_buttons_.append(pbNData);
	stat_buttons_.append(pbMean);
	stat_buttons_.append(pbSd);
	stat_buttons_.append(pbMed);
	stat_buttons_.append(pbDat);
	stat_buttons_.append(pbCSt);

	pbNData->addMode(ModeNormal, i18nc("Number of data entered", "N"), i18n("Number of data entered"));
	pbNData->addMode(ModeShift, QString::fromUtf8("\xce\xa3") + QLatin1Char('x'), i18n("Sum of all data items"));
	connect(this, &KCalculator::switchShowAccels, pbNData, &KCalcButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbNData, &KCalcButton::slotSetMode);
	connect(pbNData, &KCalcButton::clicked, this, &KCalculator::slotStatNumclicked);

	pbMean->addMode(ModeNormal, i18nc("Mean", "Mea"), i18n("Mean"));
	pbMean->addMode(ModeShift, QString::fromUtf8("\xce\xa3") + QLatin1String("x<sup>2</sup>"), i18n("Sum of all data items squared"));
	connect(this, &KCalculator::switchShowAccels, pbMean, &KCalcButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbMean, &KCalcButton::slotSetMode);
	connect(pbMean, &KCalcButton::clicked, this, &KCalculator::slotStatMeanclicked);

	pbSd->addMode(ModeNormal, QString::fromUtf8("\xcf\x83") + QLatin1String("<sub>N</sub>"), i18n("Standard deviation"));
	pbSd->addMode(ModeShift, QString::fromUtf8("\xcf\x83") + QLatin1String("<sub>N-1</sub>"), i18n("Sample standard deviation"));
	connect(this, &KCalculator::switchShowAccels, pbSd, &KCalcButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbSd, &KCalcButton::slotSetMode);
	connect(pbSd, &KCalcButton::clicked, this, &KCalculator::slotStatStdDevclicked);

	connect(this, &KCalculator::switchShowAccels, pbMed, &KCalcButton::slotSetAccelDisplayMode);
	connect(pbMed, &KCalcButton::clicked, this, &KCalculator::slotStatMedianclicked);

	pbDat->addMode(ModeNormal, i18nc("Enter data", "Dat"), i18n("Enter data"));
	pbDat->addMode(ModeShift, i18nc("Delete last data item", "CDat"), i18n("Delete last data item"));
	connect(this, &KCalculator::switchShowAccels, pbDat, &KCalcButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbDat, &KCalcButton::slotSetMode);
	connect(pbDat, &KCalcButton::clicked, this, &KCalculator::slotStatDataInputclicked);

	connect(this, &KCalculator::switchShowAccels, pbCSt, &KCalcButton::slotSetAccelDisplayMode);
	connect(pbCSt, &KCalcButton::clicked, this, &KCalculator::slotStatClearDataclicked);
}

//------------------------------------------------------------------------------
// Name: setupConstantsKeys
// Desc: sets up constants keys and related shortcuts
//------------------------------------------------------------------------------
void KCalculator::setupConstantsKeys() {

	const_buttons_.append(pbC1);
	const_buttons_.append(pbC2);
	const_buttons_.append(pbC3);
	const_buttons_.append(pbC4);
	const_buttons_.append(pbC5);
	const_buttons_.append(pbC6);

	pbC1->setButtonNumber(0);
	connect(this, &KCalculator::switchShowAccels, pbC1, &KCalcConstButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbC1, &KCalcConstButton::slotSetMode);
	connect(pbC1, &KCalcConstButton::clicked, this, &KCalculator::slotConstclicked);

	pbC2->setButtonNumber(1);
	connect(this, &KCalculator::switchShowAccels, pbC2, &KCalcConstButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbC2, &KCalcConstButton::slotSetMode);
	connect(pbC2, &KCalcConstButton::clicked, this, &KCalculator::slotConstclicked);

	pbC3->setButtonNumber(2);
	connect(this, &KCalculator::switchShowAccels, pbC3, &KCalcConstButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbC3, &KCalcConstButton::slotSetMode);
	connect(pbC3, &KCalcConstButton::clicked, this, &KCalculator::slotConstclicked);

	pbC4->setButtonNumber(3);
	connect(this, &KCalculator::switchShowAccels, pbC4, &KCalcConstButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbC4, &KCalcConstButton::slotSetMode);
	connect(pbC4, &KCalcConstButton::clicked, this, &KCalculator::slotConstclicked);

	pbC5->setButtonNumber(4);
	connect(this, &KCalculator::switchShowAccels, pbC5, &KCalcConstButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbC5, &KCalcConstButton::slotSetMode);
	connect(pbC5, &KCalcConstButton::clicked, this, &KCalculator::slotConstclicked);

	pbC6->setButtonNumber(5);
	connect(this, &KCalculator::switchShowAccels, pbC6, &KCalcConstButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbC6, &KCalcConstButton::slotSetMode);
	connect(pbC6, &KCalcConstButton::clicked, this, &KCalculator::slotConstclicked);

	changeButtonNames();
}

//------------------------------------------------------------------------------
// Name: setupMiscKeys
// Desc: sets up misc keys and related shortcuts
//------------------------------------------------------------------------------
void KCalculator::setupMiscKeys() {

	pbMod->addMode(ModeNormal, i18nc("Modulo", "Mod"), i18n("Modulo"));
	pbMod->addMode(ModeShift, i18nc("Integer division", "IntDiv"), i18n("Integer division"));
	connect(this, &KCalculator::switchMode, pbMod, &KCalcButton::slotSetMode);
	connect(this, &KCalculator::switchShowAccels, pbMod, &KCalcButton::slotSetAccelDisplayMode);
	pbMod->setShortcut(QKeySequence(Qt::Key_Colon));
	connect(pbMod, &KCalcButton::clicked, this, &KCalculator::slotModclicked);

	pbReci->addMode(ModeNormal, i18nc("Reciprocal", "1/x"), i18n("Reciprocal"));
	pbReci->addMode(ModeShift, i18nc("n Choose m", "nCm"), i18n("n Choose m"));
	connect(this, &KCalculator::switchMode, pbReci, &KCalcButton::slotSetMode);
	connect(this, &KCalculator::switchShowAccels, pbReci, &KCalcButton::slotSetAccelDisplayMode);
	connect(pbReci, &KCalcButton::clicked, this, &KCalculator::slotReciclicked);

	pbFactorial->addMode(ModeNormal, i18nc("Factorial", "x!"), i18n("Factorial"));
	pbFactorial->addMode(ModeShift, QLatin1String("&#915;"), i18n("Gamma"));
	pbFactorial->setShortcut(QKeySequence(Qt::Key_Exclam));
	connect(this, &KCalculator::switchShowAccels, pbFactorial, &KCalcButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbFactorial, &KCalcButton::slotSetMode);
	connect(pbFactorial, &KCalcButton::clicked, this, &KCalculator::slotFactorialclicked);

	pbSquare->addMode(ModeNormal, i18nc("Square", "x<sup>2</sup>"), i18n("Square"));
	pbSquare->addMode(ModeShift, QLatin1String("&radic;x"), i18n("Square root"));
	pbSquare->setShortcut(QKeySequence(Qt::Key_BracketLeft));
	new QShortcut(Qt::Key_twosuperior, pbSquare, SLOT(animateClick()));
	connect(this, &KCalculator::switchShowAccels, pbSquare, &KCalcButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbSquare, &KCalcButton::slotSetMode);
	connect(pbSquare, &KCalcButton::clicked, this, &KCalculator::slotSquareclicked);

	pbPower->addMode(ModeNormal, i18nc("x to the power of y", "x<sup>y</sup>"), i18n("x to the power of y"));
	pbPower->addMode(ModeShift, i18nc("x to the power of 1/y", "x<sup>1/y</sup>"), i18n("x to the power of 1/y"));
	connect(this, &KCalculator::switchShowAccels, pbPower, &KCalcButton::slotSetAccelDisplayMode);
	connect(this, &KCalculator::switchMode, pbPower, &KCalcButton::slotSetMode);
	pbPower->setShortcut(QKeySequence(Qt::Key_AsciiCircum));
	connect(pbPower, &KCalcButton::clicked, this, &KCalculator::slotPowerclicked);

	pbEE->addMode(ModeNormal, QLatin1String("x<small>" "\xb7" "10</small><sup>y</sup>"), i18n("Exponent"));
	connect(this, &KCalculator::switchShowAccels, pbEE, &KCalcButton::slotSetAccelDisplayMode);
	connect(pbEE, &KCalcButton::clicked, this, &KCalculator::slotEEclicked);
}

//------------------------------------------------------------------------------
// Name: createConstantsMenu
// Desc: additional setup for button keys
// NOTE: all alphanumeric shorts set in ui file
//------------------------------------------------------------------------------
void KCalculator::setupKeys() {

	setupNumberKeys();
	setupRightKeypad();
	setupNumericKeypad();
	setupLogicKeys();
	setupScientificKeys();
	setupStatisticKeys();
	setupConstantsKeys();
	setupMiscKeys();

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
	function_button_list_.append(pbCube);

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
}

//------------------------------------------------------------------------------
// Name: updateGeometry
// Desc: makes all the buttons have reasonable sizes
//------------------------------------------------------------------------------
void KCalculator::updateGeometry() {

	const QSize em = pbAND->fontMetrics().size(0, QLatin1String("M"));
	int margin = QApplication::style()->pixelMetric(QStyle::PM_ButtonMargin, 0, 0);
	margin = qMax(qMin(margin / 2, 3), 3);

	// left pad
	foreach(QObject *obj, leftPad->children()) {
		if (KCalcButton *const button = qobject_cast<KCalcButton *>(obj)) {
			button->setFixedWidth(em.width() * 4 + margin * 2);
			button->installEventFilter(this);
		}
	}

	// right pad
	foreach(QObject *obj, rightPad->children()) {
		KCalcButton *const button = qobject_cast<KCalcButton *>(obj);
		// let Shift expand freely
		if (button && button != pbShift) {
			button->setFixedWidth(em.width() * 3 + margin * 2);
			button->installEventFilter(this);
		}
	}

	// numeric pad
	foreach(QObject *obj, numericPad->children()) {
		if (KCalcButton *const button = qobject_cast<KCalcButton *>(obj)) {
			// let pb0 expand freely
			if (button != pb0) {
				button->setFixedWidth(em.width() * 3 + margin * 2);
			}
			button->installEventFilter(this);
		}
	}
}

//------------------------------------------------------------------------------
// Name: slotConstantToDisplay
// Desc: inserts a constant
//------------------------------------------------------------------------------
void KCalculator::slotConstantToDisplay(const science_constant &const_chosen) {

	QString val = const_chosen.value;
	val.replace(QLatin1Char('.'), KNumber::decimalSeparator());
	calc_display->setAmount(KNumber(val));
    updateDisplay(0);
}

//------------------------------------------------------------------------------
// Name: slotBaseSelected
// Desc: changes the selected numeric base
//------------------------------------------------------------------------------
void KCalculator::slotBaseSelected(int base) {

	int current_base;

	// set display & statusbar (if item exist in statusbar)
	statusBar()->setBase(base);
	switch (base) {
	case BinMode:
		current_base = calc_display->setBase(NumBase(2));
		calc_display->setStatusText(BaseField, QLatin1String("Bin"));
		break;
	case OctMode:
		current_base = calc_display->setBase(NumBase(8));
		calc_display->setStatusText(BaseField, QLatin1String("Oct"));
		break;
	case DecMode:
		current_base = calc_display->setBase(NumBase(10));
		calc_display->setStatusText(BaseField, QLatin1String("Dec"));
		break;
	case HexMode:
		current_base = calc_display->setBase(NumBase(16));
		calc_display->setStatusText(BaseField, QLatin1String("Hex"));
		break;
	default:
		calc_display->setStatusText(BaseField, QLatin1String("Error"));
		return;
	}

	// Enable the buttons available in this base
	for (int i = 0; i < current_base; ++i) {
		(num_button_group_->buttons()[i])->setEnabled(true);
	}

	// Disable the buttons not available in this base
	for (int i = current_base; i < 16; ++i) {
		(num_button_group_->buttons()[i])->setEnabled(false);
	}

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

//------------------------------------------------------------------------------
// Name: keyPressEvent
// Desc: handles keypress events
//------------------------------------------------------------------------------
void KCalculator::keyPressEvent(QKeyEvent *e) {

	// Fix for bug #314586
	// Basically, on some keyboards such as French, even though the decimal separator
	// is "," the numeric keypad has a "." key. So we fake it so people can more seemlessly
	// enter numbers using the keypad
	if(KNumber::decimalSeparator() != ".") {
		if(e->key() == Qt::Key_Period && e->modifiers() & Qt::KeypadModifier) {
			pbPeriod->animateClick();
		}
	}


	if (((e->modifiers() & Qt::NoModifier) == 0) || (e->modifiers() & Qt::ShiftModifier)) {
		switch (e->key()) {
		case Qt::Key_Backspace:
			calc_display->deleteLastDigit();
			break;
		}
	}

	if (e->key() == Qt::Key_Control) {
		emit switchShowAccels(true);
	}
}

//------------------------------------------------------------------------------
// Name: keyReleaseEvent
// Desc: handles keyrelease events
//------------------------------------------------------------------------------
void KCalculator::keyReleaseEvent(QKeyEvent *e) {

	if (e->key() == Qt::Key_Control) {
		emit switchShowAccels(false);
	}
}

//------------------------------------------------------------------------------
// Name: slotAngleSelected
// Desc: changes the selected angle system
//------------------------------------------------------------------------------
void KCalculator::slotAngleSelected(int mode) {

	angle_mode_ = mode;

	statusBar()->setAngleMode(KCalcStatusBar::AngleMode(mode));
	switch (mode) {
	case DegMode:
		calc_display->setStatusText(AngleField, QLatin1String("Deg"));
		break;
	case RadMode:
		calc_display->setStatusText(AngleField, QLatin1String("Rad"));
		break;
	case GradMode:
		calc_display->setStatusText(AngleField, QLatin1String("Gra"));
		break;
	default: // we shouldn't ever end up here
		angle_mode_ = RadMode;
	}

	KCalcSettings::setAngleMode(angle_mode_);
}

//------------------------------------------------------------------------------
// Name: slotEEclicked
// Desc: starts the entering of numers using scientific notation
//------------------------------------------------------------------------------
void KCalculator::slotEEclicked() {
    calc_display->newCharacter(QLatin1Char('e'));
}

//------------------------------------------------------------------------------
// Name: slotShifttoggled
// Desc: updates the shift state for alternate button functionality
//------------------------------------------------------------------------------
void KCalculator::slotShifttoggled(bool flag) {

	shift_mode_ = flag;

	emit switchMode(ModeShift, flag);

	statusBar()->setShiftIndicator(shift_mode_);
	if (shift_mode_) {
		calc_display->setStatusText(ShiftField, i18n("Shift"));
	} else {
		calc_display->setStatusText(ShiftField, QString());
	}
}

//------------------------------------------------------------------------------
// Name: slotHyptoggled
// Desc: updates the Hyp state for alternate trig button functionality
//------------------------------------------------------------------------------
void KCalculator::slotHyptoggled(bool flag) {

	// toggle between hyperbolic and standart trig functions
	hyp_mode_ = flag;

	emit switchMode(ModeHyperbolic, flag);
}

//------------------------------------------------------------------------------
// Name: slotMemRecallclicked
// Desc: recalls a value from memory
//------------------------------------------------------------------------------
void KCalculator::slotMemRecallclicked() {

	// temp. work-around
	calc_display->sendEvent(KCalcDisplay::EventReset);

	calc_display->setAmount(memory_num_);
	updateDisplay(0);
}

//------------------------------------------------------------------------------
// Name: slotMemStoreclicked
// Desc: stores a value into memory
//------------------------------------------------------------------------------
void KCalculator::slotMemStoreclicked() {

	EnterEqual();

	memory_num_ = calc_display->getAmount();
	calc_display->setStatusText(MemField, QLatin1String("M"));
	statusBar()->setMemoryIndicator(true);
	pbMemRecall->setEnabled(true);
}

//------------------------------------------------------------------------------
// Name: slotNumberclicked
// Desc: user has entered a digit
//------------------------------------------------------------------------------
void KCalculator::slotNumberclicked(int number_clicked) {

    calc_display->enterDigit(number_clicked);
}

//------------------------------------------------------------------------------
// Name: slotSinclicked
// Desc: executes the sine function
//------------------------------------------------------------------------------
void KCalculator::slotSinclicked() {

	if (hyp_mode_) {
		// sinh or arsinh
		if (!shift_mode_) {
			core.SinHyp(calc_display->getAmount());
		} else {
			core.AreaSinHyp(calc_display->getAmount());
		}
	} else {
		// sine or arcsine
		if (!shift_mode_) {
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
		} else {
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
	}

	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotPlusMinusclicked
// Desc: changes sign of number being displayed
//------------------------------------------------------------------------------
void KCalculator::slotPlusMinusclicked() {

	// display can only change sign, when in input mode, otherwise we
	// need the core to do this.
	if (!calc_display->sendEvent(KCalcDisplay::EventChangeSign)) {
		core.InvertSign(calc_display->getAmount());
		updateDisplay(UPDATE_FROM_CORE);
	}
}

//------------------------------------------------------------------------------
// Name: slotMemPlusMinusclicked
// Desc: handles arithmetic on values stored in memory
//------------------------------------------------------------------------------
void KCalculator::slotMemPlusMinusclicked() {

	bool tmp_shift_mode = shift_mode_; // store this, because next command deletes shift_mode_
	EnterEqual(); // finish calculation so far, to store result into MEM

	if (!tmp_shift_mode) {
		memory_num_ += calc_display->getAmount();
	} else {
		memory_num_ -= calc_display->getAmount();
	}

	pbShift->setChecked(false);
	statusBar()->setMemoryIndicator(true);
	calc_display->setStatusText(MemField, i18n("M"));
	pbMemRecall->setEnabled(true);
}

//------------------------------------------------------------------------------
// Name: slotSinclicked
// Desc: executes the cosine function
//------------------------------------------------------------------------------
void KCalculator::slotCosclicked() {

	if (hyp_mode_) {
		// cosh or arcosh
		if (!shift_mode_) {
			core.CosHyp(calc_display->getAmount());
		} else {
			core.AreaCosHyp(calc_display->getAmount());
		}
	} else {
		// cosine or arccosine
		if (!shift_mode_) {
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
		} else {
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
	}

	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotSinclicked
// Desc: executes the recipricol function
//------------------------------------------------------------------------------
void KCalculator::slotReciclicked() {

	if (shift_mode_) {
		core.enterOperation(calc_display->getAmount(), CalcEngine::FUNC_BINOM);
	} else {
		core.Reciprocal(calc_display->getAmount());
		updateDisplay(UPDATE_FROM_CORE);
		return;
	}

	// temp. work-around
	KNumber tmp_num = calc_display->getAmount();
	calc_display->sendEvent(KCalcDisplay::EventReset);
	calc_display->setAmount(tmp_num);
	updateDisplay(0);
}

//------------------------------------------------------------------------------
// Name: slotSinclicked
// Desc: executes the tangent function
//------------------------------------------------------------------------------
void KCalculator::slotTanclicked() {

	if (hyp_mode_) {
		// tanh or artanh
		if (!shift_mode_) {
			core.TangensHyp(calc_display->getAmount());
		} else {
			core.AreaTangensHyp(calc_display->getAmount());
		}
	} else {
		// tan or arctan
		if (!shift_mode_) {
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
		} else {
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
	}

	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotFactorialclicked
// Desc: executes the factorial function
//------------------------------------------------------------------------------
void KCalculator::slotFactorialclicked() {

    // Set WaitCursor, as this operation may take looooong
    // time and UI frezes with large numbers. User needs some
    // visual feedback.
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	if (!shift_mode_) {
	    core.Factorial(calc_display->getAmount());
	} else {
		core.Gamma(calc_display->getAmount());
	}
    QApplication::restoreOverrideCursor();
    updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotLogclicked
// Desc: executes the Log function
//------------------------------------------------------------------------------
void KCalculator::slotLogclicked() {

	if (!shift_mode_) {
		core.Log10(calc_display->getAmount());
	} else {
		core.Exp10(calc_display->getAmount());
	}

	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotSquareclicked
// Desc: executes the x^2 function
//------------------------------------------------------------------------------
void KCalculator::slotSquareclicked() {

	if (!shift_mode_) {
		core.Square(calc_display->getAmount());
	} else {
		core.SquareRoot(calc_display->getAmount());
	}

	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotCubeclicked
// Desc: executes the x^3 function
//------------------------------------------------------------------------------
void KCalculator::slotCubeclicked() {

	if (!shift_mode_) {
		core.Cube(calc_display->getAmount());
	} else {
		core.CubeRoot(calc_display->getAmount());
	}

	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotCubeclicked
// Desc: executes the ln function
//------------------------------------------------------------------------------
void KCalculator::slotLnclicked() {

	if (!shift_mode_) {
		core.Ln(calc_display->getAmount());
	} else {
		core.Exp(calc_display->getAmount());
	}

	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotPowerclicked
// Desc: executes the x^y function
//------------------------------------------------------------------------------
void KCalculator::slotPowerclicked() {

	if (shift_mode_) {
		core.enterOperation(calc_display->getAmount(), CalcEngine::FUNC_PWR_ROOT);
		pbShift->setChecked(false);
	} else {
		core.enterOperation(calc_display->getAmount(), CalcEngine::FUNC_POWER);
	}

	// temp. work-around
	KNumber tmp_num = calc_display->getAmount();
	calc_display->sendEvent(KCalcDisplay::EventReset);
	calc_display->setAmount(tmp_num);
	updateDisplay(0);
}

//------------------------------------------------------------------------------
// Name: slotMemClearclicked
// Desc: executes the MC function
//------------------------------------------------------------------------------
void KCalculator::slotMemClearclicked() {

	memory_num_ = KNumber::Zero;
	statusBar()->setMemoryIndicator(false);
	calc_display->setStatusText(MemField, QString());
	pbMemRecall->setDisabled(true);
}

//------------------------------------------------------------------------------
// Name: slotBackspaceclicked
// Desc: removes the last input digit
//------------------------------------------------------------------------------
void KCalculator::slotBackspaceclicked() {

    calc_display->deleteLastDigit();
}

//------------------------------------------------------------------------------
// Name: slotClearclicked
// Desc: clears the display
//------------------------------------------------------------------------------
void KCalculator::slotClearclicked() {

    calc_display->sendEvent(KCalcDisplay::EventClear);
}

//------------------------------------------------------------------------------
// Name: slotAllClearclicked
// Desc: clears everything
//------------------------------------------------------------------------------
void KCalculator::slotAllClearclicked() {

	core.Reset();
	calc_display->sendEvent(KCalcDisplay::EventReset);
	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotParenOpenclicked
// Desc: starts a sub-expression
//------------------------------------------------------------------------------
void KCalculator::slotParenOpenclicked() {

    core.ParenOpen(calc_display->getAmount());
}

//------------------------------------------------------------------------------
// Name: slotParenCloseclicked
// Desc: ends a sub-expression
//------------------------------------------------------------------------------
void KCalculator::slotParenCloseclicked() {

    core.ParenClose(calc_display->getAmount());
    updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotANDclicked
// Desc: executes a bitwise AND
//------------------------------------------------------------------------------
void KCalculator::slotANDclicked() {

	core.enterOperation(calc_display->getAmount(), CalcEngine::FUNC_AND);
	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotMultiplicationclicked
// Desc: executes multiplication
//------------------------------------------------------------------------------
void KCalculator::slotMultiplicationclicked() {

	core.enterOperation(calc_display->getAmount(), CalcEngine::FUNC_MULTIPLY);
	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotDivisionclicked
// Desc: executes division
//------------------------------------------------------------------------------
void KCalculator::slotDivisionclicked() {

    core.enterOperation(calc_display->getAmount(), CalcEngine::FUNC_DIVIDE);
    updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotORclicked
// Desc: executes a bitwise OR
//------------------------------------------------------------------------------
void KCalculator::slotORclicked() {

	core.enterOperation(calc_display->getAmount(), CalcEngine::FUNC_OR);
	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotXORclicked
// Desc: executes a bitwise XOR
//------------------------------------------------------------------------------
void KCalculator::slotXORclicked() {

	core.enterOperation(calc_display->getAmount(), CalcEngine::FUNC_XOR);
	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotPlusclicked
// Desc: executes addition
//------------------------------------------------------------------------------
void KCalculator::slotPlusclicked() {

	core.enterOperation(calc_display->getAmount(), CalcEngine::FUNC_ADD);
	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotPlusclicked
// Desc: executes subtraction
//------------------------------------------------------------------------------
void KCalculator::slotMinusclicked() {

    core.enterOperation(calc_display->getAmount(), CalcEngine::FUNC_SUBTRACT);
    updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotLeftShiftclicked
// Desc: executes a bitwise left shift
//------------------------------------------------------------------------------
void KCalculator::slotLeftShiftclicked() {

	core.enterOperation(calc_display->getAmount(), CalcEngine::FUNC_LSH);
	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotLeftShiftclicked
// Desc: executes a bitwise right shift
//------------------------------------------------------------------------------
void KCalculator::slotRightShiftclicked() {

    core.enterOperation(calc_display->getAmount(), CalcEngine::FUNC_RSH);
    updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotPeriodclicked
// Desc: enters a decimal into the input stream
//------------------------------------------------------------------------------
void KCalculator::slotPeriodclicked() {

	// i know this isn't locale friendly, should be converted to appropriate
	// value at lower levels
    calc_display->newCharacter(QLocale().decimalPoint());
}

//------------------------------------------------------------------------------
// Name: EnterEqual
// Desc: calculates and displays the result of the pending operations
//------------------------------------------------------------------------------
void KCalculator::EnterEqual() {

    core.enterOperation(calc_display->getAmount(), CalcEngine::FUNC_EQUAL);
    updateDisplay(UPDATE_FROM_CORE | UPDATE_STORE_RESULT);
}

//------------------------------------------------------------------------------
// Name: slotEqualclicked
// Desc: calculates and displays the result of the pending operations
//------------------------------------------------------------------------------
void KCalculator::slotEqualclicked() {

    EnterEqual();
}

//------------------------------------------------------------------------------
// Name: slotPercentclicked
// Desc: calculates and displays the result of the pending operations as a percent
//------------------------------------------------------------------------------
void KCalculator::slotPercentclicked() {

    core.enterOperation(calc_display->getAmount(), CalcEngine::FUNC_PERCENT);
    updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotNegateclicked
// Desc: executes a bitwise 2's compliment
// NOTE: implicitly converts the value to an unsigned quantity
//------------------------------------------------------------------------------
void KCalculator::slotNegateclicked() {

	core.Complement(calc_display->getAmount());
	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotModclicked
// Desc: executes modulous (remainder division)
//------------------------------------------------------------------------------
void KCalculator::slotModclicked(){

	if (shift_mode_) {
		core.enterOperation(calc_display->getAmount(), CalcEngine::FUNC_INTDIV);
	} else {
		core.enterOperation(calc_display->getAmount(), CalcEngine::FUNC_MOD);
	}

	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotStatNumclicked
// Desc: executes Sum function
//------------------------------------------------------------------------------
void KCalculator::slotStatNumclicked() {

	if (!shift_mode_) {
		core.StatCount(KNumber::Zero);
	} else {
		pbShift->setChecked(false);
		core.StatSum(KNumber::Zero);
	}

	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotStatMeanclicked
// Desc: executes Mean function
//------------------------------------------------------------------------------
void KCalculator::slotStatMeanclicked() {

	if (!shift_mode_) {
		core.StatMean(KNumber::Zero);
	} else {
		pbShift->setChecked(false);
		core.StatSumSquares(KNumber::Zero);
	}

	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotStatStdDevclicked
// Desc: executes STD function
//------------------------------------------------------------------------------
void KCalculator::slotStatStdDevclicked() {

	if (shift_mode_) {
		// std (n-1)
		core.StatStdDeviation(KNumber::Zero);
		pbShift->setChecked(false);
	} else {
		// std (n)
		core.StatStdSample(KNumber::Zero);
	}

	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotStatMedianclicked
// Desc: executes Median function
//------------------------------------------------------------------------------
void KCalculator::slotStatMedianclicked() {

	if (!shift_mode_) {
		// std (n-1)
		core.StatMedian(KNumber::Zero);
	} else {
		// std (n)
		core.StatMedian(KNumber::Zero);
		pbShift->setChecked(false);
	}

	// TODO: it seems two different modes should be implemented, but...?
	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotStatDataInputclicked
// Desc: enters a value for statistical functions
//------------------------------------------------------------------------------
void KCalculator::slotStatDataInputclicked() {

	if (!shift_mode_) {
		core.StatDataNew(calc_display->getAmount());
	} else {
		pbShift->setChecked(false);
		core.StatDataDel(KNumber::Zero);
		statusBar()->showMessage(i18n("Last stat item erased"), 3000);
	}

	updateDisplay(UPDATE_FROM_CORE);
}

//------------------------------------------------------------------------------
// Name: slotStatClearDataclicked
// Desc: clears memory for statical functions
//------------------------------------------------------------------------------
void KCalculator::slotStatClearDataclicked() {

	if (!shift_mode_) {
		core.StatClearAll(KNumber::Zero);
		statusBar()->showMessage(i18n("Stat mem cleared"), 3000);
	} else {
		pbShift->setChecked(false);
		updateDisplay(0);
	}
}

//------------------------------------------------------------------------------
// Name: slotConstclicked
// Desc: enters a constant
//------------------------------------------------------------------------------
void KCalculator::slotConstclicked(int button) {

	if(KCalcConstButton *btn = qobject_cast<KCalcConstButton*>(const_buttons_[button])) {
		if (!shift_mode_) {
			// set the display to the configured value of constant button
			// internally, we deal with C locale style numbers, we need to convert
			QString val = btn->constant();
			val.replace(QLatin1Char('.'), KNumber::decimalSeparator());
			calc_display->setAmount(KNumber(val));

		} else {
			pbShift->setChecked(false);

			// internally, we deal with C locale style numbers, we need to convert
			QString val = calc_display->text();
			val.replace(KNumber::decimalSeparator(), QLatin1String("."));
			KCalcSettings::setValueConstant(button, val);

			// below set new tooltip
			btn->setLabelAndTooltip();

			// work around: after storing a number, pressing a digit should start
			// a new number
			calc_display->setAmount(calc_display->getAmount());
		}

		updateDisplay(0);
	}
}

//------------------------------------------------------------------------------
// Name: showSettings
// Desc: opens the shows the settings dialog
//------------------------------------------------------------------------------
void KCalculator::showSettings() {

	// Check if there is already a dialog and if so bring
	// it to the foreground.
	if (KConfigDialog::showDialog(QLatin1String("settings"))) {
		return;
	}

	// Create a new dialog with the same name as the above checking code.
	KConfigDialog *const dialog = new KConfigDialog(this, QLatin1String("settings"), KCalcSettings::self());

	// general settings
	General *const general = new General(0);
	general->kcfg_Precision->setMaximum(maxprecision);
	dialog->addPage(general, i18n("General"), QLatin1String("accessories-calculator"), i18n("General Settings"));

	// font settings
	Fonts *const fonts = new Fonts(0);
	dialog->addPage(fonts, i18n("Font"), QLatin1String("preferences-desktop-font"), i18n("Select Display Font"));

	// color settings
	Colors *const color = new Colors(0);
	dialog->addPage(color, i18n("Colors"), QLatin1String("format-fill-color"), i18n("Button & Display Colors"));

	// constant settings
	if (!constants_) {
		constants_ = new Constants(0);
	}

	KCalcConstMenu *tmp_menu;
	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu, &KCalcConstMenu::triggeredConstant, this, &KCalculator::slotChooseScientificConst0);
	constants_->pushButton0->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu, &KCalcConstMenu::triggeredConstant, this, &KCalculator::slotChooseScientificConst1);
	constants_->pushButton1->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu, &KCalcConstMenu::triggeredConstant, this, &KCalculator::slotChooseScientificConst2);
	constants_->pushButton2->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu, &KCalcConstMenu::triggeredConstant, this, &KCalculator::slotChooseScientificConst3);
	constants_->pushButton3->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu, &KCalcConstMenu::triggeredConstant, this, &KCalculator::slotChooseScientificConst4);
	constants_->pushButton4->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu, &KCalcConstMenu::triggeredConstant, this, &KCalculator::slotChooseScientificConst5);
	constants_->pushButton5->setMenu(tmp_menu);

	dialog->addPage(constants_, i18n("Constants"), QLatin1String("preferences-kcalc-constants"), i18n("Define Constants"));

	// When the user clicks OK or Apply we want to update our settings.
	connect(dialog, &KConfigDialog::settingsChanged, this, &KCalculator::updateSettings);

	// Display the dialog.
	dialog->show();
}


// these 6 slots are just a quick hack, instead of setting the
// TextEdit fields in the configuration dialog, we are setting the
// Settingvalues themselves!!

//------------------------------------------------------------------------------
// Name: slotChooseScientificConst0
// Desc: updates constants value
//------------------------------------------------------------------------------
void KCalculator::slotChooseScientificConst0(const science_constant &chosen_const) {
	constants_->kcfg_valueConstant0->setText(chosen_const.value);
	constants_->kcfg_nameConstant0->setText(chosen_const.label);
}

//------------------------------------------------------------------------------
// Name: slotChooseScientificConst1
// Desc: updates constants value
//------------------------------------------------------------------------------
void KCalculator::slotChooseScientificConst1(const science_constant &chosen_const) {
	constants_->kcfg_valueConstant1->setText(chosen_const.value);
	constants_->kcfg_nameConstant1->setText(chosen_const.label);
}

//------------------------------------------------------------------------------
// Name: slotChooseScientificConst2
// Desc: updates constants value
//------------------------------------------------------------------------------
void KCalculator::slotChooseScientificConst2(const science_constant &chosen_const) {
	constants_->kcfg_valueConstant2->setText(chosen_const.value);
	constants_->kcfg_nameConstant2->setText(chosen_const.label);
}

//------------------------------------------------------------------------------
// Name: slotChooseScientificConst3
// Desc: updates constants value
//------------------------------------------------------------------------------
void KCalculator::slotChooseScientificConst3(const science_constant &chosen_const) {
	constants_->kcfg_valueConstant3->setText(chosen_const.value);
	constants_->kcfg_nameConstant3->setText(chosen_const.label);
}

//------------------------------------------------------------------------------
// Name: slotChooseScientificConst4
// Desc: updates constants value
//------------------------------------------------------------------------------
void KCalculator::slotChooseScientificConst4(const science_constant &chosen_const) {
	constants_->kcfg_valueConstant4->setText(chosen_const.value);
	constants_->kcfg_nameConstant4->setText(chosen_const.label);
}

//------------------------------------------------------------------------------
// Name: slotChooseScientificConst5
// Desc: updates constants value
//------------------------------------------------------------------------------
void KCalculator::slotChooseScientificConst5(const science_constant &chosen_const) {
	constants_->kcfg_valueConstant5->setText(chosen_const.value);
	constants_->kcfg_nameConstant5->setText(chosen_const.label);
}

//------------------------------------------------------------------------------
// Name: slotSetSimpleMode
// Desc: sets the calculator to have a simple layout
//------------------------------------------------------------------------------
void KCalculator::slotSetSimpleMode() {

	action_constants_show_->setChecked(false);
	action_constants_show_->setEnabled(false);
	action_bitset_show_->setChecked(false);
	action_bitset_show_->setEnabled(false);
	showMemButtons(false);
	showScienceButtons(false);
	showStatButtons(false);
	showLogicButtons(false);

	// hide some individual buttons, which are not in one of the above groups
	pbShift->hide();
	pbMod->hide();
	pbReci->hide();
	pbFactorial->hide();
	pbSquare->hide();
	pbPower->hide();
	pbCube->hide();
	pbBackspace->hide();
	pbEE->hide();

	// delete the constant menu since it doesn't fit
	delete constants_menu_;
	constants_menu_ = 0;

	KCalcSettings::setCalculatorMode(KCalcSettings::EnumCalculatorMode::simple);
}

//------------------------------------------------------------------------------
// Name: slotSetScienceMode
// Desc: sets the calculator to science mode
//------------------------------------------------------------------------------
void KCalculator::slotSetScienceMode() {

	action_constants_show_->setEnabled(true);
	action_constants_show_->setChecked(KCalcSettings::showConstants());
	action_bitset_show_->setChecked(false);
	action_bitset_show_->setEnabled(false);

	// show some individual buttons
	pbShift->show();
	pbMod->show();
	pbReci->show();
	pbFactorial->show();
	pbSquare->show();
	pbPower->show();
	pbCube->show();
	pbBackspace->show();
	pbEE->show();

	// show or hide some groups of buttons
	showMemButtons(true);
	showScienceButtons(true);
	showStatButtons(false);
	showLogicButtons(false);

	if(!constants_menu_) {
		constants_menu_ = createConstantsMenu();
		menuBar()->insertMenu((menuBar()->actions)()[2], constants_menu_);
	}

	KCalcSettings::setCalculatorMode(KCalcSettings::EnumCalculatorMode::science);
}

//------------------------------------------------------------------------------
// Name: slotSetStatisticMode
// Desc: sets the calculator to stats mode
//------------------------------------------------------------------------------
void KCalculator::slotSetStatisticMode() {

	action_constants_show_->setEnabled(true);
	action_constants_show_->setChecked(KCalcSettings::showConstants());
	action_bitset_show_->setChecked(false);
	action_bitset_show_->setEnabled(false);

	// show some individual buttons
	pbShift->show();
	pbMod->show();
	pbReci->show();
	pbFactorial->show();
	pbSquare->show();
	pbPower->show();
	pbCube->show();
	pbBackspace->show();
	pbEE->show();

	// show or hide some groups of buttons
	showMemButtons(true);
	showScienceButtons(true);
	showStatButtons(true);
	showLogicButtons(false);

	if(!constants_menu_) {
		constants_menu_ = createConstantsMenu();
		menuBar()->insertMenu((menuBar()->actions)()[2], constants_menu_);
	}

	KCalcSettings::setCalculatorMode(KCalcSettings::EnumCalculatorMode::statistics);
}

//------------------------------------------------------------------------------
// Name: slotSetNumeralMode
// Desc: sets the calculator to numerical ("programmers") mode
//------------------------------------------------------------------------------
void KCalculator::slotSetNumeralMode() {

	action_constants_show_->setChecked(false);
	action_constants_show_->setEnabled(false);
	action_bitset_show_->setEnabled(true);
	action_bitset_show_->setChecked(KCalcSettings::showBitset());

	// show some individual buttons
	pbShift->show();
	pbMod->show();
	pbReci->show();
	pbFactorial->show();
	pbSquare->show();
	pbPower->show();
	pbCube->show();
	pbBackspace->show();
	pbEE->show();

	// show or hide some groups of buttons
	showMemButtons(true);
	showScienceButtons(false);
	showStatButtons(false);
	showLogicButtons(true);

	if(!constants_menu_) {
		constants_menu_ = createConstantsMenu();
		menuBar()->insertMenu((menuBar()->actions)()[2], constants_menu_);
	}

	KCalcSettings::setCalculatorMode(KCalcSettings::EnumCalculatorMode::numeral);
}

//------------------------------------------------------------------------------
// Name: showMemButtons
// Desc: hides or shows the memory buttons
//------------------------------------------------------------------------------
void KCalculator::showMemButtons(bool toggled) {

	if (toggled) {
		foreach(QAbstractButton *btn, mem_button_list_) {
			btn->show();
		}
	} else {
		foreach(QAbstractButton *btn, mem_button_list_) {
			btn->hide();
		}

		// these are in the mem_button_list_ but should not be hidden
		pbClear->show();
		pbAllClear->show();
	}
}

//------------------------------------------------------------------------------
// Name: showStatButtons
// Desc: hides or shows the stat buttons
//------------------------------------------------------------------------------
void KCalculator::showStatButtons(bool toggled) {

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

//------------------------------------------------------------------------------
// Name: showScienceButtons
// Desc: hides or shows the science buttons
//------------------------------------------------------------------------------
void KCalculator::showScienceButtons(bool toggled) {

	if (toggled) {
		foreach(QAbstractButton* btn, scientific_buttons_) {
			btn->show();
		}

		foreach(QAbstractButton* btn, angle_choose_group_->buttons()) {
			btn->show();
		}

		setAngle();
		statusBar()->setAngleModeIndicatorVisible(true);
	} else {
		foreach(QAbstractButton* btn, scientific_buttons_) {
			btn->hide();
		}

		foreach(QAbstractButton* btn, angle_choose_group_->buttons()) {
			btn->hide();
		}

		statusBar()->setAngleModeIndicatorVisible(false);
		calc_display->setStatusText(AngleField, QString());
	}
}

//------------------------------------------------------------------------------
// Name: showLogicButtons
// Desc: hides or shows the logic buttons
//------------------------------------------------------------------------------
void KCalculator::showLogicButtons(bool toggled) {

	if (toggled) {
		mBitset->setEnabled(true);
		connect(mBitset, &KCalcBitset::valueChanged, this, &KCalculator::slotBitsetChanged);
		connect(calc_display, &KCalcDisplay::changedAmount, this, &KCalculator::slotUpdateBitset);

		foreach(QAbstractButton* btn, logic_buttons_) {
			btn->show();
		}

		setBase();
		statusBar()->setBaseIndicatorVisible(true);

		foreach(QAbstractButton *btn, base_choose_group_->buttons()) {
			btn->show();
		}

		for (int i = 10; i < 16; ++i) {
			(num_button_group_->button(i))->show();
		}
	} else {
		mBitset->setEnabled(false);
		disconnect(mBitset, &KCalcBitset::valueChanged, this, &KCalculator::slotBitsetChanged);
		disconnect(calc_display, &KCalcDisplay::changedAmount, this, &KCalculator::slotUpdateBitset);

		foreach(QAbstractButton* btn, logic_buttons_) {
			btn->hide();
		}

		// Hide Hex-Buttons, but first switch back to decimal
		decRadio->animateClick(0);

		foreach(QAbstractButton *btn, base_choose_group_->buttons()) {
			btn->hide();
		}

		statusBar()->setBaseIndicatorVisible(false);
		calc_display->setStatusText(BaseField, QString());
		for (int i = 10; i < 16; ++i) {
			(num_button_group_->button(i))->hide();
		}
	}
}

//------------------------------------------------------------------------------
// Name: slotConstantsShow
// Desc: hides or shows the constants buttons
//------------------------------------------------------------------------------
void KCalculator::slotConstantsShow(bool toggled) {

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

//------------------------------------------------------------------------------
// Name: slotBitsetshow
// Desc: hides or shows the bitset buttons
//------------------------------------------------------------------------------
void KCalculator::slotBitsetshow(bool toggled) {

	mBitset->setVisible(toggled);
	KCalcSettings::setShowBitset(toggled);
}

//------------------------------------------------------------------------------
// Name: slotBitsetshow
// Desc: This function is for setting the constant names configured in the
//       kcalc settings menu. If the user doesn't enter a name for the
//       constant C1 to C6 is used.
//------------------------------------------------------------------------------
void KCalculator::changeButtonNames() {

    foreach(QAbstractButton *btn, const_buttons_) {
        if (KCalcConstButton *const constbtn = qobject_cast<KCalcConstButton*>(btn)) {
			constbtn->setLabelAndTooltip();
		}
    }
}

//------------------------------------------------------------------------------
// Name: slotBitsetChanged
// Desc: updates the bitset display
// NOTE: sets display to *unsigned* value
//------------------------------------------------------------------------------
void KCalculator::slotBitsetChanged(quint64 value) {

    calc_display->setAmount(KNumber(value));
    updateDisplay(0);
}

//------------------------------------------------------------------------------
// Name: slotUpdateBitset
// Desc: updates the bitset itself
//------------------------------------------------------------------------------
void KCalculator::slotUpdateBitset(const KNumber &nr) {

    mBitset->setValue(nr.toUint64());
}

//------------------------------------------------------------------------------
// Name: updateSettings
// Desc: updates the persistent settings
//------------------------------------------------------------------------------
void KCalculator::updateSettings() {

	changeButtonNames();
	setColors();
	setFonts();
	setPrecision();

	// Show the result in the app's caption in taskbar (wishlist - bug #52858)
	disconnect(calc_display, SIGNAL(changedText(QString)), this, 0);

	if (KCalcSettings::captionResult()) {
		connect(calc_display, &KCalcDisplay::changedText, this, &KCalculator::setWindowTitle);
	} else {
		setCaption(QString());
	}

	calc_display->changeSettings();
	updateGeometry();
}

//------------------------------------------------------------------------------
// Name: updateDisplay
// Desc: updates the display
//------------------------------------------------------------------------------
void KCalculator::updateDisplay(UpdateFlags flags) {

	if(flags & UPDATE_FROM_CORE) {
		calc_display->updateFromCore(core, (flags & UPDATE_STORE_RESULT) != 0);
	} else {
		calc_display->update();
	}

	pbShift->setChecked(false);

}

//------------------------------------------------------------------------------
// Name: setColors
// Desc: set the various colours
//------------------------------------------------------------------------------
void KCalculator::setColors() {

	calc_display->changeSettings();

	KColorScheme schemeButtons(QPalette::Active, KColorScheme::Button);
	const QColor defaultColor = schemeButtons.background().color();

	if (KCalcSettings::numberButtonsColor()           == defaultColor
			&& KCalcSettings::functionButtonsColor()  == defaultColor
			&& KCalcSettings::statButtonsColor()      == defaultColor
			&& KCalcSettings::hexButtonsColor()       == defaultColor
			&& KCalcSettings::memoryButtonsColor()    == defaultColor
			&& KCalcSettings::operationButtonsColor() == defaultColor) {
		return;
	}

	const QString sheet = QLatin1String("QPushButton { background-color: %1 }");

	const QColor numPal(KCalcSettings::numberButtonsColor());
	for (int i = 0; i < 10; ++i) {
		(num_button_group_->button(i))->setStyleSheet(sheet.arg(numPal.name()));
	}

	const QColor funcPal(KCalcSettings::functionButtonsColor());
	foreach(QAbstractButton *btn, function_button_list_) {
		btn->setStyleSheet(sheet.arg(funcPal.name()));
	}

	const QColor statPal(KCalcSettings::statButtonsColor());
	foreach(QAbstractButton *btn, stat_buttons_) {
	btn->setStyleSheet(sheet.arg(statPal.name()));
	}

	const QColor hexPal(KCalcSettings::hexButtonsColor());
	for (int i = 10; i < 16; ++i) {
		(num_button_group_->button(i))->setStyleSheet(sheet.arg(hexPal.name()));
	}

	const QColor memPal(KCalcSettings::memoryButtonsColor());
	foreach(QAbstractButton *btn, mem_button_list_) {
		btn->setStyleSheet(sheet.arg(memPal.name()));
	}

	const QColor opPal(KCalcSettings::operationButtonsColor());
	foreach(QAbstractButton *btn, operation_button_list_) {
		btn->setStyleSheet(sheet.arg(opPal.name()));
	}
}

//------------------------------------------------------------------------------
// Name: setFonts
// Desc: set the various fonts
//------------------------------------------------------------------------------
void KCalculator::setFonts() {

	foreach(QObject *obj, leftPad->children()) {
		if (KCalcButton *const button = qobject_cast<KCalcButton*>(obj)) {
			button->setFont(KCalcSettings::buttonFont());
		}
	}

	foreach(QObject *obj, numericPad->children()) {
		if (KCalcButton *const button = qobject_cast<KCalcButton*>(obj)) {
			button->setFont(KCalcSettings::buttonFont());
		}
	}

	foreach(QObject *obj, rightPad->children()) {
		if (KCalcButton *const button = qobject_cast<KCalcButton*>(obj)) {
			button->setFont(KCalcSettings::buttonFont());
		}
	}

	updateGeometry();
}

//------------------------------------------------------------------------------
// Name: event
// Desc: catch application's palette and font change events
//------------------------------------------------------------------------------
bool KCalculator::event(QEvent *e) {

	switch (e->type()) {
	case QEvent::ApplicationFontChange:
		setFonts();
		break;
	case QEvent::ApplicationPaletteChange:
		setColors();
		break;
	default:
		break;
	}
	return KXmlGuiWindow::event(e);
}

//------------------------------------------------------------------------------
// Name: setPrecision
// Desc: set the precision of the display
//------------------------------------------------------------------------------
void KCalculator::setPrecision() {

	KNumber::setDefaultFloatPrecision(KCalcSettings::precision());
	updateDisplay(0);
}

//------------------------------------------------------------------------------
// Name: setAngle
// Desc: sets the angle mode
//------------------------------------------------------------------------------
void KCalculator::setAngle() {

    if (QAbstractButton *const btn = angle_choose_group_->button(KCalcSettings::angleMode())) {
		btn->animateClick(0);
	}
}

//------------------------------------------------------------------------------
// Name: setBase
// Desc: sets the numeric base
//------------------------------------------------------------------------------
void KCalculator::setBase() {
	if (QAbstractButton *const btn = base_choose_group_->button(KCalcSettings::baseMode())) {
		btn->animateClick(0);
	}
}

//------------------------------------------------------------------------------
// Name: eventFilter
// Desc: general event filter used to track events like drag/drop
//------------------------------------------------------------------------------
bool KCalculator::eventFilter(QObject *o, QEvent *e) {

	switch (e->type()) {
	case QEvent::DragEnter: {
		QDragEnterEvent *const ev = reinterpret_cast<QDragEnterEvent *>(e);
		ev->setAccepted(KColorMimeData::canDecode(ev->mimeData()));
		return true;
	}
	case QEvent::DragLeave: {
		return true;
	}
	case QEvent::Drop: {
		KCalcButton *const calcButton = qobject_cast<KCalcButton *>(o);
		if (!calcButton) {
			return false;
		}

		QDropEvent *const ev = reinterpret_cast<QDropEvent *>(e);
		QColor c = KColorMimeData::fromMimeData(ev->mimeData());

		if (c.isValid()) {
			QString cn = c.name();
			QString sheet = QLatin1String("background-color: %1");

			QList<QAbstractButton*> *list;
			const int num_but = num_button_group_->buttons().indexOf(calcButton);
			if (num_but != -1) {
				// Was it hex-button or normal digit??
				if (num_but < 10) {
					for (int i = 0; i < 10; ++i) {
						(num_button_group_->buttons()[i])->setStyleSheet(sheet.arg(cn));
					}
				} else {
					for (int i = 10; i < 16; ++i) {
						(num_button_group_->buttons()[i])->setStyleSheet(sheet.arg(cn));
					}
				}
				return true;
			} else if (function_button_list_.contains(calcButton)) {
				list = &function_button_list_;
			} else if (stat_button_list_.contains(calcButton)) {
				list = &stat_button_list_;
			} else if (mem_button_list_.contains(calcButton)) {
				list = &mem_button_list_;
			} else if (operation_button_list_.contains(calcButton)) {
				list = &operation_button_list_;
			} else {
				return false;
			}

			for (int i = 0; i < list->size(); ++i) {
				list->at(i)->setStyleSheet(sheet.arg(cn));
			}
		}
		return true;
	}
	// FALL THROUGH
	default:
		return KXmlGuiWindow::eventFilter(o, e);
	}
}

////////////////////////////////////////////////////////////////
// Include the meta-object code for classes in this file
//


//------------------------------------------------------------------------------
// Name: kdemain
// Desc: entry point of the application
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT int kdemain(int argc, char *argv[]) {

    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("kcalc");

    /**
     * enable high dpi support
     */
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    Kdelibs4ConfigMigrator migrate(QLatin1String("kcalc"));
    migrate.setConfigFiles(QStringList() << QLatin1String("kcalcrc"));
    migrate.setUiFiles(QStringList() << QLatin1String("kcalcui.rc"));
    migrate.migrate();

    KAboutData aboutData(QStringLiteral("kcalc"),
                         i18n("KCalc"),
                         QStringLiteral(KCALC_VERSION_STRING),
                         i18n(description),
                         KAboutLicense::GPL,
                         i18n("Copyright © 2008-2013, Evan Teran\n"
                              "Copyright © 2000-2008, The KDE Team\n"
                              "Copyright © 2003-2005, Klaus Niederkr" "\xc3\xbc" "ger\n"
                              "Copyright © 1996-2000, Bernd Johannes Wuebben"),
                         QStringLiteral(),
                         QStringLiteral("http://utils.kde.org/projects/kcalc"));

	// Klaus Niederkrueger
	aboutData.addAuthor(i18n("Klaus Niederkr" "\xc3\xbc" "ger"), QString(), "kniederk@math.uni-koeln.de");
	aboutData.addAuthor(i18n("Bernd Johannes Wuebben"), QString(), "wuebben@kde.org");
	aboutData.addAuthor(i18n("Evan Teran"), i18n("Maintainer"), "eteran@alum.rit.edu");
	aboutData.addAuthor(i18n("Espen Sand"), QString(), "espen@kde.org");
	aboutData.addAuthor(i18n("Chris Howells"), QString(), "howells@kde.org");
	aboutData.addAuthor(i18n("Aaron J. Seigo"), QString(), "aseigo@olympusproject.org");
	aboutData.addAuthor(i18n("Charles Samuels"), QString(), "charles@altair.dhs.org");
	// Rene Merou
	aboutData.addAuthor(i18n("Ren" "\xc3\xa9" " M" "\xc3\xa9"  "rou"), QString(), "ochominutosdearco@yahoo.es");
	aboutData.addAuthor(i18n("Michel Marti"), QString(), "mma@objectxp.com");
	aboutData.addAuthor(i18n("David Johnson"), QString(), "david@usermode.org");

	KAboutData::setApplicationData(aboutData);
	app.setWindowIcon(QIcon::fromTheme(QLatin1String("accessories-calculator"), app.windowIcon()));


	QCommandLineParser parser;
	parser.addHelpOption();
	parser.addVersionOption();
        aboutData.setupCommandLine(&parser);
	parser.process(app);
        aboutData.processCommandLine(&parser);

	// force system locale to "C" internally [bug 159168]
	setlocale(LC_NUMERIC, "C");

    KNumber::setGroupSeparator(QLocale().groupSeparator());
    KNumber::setDecimalSeparator(QString(QLocale().decimalPoint()));

	KCalculator *calc = new KCalculator(0);

	calc->show();
	return app.exec();
}
