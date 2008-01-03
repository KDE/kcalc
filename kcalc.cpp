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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


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
#include <kfontdialog.h>
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


#include "dlabel.h"
#include "kcalc.h"
#include "kcalc_const_menu.h"
#include "version.h"
#include "kcalc_settings.h"
#include "kcalc_bitset.h"


static const char description[] = I18N_NOOP("KDE Calculator");
static const char version[] = KCALCVERSION;

static const int maxprecision = 1000;

KCalculator::KCalculator(QWidget *parent)
	: KXmlGuiWindow(parent), inverse(false), hyp_mode(false),
	  memory_num(0.0), constants(0), core()
{
	/* central widget to contain all the elements */
	QWidget *central = new QWidget(this);
	setCentralWidget(central);
	KAcceleratorManager::setNoAccel( central );

	// load science constants from xml-file
	KCalcConstMenu::init_consts();

	// setup interface (order is critical)

	setupUi(central);

	setupMainActions();

	setupStatusbar();

	createGUI();

	setupKeys();

	toolBar()->hide(); // hide by default

	// create button groups

	BaseChooseGroup = new QButtonGroup(this);
	BaseChooseGroup->setExclusive(true);
	BaseChooseGroup->addButton(hexRadio, 0);
	BaseChooseGroup->addButton(decRadio, 1);
	BaseChooseGroup->addButton(octRadio, 2);
	BaseChooseGroup->addButton(binRadio, 3);
	connect(BaseChooseGroup, SIGNAL(buttonClicked(int)),
			SLOT(slotBaseSelected(int)));

	AngleChooseGroup =  new QButtonGroup(this);
	AngleChooseGroup->setExclusive(true);
	AngleChooseGroup->addButton(degRadio, 0);
	AngleChooseGroup->addButton(radRadio, 1);
	AngleChooseGroup->addButton(gradRadio, 2);
	connect(AngleChooseGroup, SIGNAL(buttonClicked(int)),
			SLOT(slotAngleSelected(int)));

	// additional menu setup

	KCalcConstMenu *tmp_menu = new KCalcConstMenu(i18n("&Constants"), this);
	menuBar()->insertMenu((menuBar()->actions)()[2], tmp_menu);
	connect(tmp_menu, SIGNAL(triggeredConstant(science_constant const &)),
			this, SLOT(slotConstantToDisplay(science_constant const &)));

	// misc setup

	set_colors();

	// Show the result in the app's caption in taskbar (wishlist - bug #52858)
	if (KCalcSettings::captionResult() == true)
		connect(calc_display, SIGNAL(changedText(const QString &)),
				SLOT(setCaption(const QString &)));
	calc_display->changeSettings();
	set_precision();

	resetBase(); // switch to decimal

	updateGeometry();

	setFixedSize(minimumSize());

	updateDisplay(true);

	// Read and set button groups

	actionStatshow->setChecked(KCalcSettings::showStat());
	slotStatshow(KCalcSettings::showStat());

	actionScientificshow->setChecked(KCalcSettings::showScientific());
	slotScientificshow(KCalcSettings::showScientific());

	actionLogicshow->setChecked(KCalcSettings::showLogic());
	slotLogicshow(KCalcSettings::showLogic());

	actionConstantsShow->setChecked(KCalcSettings::showConstants());
	slotConstantsShow(KCalcSettings::showConstants());

	// connections

	connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
			SLOT(set_colors()));
}

KCalculator::~KCalculator()
{
	KCalcSettings::self()->writeConfig();
}

void KCalculator::setupMainActions(void)
{
	// file menu
	KStandardAction::quit(this, SLOT(close()), actionCollection());

	// edit menu
	KStandardAction::undo(calc_display, SLOT(history_back()), actionCollection());
	KStandardAction::redo(calc_display, SLOT(history_forward()), actionCollection());
	KStandardAction::cut(calc_display, SLOT(slotCut()), actionCollection());
	KStandardAction::copy(calc_display, SLOT(slotCopy()), actionCollection());
	KStandardAction::paste(calc_display, SLOT(slotPaste()), actionCollection());

	// settings menu
	actionStatshow = actionCollection()->add<KToggleAction>( "show_stat" );
	actionStatshow->setText( i18n("&Statistic Buttons") );
	actionStatshow->setChecked(true);
	connect(actionStatshow, SIGNAL(toggled(bool)),
			SLOT(slotStatshow(bool)));

	actionScientificshow = actionCollection()->add<KToggleAction>( "show_science" );
	actionScientificshow->setText( i18n("Science/&Engineering Buttons") );
	actionScientificshow->setChecked(true);
	connect(actionScientificshow, SIGNAL(toggled(bool)),
			SLOT(slotScientificshow(bool)));

	actionLogicshow = actionCollection()->add<KToggleAction>( "show_logic");
	actionLogicshow->setText( i18n("&Logic Buttons") );
	actionLogicshow->setChecked(true);
	connect(actionLogicshow, SIGNAL(toggled(bool)),
			SLOT(slotLogicshow(bool)));

	actionConstantsShow=actionCollection()->add<KToggleAction>( "show_constants" );
	actionConstantsShow->setText( i18n("&Constants Buttons") );
	actionConstantsShow->setChecked(true);
	connect(actionConstantsShow, SIGNAL(toggled(bool)),
			SLOT(slotConstantsShow(bool)));

	QAction *showAct = actionCollection()->addAction( "show_all" );
	showAct->setText( i18n("&Show All") );
	connect(showAct, SIGNAL(triggered()), this, SLOT(slotShowAll()));

	QAction *hideAct = actionCollection()->addAction("hide_all");
	hideAct->setText(i18n("&Hide All"));
	connect(hideAct, SIGNAL(triggered()), this, SLOT(slotHideAll()));

	KStandardAction::preferences(this, SLOT(showSettings()), actionCollection());
	KStandardAction::keyBindings(guiFactory(), SLOT(configureShortcuts()),
actionCollection());
}

void KCalculator::setupStatusbar(void)
{
	// Status bar contents
	statusBar()->insertPermanentFixedItem(" NORM ", 0);
	statusBar()->setItemAlignment(0, Qt::AlignCenter);

	statusBar()->insertPermanentFixedItem(" HEX ", 1);
	statusBar()->setItemAlignment(1, Qt::AlignCenter);

	statusBar()->insertPermanentFixedItem(" DEG ", 2);
	statusBar()->setItemAlignment(2, Qt::AlignCenter);

	statusBar()->insertPermanentFixedItem(" \xa0\xa0 ", 3); // Memory indicator
	statusBar()->setItemAlignment(3, Qt::AlignCenter);
}

