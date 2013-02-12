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

#include "kcalc.h"

#include <cctype>
#include <clocale>
#include <cstdio>
#include <cstdlib>
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
#include <kstandardaction.h>
#include <kstatusbar.h>
#include <ktoggleaction.h>
#include <ktoolbar.h>
#include <kxmlguifactory.h>

#include "kcalcdisplay.h"
#include "kcalc_const_menu.h"
#include "version.h"
#include "kcalc_settings.h"
#include "kcalc_bitset.h"

class QActionGroup;

namespace {
const char description[] = I18N_NOOP("KDE Calculator");
const char version[]	 = KCALCVERSION;
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
	constants_menu_ = createConstantsMenu();
	menuBar()->insertMenu((menuBar()->actions)()[2], constants_menu_);

	// misc setup
	setColors();
	setFonts();

	// Show the result in the app's caption in taskbar (wishlist - bug #52858)
	if (KCalcSettings::captionResult() == true) {
		connect(calc_display, SIGNAL(changedText(QString)), SLOT(setCaption(QString)));
	}

	calc_display->changeSettings();
	setPrecision();

	updateGeometry();

	setFixedSize(minimumSize());

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

	// connections
	connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), SLOT(setColors()));
	connect(KGlobalSettings::self(), SIGNAL(kdisplayFontChanged()), SLOT(setFonts()));

	calc_display->setFocus();
}

//------------------------------------------------------------------------------
// Name: ~KCalculator
// Desc: deconstructor
//------------------------------------------------------------------------------
KCalculator::~KCalculator() {

	KCalcSettings::self()->writeConfig();
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
	connect(action_mode_simple_, SIGNAL(toggled(bool)), SLOT(slotSetSimpleMode()));

	action_mode_science_ = actionCollection()->add<KToggleAction>(QLatin1String("mode_science"));
	action_mode_science_->setActionGroup(modeGroup);
	action_mode_science_->setText(i18n("Science Mode"));
	connect(action_mode_science_, SIGNAL(toggled(bool)), SLOT(slotSetScienceMode()));

	action_mode_statistic_ = actionCollection()->add<KToggleAction>(QLatin1String("mode_statistics"));
	action_mode_statistic_->setActionGroup(modeGroup);
	action_mode_statistic_->setText(i18n("Statistic Mode"));
	connect(action_mode_statistic_, SIGNAL(toggled(bool)), SLOT(slotSetStatisticMode()));

	action_mode_numeral_ = actionCollection()->add<KToggleAction>(QLatin1String("mode_numeral"));
	action_mode_numeral_->setActionGroup(modeGroup);
	action_mode_numeral_->setText(i18n("Numeral System Mode"));
	connect(action_mode_numeral_, SIGNAL(toggled(bool)), SLOT(slotSetNumeralMode()));

	// settings menu
	action_constants_show_ = actionCollection()->add<KToggleAction>(QLatin1String("show_constants"));
	action_constants_show_->setText(i18n("Constants &Buttons"));
	action_constants_show_->setChecked(true);
	connect(action_constants_show_, SIGNAL(toggled(bool)), SLOT(slotConstantsShow(bool)));

	action_bitset_show_ = actionCollection()->add<KToggleAction>(QLatin1String("show_bitset"));
	action_bitset_show_->setText(i18n("Show B&it Edit"));
	action_bitset_show_->setChecked(true);
	connect(action_bitset_show_, SIGNAL(toggled(bool)), SLOT(slotBitsetshow(bool)));

	KStandardAction::preferences(this, SLOT(showSettings()), actionCollection());
	KStandardAction::keyBindings(guiFactory(), SLOT(configureShortcuts()), actionCollection());
}

//------------------------------------------------------------------------------
// Name: createConstantsMenu
// Desc: creates and returns a pointer to the constant menu
//------------------------------------------------------------------------------
KCalcConstMenu *KCalculator::createConstantsMenu() {

	KCalcConstMenu *const menu = new KCalcConstMenu(i18n("&Constants"), this);
	connect(menu, SIGNAL(triggeredConstant(science_constant)), this, SLOT(slotConstantToDisplay(science_constant)));
	return menu;
}