// additional setup for button keys

void KCalculator::setupKeys()
{
	// numbers

	NumButtonGroup = new QButtonGroup(this);
	connect(NumButtonGroup, SIGNAL(buttonClicked(int)),
			SLOT(slotNumberclicked(int)));

	NumButtonGroup->addButton(pb0, 0);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pb0, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(pb1, 1);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pb1, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(pb2, 2);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pb2, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(pb3, 3);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pb3, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(pb4, 4);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pb4, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(pb5, 5);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pb5, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(pb6, 6);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pb6, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(pb7, 7);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pb7, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(pb8, 8);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pb8, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(pb9, 9);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pb9, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(pbA, 0xA);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbA, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(pbB, 0xB);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbB, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(pbC, 0xC);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbC, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(pbD, 0xD);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbD, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(pbE, 0xE);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbE, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(pbF, 0xF);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbF, SLOT(slotSetAccelDisplayMode(bool)));

	// right keypad

    pbInv->installEventFilter(this);
	connect(pbInv, SIGNAL(toggled(bool)),
			SLOT(slotInvtoggled(bool)));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbInv, SLOT(slotSetAccelDisplayMode(bool)));

	new QShortcut(Qt::Key_Escape, pbClear, SLOT(animateClick()));
	connect(pbClear, SIGNAL(clicked(void)),
			SLOT(slotClearclicked(void)));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbClear, SLOT(slotSetAccelDisplayMode(bool)));

	pbAllClear->setShortcut(QKeySequence(Qt::Key_PageDown));
	connect(pbAllClear, SIGNAL(clicked(void)),
			SLOT(slotAllClearclicked(void)));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbAllClear, SLOT(slotSetAccelDisplayMode(bool)));

	pbParenOpen->setShortcut(QKeySequence(Qt::Key_ParenLeft));
	connect(pbParenOpen, SIGNAL(clicked(void)),
			SLOT(slotParenOpenclicked(void)));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbParenOpen, SLOT(slotSetAccelDisplayMode(bool)));

	pbParenClose->setShortcut(QKeySequence(Qt::Key_ParenRight));
	connect(pbParenClose, SIGNAL(clicked(void)),
			SLOT(slotParenCloseclicked(void)));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbParenClose, SLOT(slotSetAccelDisplayMode(bool)));

	pbMemRecall->setDisabled(true); // nothing in memory at start
	connect(pbMemRecall, SIGNAL(clicked(void)),
			SLOT(slotMemRecallclicked(void)));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbMemRecall, SLOT(slotSetAccelDisplayMode(bool)));

	connect(pbMemClear, SIGNAL(clicked(void)),
			SLOT(slotMemClearclicked(void)));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbMemClear, SLOT(slotSetAccelDisplayMode(bool)));

	pbMemPlusMinus->addMode(ModeNormal, "M+", i18n("Add display to memory"));
	pbMemPlusMinus->addMode(ModeInverse, "M-", i18n("Subtract from memory"));
	connect(pbMemPlusMinus,SIGNAL(clicked(void)),
			SLOT(slotMemPlusMinusclicked(void)));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbMemPlusMinus, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			pbMemPlusMinus, SLOT(slotSetMode(ButtonModeFlags,bool)));

	connect(pbMemStore, SIGNAL(clicked(void)),
			SLOT(slotMemStoreclicked(void)));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbMemStore, SLOT(slotSetAccelDisplayMode(bool)));

	pbPercent->setShortcut(QKeySequence(Qt::Key_Percent));
	connect(pbPercent, SIGNAL(clicked(void)),
			SLOT(slotPercentclicked(void)));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbPercent, SLOT(slotSetAccelDisplayMode(bool)));

	pbPlusMinus->setShortcut(QKeySequence(Qt::Key_Backslash));
	connect(pbPlusMinus, SIGNAL(clicked(void)),
			SLOT(slotPlusMinusclicked(void)));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbPlusMinus, SLOT(slotSetAccelDisplayMode(bool)));

	// numeric keypad

	pbRoot->addMode(ModeNormal, QString(), i18n("Square root"), false,
					KIcon("math_sqrt"));
	pbRoot->addMode(ModeInverse, QString(), i18n("Cube root"), false,
					KIcon("math_cbrt"));
	connect(pbRoot, SIGNAL(clicked(void)),
			SLOT(slotRootclicked(void)));
	connect(this, SIGNAL(switchShowAccels(bool)),
	        pbRoot, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags, bool)),
	        pbRoot, SLOT(slotSetMode(ButtonModeFlags, bool)));

	pbDivision->setShortcut(QKeySequence(Qt::Key_Slash));
	new QShortcut( Qt::Key_division, pbDivision, SLOT(animateClick()) );
	connect(pbDivision, SIGNAL(clicked(void)),
			SLOT(slotDivisionclicked(void)));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbDivision, SLOT(slotSetAccelDisplayMode(bool)));

	pbMultiplication->setShortcut(QKeySequence(Qt::Key_multiply));
	new QShortcut( Qt::Key_Asterisk, pbMultiplication, SLOT(animateClick()) );
	connect(pbMultiplication, SIGNAL(clicked(void)),
			SLOT(slotMultiplicationclicked(void)));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbMultiplication, SLOT(slotSetAccelDisplayMode(bool)));

	pbMinus->setShortcut(QKeySequence(Qt::Key_Minus));
	connect(pbMinus, SIGNAL(clicked(void)),
			SLOT(slotMinusclicked(void)));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbMinus, SLOT(slotSetAccelDisplayMode(bool)));

	pbPlus->setShortcut(QKeySequence(Qt::Key_Plus));
	connect(pbPlus, SIGNAL(clicked(void)),
			SLOT(slotPlusclicked(void)));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbPlus, SLOT(slotSetAccelDisplayMode(bool)));

	pbPeriod->setShortcut(QKeySequence(Qt::Key_Period));
	new QShortcut( Qt::Key_Comma, pbPeriod, SLOT(animateClick()) );
	pbPeriod->setText(KGlobal::locale()->decimalSymbol());
	connect(pbPeriod, SIGNAL(clicked(void)),
			SLOT(slotPeriodclicked(void)));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbPeriod, SLOT(slotSetAccelDisplayMode(bool)));

	pbEqual->setShortcut(QKeySequence(Qt::Key_Enter));
	new QShortcut( Qt::Key_Equal, pbEqual, SLOT(animateClick()) );
	new QShortcut( Qt::Key_Return, pbEqual, SLOT(animateClick()) );
	connect(pbEqual, SIGNAL(clicked(void)),
			SLOT(slotEqualclicked(void)));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbEqual, SLOT(slotSetAccelDisplayMode(bool)));

	// logic keys

	logicButtons.append(pbAND);
	logicButtons.append(pbOR);
	logicButtons.append(pbXOR);
	logicButtons.append(pbLsh);
	logicButtons.append(pbRsh);
	logicButtons.append(pbCmp);

	pbAND->setShortcut(QKeySequence(Qt::Key_Ampersand));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbAND, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbAND, SIGNAL(clicked(void)), SLOT(slotANDclicked(void)));

	pbOR->setShortcut(QKeySequence(Qt::Key_Bar));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbOR, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbOR, SIGNAL(clicked(void)), SLOT(slotORclicked(void)));

	connect(this, SIGNAL(switchShowAccels(bool)),
			pbXOR, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbXOR, SIGNAL(clicked(void)), SLOT(slotXORclicked(void)));

	pbLsh->setShortcut(QKeySequence(Qt::Key_Less));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbLsh, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbLsh, SIGNAL(clicked(void)),
			SLOT(slotLeftShiftclicked(void)));

	pbRsh->setShortcut(QKeySequence(Qt::Key_Greater));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbRsh, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbRsh, SIGNAL(clicked(void)),
			SLOT(slotRightShiftclicked(void)));

	pbCmp->setShortcut(QKeySequence(Qt::Key_AsciiTilde));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbCmp, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbCmp, SIGNAL(clicked(void)), SLOT(slotNegateclicked(void)));

	// scientific keys

	scientificButtons.append(pbHyp);
	scientificButtons.append(pbSin);
	scientificButtons.append(pbCos);
	scientificButtons.append(pbTan);
	scientificButtons.append(pbLog);
	scientificButtons.append(pbLn);

	connect(this, SIGNAL(switchShowAccels(bool)),
			pbHyp, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbHyp, SIGNAL(toggled(bool)), SLOT(slotHyptoggled(bool)));

	pbSin->addMode(ModeNormal, "Sin", i18n("Sine"));
	pbSin->addMode(ModeInverse, "Asin", i18n("Arc sine"));
	pbSin->addMode(ModeHyperbolic, "Sinh", i18n("Hyperbolic sine"));
	pbSin->addMode(ButtonModeFlags(ModeInverse | ModeHyperbolic),
				   "Asinh", i18n("Inverse hyperbolic sine"));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbSin, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			pbSin, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbSin, SIGNAL(clicked(void)), SLOT(slotSinclicked(void)));

	pbCos->addMode(ModeNormal, "Cos", i18n("Cosine"));
	pbCos->addMode(ModeInverse, "Acos", i18n("Arc cosine"));
	pbCos->addMode(ModeHyperbolic, "Cosh", i18n("Hyperbolic cosine"));
	pbCos->addMode(ButtonModeFlags(ModeInverse | ModeHyperbolic),
				   "Acosh", i18n("Inverse hyperbolic cosine"));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbCos, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			pbCos, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbCos, SIGNAL(clicked(void)), SLOT(slotCosclicked(void)));

	pbTan->addMode(ModeNormal, "Tan", i18n("Tangent"));
	pbTan->addMode(ModeInverse, "Atan", i18n("Arc tangent"));
	pbTan->addMode(ModeHyperbolic, "Tanh", i18n("Hyperbolic tangent"));
	pbTan->addMode(ButtonModeFlags(ModeInverse | ModeHyperbolic),
				   "Atanh", i18n("Inverse hyperbolic tangent"));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbTan, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			pbTan, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbTan, SIGNAL(clicked(void)),SLOT(slotTanclicked(void)));

	pbLog->addMode(ModeNormal, "Log", i18n("Logarithm to base 10"));
	pbLog->addMode(ModeInverse, "10<sup>x</sup>", i18n("10 to the power of x"),
				   true);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbLog, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			pbLog, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbLog, SIGNAL(clicked(void)), SLOT(slotLogclicked(void)));
	pbLn->addMode(ModeNormal, "Ln", i18n("Natural log"));
	pbLn->addMode(ModeInverse, "e<sup>x</sup>", i18n("Exponential function"),
				  true);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbLn, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			pbLn, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbLn, SIGNAL(clicked(void)), SLOT(slotLnclicked(void)));

	// statistic buttons

	statButtons.append(pbNData);
	statButtons.append(pbMean);
	statButtons.append(pbSd);
	statButtons.append(pbMed);
	statButtons.append(pbDat);
	statButtons.append(pbCSt);

	pbNData->addMode(ModeNormal, "N", i18n("Number of data entered"));
	pbNData->addMode(ModeInverse, QString::fromUtf8("\xce\xa3")
					 + 'x', i18n("Sum of all data items"));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbNData, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			pbNData, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbNData, SIGNAL(clicked(void)), SLOT(slotStatNumclicked(void)));

	pbMean->addMode(ModeNormal, "Mea", i18n("Mean"));
	pbMean->addMode(ModeInverse, QString::fromUtf8("\xce\xa3")
					+ "x<sup>2</sup>",
					i18n("Sum of all data items squared"), true);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbMean, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			pbMean, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbMean, SIGNAL(clicked(void)), SLOT(slotStatMeanclicked(void)));

	pbSd->addMode(ModeNormal, QString::fromUtf8("σ",-1) + "<sub>N</sub>",
				  i18n("Standard deviation"), true);
	pbSd->addMode(ModeInverse, QString::fromUtf8("σ",-1) + "<sub>N-1</sub>",
				  i18n("Sample standard deviation"), true);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbSd, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			pbSd, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbSd, SIGNAL(clicked(void)), SLOT(slotStatStdDevclicked(void)));

	connect(this, SIGNAL(switchShowAccels(bool)),
			pbMed, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbMed, SIGNAL(clicked(void)), SLOT(slotStatMedianclicked(void)));

	pbDat->addMode(ModeNormal, "Dat", i18n("Enter data"));
	pbDat->addMode(ModeInverse, "CDat", i18n("Delete last data item"));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbDat, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			pbDat, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbDat, SIGNAL(clicked(void)), SLOT(slotStatDataInputclicked(void)));

	connect(this, SIGNAL(switchShowAccels(bool)),
			pbCSt, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbCSt, SIGNAL(clicked(void)), SLOT(slotStatClearDataclicked(void)));

	// constants keys

	constButtons.append(pbC1);
	constButtons.append(pbC2);
	constButtons.append(pbC3);
	constButtons.append(pbC4);
	constButtons.append(pbC5);
	constButtons.append(pbC6);

	pbC1->setButtonNumber(0);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbC1, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			pbC1, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbC1, SIGNAL(clicked(int)), this, SLOT(slotConstclicked(int)));

	pbC2->setButtonNumber(1);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbC2, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			pbC2, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbC2, SIGNAL(clicked(int)), this, SLOT(slotConstclicked(int)));

	pbC3->setButtonNumber(2);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbC3, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			pbC3, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbC3, SIGNAL(clicked(int)), this, SLOT(slotConstclicked(int)));

	pbC4->setButtonNumber(3);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbC4, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			pbC4, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbC4, SIGNAL(clicked(int)), this, SLOT(slotConstclicked(int)));

	pbC5->setButtonNumber(4);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbC5, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			pbC5, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbC5, SIGNAL(clicked(int)), this, SLOT(slotConstclicked(int)));

	pbC6->setButtonNumber(5);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbC6, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			pbC6, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbC6, SIGNAL(clicked(int)), this, SLOT(slotConstclicked(int)));

	changeButtonNames();

	// misc buttons

	pbMod->addMode(ModeNormal, "Mod", i18n("Modulo"));
	pbMod->addMode(ModeInverse, "IntDiv", i18n("Integer division"));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			pbMod, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbMod, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbMod, SIGNAL(clicked(void)), SLOT(slotModclicked(void)));

	pbReci->addMode(ModeNormal, "1/x", i18n("Reciprocal"));
	pbReci->addMode(ModeInverse, "nCm", i18n("n Choose m")); 
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)), 
			pbReci, SLOT(slotSetMode(ButtonModeFlags,bool))); 
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbReci, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbReci, SIGNAL(clicked(void)), SLOT(slotReciclicked(void)));

	pbFactorial->addMode(ModeNormal, "x!", i18n("Factorial"));
	pbFactorial->setShortcut(QKeySequence(Qt::Key_Exclam));
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbFactorial, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbFactorial, SIGNAL(clicked(void)),SLOT(slotFactorialclicked(void)));

	pbSquare->addMode(ModeNormal, "x<sup>2</sup>", i18n("Square"), true);
	pbSquare->addMode(ModeInverse, "x<sup>3</sup>", i18n("Third power"), true);
	pbSquare->setShortcut(QKeySequence(Qt::Key_BracketLeft));
	new QShortcut( Qt::Key_twosuperior, pbSquare, SLOT(animateClick()) );
	connect(this, SIGNAL(switchShowAccels(bool)),
	        pbSquare, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
	        pbSquare, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbSquare, SIGNAL(clicked(void)), SLOT(slotSquareclicked(void)));

	pbPower->addMode(ModeNormal, "x<sup>y</sup>", i18n("x to the power of y"), true);
	pbPower->addMode(ModeInverse, "x<sup>1/y</sup>", i18n("x to the power of 1/y"), true);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbPower, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			pbPower, SLOT(slotSetMode(ButtonModeFlags,bool)));
	pbPower->setShortcut(QKeySequence(Qt::Key_AsciiCircum));
	connect(pbPower, SIGNAL(clicked(void)), SLOT(slotPowerclicked(void)));

	pbEE->addMode(ModeNormal, "x<small>" "\xb7" "10</small><sup>y</sup>",
				  i18n("Exponent"), true);
	connect(this, SIGNAL(switchShowAccels(bool)),
			pbEE, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbEE, SIGNAL(clicked(void)), SLOT(slotEEclicked(void)));

	// other button lists

	mFunctionButtonList.append(pbHyp);
	mFunctionButtonList.append(pbInv);
	mFunctionButtonList.append(pbEE);
	mFunctionButtonList.append(pbSin);
	mFunctionButtonList.append(pbPlusMinus);
	mFunctionButtonList.append(pbCos);
	mFunctionButtonList.append(pbReci);
	mFunctionButtonList.append(pbTan);
	mFunctionButtonList.append(pbFactorial);
	mFunctionButtonList.append(pbLog);
	mFunctionButtonList.append(pbSquare);
	mFunctionButtonList.append(pbLn);
	mFunctionButtonList.append(pbPower);

	mMemButtonList.append(pbMemRecall);
	mMemButtonList.append(pbMemPlusMinus);
	mMemButtonList.append(pbMemStore);
	mMemButtonList.append(pbMemClear);
	mMemButtonList.append(pbClear);
	mMemButtonList.append(pbAllClear);

	mOperationButtonList.append(pbMultiplication);
	mOperationButtonList.append(pbParenOpen);
	mOperationButtonList.append(pbParenClose);
	mOperationButtonList.append(pbAND);
	mOperationButtonList.append(pbDivision);
	mOperationButtonList.append(pbOR);
	mOperationButtonList.append(pbXOR);
	mOperationButtonList.append(pbPlus);
	mOperationButtonList.append(pbMinus);
	mOperationButtonList.append(pbLsh);
	mOperationButtonList.append(pbRsh);
	mOperationButtonList.append(pbPeriod);
	mOperationButtonList.append(pbEqual);
	mOperationButtonList.append(pbPercent);
	mOperationButtonList.append(pbCmp);
	mOperationButtonList.append(pbMod);
	mOperationButtonList.append(pbRoot);
}

void KCalculator::updateGeometry(void)
{
	QWidget *widget;
	QSize em = fontMetrics().size(0, "M");
    int margin =
		QApplication::style()->pixelMetric(QStyle::PM_ButtonMargin, 0, 0) + 1;

	// left pad

	foreach (QObject *obj, leftPad->children()) {
        if ( obj->isWidgetType() ) {
            widget = static_cast<QWidget*>(obj);
            widget->setFixedWidth(em.width()*4 + margin*2);
            widget->installEventFilter( this );
        }
    }

	// right pad

	foreach (QObject *obj, rightPad->children()) {
        if (obj->isWidgetType()) {
            widget = static_cast<QWidget*>(obj);
            widget->setFixedWidth(em.width()*3 + margin*2);
            widget->installEventFilter(this);
        }
    }

	foreach (QObject *obj, numericPad->children()) {
        if (obj->isWidgetType()) {
            widget = static_cast<QWidget *>(obj);
			if (widget != pb0) { // let pb0 expand freely
				widget->setFixedWidth(em.width()*3 + margin*2);
			}
            widget->installEventFilter(this);
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
	switch(base) {
	  case 3:
		  current_base = calc_display->setBase(NumBase(2));
		  if (statusBar()->hasItem(1)) statusBar()->changeItem("BIN",1);
		  calc_display->setStatusText(1, "Bin");
		  break;
	  case 2:
		  current_base = calc_display->setBase(NumBase(8));
		  if (statusBar()->hasItem(1)) statusBar()->changeItem("OCT",1);
		  calc_display->setStatusText(1, "Oct");
		  break;
	  case 1:
		  current_base = calc_display->setBase(NumBase(10));
		  if (statusBar()->hasItem(1)) statusBar()->changeItem("DEC",1);
		  calc_display->setStatusText(1, "Dec");
		  break;
	  case 0:
		  current_base = calc_display->setBase(NumBase(16));
		  if (statusBar()->hasItem(1)) statusBar()->changeItem("HEX",1);
		  calc_display->setStatusText(1, "Hex");
		  break;
	  default:
		  if (statusBar()->hasItem(1)) statusBar()->changeItem("Error",1);
		  calc_display->setStatusText(1, "Error");
		  return;
	}

	// Enable the buttons not available in this base
	for (int i=0; i<current_base; i++)
		(NumButtonGroup->buttons()[i])->setEnabled (true);
  
	// Disable the buttons not available in this base
	for (int i=current_base; i<16; i++)
		(NumButtonGroup->buttons()[i])->setEnabled (false);
  
	// Only enable the decimal point in decimal
	pbPeriod->setEnabled(current_base == NB_DECIMAL);
	// Only enable the x*10^y button in decimal
	pbEE->setEnabled(current_base == NB_DECIMAL);
  
	// Disable buttons that make only sense with floating point numbers
	if(current_base != NB_DECIMAL)  {
		foreach (QAbstractButton *btn, scientificButtons) {
			btn->setEnabled(false);
		}
	} else {
		foreach (QAbstractButton *btn, scientificButtons) {
			btn->setEnabled(true);
		}
	}
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

void KCalculator::slotAngleSelected(int number)
{
	switch(number)
	{
	case 0:
		_angle_mode = DegMode;
		statusBar()->changeItem("DEG", 2);
		calc_display->setStatusText(2, "Deg");
		break;
	case 1:
		_angle_mode = RadMode;
		statusBar()->changeItem("RAD", 2);
		calc_display->setStatusText(2, "Rad");
		break;
	case 2:
		_angle_mode = GradMode;
		statusBar()->changeItem("GRA", 2);
		calc_display->setStatusText(2, "Gra");
		break;
	default: // we shouldn't ever end up here
		_angle_mode = RadMode;
	}
}

void KCalculator::slotEEclicked(void)
{
	calc_display->newCharacter('e');
}

void KCalculator::slotInvtoggled(bool flag)
{
	inverse = flag;

	emit switchMode(ModeInverse, flag);

	if (inverse)
	{
		statusBar()->changeItem("INV", 0);
		calc_display->setStatusText(0, "Inv");
	}
	else
	{
		statusBar()->changeItem("NORM", 0);
		calc_display->setStatusText(0, QString());
	}
}

void KCalculator::slotHyptoggled(bool flag)
{
	// toggle between hyperbolic and standart trig functions
	hyp_mode = flag;

	emit switchMode(ModeHyperbolic, flag);
}



void KCalculator::slotMemRecallclicked(void)
{
	// temp. work-around
	calc_display->sendEvent(KCalcDisplay::EventReset);

	calc_display->setAmount(memory_num);
	updateDisplay(false);
}

void KCalculator::slotMemStoreclicked(void)
{
	EnterEqual();

	memory_num = calc_display->getAmount();
	calc_display->setStatusText(3, "M");
	statusBar()->changeItem("M",3);
	pbMemRecall->setEnabled(true);
}

void KCalculator::slotNumberclicked(int number_clicked)
{
	calc_display->EnterDigit(number_clicked);
}

void KCalculator::slotSinclicked(void)
{
	if (hyp_mode)
	{
		// sinh or arsinh
		if (!inverse)
			core.SinHyp(calc_display->getAmount());
		else
			core.AreaSinHyp(calc_display->getAmount());
	}
	else
	{
		// sine or arcsine
		if (!inverse)
			switch(_angle_mode)
			{
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
			switch(_angle_mode)
			{
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

void KCalculator::slotPlusMinusclicked(void)
{
	// display can only change sign, when in input mode, otherwise we
	// need the core to do this.
	if (!calc_display->sendEvent(KCalcDisplay::EventChangeSign))
	{
	    core.InvertSign(calc_display->getAmount());
	    updateDisplay(true);
	}
}

void KCalculator::slotMemPlusMinusclicked(void)
{
	bool tmp_inverse = inverse; // store this, because next command deletes inverse
	EnterEqual(); // finish calculation so far, to store result into MEM

	if (!tmp_inverse)	memory_num += calc_display->getAmount();
	else 			memory_num -= calc_display->getAmount();

	pbInv->setChecked(false);
	statusBar()->changeItem("M",3);
	calc_display->setStatusText(3, "M");
	pbMemRecall->setEnabled(true);
}

void KCalculator::slotCosclicked(void)
{
	if (hyp_mode)
	{
		// cosh or arcosh
		if (!inverse)
			core.CosHyp(calc_display->getAmount());
		else
			core.AreaCosHyp(calc_display->getAmount());
	}
	else
	{
		// cosine or arccosine
		if (!inverse)
			switch(_angle_mode)
			{
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
			switch(_angle_mode)
			{
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

void KCalculator::slotReciclicked(void)
{
	if (inverse) 
	{ 
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

void KCalculator::slotTanclicked(void)
{
	if (hyp_mode)
	{
		// tanh or artanh
		if (!inverse)
			core.TangensHyp(calc_display->getAmount());
		else
			core.AreaTangensHyp(calc_display->getAmount());
	}
	else
	{
		// tan or arctan
		if (!inverse)
			switch(_angle_mode)
			{
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
			switch(_angle_mode)
			{
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

void KCalculator::slotFactorialclicked(void)
{
	core.Factorial(calc_display->getAmount());

	updateDisplay(true);
}

void KCalculator::slotLogclicked(void)
{
	if (!inverse)
		core.Log10(calc_display->getAmount());
	else
		core.Exp10(calc_display->getAmount());

	updateDisplay(true);
}


void KCalculator::slotSquareclicked(void)
{
	if (!inverse)
		core.Square(calc_display->getAmount());
	else
		core.Cube(calc_display->getAmount());

	updateDisplay(true);
}

void KCalculator::slotRootclicked(void)
{
	if (!inverse)
		core.SquareRoot(calc_display->getAmount());
	else
		core.CubeRoot(calc_display->getAmount());

	updateDisplay(true);
}

void KCalculator::slotLnclicked(void)
{
	if (!inverse)
		core.Ln(calc_display->getAmount());
	else
		core.Exp(calc_display->getAmount());

	updateDisplay(true);
}

void KCalculator::slotPowerclicked(void)
{
	if (inverse)
	{
		core.enterOperation(calc_display->getAmount(),
				    CalcEngine::FUNC_PWR_ROOT);
		pbInv->setChecked(false);
	}
	else
	{
		core.enterOperation(calc_display->getAmount(),
				    CalcEngine::FUNC_POWER);
	}
	// temp. work-around
	KNumber tmp_num = calc_display->getAmount();
	calc_display->sendEvent(KCalcDisplay::EventReset);
	calc_display->setAmount(tmp_num);
	updateDisplay(false);
}

void KCalculator::slotMemClearclicked(void)
{
	memory_num		= 0;
	statusBar()->changeItem(" \xa0\xa0 ",3);
	calc_display->setStatusText(3, QString());
	pbMemRecall->setDisabled(true);
}

void KCalculator::slotClearclicked(void)
{
	calc_display->sendEvent(KCalcDisplay::EventClear);
}

void KCalculator::slotAllClearclicked(void)
{
	core.Reset();
	calc_display->sendEvent(KCalcDisplay::EventReset);

	updateDisplay(true);
}

void KCalculator::slotParenOpenclicked(void)
{
	core.ParenOpen(calc_display->getAmount());

	// What behavior, if e.g.: "12(6*6)"??
	//updateDisplay(true);
}

void KCalculator::slotParenCloseclicked(void)
{
	core.ParenClose(calc_display->getAmount());

	updateDisplay(true);
}

void KCalculator::slotANDclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_AND);

	updateDisplay(true);
}

void KCalculator::slotMultiplicationclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_MULTIPLY);

	updateDisplay(true);
}

void KCalculator::slotDivisionclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_DIVIDE);

	updateDisplay(true);
}

void KCalculator::slotORclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_OR);

	updateDisplay(true);
}

void KCalculator::slotXORclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_XOR);

	updateDisplay(true);
}

void KCalculator::slotPlusclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_ADD);

	updateDisplay(true);
}

void KCalculator::slotMinusclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_SUBTRACT);

	updateDisplay(true);
}

void KCalculator::slotLeftShiftclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_LSH);

	updateDisplay(true);
}

void KCalculator::slotRightShiftclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_RSH);

	updateDisplay(true);
}