//------------------------------------------------------------------------------
// Name: setupStatusbar
// Desc: sets up the status bar with default text
//------------------------------------------------------------------------------
void KCalculator::setupStatusbar() {

	// Status bar contents
	statusBar()->insertPermanentFixedItem(QLatin1String(" NORM "), ShiftField);
	statusBar()->setItemAlignment(ShiftField, Qt::AlignCenter);

	statusBar()->insertPermanentFixedItem(QLatin1String(" HEX "), BaseField);
	statusBar()->setItemAlignment(BaseField, Qt::AlignCenter);

	statusBar()->insertPermanentFixedItem(QLatin1String(" DEG "), AngleField);
	statusBar()->setItemAlignment(AngleField, Qt::AlignCenter);

	statusBar()->insertPermanentFixedItem(QLatin1String(" \xa0\xa0 "), MemField);   // nbsp
	statusBar()->setItemAlignment(MemField, Qt::AlignCenter);
}

//------------------------------------------------------------------------------
// Name: setupNumberKeys
// Desc: sets up number keys and related shortcuts
//------------------------------------------------------------------------------
void KCalculator::setupNumberKeys() {

    num_button_group_ = new QButtonGroup(this);
    connect(num_button_group_, SIGNAL(buttonClicked(int)), SLOT(slotNumberclicked(int)));

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
    connect(this, SIGNAL(switchShowAccels(bool)), pb0, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchShowAccels(bool)), pb1, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchShowAccels(bool)), pb2, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchShowAccels(bool)), pb3, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchShowAccels(bool)), pb4, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchShowAccels(bool)), pb5, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchShowAccels(bool)), pb6, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchShowAccels(bool)), pb7, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchShowAccels(bool)), pb8, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchShowAccels(bool)), pb9, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchShowAccels(bool)), pbA, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchShowAccels(bool)), pbB, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchShowAccels(bool)), pbC, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchShowAccels(bool)), pbD, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchShowAccels(bool)), pbE, SLOT(slotSetAccelDisplayMode(bool)));
    connect(this, SIGNAL(switchShowAccels(bool)), pbF, SLOT(slotSetAccelDisplayMode(bool)));
}