void KCalculator::slotPeriodclicked(void)
{
	calc_display->newCharacter('.');
}

void KCalculator::EnterEqual()
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_EQUAL);

	updateDisplay(true, true);
}

void KCalculator::slotEqualclicked(void)
{
	EnterEqual();
}

void KCalculator::slotPercentclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_PERCENT);

	updateDisplay(true);
}

void KCalculator::slotNegateclicked(void)
{
	core.Complement(calc_display->getAmount());

	updateDisplay(true);
}

void KCalculator::slotModclicked(void)
{
	if (inverse)
		core.enterOperation(calc_display->getAmount(),
				    CalcEngine::FUNC_INTDIV);
	else
		core.enterOperation(calc_display->getAmount(),
				    CalcEngine::FUNC_MOD);

	updateDisplay(true);
}

void KCalculator::slotStatNumclicked(void)
{
	if(!inverse)
	{
		core.StatCount(0);
	}
	else
	{
		pbInv->setChecked(false);
		core.StatSum(0);
	}

	updateDisplay(true);
}

void KCalculator::slotStatMeanclicked(void)
{
	if(!inverse)
		core.StatMean(0);
	else
	{
		pbInv->setChecked(false);
		core.StatSumSquares(0);
	}

	updateDisplay(true);
}

void KCalculator::slotStatStdDevclicked(void)
{
	if(inverse)
	{
		// std (n-1)
		core.StatStdDeviation(0);
		pbInv->setChecked(false);
	}
	else
	{
		// std (n)
		core.StatStdSample(0);
	}

	updateDisplay(true);
}

void KCalculator::slotStatMedianclicked(void)
{
	if(!inverse)
	{
		// std (n-1)
		core.StatMedian(0);
	}
	else
	{
		// std (n)
		core.StatMedian(0);
		pbInv->setChecked(false);
	}
	// it seems two different modes should be implemented, but...?
	updateDisplay(true);
}

void KCalculator::slotStatDataInputclicked(void)
{
	if(!inverse)
	{
		core.StatDataNew(calc_display->getAmount());
	}
	else
	{
		pbInv->setChecked(false);
		core.StatDataDel(0);
		statusBar()->showMessage(i18n("Last stat item erased"), 3000);
	}

	updateDisplay(true);
}

void KCalculator::slotStatClearDataclicked(void)
{
        if(!inverse)
	{
		core.StatClearAll(0);
		statusBar()->showMessage(i18n("Stat mem cleared"), 3000);
	}
	else
	{
		pbInv->setChecked(false);
		updateDisplay(false);
	}
}

void KCalculator::slotConstclicked(int button)
{
	KCalcConstButton *btn = qobject_cast<KCalcConstButton*>(constButtons[button]);
	if (!btn) return;

	if(!inverse)
	{
		// set the display to the configured value of constant button
		calc_display->setAmount(btn->constant());
	}
	else
	{
		pbInv->setChecked(false);
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
	if(KConfigDialog::showDialog("settings"))
		return;

	// Create a new dialog with the same name as the above checking code.
	KConfigDialog *dialog = new KConfigDialog(this, "settings", KCalcSettings::self());
	dialog->showButtonSeparator( true );

	// general settings

	General *general = new General(0);
	general->kcfg_Precision->setMaximum(maxprecision);
	dialog->addPage(general, i18n("General"), "kcalc", i18n("General Settings"));

	// font settings

	KFontChooser *fontChooser = new KFontChooser(0);
	fontChooser->setObjectName("kcfg_Font");
	dialog->addPage(fontChooser, i18n("Font"), "preferences-desktop-font", i18n("Select Display Font"));

	// color settings

	Colors *color = new Colors(0);
	dialog->addPage(color, i18n("Colors"), "format-fill-color", i18n("Button & Display Colors"));

	// constant settings

	if (!constants)
		constants = new Constants(0);

	KCalcConstMenu *tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu,
		SIGNAL(triggeredConstant(science_constant const &)),
		this,
		SLOT(slotChooseScientificConst0(science_constant const &)));
	constants->pushButton0->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu,
		SIGNAL(triggeredConstant(science_constant const &)),
		this,
		SLOT(slotChooseScientificConst1(science_constant const &)));
	constants->pushButton1->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu,
		SIGNAL(triggeredConstant(science_constant const &)),
		this,
		SLOT(slotChooseScientificConst2(science_constant const &)));
	constants->pushButton2->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu,
		SIGNAL(triggeredConstant(science_constant const &)),
		this,
		SLOT(slotChooseScientificConst3(science_constant const &)));
	constants->pushButton3->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu,
		SIGNAL(triggeredConstant(science_constant const &)),
		this,
		SLOT(slotChooseScientificConst4(science_constant const &)));
	constants->pushButton4->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu,
		SIGNAL(triggeredConstant(science_constant const &)),
		this,
		SLOT(slotChooseScientificConst5(science_constant const &)));
	constants->pushButton5->setMenu(tmp_menu);

	dialog->addPage(constants, i18n("Constants"), "preferences-kcalc-constants", i18n("Define constants"));

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
  constants->kcfg_valueConstant0->setText(chosen_const.value);

  constants->kcfg_nameConstant0->setText(chosen_const.label);
}