//------------------------------------------------------------------------------
// Name: setupRightKeypad
// Desc: sets up right keypad keys and related shortcuts
//------------------------------------------------------------------------------
void KCalculator::setupRightKeypad() {

	connect(pbShift, SIGNAL(toggled(bool)), SLOT(slotShifttoggled(bool)));
	connect(this, SIGNAL(switchShowAccels(bool)), pbShift, SLOT(slotSetAccelDisplayMode(bool)));

	pbBackspace->setShortcut(QKeySequence(Qt::Key_Backspace));
	new QShortcut(Qt::Key_PageUp, pbBackspace, SLOT(animateClick()));
	connect(pbBackspace, SIGNAL(clicked()), SLOT(slotBackspaceclicked()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbBackspace, SLOT(slotSetAccelDisplayMode(bool)));

	pbClear->setShortcut(QKeySequence(Qt::Key_Escape));
	new QShortcut(Qt::Key_PageUp, pbClear, SLOT(animateClick()));
	connect(pbClear, SIGNAL(clicked()), SLOT(slotClearclicked()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbClear, SLOT(slotSetAccelDisplayMode(bool)));

	pbAllClear->setShortcut(QKeySequence(Qt::Key_Delete));
	new QShortcut(Qt::Key_PageDown, pbAllClear, SLOT(animateClick()));
	connect(pbAllClear, SIGNAL(clicked()), SLOT(slotAllClearclicked()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbAllClear, SLOT(slotSetAccelDisplayMode(bool)));

	pbParenOpen->setShortcut(QKeySequence(Qt::Key_ParenLeft));
	connect(pbParenOpen, SIGNAL(clicked()), SLOT(slotParenOpenclicked()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbParenOpen, SLOT(slotSetAccelDisplayMode(bool)));

	pbParenClose->setShortcut(QKeySequence(Qt::Key_ParenRight));
	connect(pbParenClose, SIGNAL(clicked()), SLOT(slotParenCloseclicked()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbParenClose, SLOT(slotSetAccelDisplayMode(bool)));

	pbMemRecall->setDisabled(true);   // nothing in memory at start
	connect(pbMemRecall, SIGNAL(clicked()), SLOT(slotMemRecallclicked()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbMemRecall, SLOT(slotSetAccelDisplayMode(bool)));

	connect(pbMemClear, SIGNAL(clicked()), SLOT(slotMemClearclicked()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbMemClear, SLOT(slotSetAccelDisplayMode(bool)));

	pbMemPlusMinus->addMode(ModeNormal, i18nc("Add display to memory", "M+"), i18n("Add display to memory"));
	pbMemPlusMinus->addMode(ModeShift, i18nc("Subtract from memory", "M\xe2\x88\x92"), i18n("Subtract from memory"));
	connect(pbMemPlusMinus, SIGNAL(clicked()), SLOT(slotMemPlusMinusclicked()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbMemPlusMinus, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbMemPlusMinus, SLOT(slotSetMode(ButtonModeFlags,bool)));

	connect(pbMemStore, SIGNAL(clicked()), SLOT(slotMemStoreclicked()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbMemStore, SLOT(slotSetAccelDisplayMode(bool)));

	pbPercent->setShortcut(QKeySequence(Qt::Key_Percent));
	connect(pbPercent, SIGNAL(clicked()), SLOT(slotPercentclicked()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbPercent, SLOT(slotSetAccelDisplayMode(bool)));

	pbPlusMinus->setShortcut(QKeySequence(Qt::Key_Backslash));
	connect(pbPlusMinus, SIGNAL(clicked()), SLOT(slotPlusMinusclicked()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbPlusMinus, SLOT(slotSetAccelDisplayMode(bool)));
}

//------------------------------------------------------------------------------
// Name: setupNumericKeypad
// Desc: sets up numeric keys and related shortcuts
//------------------------------------------------------------------------------
void KCalculator::setupNumericKeypad() {

	pbCube->addMode(ModeNormal, i18nc("Third power", "x<sup>3</sup>"), i18n("Third power"));
	pbCube->addMode(ModeShift, QLatin1String("<sup>3</sup>&radic;x"), i18n("Cube root"));
	connect(pbCube, SIGNAL(clicked()), SLOT(slotCubeclicked()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbCube, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbCube, SLOT(slotSetMode(ButtonModeFlags,bool)));

	pbDivision->setShortcut(QKeySequence(Qt::Key_Slash));
	new QShortcut(Qt::Key_division, pbDivision, SLOT(animateClick()));
	connect(pbDivision, SIGNAL(clicked()), SLOT(slotDivisionclicked()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbDivision, SLOT(slotSetAccelDisplayMode(bool)));

	pbMultiplication->setShortcut(QKeySequence(Qt::Key_Asterisk));
	new QShortcut(Qt::Key_X, pbMultiplication, SLOT(animateClick()));
	new QShortcut(Qt::Key_multiply, pbMultiplication, SLOT(animateClick()));
	connect(pbMultiplication, SIGNAL(clicked()), SLOT(slotMultiplicationclicked()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbMultiplication, SLOT(slotSetAccelDisplayMode(bool)));

	pbMinus->setShortcut(QKeySequence(Qt::Key_Minus));
	connect(pbMinus, SIGNAL(clicked()), SLOT(slotMinusclicked()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbMinus, SLOT(slotSetAccelDisplayMode(bool)));

	pbPlus->setShortcut(QKeySequence(Qt::Key_Plus));
	connect(pbPlus, SIGNAL(clicked()), SLOT(slotPlusclicked()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbPlus, SLOT(slotSetAccelDisplayMode(bool)));

	pbPeriod->setText(KGlobal::locale()->decimalSymbol());
	pbPeriod->setShortcut(KGlobal::locale()->decimalSymbol());

	// TODO: is this needed? the above line look slike it should do the right thing?
	/*
	if (KGlobal::locale()->decimalSymbol() == QLatin1String(".")) {
		new QShortcut(Qt::Key_Comma, pbPeriod, SLOT(animateClick()));
	} else if (KGlobal::locale()->decimalSymbol() == QLatin1String(",")) {
		new QShortcut(Qt::Key_Period, pbPeriod, SLOT(animateClick()));
	}
	*/

	connect(pbPeriod, SIGNAL(clicked()), SLOT(slotPeriodclicked()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbPeriod, SLOT(slotSetAccelDisplayMode(bool)));

	pbEqual->setShortcut(QKeySequence(Qt::Key_Enter));
	new QShortcut(Qt::Key_Equal, pbEqual, SLOT(animateClick()));
	new QShortcut(Qt::Key_Return, pbEqual, SLOT(animateClick()));
	connect(pbEqual, SIGNAL(clicked()), SLOT(slotEqualclicked()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbEqual, SLOT(slotSetAccelDisplayMode(bool)));
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
	connect(this, SIGNAL(switchShowAccels(bool)), pbAND, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbAND, SIGNAL(clicked()), SLOT(slotANDclicked()));

	pbOR->setShortcut(QKeySequence(Qt::Key_Bar));
	connect(this, SIGNAL(switchShowAccels(bool)), pbOR, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbOR, SIGNAL(clicked()), SLOT(slotORclicked()));

	connect(this, SIGNAL(switchShowAccels(bool)), pbXOR, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbXOR, SIGNAL(clicked()), SLOT(slotXORclicked()));

	pbLsh->setShortcut(QKeySequence(Qt::Key_Less));
	connect(this, SIGNAL(switchShowAccels(bool)), pbLsh, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbLsh, SIGNAL(clicked()), SLOT(slotLeftShiftclicked()));

	pbRsh->setShortcut(QKeySequence(Qt::Key_Greater));
	connect(this, SIGNAL(switchShowAccels(bool)), pbRsh, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbRsh, SIGNAL(clicked()), SLOT(slotRightShiftclicked()));

	pbCmp->setShortcut(QKeySequence(Qt::Key_AsciiTilde));
	connect(this, SIGNAL(switchShowAccels(bool)), pbCmp, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbCmp, SIGNAL(clicked()), SLOT(slotNegateclicked()));
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

	connect(this, SIGNAL(switchShowAccels(bool)), pbHyp, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbHyp, SIGNAL(toggled(bool)), SLOT(slotHyptoggled(bool)));

	pbSin->addMode(ModeNormal, i18nc("Sine", "Sin"), i18n("Sine"));
	pbSin->addMode(ModeShift, i18nc("Arc sine", "Asin"), i18n("Arc sine"));
	pbSin->addMode(ModeHyperbolic, i18nc("Hyperbolic sine", "Sinh"), i18n("Hyperbolic sine"));
	pbSin->addMode(ButtonModeFlags(ModeShift | ModeHyperbolic), i18nc("Inverse hyperbolic sine", "Asinh"), i18n("Inverse hyperbolic sine"));
	connect(this, SIGNAL(switchShowAccels(bool)), pbSin, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbSin, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbSin, SIGNAL(clicked()), SLOT(slotSinclicked()));

	pbCos->addMode(ModeNormal, i18nc("Cosine", "Cos"), i18n("Cosine"));
	pbCos->addMode(ModeShift, i18nc("Arc cosine", "Acos"), i18n("Arc cosine"));
	pbCos->addMode(ModeHyperbolic, i18nc("Hyperbolic cosine", "Cosh"), i18n("Hyperbolic cosine"));
	pbCos->addMode(ButtonModeFlags(ModeShift | ModeHyperbolic), i18nc("Inverse hyperbolic cosine", "Acosh"), i18n("Inverse hyperbolic cosine"));
	connect(this, SIGNAL(switchShowAccels(bool)), pbCos, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbCos, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbCos, SIGNAL(clicked()), SLOT(slotCosclicked()));

	pbTan->addMode(ModeNormal, i18nc("Tangent", "Tan"), i18n("Tangent"));
	pbTan->addMode(ModeShift, i18nc("Arc tangent", "Atan"), i18n("Arc tangent"));
	pbTan->addMode(ModeHyperbolic, i18nc("Hyperbolic tangent", "Tanh"), i18n("Hyperbolic tangent"));
	pbTan->addMode(ButtonModeFlags(ModeShift | ModeHyperbolic), i18nc("Inverse hyperbolic tangent", "Atanh"), i18n("Inverse hyperbolic tangent"));
	connect(this, SIGNAL(switchShowAccels(bool)), pbTan, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbTan, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbTan, SIGNAL(clicked()), SLOT(slotTanclicked()));

	pbLog->addMode(ModeNormal, i18nc("Logarithm to base 10", "Log"), i18n("Logarithm to base 10"));
	pbLog->addMode(ModeShift, i18nc("10 to the power of x", "10<sup>x</sup>"), i18n("10 to the power of x"));
	connect(this, SIGNAL(switchShowAccels(bool)), pbLog, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbLog, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbLog, SIGNAL(clicked()), SLOT(slotLogclicked()));
	pbLn->addMode(ModeNormal, i18nc("Natural log", "Ln"), i18n("Natural log"));
	pbLn->addMode(ModeShift, i18nc("Exponential function", "e<sup>x</sup>"), i18n("Exponential function"));
	connect(this, SIGNAL(switchShowAccels(bool)), pbLn, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbLn, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbLn, SIGNAL(clicked()), SLOT(slotLnclicked()));
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
	connect(this, SIGNAL(switchShowAccels(bool)), pbNData, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbNData, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbNData, SIGNAL(clicked()), SLOT(slotStatNumclicked()));

	pbMean->addMode(ModeNormal, i18nc("Mean", "Mea"), i18n("Mean"));
	pbMean->addMode(ModeShift, QString::fromUtf8("\xce\xa3") + QLatin1String("x<sup>2</sup>"), i18n("Sum of all data items squared"));
	connect(this, SIGNAL(switchShowAccels(bool)), pbMean, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbMean, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbMean, SIGNAL(clicked()), SLOT(slotStatMeanclicked()));

	pbSd->addMode(ModeNormal, QString::fromUtf8("\xcf\x83") + QLatin1String("<sub>N</sub>"), i18n("Standard deviation"));
	pbSd->addMode(ModeShift, QString::fromUtf8("\xcf\x83") + QLatin1String("<sub>N-1</sub>"), i18n("Sample standard deviation"));
	connect(this, SIGNAL(switchShowAccels(bool)), pbSd, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbSd, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbSd, SIGNAL(clicked()), SLOT(slotStatStdDevclicked()));

	connect(this, SIGNAL(switchShowAccels(bool)), pbMed, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbMed, SIGNAL(clicked()), SLOT(slotStatMedianclicked()));

	pbDat->addMode(ModeNormal, i18nc("Enter data", "Dat"), i18n("Enter data"));
	pbDat->addMode(ModeShift, i18nc("Delete last data item", "CDat"), i18n("Delete last data item"));
	connect(this, SIGNAL(switchShowAccels(bool)), pbDat, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbDat, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbDat, SIGNAL(clicked()), SLOT(slotStatDataInputclicked()));

	connect(this, SIGNAL(switchShowAccels(bool)), pbCSt, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbCSt, SIGNAL(clicked()), SLOT(slotStatClearDataclicked()));
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
	connect(this, SIGNAL(switchShowAccels(bool)), pbC1, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbC1, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbC1, SIGNAL(clicked(int)), this, SLOT(slotConstclicked(int)));

	pbC2->setButtonNumber(1);
	connect(this, SIGNAL(switchShowAccels(bool)), pbC2, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbC2, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbC2, SIGNAL(clicked(int)), this, SLOT(slotConstclicked(int)));

	pbC3->setButtonNumber(2);
	connect(this, SIGNAL(switchShowAccels(bool)), pbC3, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbC3, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbC3, SIGNAL(clicked(int)), this, SLOT(slotConstclicked(int)));

	pbC4->setButtonNumber(3);
	connect(this, SIGNAL(switchShowAccels(bool)), pbC4, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbC4, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbC4, SIGNAL(clicked(int)), this, SLOT(slotConstclicked(int)));

	pbC5->setButtonNumber(4);
	connect(this, SIGNAL(switchShowAccels(bool)), pbC5, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbC5, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbC5, SIGNAL(clicked(int)), this, SLOT(slotConstclicked(int)));

	pbC6->setButtonNumber(5);
	connect(this, SIGNAL(switchShowAccels(bool)), pbC6, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbC6, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbC6, SIGNAL(clicked(int)), this, SLOT(slotConstclicked(int)));

	changeButtonNames();
}

//------------------------------------------------------------------------------
// Name: setupMiscKeys
// Desc: sets up misc keys and related shortcuts
//------------------------------------------------------------------------------
void KCalculator::setupMiscKeys() {

	pbMod->addMode(ModeNormal, i18nc("Modulo", "Mod"), i18n("Modulo"));
	pbMod->addMode(ModeShift, i18nc("Integer division", "IntDiv"), i18n("Integer division"));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbMod, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(this, SIGNAL(switchShowAccels(bool)), pbMod, SLOT(slotSetAccelDisplayMode(bool)));
	pbMod->setShortcut(QKeySequence(Qt::Key_Colon));
	connect(pbMod, SIGNAL(clicked()), SLOT(slotModclicked()));

	pbReci->addMode(ModeNormal, i18nc("Reciprocal", "1/x"), i18n("Reciprocal"));
	pbReci->addMode(ModeShift, i18nc("n Choose m", "nCm"), i18n("n Choose m"));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbReci, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(this, SIGNAL(switchShowAccels(bool)), pbReci, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbReci, SIGNAL(clicked()), SLOT(slotReciclicked()));

	pbFactorial->addMode(ModeNormal, i18nc("Factorial", "x!"), i18n("Factorial"));
	pbFactorial->setShortcut(QKeySequence(Qt::Key_Exclam));
	connect(this, SIGNAL(switchShowAccels(bool)), pbFactorial, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbFactorial, SIGNAL(clicked()), SLOT(slotFactorialclicked()));

	pbSquare->addMode(ModeNormal, i18nc("Square", "x<sup>2</sup>"), i18n("Square"));
	pbSquare->addMode(ModeShift, QLatin1String("&radic;x"), i18n("Square root"));
	pbSquare->setShortcut(QKeySequence(Qt::Key_BracketLeft));
	new QShortcut(Qt::Key_twosuperior, pbSquare, SLOT(animateClick()));
	connect(this, SIGNAL(switchShowAccels(bool)), pbSquare, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbSquare, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbSquare, SIGNAL(clicked()), SLOT(slotSquareclicked()));

	pbPower->addMode(ModeNormal, i18nc("x to the power of y", "x<sup>y</sup>"), i18n("x to the power of y"));
	pbPower->addMode(ModeShift, i18nc("x to the power of 1/y", "x<sup>1/y</sup>"), i18n("x to the power of 1/y"));
	connect(this, SIGNAL(switchShowAccels(bool)), pbPower, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), pbPower, SLOT(slotSetMode(ButtonModeFlags,bool)));
	pbPower->setShortcut(QKeySequence(Qt::Key_AsciiCircum));
	connect(pbPower, SIGNAL(clicked()), SLOT(slotPowerclicked()));

	pbEE->addMode(ModeNormal, QLatin1String("x<small>" "\xb7" "10</small><sup>y</sup>"), i18n("Exponent"));
	connect(this, SIGNAL(switchShowAccels(bool)), pbEE, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbEE, SIGNAL(clicked()), SLOT(slotEEclicked()));
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
	switch (base) {
	case BinMode:
		current_base = calc_display->setBase(NumBase(2));
		statusBar()->changeItem(QLatin1String("BIN"), BaseField);
		calc_display->setStatusText(BaseField, QLatin1String("Bin"));
		break;
	case OctMode:
		current_base = calc_display->setBase(NumBase(8));
		statusBar()->changeItem(QLatin1String("OCT"), BaseField);
		calc_display->setStatusText(BaseField, QLatin1String("Oct"));
		break;
	case DecMode:
		current_base = calc_display->setBase(NumBase(10));
		statusBar()->changeItem(QLatin1String("DEC"), BaseField);
		calc_display->setStatusText(BaseField, QLatin1String("Dec"));
		break;
	case HexMode:
		current_base = calc_display->setBase(NumBase(16));
		statusBar()->changeItem(QLatin1String("HEX"), BaseField);
		calc_display->setStatusText(BaseField, QLatin1String("Hex"));
		break;
	default:
		statusBar()->changeItem(QLatin1String("Error"), BaseField);
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

	switch (mode) {
	case DegMode:
		statusBar()->changeItem(QLatin1String("DEG"), AngleField);
		calc_display->setStatusText(AngleField, QLatin1String("Deg"));
		break;
	case RadMode:
		statusBar()->changeItem(QLatin1String("RAD"), AngleField);
		calc_display->setStatusText(AngleField, QLatin1String("Rad"));
		break;
	case GradMode:
		statusBar()->changeItem(QLatin1String("GRA"), AngleField);
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

	if (shift_mode_) {
		statusBar()->changeItem(i18nc("Second button functions are active", "SHIFT"), ShiftField);
		calc_display->setStatusText(ShiftField, i18n("Shift"));
	} else {
		statusBar()->changeItem(i18nc("Normal button functions are active", "NORM"), ShiftField);
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
	statusBar()->changeItem(QLatin1String("M"), MemField);
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
	statusBar()->changeItem(i18n("M"), MemField);
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
    core.Factorial(calc_display->getAmount());
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
	statusBar()->changeItem(QLatin1String(" \xa0\xa0 "), MemField); // nbsp
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
    calc_display->newCharacter(KGlobal::locale()->decimalSymbol()[0]);
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
	dialog->showButtonSeparator(true);

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
	connect(tmp_menu, SIGNAL(triggeredConstant(science_constant)), this, SLOT(slotChooseScientificConst0(science_constant)));
	constants_->pushButton0->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu, SIGNAL(triggeredConstant(science_constant)), this, SLOT(slotChooseScientificConst1(science_constant)));
	constants_->pushButton1->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu, SIGNAL(triggeredConstant(science_constant)), this, SLOT(slotChooseScientificConst2(science_constant)));
	constants_->pushButton2->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu, SIGNAL(triggeredConstant(science_constant)), this, SLOT(slotChooseScientificConst3(science_constant)));
	constants_->pushButton3->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu, SIGNAL(triggeredConstant(science_constant)), this, SLOT(slotChooseScientificConst4(science_constant)));
	constants_->pushButton4->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu, SIGNAL(triggeredConstant(science_constant)), this, SLOT(slotChooseScientificConst5(science_constant)));
	constants_->pushButton5->setMenu(tmp_menu);

	dialog->addPage(constants_, i18n("Constants"), QLatin1String("preferences-kcalc-constants_"), i18n("Define Constants"));

	// When the user clicks OK or Apply we want to update our settings.
	connect(dialog, SIGNAL(settingsChanged(QString)), SLOT(updateSettings()));

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
		statusBar()->setItemFixed(AngleField, -1);
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

//------------------------------------------------------------------------------
// Name: showLogicButtons
// Desc: hides or shows the logic buttons
//------------------------------------------------------------------------------
void KCalculator::showLogicButtons(bool toggled) {

	if (toggled) {
		mBitset->setEnabled(true);
		connect(mBitset, SIGNAL(valueChanged(quint64)), this, SLOT(slotBitsetChanged(quint64)));
		connect(calc_display, SIGNAL(changedAmount(KNumber)), SLOT(slotUpdateBitset(KNumber)));

		foreach(QAbstractButton* btn, logic_buttons_) {
			btn->show();
		}

		setBase();
		statusBar()->setItemFixed(BaseField, -1);

		foreach(QAbstractButton *btn, base_choose_group_->buttons()) {
			btn->show();
		}

		for (int i = 10; i < 16; ++i) {
			(num_button_group_->button(i))->show();
		}
	} else {
		mBitset->setEnabled(false);
		disconnect(mBitset, SIGNAL(valueChanged(quint64)), this, SLOT(slotBitsetChanged(quint64)));
		disconnect(calc_display, SIGNAL(changedAmount(KNumber)), this, SLOT(slotUpdateBitset(KNumber)));

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
		connect(calc_display, SIGNAL(changedText(QString)), SLOT(setCaption(QString)));
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

	const QString sheet = QLatin1String("KPushButton { background-color: %1 }");

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
#include "kcalc.moc"

//------------------------------------------------------------------------------
// Name: kdemain
// Desc: entry point of the application
//------------------------------------------------------------------------------
extern "C" KDE_EXPORT int kdemain(int argc, char *argv[]) {

	KAboutData aboutData("kcalc",
		0,
		ki18n("KCalc"),
		version,
		ki18n(description),
		KAboutData::License_GPL,
		ki18n(
			"&copy; 2008-2012, Evan Teran\n"
			"&copy; 2000-2008, The KDE Team\n"
			"&copy; 2003-2005, Klaus Niederkr" "\xc3\xbc" "ger\n"
			"&copy; 1996-2000, Bernd Johannes Wuebben"),
		KLocalizedString(),
		"http://utils.kde.org/projects/kcalc");

	// Klaus Niederkrueger
	aboutData.addAuthor(ki18n("Klaus Niederkr" "\xc3\xbc" "ger"), KLocalizedString(), "kniederk@math.uni-koeln.de");
	aboutData.addAuthor(ki18n("Bernd Johannes Wuebben"), KLocalizedString(), "wuebben@kde.org");
	aboutData.addAuthor(ki18n("Evan Teran"), ki18n("Maintainer"), "eteran@alum.rit.edu");
	aboutData.addAuthor(ki18n("Espen Sand"), KLocalizedString(), "espen@kde.org");
	aboutData.addAuthor(ki18n("Chris Howells"), KLocalizedString(), "howells@kde.org");
	aboutData.addAuthor(ki18n("Aaron J. Seigo"), KLocalizedString(), "aseigo@olympusproject.org");
	aboutData.addAuthor(ki18n("Charles Samuels"), KLocalizedString(), "charles@altair.dhs.org");
	// Rene Merou
	aboutData.addAuthor(ki18n("Ren" "\xc3\xa9" " M" "\xc3\xa9"  "rou"), KLocalizedString(), "ochominutosdearco@yahoo.es");
	aboutData.addAuthor(ki18n("Michel Marti"), KLocalizedString(), "mma@objectxp.com");
	aboutData.addAuthor(ki18n("David Johnson"), KLocalizedString(), "david@usermode.org");

	aboutData.setProgramIconName(QLatin1String("accessories-calculator"));

	KCmdLineArgs::init(argc, argv, &aboutData);

	KApplication app;

	// force system locale to "C" internally [bug 159168]
	setlocale(LC_NUMERIC, "C");

	KNumber::setGroupSeparator(KGlobal::locale()->thousandsSeparator());
	KNumber::setDecimalSeparator(KGlobal::locale()->decimalSymbol());

	KCalculator *calc = new KCalculator(0);
	app.setTopWidget(calc);

	calc->show();
	return app.exec();
}