void KCalculator::slotChooseScientificConst1(struct science_constant const & chosen_const)
{
  constants->kcfg_valueConstant1->setText(chosen_const.value);

  constants->kcfg_nameConstant1->setText(chosen_const.label);
}

void KCalculator::slotChooseScientificConst2(struct science_constant const & chosen_const)
{
  constants->kcfg_valueConstant2->setText(chosen_const.value);

  constants->kcfg_nameConstant2->setText(chosen_const.label);
}

void KCalculator::slotChooseScientificConst3(struct science_constant const & chosen_const)
{
  constants->kcfg_valueConstant3->setText(chosen_const.value);

  constants->kcfg_nameConstant3->setText(chosen_const.label);
}

void KCalculator::slotChooseScientificConst4(struct science_constant const & chosen_const)
{
  constants->kcfg_valueConstant4->setText(chosen_const.value);

  constants->kcfg_nameConstant4->setText(chosen_const.label);
}

void KCalculator::slotChooseScientificConst5(struct science_constant const & chosen_const)
{
  constants->kcfg_valueConstant5->setText(chosen_const.value);

  constants->kcfg_nameConstant5->setText(chosen_const.label);
}


void KCalculator::slotStatshow(bool toggled)
{
	if(toggled)
	{
		foreach (QAbstractButton *btn, statButtons) {
			btn->show();
		}
	}
	else
	{
		foreach (QAbstractButton *btn, statButtons) {
			btn->hide();
		}
	}
	KCalcSettings::setShowStat(toggled);
}

void KCalculator::slotScientificshow(bool toggled)
{
	if(toggled)
	{
		foreach (QAbstractButton* btn, scientificButtons) {
			btn->show();
		}
		foreach (QAbstractButton* btn, AngleChooseGroup->buttons()) {
			btn->show();
		}
		if(!statusBar()->hasItem(2))
			statusBar()->insertFixedItem(" DEG ", 2);
		statusBar()->setItemAlignment(2, Qt::AlignCenter);
		calc_display->setStatusText(2, "Deg");
		degRadio->setChecked(true);
	}
	else
	{
		foreach (QAbstractButton* btn, scientificButtons) {
			btn->hide();
		}
		foreach (QAbstractButton* btn, AngleChooseGroup->buttons()) {
			btn->hide();
		}
		if(statusBar()->hasItem(2))
			statusBar()->removeItem(2);
		calc_display->setStatusText(2, QString());
	}
	KCalcSettings::setShowScientific(toggled);
}

void KCalculator::slotLogicshow(bool toggled)
{
	if(toggled)
	{
		mBitset->show();
		connect(mBitset, SIGNAL(valueChanged(unsigned long long)),
			this, SLOT(slotBitsetChanged(unsigned long long)));
		connect(calc_display, SIGNAL(changedAmount(const KNumber &)),
			SLOT(slotUpdateBitset(const KNumber &)));
		foreach (QAbstractButton* btn, logicButtons) {
			btn->show();
		}
		if(!statusBar()->hasItem(1))
			statusBar()->insertFixedItem(" HEX ", 1);
		statusBar()->setItemAlignment(1, Qt::AlignCenter);
		calc_display->setStatusText(1, "Hex");
		foreach (QAbstractButton *btn, BaseChooseGroup->buttons()) {
			btn->show();
		}
		resetBase();
		for (int i=10; i<16; i++)
			(NumButtonGroup->button(i))->show();
	}
	else
	{
		mBitset->hide();
		disconnect(mBitset, SIGNAL(valueChanged(unsigned long long)),
			this, SLOT(slotBitsetChanged(unsigned long long)));
		disconnect(calc_display, SIGNAL(changedAmount(const KNumber &)),
			this, SLOT(slotUpdateBitset(const KNumber &)));
		foreach (QAbstractButton* btn, logicButtons) {
			btn->hide();
		}
		// Hide Hex-Buttons, but first switch back to decimal
		resetBase();
		foreach (QAbstractButton *btn, BaseChooseGroup->buttons()) {
			btn->hide();
		}
		if(statusBar()->hasItem(1))
			statusBar()->removeItem(1);
		calc_display->setStatusText(1, QString());
		for (int i=10; i<16; i++)
			(NumButtonGroup->button(i))->hide();
	}
	KCalcSettings::setShowLogic(toggled);
}

void KCalculator::slotConstantsShow(bool toggled)
{
	if(toggled)
	{
		foreach (QAbstractButton *btn, constButtons) {
			btn->show();
		}
	}
	else
	{
		foreach (QAbstractButton *btn, constButtons) {
			btn->hide();
		}
	}
	KCalcSettings::setShowConstants(toggled);
}

// This function is for setting the constant names configured in the
// kcalc settings menu. If the user doesn't enter a name for the
// constant C1 to C6 is used.
void KCalculator::changeButtonNames()
{
	KCalcConstButton *constbtn;
	foreach (QAbstractButton *btn, constButtons) {
		constbtn = qobject_cast<KCalcConstButton*>(btn);
		if (constbtn) constbtn->setLabelAndTooltip();
	}
}

void KCalculator::slotShowAll(void)
{
	// I wonder why "setChecked" does not emit "toggled"
	if(!actionStatshow->isChecked()) actionStatshow->trigger();
	if(!actionScientificshow->isChecked()) actionScientificshow->trigger();
	if(!actionLogicshow->isChecked()) actionLogicshow->trigger();
	if(!actionConstantsShow->isChecked()) actionConstantsShow->trigger();
}

void KCalculator::slotHideAll(void)
{
	// I wonder why "setChecked" does not emit "toggled"
	if(actionStatshow->isChecked()) actionStatshow->trigger();
	if(actionScientificshow->isChecked()) actionScientificshow->trigger();
	if(actionLogicshow->isChecked()) actionLogicshow->trigger();
	if(actionConstantsShow->isChecked()) actionConstantsShow->trigger();
}

void KCalculator::slotBitsetChanged(unsigned long long value) {
	calc_display->setAmount(value);
	updateDisplay(false);
}

void KCalculator::slotUpdateBitset(const KNumber &nr) {
	mBitset->setValue(nr);
}

void KCalculator::updateSettings()
{
	changeButtonNames();
	set_colors();
	set_precision();
	// Show the result in the app's caption in taskbar (wishlist - bug #52858)
	disconnect(calc_display, SIGNAL(changedText(const QString &)),
		   this, 0);
	if (KCalcSettings::captionResult())
	{
		connect(calc_display,
			SIGNAL(changedText(const QString &)),
			SLOT(setCaption(const QString &)));
	}
	else
	{
		setCaption(QString::null);	//krazy:exclude=nullstrassign for old broken gcc
	}
	calc_display->changeSettings();

	updateGeometry();
}

void KCalculator::updateDisplay(bool get_amount_from_core,
				bool store_result_in_history)
{
	if(get_amount_from_core)
	{
		calc_display->update_from_core(core, store_result_in_history);
	}
	else
	{
		calc_display->update();
	}

	pbInv->setChecked(false);

}

void KCalculator::set_colors()
{
	calc_display->changeSettings();

	QColor bg = palette().color(QPalette::Active, QPalette::Background);

	QPalette numPal(KCalcSettings::numberButtonsColor(), bg);
	for(int i=0; i<10; i++)
	{
		(NumButtonGroup->button(i))->setPalette(numPal);
	}

	QPalette funcPal(KCalcSettings::functionButtonsColor(), bg);
	foreach (QAbstractButton *btn, mFunctionButtonList)
	{
		btn->setPalette(funcPal);
	}

	QPalette statPal(KCalcSettings::statButtonsColor(), bg);
	foreach (QAbstractButton *btn, statButtons)
	{
		btn->setPalette(statPal);
	}

	QPalette hexPal(KCalcSettings::hexButtonsColor(), bg);
	for(int i=10; i<16; i++)
	{
		(NumButtonGroup->button(i))->setPalette(hexPal);
	}

	QPalette memPal(KCalcSettings::memoryButtonsColor(), bg);
	foreach (QAbstractButton *btn, mMemButtonList)
	{
		btn->setPalette(memPal);
	}

	QPalette opPal(KCalcSettings::operationButtonsColor(), bg);
	foreach (QAbstractButton *btn, mOperationButtonList)
	{
		btn->setPalette(opPal);
	}
}

void KCalculator::set_precision()
{
	KNumber:: setDefaultFloatPrecision(KCalcSettings::precision());
	updateDisplay(false);
}

bool KCalculator::eventFilter(QObject *o, QEvent *e)
{
	switch(e->type())
	{
	case QEvent::DragEnter:
	{
		QDragEnterEvent *ev = (QDragEnterEvent *)e;
		ev->setAccepted( KColorMimeData::canDecode( ev->mimeData() ) );
		return true;
	}
	case QEvent::DragLeave:
	{
		return true;
	}
	case QEvent::Drop:
	{
		KCalcButton* calcButton = qobject_cast<KCalcButton *>(o);
		if(!calcButton)
			return false;

		QDropEvent *ev = (QDropEvent *)e;
		QColor c = KColorMimeData::fromMimeData( ev->mimeData() );
		if ( c.isValid() )
		{
			QList<QAbstractButton*> *list;
			int num_but;
			if((num_but = NumButtonGroup->buttons().indexOf(calcButton)) != -1)
			{
			  QPalette pal(c, palette().color(QPalette::Active, QPalette::Background));
			  // Was it hex-button or normal digit??
			  if (num_but <10)
			    for(int i=0; i<10; i++)
			      (NumButtonGroup->buttons()[i])->setPalette(pal);
			  else
			    for(int i=10; i<16; i++)
			      (NumButtonGroup->buttons()[i])->setPalette(pal);

			  return true;
			}
			else if(mFunctionButtonList.contains(calcButton))
			{
				list = &mFunctionButtonList;
			}
			else if(mStatButtonList.contains(calcButton))
			{
				list = &mStatButtonList;
			}
			else if(mMemButtonList.contains(calcButton))
			{
				list = &mMemButtonList;
			}
			else if( mOperationButtonList.contains(calcButton))
			{
				list = &mOperationButtonList;
			}
			else
				return false;

			QPalette pal(c, palette().color(QPalette::Active, QPalette::Background));

			for(int i = 0; i < list->size(); i++)
				list->at(i)->setPalette(pal);
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
	KAboutData aboutData( "kcalc", 0, ki18n("KCalc"),
                          version, ki18n(description), KAboutData::License_GPL,
                          ki18n("(c) 2003-2005, Klaus Niederkr" "\xc3\xbc" "ger\n"
				    "(c) 1996-2000, Bernd Johannes Wuebben\n"
                                    "(c) 2000-2005, The KDE Team"));

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
	aboutData.addAuthor(ki18n("David Johnson"), KLocalizedString(), "david@usermode.org");
	KCmdLineArgs::init(argc, argv, &aboutData);

	KApplication app;

	KCalculator *calc = new KCalculator(0);
	app.setTopWidget(calc);

	calc->show();

	return app.exec();
}

