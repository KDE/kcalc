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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "../config.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <qlayout.h>
#include <qobjectlist.h>
#include <qbuttongroup.h>
#include <qhbuttongroup.h>
#include <qradiobutton.h>
#include <qtooltip.h>
#include <qstyle.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qfont.h>

#include <kaboutdata.h>
#include <kaccel.h>
#include <kaction.h>
#include <kapplication.h>
#include <kconfigdialog.h>
#include <kcmdlineargs.h>
#include <kcolordrag.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <knotifyclient.h>
#include <kpushbutton.h>
#include <kpopupmenu.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <kfontdialog.h>
#include <qlayout.h>
#include <kdialog.h>
#include <kcolorbutton.h>
#include <qspinbox.h>
#include <kkeydialog.h>
#include <knumvalidator.h>

#include "dlabel.h"
#include "kcalc.h"
#include "version.h"
#include "general.h"
#include "colors.h"
#include "constants.h"

#include "kcalc_settings.h"

const CALCAMNT KCalculator::pi = (ASIN(1L) * 2L);


static const char description[] = I18N_NOOP("KDE Calculator");
static const char version[] = KCALCVERSION;


KCalculator::KCalculator(QWidget *parent, const char *name)
	: KMainWindow(parent, name), inverse(false),
	  hyp_mode(false), memory_num(0.0),calc_display(NULL),
	  mInternalSpacing(4),
	  core()
{
	/* central widget to contain all the elements */
	QWidget *central = new QWidget(this);
	setCentralWidget(central);

	// Detect color change
	connect(kapp,SIGNAL(kdisplayPaletteChanged()), SLOT(set_colors()));

	calc_display = new DispLogic(central, "display");


	setupMainActions();

	setupStatusbar();

	createGUI();

	// How can I make the toolBar not appear at all?
	// This is not a nice solution.
	toolBar()->close();

	// Create Button to select BaseMode
	BaseChooseGroup = new QHButtonGroup(i18n("Base"), central);
	connect(BaseChooseGroup, SIGNAL(clicked(int)), SLOT(slotBaseSelected(int)));


	pbBaseChoose[0] =  new QRadioButton(i18n("&Hex"), BaseChooseGroup,
					    "Hexadecimal-Switch");
	QToolTip::add(pbBaseChoose[0], i18n("Switch base to hexadecimal."));

	pbBaseChoose[1] =  new QRadioButton(i18n("&Dec"), BaseChooseGroup,
					    "Decimal-Switch");
	QToolTip::add(pbBaseChoose[1], i18n("Switch base to decimal."));

	pbBaseChoose[2] =  new QRadioButton(i18n("&Oct"), BaseChooseGroup,
					    "Octal-Switch");
	QToolTip::add(pbBaseChoose[2], i18n("Switch base to octal."));

	pbBaseChoose[3] =  new QRadioButton(i18n("&Bin"), BaseChooseGroup,
					    "Binary-Switch");
	QToolTip::add(pbBaseChoose[3], i18n("Switch base to binary."));


	// Create Button to select AngleMode
	pbAngleChoose =  new QPushButton(i18n("&Angle"),
					 central, "ChooseAngleMode-Button");
	QToolTip::add(pbAngleChoose, i18n("???"));
	pbAngleChoose->setAutoDefault(false);

	KPopupMenu *base_menu = new KPopupMenu(pbAngleChoose, "AngleMode-Selection-Menu");
	connect(base_menu, SIGNAL(activated(int)), SLOT(slotAngleSelected(int)));
	base_menu->insertItem(i18n("Degrees"), 0);
	base_menu->insertItem(i18n("Radians"), 1);
	base_menu->insertItem(i18n("Gradians"), 2);

	pbAngleChoose->setPopup(base_menu);



	pbInv = new KCalcButton("Inv", central, "Inverse-Button",
				i18n("Inverse mode"));
	pbInv->setAccel(Key_I);
	connect(pbInv, SIGNAL(toggled(bool)), SLOT(slotInvtoggled(bool)));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbInv, SLOT(slotSetAccelDisplayMode(bool)));
	pbInv->setToggleButton(true);

	//
	//  Create Calculator Buttons
	//

	// First the widgets that are the parents of the buttons
	mSmallPage = new QWidget(central);
	mLargePage = new QWidget(central);
	mNumericPage = 	setupNumericKeys(central);

	setupLogicKeys(mSmallPage);
	setupLogExpKeys(mSmallPage);
	setupStatisticKeys(mSmallPage);
	setupTrigKeys(mSmallPage);
	setupConstantsKeys(mSmallPage);

	pbPi = new KCalcButton(mSmallPage, "Pi-Button");
	pbPi->addMode(ModeNormal, QString::fromUtf8("Ï€", -1), // Pi in utf8
		      i18n("Pi=3.1415..."));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbPi, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbPi, SIGNAL(clicked(void)), SLOT(slotPiclicked(void)));

	pbMod = new KCalcButton(mSmallPage, "Modulo-Button");
	pbMod->addMode(ModeNormal, "Mod", i18n("Modulo"));
	pbMod->setAccel(Key_Colon);
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		pbMod, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbMod, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbMod, SIGNAL(clicked(void)), SLOT(slotModclicked(void)));

	pbReci = new KCalcButton(mSmallPage, "Reciprocal-Button");
	pbReci->addMode(ModeNormal, "1/x", i18n("Reciprocal"));
	pbReci->setAccel(Key_R);
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbReci, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbReci, SIGNAL(clicked(void)), SLOT(slotReciclicked(void)));

	pbFactorial = new KCalcButton(mSmallPage, "Factorial-Button");
	pbFactorial->addMode(ModeNormal, "x!", i18n("Factorial"));
	pbFactorial->setAccel(Key_Exclam);
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbFactorial, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbFactorial, SIGNAL(clicked(void)),SLOT(slotFactorialclicked(void)));

	// Representation of x^2 is moved to the function
	// changeRepresentation() that paints the letters When
	// pressing the INV Button a sqrt symbol will be drawn on that
	// button
	pbSquare = new KCalcButton(mSmallPage, "Square-Button");
	pbSquare->addMode(ModeNormal, "x<sup>2</sup>", i18n("Square"), true);
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbSquare, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbSquare, SIGNAL(clicked(void)), SLOT(slotSquareclicked(void)));

	// Representation of x^y is moved to the function
	// changeRepresentation() that paints the letters When
	// pressing the INV Button y^x will be drawn on that button
	pbPower = new KCalcButton(mSmallPage, "Power-Button");
	pbPower->addMode(ModeNormal, "x<sup>y</sup>", i18n("x to the power of y"), true);
	pbPower->addMode(ModeInverse, "x<sup>1/y</sup>", i18n("x to the power of 1/y"), true);
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbPower, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		pbPower, SLOT(slotSetMode(ButtonModeFlags,bool)));
	pbPower->setAccel(Key_AsciiCircum);
	connect(pbPower, SIGNAL(clicked(void)), SLOT(slotPowerclicked(void)));


	//
	// All these layouts are needed because all the groups have their
	// own size per row so we can't use one huge QGridLayout (mosfet)
	//
	QGridLayout *smallBtnLayout = new QGridLayout(mSmallPage, 6, 4, 0,
		mInternalSpacing);
	QGridLayout *largeBtnLayout = new QGridLayout(mLargePage, 5, 2, 0,
		mInternalSpacing);

	QHBoxLayout *topLayout		= new QHBoxLayout();
	QHBoxLayout *btnLayout		= new QHBoxLayout();

	// bring them all together
	QVBoxLayout *mainLayout = new QVBoxLayout(central, mInternalSpacing,
		mInternalSpacing);

	mainLayout->addLayout(topLayout);
	mainLayout->addLayout(btnLayout);

	// button layout
	btnLayout->addWidget(mSmallPage, 0, AlignTop);
	btnLayout->addSpacing(2*mInternalSpacing);
	btnLayout->addWidget(mNumericPage, 0, AlignTop);
	btnLayout->addSpacing(2*mInternalSpacing);
	btnLayout->addWidget(mLargePage, 0, AlignTop);

	// small button layout
	smallBtnLayout->addWidget(pbStat["NumData"], 0, 0);
	smallBtnLayout->addWidget(pbTrig["HypMode"], 0, 1);
	smallBtnLayout->addWidget(pbLogic["AND"], 0, 2);
	smallBtnLayout->addWidget(pbPi, 0, 3);
	smallBtnLayout->addWidget(NumButtonGroup->find(0xA), 0, 4);
	smallBtnLayout->addWidget(pbConstant["C1"], 0, 5);

	smallBtnLayout->addWidget(pbStat["Mean"], 1, 0);
	smallBtnLayout->addWidget(pbTrig["Sine"], 1, 1);
	smallBtnLayout->addWidget(pbLogic["OR"], 1, 2);
	smallBtnLayout->addWidget(pbMod, 1, 3);
	smallBtnLayout->addWidget(NumButtonGroup->find(0xB), 1, 4);
	smallBtnLayout->addWidget(pbConstant["C2"], 1, 5);

	smallBtnLayout->addWidget(pbStat["StandardDeviation"], 2, 0);
	smallBtnLayout->addWidget(pbTrig["Cosine"], 2, 1);
	smallBtnLayout->addWidget(pbLogic["XOR"], 2, 2);
	smallBtnLayout->addWidget(pbReci, 2, 3);
	smallBtnLayout->addWidget(NumButtonGroup->find(0xC), 2, 4);
	smallBtnLayout->addWidget(pbConstant["C3"], 2, 5);

	smallBtnLayout->addWidget(pbStat["Median"], 3, 0);
	smallBtnLayout->addWidget(pbTrig["Tangent"], 3, 1);
	smallBtnLayout->addWidget(pbLogic["LeftShift"], 3, 2);
	smallBtnLayout->addWidget(pbFactorial, 3, 3);
	smallBtnLayout->addWidget(NumButtonGroup->find(0xD), 3, 4);
	smallBtnLayout->addWidget(pbConstant["C4"], 3, 5);

	smallBtnLayout->addWidget(pbStat["InputData"], 4, 0);
	smallBtnLayout->addWidget(pbExp["Log10"], 4, 1);
	smallBtnLayout->addWidget(pbLogic["RightShift"], 4, 2);
	smallBtnLayout->addWidget(pbSquare, 4, 3);
	smallBtnLayout->addWidget(NumButtonGroup->find(0xE), 4, 4);
	smallBtnLayout->addWidget(pbConstant["C5"], 4, 5);

	smallBtnLayout->addWidget(pbStat["ClearData"], 5, 0);
	smallBtnLayout->addWidget(pbExp["LogNatural"], 5, 1);
	smallBtnLayout->addWidget(pbLogic["One-Complement"], 5, 2);
	smallBtnLayout->addWidget(pbPower, 5, 3);
	smallBtnLayout->addWidget(NumButtonGroup->find(0xF), 5, 4);
	smallBtnLayout->addWidget(pbConstant["C6"],5, 5);

	smallBtnLayout->setRowStretch(0, 0);
	smallBtnLayout->setRowStretch(1, 0);
	smallBtnLayout->setRowStretch(2, 0);
	smallBtnLayout->setRowStretch(3, 0);
	smallBtnLayout->setRowStretch(4, 0);
	smallBtnLayout->setRowStretch(5, 0);

	// large button layout
	largeBtnLayout->addWidget(pbClear, 0, 0);
	largeBtnLayout->addWidget(pbAC, 0, 1);

	largeBtnLayout->addWidget(pbParenOpen, 1, 0);
	largeBtnLayout->addWidget(pbParenClose, 1, 1);

	largeBtnLayout->addWidget(pbMR, 2, 0);

	largeBtnLayout->addWidget(pbMPlusMinus, 3, 0);
	largeBtnLayout->addWidget(pbPercent, 3, 1);

	largeBtnLayout->addWidget(pbMC, 4, 0);
	largeBtnLayout->addWidget(pbPlusMinus, 4, 1);

	// top layout
	topLayout->addWidget(pbAngleChoose);
	topLayout->addWidget(BaseChooseGroup);
	topLayout->addWidget(pbInv);
	topLayout->addWidget(calc_display, 10);

	mFunctionButtonList.append(pbTrig["HypMode"]);
	mFunctionButtonList.append(pbInv);
	mFunctionButtonList.append(pbPi);
	mFunctionButtonList.append(pbTrig["Sine"]);
	mFunctionButtonList.append(pbPlusMinus);
	mFunctionButtonList.append(pbTrig["Cosine"]);
	mFunctionButtonList.append(pbReci);
	mFunctionButtonList.append(pbTrig["Tangent"]);
	mFunctionButtonList.append(pbFactorial);
	mFunctionButtonList.append(pbExp["Log10"]);
	mFunctionButtonList.append(pbSquare);
	mFunctionButtonList.append(pbExp["LogNatural"]);
	mFunctionButtonList.append(pbPower);

	mMemButtonList.append(pbEE);
	mMemButtonList.append(pbMR);
	mMemButtonList.append(pbMPlusMinus);
	mMemButtonList.append(pbMC);
	mMemButtonList.append(pbClear);
	mMemButtonList.append(pbAC);

	mOperationButtonList.append(pbX);
	mOperationButtonList.append(pbParenOpen);
	mOperationButtonList.append(pbParenClose);
	mOperationButtonList.append(pbLogic["AND"]);
	mOperationButtonList.append(pbDivision);
	mOperationButtonList.append(pbLogic["OR"]);
	mOperationButtonList.append(pbLogic["XOR"]);
	mOperationButtonList.append(pbPlus);
	mOperationButtonList.append(pbMinus);
	mOperationButtonList.append(pbLogic["LeftShift"]);
	mOperationButtonList.append(pbLogic["RightShift"]);
	mOperationButtonList.append(pbPeriod);
	mOperationButtonList.append(pbEqual);
	mOperationButtonList.append(pbPercent);
	mOperationButtonList.append(pbLogic["One-Complement"]);
	mOperationButtonList.append(pbMod);

	mConstButtonList.append(pbConstant["C1"]);
	mConstButtonList.append(pbConstant["C2"]);
	mConstButtonList.append(pbConstant["C3"]);
	mConstButtonList.append(pbConstant["C4"]);
	mConstButtonList.append(pbConstant["C5"]);
	mConstButtonList.append(pbConstant["C6"]);

	set_colors();
	// Show the result in the app's caption in taskbar (wishlist - bug #52858)
	if (KCalcSettings::captionResult() == true)
		connect(calc_display,
			SIGNAL(changedText(const QString &)),
			SLOT(setCaption(const QString &)));
	calc_display->changeSettings();
	set_precision();

	// Switch to decimal
	resetBase();
	slotAngleSelected(0);

	updateGeometry();

	adjustSize();
	setFixedSize(sizeHint());

	UpdateDisplay(true);

	// Read and set button groups

	actionStatshow->setChecked(KCalcSettings::showStat());
	slotStatshow(KCalcSettings::showStat());

	actionExpLogshow->setChecked(KCalcSettings::showExpLog());
	slotExpLogshow(KCalcSettings::showExpLog());

	actionTrigshow->setChecked(KCalcSettings::showTrig());
	slotTrigshow(KCalcSettings::showTrig());

	actionLogicshow->setChecked(KCalcSettings::showLogic());
	slotLogicshow(KCalcSettings::showLogic());

	actionConstantsShow->setChecked(KCalcSettings::showConstants());
	slotConstantsShow(KCalcSettings::showConstants());
}

KCalculator::~KCalculator()
{
	KCalcSettings::writeConfig();
	delete calc_display;
}

void KCalculator::setupMainActions(void)
{
	// file menu
	KStdAction::quit(this, SLOT(close()), actionCollection());

	// edit menu
	KStdAction::cut(calc_display, SLOT(slotCut()), actionCollection());
	KStdAction::copy(calc_display, SLOT(slotCopy()), actionCollection());
	KStdAction::paste(calc_display, SLOT(slotPaste()), actionCollection());

	// settings menu
	actionStatshow =  new KToggleAction(i18n("&Statistic Buttons"), 0,
					    actionCollection(), "show_stat");
	actionStatshow->setChecked(true);
	connect(actionStatshow, SIGNAL(toggled(bool)),
		SLOT(slotStatshow(bool)));

	actionExpLogshow = new KToggleAction(i18n("&Exp/Log-Buttons"), 0,
					     actionCollection(),
					     "show_explog");
	actionExpLogshow->setChecked(true);
	connect(actionExpLogshow, SIGNAL(toggled(bool)),
		SLOT(slotExpLogshow(bool)));

	actionTrigshow = new KToggleAction(i18n("&Trigonometric Buttons"), 0,
					   actionCollection(), "show_trig");
	actionTrigshow->setChecked(true);
	connect(actionTrigshow, SIGNAL(toggled(bool)),
		SLOT(slotTrigshow(bool)));

	actionLogicshow = new KToggleAction(i18n("&Logic Buttons"), 0,
					    actionCollection(), "show_logic");
	actionLogicshow->setChecked(true);
	connect(actionLogicshow, SIGNAL(toggled(bool)),
		SLOT(slotLogicshow(bool)));

	actionConstantsShow = new KToggleAction(i18n("&Constants Buttons"), 0,
					    actionCollection(), "show_constants");
	actionConstantsShow->setChecked(true);
	connect(actionConstantsShow, SIGNAL(toggled(bool)),
		SLOT(slotConstantsShow(bool)));


	(void) new KAction(i18n("&Show All"), 0, this, SLOT(slotShowAll()),
			   actionCollection(), "show_all");

	(void) new KAction(i18n("&Hide All"), 0, this, SLOT(slotHideAll()),
			   actionCollection(), "hide_all");

	KStdAction::preferences(this, SLOT(showSettings()), actionCollection());

	KStdAction::keyBindings( this, SLOT( slotConfigureKeys() ), actionCollection() );
}

void KCalculator::setupStatusbar(void)
{
	// Status bar contents
	statusBar()->insertFixedItem(" NORM ", 0, true);
	statusBar()->setItemAlignment(0, AlignCenter);

	statusBar()->insertFixedItem(" HEX ", 1, true);
	statusBar()->setItemAlignment(1, AlignCenter);

	statusBar()->insertFixedItem(" DEG ", 2, true);
	statusBar()->setItemAlignment(2, AlignCenter);
}

QWidget* KCalculator::setupNumericKeys(QWidget *parent)
{
	Q_CHECK_PTR(mSmallPage);
	Q_CHECK_PTR(mLargePage);

	QWidget *thisPage = new QWidget(parent);

	KCalcButton *tmp_pb;

	NumButtonGroup = new QButtonGroup(0, "Num-Button-Group");
	connect(NumButtonGroup, SIGNAL(clicked(int)),
		SLOT(slotNumberclicked(int)));

	tmp_pb = new KCalcButton("0", thisPage, "0-Button");
	tmp_pb->setAccel(Key_0);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
        NumButtonGroup->insert(tmp_pb, 0);

	tmp_pb = new KCalcButton("1", thisPage, "1-Button");
	tmp_pb->setAccel(Key_1);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
        NumButtonGroup->insert(tmp_pb, 1);

	tmp_pb = new KCalcButton("2", thisPage, "2-Button");
	tmp_pb->setAccel(Key_2);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
        NumButtonGroup->insert(tmp_pb, 2);

	tmp_pb = new KCalcButton("3", thisPage, "3-Button");
	tmp_pb->setAccel(Key_3);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
        NumButtonGroup->insert(tmp_pb, 3);

	tmp_pb = new KCalcButton("4", thisPage, "4-Button");
	tmp_pb->setAccel(Key_4);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
        NumButtonGroup->insert(tmp_pb, 4);

	tmp_pb = new KCalcButton("5", thisPage, "5-Button");
	tmp_pb->setAccel(Key_5);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
        NumButtonGroup->insert(tmp_pb, 5);

	tmp_pb = new KCalcButton("6", thisPage, "6-Button");
	tmp_pb->setAccel(Key_6);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
        NumButtonGroup->insert(tmp_pb, 6);

	tmp_pb = new KCalcButton("7", thisPage, "7-Button");
	tmp_pb->setAccel(Key_7);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
        NumButtonGroup->insert(tmp_pb, 7);

	tmp_pb = new KCalcButton("8", thisPage, "8-Button");
	tmp_pb->setAccel(Key_8);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
        NumButtonGroup->insert(tmp_pb, 8);

	tmp_pb = new KCalcButton("9", thisPage, "9-Button");
	tmp_pb->setAccel(Key_9);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
        NumButtonGroup->insert(tmp_pb, 9);

	pbEE = new KCalcButton(thisPage, "EE-Button");
	pbEE->addMode(ModeNormal, "x<small>·10</small><sup>y</sup>",
			i18n("Exponent"), true);
	pbEE->setAccel(Key_E);
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbEE, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbEE, SIGNAL(clicked(void)), SLOT(slotEEclicked(void)));

	pbParenClose = new KCalcButton(")", mLargePage, "ParenClose-Button");
	pbParenClose->setAccel(Key_ParenRight);
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbParenClose, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbParenClose,SIGNAL(clicked(void)),SLOT(slotParenCloseclicked(void)));

	pbX = new KCalcButton("X", thisPage, "Multiply-Button", i18n("Multiplication"));
	pbX->setAccel(Key_multiply);
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbX, SLOT(slotSetAccelDisplayMode(bool)));
	accel()->insert("Pressed '*'", i18n("Pressed Multiplication-Button"),
			0, Key_Asterisk, pbX, SLOT(animateClick()));
	connect(pbX, SIGNAL(clicked(void)), SLOT(slotXclicked(void)));

	pbDivision = new KCalcButton("/", thisPage, "Division-Button", i18n("Division"));
	pbDivision->setAccel(Key_Slash);
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbDivision, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbDivision, SIGNAL(clicked(void)), SLOT(slotDivisionclicked(void)));

	pbPlus = new KCalcButton("+", thisPage, "Plus-Button", i18n("Addition"));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbPlus, SLOT(slotSetAccelDisplayMode(bool)));
	pbPlus->setAccel(Key_Plus);
	connect(pbPlus, SIGNAL(clicked(void)), SLOT(slotPlusclicked(void)));

	pbMinus = new KCalcButton("-", thisPage, "Minus-Button", i18n("Subtraction"));
	pbMinus->setAccel(Key_Minus);
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbMinus, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbMinus, SIGNAL(clicked(void)), SLOT(slotMinusclicked(void)));

	pbPeriod = new KCalcButton(KGlobal::locale()->decimalSymbol(), thisPage,
					"Period-Button", i18n("Decimal point"));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbPeriod, SLOT(slotSetAccelDisplayMode(bool)));
	accel()->insert("Decimal Point (Period)", i18n("Pressed Decimal Point"),
			0, Key_Period, pbPeriod, SLOT(animateClick()));
	accel()->insert("Decimal Point (Comma)", i18n("Pressed Decimal Point"),
			0, Key_Comma, pbPeriod, SLOT(animateClick()));
	connect(pbPeriod, SIGNAL(clicked(void)), SLOT(slotPeriodclicked(void)));

	pbEqual = new KCalcButton("=", thisPage, "Equal-Button", i18n("Result"));
	pbEqual->setAccel(Key_Enter);
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbEqual, SLOT(slotSetAccelDisplayMode(bool)));
	accel()->insert("Entered Equal", i18n("Pressed Equal-Button"),
			0, Key_Equal, pbEqual, SLOT(animateClick()));
	accel()->insert("Entered Return", i18n("Pressed Equal-Button"),
			0, Key_Return, pbEqual, SLOT(animateClick()));
	connect(pbEqual, SIGNAL(clicked(void)), SLOT(slotEqualclicked(void)));


	QGridLayout *thisLayout = new QGridLayout(thisPage, 5, 4, 0,
						  mInternalSpacing);

	// large button layout
	thisLayout->addWidget(pbEE, 0, 0);
	thisLayout->addWidget(pbDivision, 0, 1);
	thisLayout->addWidget(pbX, 0, 2);
	thisLayout->addWidget(pbMinus, 0, 3);

	thisLayout->addWidget(NumButtonGroup->find(7), 1, 0);
	thisLayout->addWidget(NumButtonGroup->find(8), 1, 1);
	thisLayout->addWidget(NumButtonGroup->find(9), 1, 2);
	thisLayout->addMultiCellWidget(pbPlus, 1, 2, 3, 3);

	thisLayout->addWidget(NumButtonGroup->find(4), 2, 0);
	thisLayout->addWidget(NumButtonGroup->find(5), 2, 1);
	thisLayout->addWidget(NumButtonGroup->find(6), 2, 2);
	//thisLayout->addMultiCellWidget(pbPlus, 1, 2, 3, 3);

	thisLayout->addWidget(NumButtonGroup->find(1), 3, 0);
	thisLayout->addWidget(NumButtonGroup->find(2), 3, 1);
	thisLayout->addWidget(NumButtonGroup->find(3), 3, 2);
	thisLayout->addMultiCellWidget(pbEqual, 3, 4, 3, 3);

	thisLayout->addMultiCellWidget(NumButtonGroup->find(0), 4, 4, 0, 1);
	thisLayout->addWidget(pbPeriod, 4, 2);
	//thisLayout->addMultiCellWidget(pbEqual, 3, 4, 3, 3);

	thisLayout->addColSpacing(0,10);
	thisLayout->addColSpacing(1,10);
	thisLayout->addColSpacing(2,10);
	thisLayout->addColSpacing(3,10);
	thisLayout->addColSpacing(4,10);


	pbMR = new KCalcButton("MR", mLargePage, "MemRecall-Button", i18n("Memory recall"));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbMR, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbMR, SIGNAL(clicked(void)), SLOT(slotMRclicked(void)));

	pbMPlusMinus = new KCalcButton("M±", mLargePage, "MPlusMinus-Button");
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbMPlusMinus, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbMPlusMinus,SIGNAL(clicked(void)),SLOT(slotMPlusMinusclicked(void)));

	pbMC = new KCalcButton("MC", mLargePage, "MemClear-Button", i18n("Clear memory"));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbMC, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbMC, SIGNAL(clicked(void)), SLOT(slotMCclicked(void)));

	pbClear = new KCalcButton("C", mLargePage, "Clear-Button", i18n("Clear"));
	pbClear->setAccel(Key_Prior);
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbClear, SLOT(slotSetAccelDisplayMode(bool)));
	accel()->insert("Entered 'ESC'", i18n("Pressed ESC-Button"), 0,
			Key_Escape, pbClear, SLOT(animateClick()));
	connect(pbClear, SIGNAL(clicked(void)), SLOT(slotClearclicked(void)));

	pbAC = new KCalcButton("AC", mLargePage, "AC-Button", i18n("Clear all"));
	pbAC->setAccel(Key_Delete);
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbAC, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbAC, SIGNAL(clicked(void)), SLOT(slotACclicked(void)));

	pbParenOpen = new KCalcButton("(", mLargePage, "ParenOpen-Button");
	pbParenOpen->setAccel(Key_ParenLeft);
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbParenOpen, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbParenOpen, SIGNAL(clicked(void)),SLOT(slotParenOpenclicked(void)));

	pbPercent = new KCalcButton("%", mLargePage, "Percent-Button", i18n("Percent"));
	pbPercent->setAccel(Key_Percent);
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbPercent, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbPercent, SIGNAL(clicked(void)), SLOT(slotPercentclicked(void)));

	pbPlusMinus = new KCalcButton("±", mLargePage, "Sign-Button", i18n("Change sign"));
	pbPlusMinus->setAccel(Key_Backslash);
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbPlusMinus, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbPlusMinus, SIGNAL(clicked(void)), SLOT(slotPlusMinusclicked(void)));


	tmp_pb = new KCalcButton("A", mSmallPage, "A-Button");
	tmp_pb->setAccel(Key_A);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
        NumButtonGroup->insert(tmp_pb, 0xA);

	tmp_pb = new KCalcButton("B", mSmallPage, "B-Button");
	tmp_pb->setAccel(Key_B);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
        NumButtonGroup->insert(tmp_pb, 0xB);

	tmp_pb = new KCalcButton("C", mSmallPage, "C-Button");
	tmp_pb->setAccel(Key_C);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
        NumButtonGroup->insert(tmp_pb, 0xC);

	tmp_pb = new KCalcButton("D", mSmallPage, "D-Button");
	tmp_pb->setAccel(Key_D);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
        NumButtonGroup->insert(tmp_pb, 0xD);

	tmp_pb = new KCalcButton("E", mSmallPage, "E-Button");
	tmp_pb->setAccel(Key_E);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
        NumButtonGroup->insert(tmp_pb, 0xE);

	tmp_pb = new KCalcButton("F", mSmallPage, "F-Button");
	tmp_pb->setAccel(Key_F);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
        NumButtonGroup->insert(tmp_pb, 0xF);

	return thisPage;
}

void KCalculator::setupLogicKeys(QWidget *parent)
{
	Q_CHECK_PTR(parent);

	KCalcButton *tmp_pb;

	tmp_pb = new KCalcButton("AND", parent, "AND-Button", i18n("Bitwise AND"));
	pbLogic.insert("AND", tmp_pb);
	tmp_pb->setAccel(Key_Ampersand);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotANDclicked(void)));

	tmp_pb = new KCalcButton("OR", parent, "OR-Button", i18n("Bitwise OR"));
	pbLogic.insert("OR", tmp_pb);
	tmp_pb->setAccel(Key_Bar);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotORclicked(void)));

	tmp_pb = new KCalcButton("XOR", parent, "XOR-Button", i18n("Bitwise XOR"));
	pbLogic.insert("XOR", tmp_pb);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotXORclicked(void)));

	tmp_pb = new KCalcButton("Cmp", parent, "One-Complement-Button",
					i18n("One's complement"));
	pbLogic.insert("One-Complement", tmp_pb);
	tmp_pb->setAccel(Key_AsciiTilde);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotNegateclicked(void)));

	tmp_pb = new KCalcButton("Lsh", parent, "LeftBitShift-Button",
					i18n("Left bit shift"));
	tmp_pb->setAccel(Key_Less);
	pbLogic.insert("LeftShift", tmp_pb);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)),
		SLOT(slotLeftShiftclicked(void)));

	tmp_pb = new KCalcButton("Rsh", parent, "RightBitShift-Button",
					i18n("Right bit shift"));
	tmp_pb->setAccel(Key_Greater);
	pbLogic.insert("RightShift", tmp_pb);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)),
		SLOT(slotRightShiftclicked(void)));
}

void KCalculator::setupLogExpKeys(QWidget *parent)
{
	Q_CHECK_PTR(parent);

	KCalcButton *tmp_pb;

	tmp_pb = new KCalcButton(parent, "Ln-Button");
	tmp_pb->addMode(ModeNormal, "Ln", i18n("Natural log"));
	tmp_pb->addMode(ModeInverse, "e<sup> x </sup>", i18n("Exponential function"),
			true);
	pbExp.insert("LogNatural", tmp_pb);
	tmp_pb->setAccel(Key_N);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotLnclicked(void)));

	tmp_pb = new KCalcButton(parent, "Log-Button");
	tmp_pb->addMode(ModeNormal, "Log", i18n("Logarithm to base 10"));
	tmp_pb->addMode(ModeInverse, "10<sup> x </sup>", i18n("10 to the power of x"),
			true);
	pbExp.insert("Log10", tmp_pb);
	tmp_pb->setAccel(Key_L);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotLogclicked(void)));

}

void KCalculator::setupTrigKeys(QWidget *parent)
{
	Q_CHECK_PTR(parent);

	KCalcButton *tmp_pb;

	tmp_pb = new KCalcButton("Hyp", parent, "Hyp-Button", i18n("Hyperbolic mode"));
	pbTrig.insert("HypMode", tmp_pb);
	tmp_pb->setAccel(Key_H);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(toggled(bool)), SLOT(slotHyptoggled(bool)));
	tmp_pb->setToggleButton(true);

	tmp_pb = new KCalcButton(parent, "Sin-Button");
	pbTrig.insert("Sine", tmp_pb);
	tmp_pb->addMode(ModeNormal, "Sin", i18n("Sine"));
	tmp_pb->addMode(ModeInverse, "Asin", i18n("Arc sine"));
	tmp_pb->addMode(ModeHyperbolic, "Sinh", i18n("Hyperbolic sine"));
	tmp_pb->addMode(ButtonModeFlags(ModeInverse | ModeHyperbolic),
			"Asinh", i18n("Inverse hyperbolic sine"));
	tmp_pb->setAccel(Key_S);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotSinclicked(void)));

	tmp_pb = new KCalcButton(parent, "Cos-Button");
	pbTrig.insert("Cosine", tmp_pb);
	tmp_pb->addMode(ModeNormal, "Cos", i18n("Cosine"));
	tmp_pb->addMode(ModeInverse, "Acos", i18n("Arc cosine"));
	tmp_pb->addMode(ModeHyperbolic, "Cosh", i18n("Hyperbolic cosine"));
	tmp_pb->addMode(ButtonModeFlags(ModeInverse | ModeHyperbolic),
			"Acosh", i18n("Inverse hyperbolic cosine"));
	tmp_pb->setAccel(Key_C);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotCosclicked(void)));

	tmp_pb = new KCalcButton(parent, "Tan-Button");
	pbTrig.insert("Tangent", tmp_pb);
	tmp_pb->addMode(ModeNormal, "Tan", i18n("Tangent"));
	tmp_pb->addMode(ModeInverse, "Atan", i18n("Arc tangent"));
	tmp_pb->addMode(ModeHyperbolic, "Tanh", i18n("Hyperbolic tangent"));
	tmp_pb->addMode(ButtonModeFlags(ModeInverse | ModeHyperbolic),
			"Atanh", i18n("Inverse hyperbolic tangent"));
	tmp_pb->setAccel(Key_T);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(tmp_pb, SIGNAL(clicked(void)),SLOT(slotTanclicked(void)));

}

void KCalculator::setupStatisticKeys(QWidget *parent)
{
	Q_CHECK_PTR(parent);

	KCalcButton *tmp_pb;

	tmp_pb = new KCalcButton(parent, "Stat.NumData-Button");
	tmp_pb->addMode(ModeNormal, "N", i18n("Number of data entered"));
	pbStat.insert("NumData", tmp_pb);
        mStatButtonList.append(tmp_pb);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotStatNumclicked(void)));

	tmp_pb = new KCalcButton(parent, "Stat.Median-Button");
	tmp_pb->addMode(ModeNormal, "Med", i18n("Median"));
	pbStat.insert("Median", tmp_pb);
        mStatButtonList.append(tmp_pb);
	QToolTip::add(tmp_pb, i18n("Median"));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotStatMedianclicked(void)));

	tmp_pb = new KCalcButton(parent, "Stat.Mean-Button");
	tmp_pb->addMode(ModeNormal, "Mea", i18n("Mean"));
	pbStat.insert("Mean", tmp_pb);
        mStatButtonList.append(tmp_pb);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotStatMeanclicked(void)));

	tmp_pb = new KCalcButton(parent, "Stat.StandardDeviation-Button");
	tmp_pb->addMode(ModeNormal, "Std", i18n("Standard deviation"));
	pbStat.insert("StandardDeviation", tmp_pb);
        mStatButtonList.append(tmp_pb);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotStatStdDevclicked(void)));

	tmp_pb = new KCalcButton(parent, "Stat.DataInput-Button");
	tmp_pb->addMode(ModeNormal, "Dat", i18n("Enter data"));
	pbStat.insert("InputData", tmp_pb);
        mStatButtonList.append(tmp_pb);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotStatDataInputclicked(void)));

	tmp_pb = new KCalcButton(parent, "Stat.ClearData-Button");
	tmp_pb->addMode(ModeNormal, "CSt", i18n("Clear data store"));
	pbStat.insert("ClearData", tmp_pb);
        mStatButtonList.append(tmp_pb);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotStatClearDataclicked(void)));
}

void KCalculator::setupConstantsKeys(QWidget *parent)
{
	Q_CHECK_PTR(parent);

	KCalcButton *tmp_pb;
	tmp_pb = new KCalcButton("C1", parent, "Constant C1 - Button");
	pbConstant.insert("C1", tmp_pb);
	mConstButtonList.append(tmp_pb);
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotC1clicked(void)));

	tmp_pb = new KCalcButton("C2", parent, "Constant C2 - Button");
	pbConstant.insert("C2", tmp_pb);
	mConstButtonList.append(tmp_pb);
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotC2clicked(void)));

	tmp_pb = new KCalcButton("C3", parent, "Constant C3 - Button");
	pbConstant.insert("C3", tmp_pb);
	mConstButtonList.append(tmp_pb);
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotC3clicked(void)));

	tmp_pb = new KCalcButton("C4", parent, "Constant C4 - Button");
	pbConstant.insert("C4", tmp_pb);
	mConstButtonList.append(tmp_pb);
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotC4clicked(void)));

	tmp_pb = new KCalcButton("C5", parent, "Constant C5 - Button");
	pbConstant.insert("C5", tmp_pb);
	mConstButtonList.append(tmp_pb);
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotC5clicked(void)));

	tmp_pb = new KCalcButton("C6", parent, "Constant C6 - Button");
	pbConstant.insert("C6", tmp_pb);
	mConstButtonList.append(tmp_pb);
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotC6clicked(void)));

	changeButtonNames();
}

void KCalculator::updateGeometry(void)
{
    QObjectList *l;
    QSize s;
    int margin;

    //
    // Calculator buttons
    //
    s.setWidth(mSmallPage->fontMetrics().width("MMMM"));
    s.setHeight(mSmallPage->fontMetrics().lineSpacing());

    l = (QObjectList*)mSmallPage->children(); // silence please

    for(uint i=0; i < l->count(); i++)
    {
        QObject *o = l->at(i);
        if( o->isWidgetType() )
        {
            margin = QApplication::style().
                pixelMetric(QStyle::PM_ButtonMargin, ((QWidget *)o))*2;
            ((QWidget*)o)->setFixedSize(s.width()+margin, s.height()+margin);
            //((QWidget*)o)->setMinimumSize(s.width()+margin, s.height()+margin);
            ((QWidget*)o)->installEventFilter( this );
            ((QWidget*)o)->setAcceptDrops(true);
        }
    }

    l = (QObjectList*)mLargePage->children(); // silence please

    int h1 = (NumButtonGroup->find(0x0F))->minimumSize().height();
    int h2 = (int)((((float)h1 + 4.0) / 5.0));
    s.setWidth(mLargePage->fontMetrics().width("MMM") +
               QApplication::style().
               pixelMetric(QStyle::PM_ButtonMargin, NumButtonGroup->find(0x0F))*2);
    s.setHeight(h1 + h2);

    for(uint i = 0; i < l->count(); i++)
    {
        QObject *o = l->at(i);
        if(o->isWidgetType())
        {
            ((QWidget*)o)->setFixedSize(s);
            ((QWidget*)o)->installEventFilter(this);
            ((QWidget*)o)->setAcceptDrops(true);
        }
    }

    pbInv->setFixedSize(s);
    pbInv->installEventFilter(this);
    pbInv->setAcceptDrops(true);



    l = (QObjectList*)mNumericPage->children(); // silence please

    h1 = (NumButtonGroup->find(0x0F))->minimumSize().height();
    h2 = (int)((((float)h1 + 4.0) / 5.0));
    s.setWidth(mLargePage->fontMetrics().width("MMM") +
               QApplication::style().
               pixelMetric(QStyle::PM_ButtonMargin, NumButtonGroup->find(0x0F))*2);
    s.setHeight(h1 + h2);

    for(uint i = 0; i < l->count(); i++)
    {
        QObject *o = l->at(i);
        if(o->isWidgetType())
        {
            ((QWidget*)o)->setFixedSize(s);
            ((QWidget*)o)->installEventFilter(this);
            ((QWidget*)o)->setAcceptDrops(true);
        }
    }

    // Set Buttons of double size
    QSize t(s);
    t.setWidth(2*s.width());
    NumButtonGroup->find(0x00)->setFixedSize(t);
    t = s;
    t.setHeight(2*s.height());
    pbEqual->setFixedSize(t);
    pbPlus->setFixedSize(t);
}

void KCalculator::slotBaseSelected(int base)
{
	int current_base;

	// set display & statusbar
	switch(base)
	{
	case 3:
	  current_base = calc_display->setBase(NumBase(2));
	  statusBar()->changeItem("BIN",1);
	  break;
	case 2:
	  current_base = calc_display->setBase(NumBase(8));
	  statusBar()->changeItem("OCT",1);
	  break;
	case 1:
	  current_base = calc_display->setBase(NumBase(10));
	  statusBar()->changeItem("DEC",1);
	  break;
	case 0:
	  current_base = calc_display->setBase(NumBase(16));
	  statusBar()->changeItem("HEX",1);
	  break;
	default:
	  statusBar()->changeItem("Error",1);
	  return;
	}

	// Enable the buttons not available in this base
	for (int i=0; i<current_base; i++)
	  NumButtonGroup->find(i)->setEnabled (true);

	// Disable the buttons not available in this base
	for (int i=current_base; i<16; i++)
	  NumButtonGroup->find(i)->setEnabled (false);

	// Only enable the decimal point in decimal
	pbPeriod->setEnabled(current_base == NB_DECIMAL);
	// Only enable the x*10^y button in decimal
	pbEE->setEnabled(current_base == NB_DECIMAL);

	// Disable buttons that make only sense with floating point
	// numbers
	if(current_base != NB_DECIMAL)
	{
	  pbTrig["HypMode"]->setEnabled(false);
	  pbTrig["Sine"]->setEnabled(false);
	  pbTrig["Cosine"]->setEnabled(false);
	  pbTrig["Tangent"]->setEnabled(false);
	  pbExp["LogNatural"]->setEnabled(false);
	  pbExp["Log10"]->setEnabled(false);
	}
	else
	{
	  pbTrig["HypMode"]->setEnabled(true);
	  pbTrig["Sine"]->setEnabled(true);
	  pbTrig["Cosine"]->setEnabled(true);
	  pbTrig["Tangent"]->setEnabled(true);
	  pbExp["LogNatural"]->setEnabled(true);
	  pbExp["Log10"]->setEnabled(true);
	}
}

void KCalculator::keyPressEvent(QKeyEvent *e)
{
    if ( ( e->state() & KeyButtonMask ) == 0 || ( e->state() & ShiftButton ) ) {
	switch (e->key())
	{
	case Key_Up:
		history_prev();
		break;
	case Key_Down:
		history_next();
		break;
	case Key_Next:
		pbAC->animateClick();
		break;
	case Key_Slash:
        case Key_division:
		pbDivision->animateClick();
		break;
 	case Key_D:
		pbStat["InputData"]->animateClick(); // stat mode
		break;
	case Key_BracketLeft:
        case Key_twosuperior:
		pbSquare->animateClick();
		break;
	case Key_Backspace:
		calc_display->deleteLastDigit();
		// pbAC->animateClick();
		break;
	}
    }

    if (e->state() & Keypad && ( e->key() != Key_Period ))
    {
        NumButtonGroup->find(e->text().toInt())->animateClick();
    }

    if (e->key() == Key_Control)
	emit switchShowAccels(true);
}

void KCalculator::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Key_Control)
	emit switchShowAccels(false);
}

void KCalculator::slotAngleSelected(int number)
{
	switch(number)
	{
	case 0:
		core.SetAngleMode(ANG_DEGREE);
		statusBar()->changeItem("DEG", 2);
		break;
	case 1:
		core.SetAngleMode(ANG_RADIAN);
		statusBar()->changeItem("RAD", 2);
		break;
	case 2:
		core.SetAngleMode(ANG_GRADIENT);
		statusBar()->changeItem("GRA", 2);
		break;
	default: // we shouldn't ever end up here
		core.SetAngleMode(ANG_RADIAN);
	}
}

void KCalculator::slotEEclicked(void)
{
	calc_display->newCharacter('e');
}

void KCalculator::slotPiclicked(void)
{
	calc_display->setAmount(pi);

	UpdateDisplay(false);
}

void KCalculator::slotInvtoggled(bool flag)
{
	inverse = flag;

	emit switchMode(ModeInverse, flag);

	if (inverse)
	{
		statusBar()->changeItem("INV", 0);
		// these statements are for the improved
		// representation of the sqrt function
		buttonpixmap.resize( 45, 15 );
		buttonpixmap.fill( KCalcSettings::functionButtonsColor() );
		buttonpainter.begin( &buttonpixmap );
		buttonpainter.drawLine(8, 11,10, 7);
		buttonpainter.drawLine(10, 7, 12, 14 );
		buttonpainter.drawLine(12, 14, 14, 1);
		buttonpainter.drawLine(14,1, 35,1);
		buttonpainter.drawLine(35,1, 35, 4);
		buttonpainter.end();
		pbSquare->setPixmap( buttonpixmap );
	}
	else
	{
		statusBar()->changeItem("NORM", 0);
		// Change the representation of x^2
		pbSquare->setText("x<sup>2</sup>");
	}
}

void KCalculator::slotHyptoggled(bool flag)
{
	// toggle between hyperbolic and standart trig functions
	hyp_mode = flag;

	emit switchMode(ModeHyperbolic, flag); 
}



void KCalculator::slotMRclicked(void)
{
	// temp. work-around
	calc_display->Reset();

	calc_display->setAmount(memory_num);
	UpdateDisplay(false);
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
			core.Sin(calc_display->getAmount());
		else
			core.ArcSin(calc_display->getAmount());
	}

	UpdateDisplay(true);
}

void KCalculator::slotPlusMinusclicked(void)
{
	calc_display->changeSign();
}

void KCalculator::slotMPlusMinusclicked(void)
{
	EnterEqual();

	if (!inverse)	memory_num += calc_display->getAmount();
	else 			memory_num -= calc_display->getAmount();

	pbInv->setOn(false);
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
			core.Cos(calc_display->getAmount());
		else
			core.ArcCos(calc_display->getAmount());
	}

	UpdateDisplay(true);
}

void KCalculator::slotReciclicked(void)
{
	core.Reciprocal(calc_display->getAmount());
	UpdateDisplay(true);
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
			core.Tangens(calc_display->getAmount());
		else
			core.ArcTangens(calc_display->getAmount());
	}

	UpdateDisplay(true);
}

void KCalculator::slotFactorialclicked(void)
{
	core.Factorial(calc_display->getAmount());

	UpdateDisplay(true);
}

void KCalculator::slotLogclicked(void)
{
	if (!inverse)
		core.Log10(calc_display->getAmount());
	else
		core.Exp10(calc_display->getAmount());

	UpdateDisplay(true);
}


void KCalculator::slotSquareclicked(void)
{
	if (!inverse)
		core.Square(calc_display->getAmount());
	else
		core.SquareRoot(calc_display->getAmount());

	UpdateDisplay(true);
}

void KCalculator::slotLnclicked(void)
{
	if (!inverse)
		core.Ln(calc_display->getAmount());
	else
		core.Exp(calc_display->getAmount());

	UpdateDisplay(true);
}

void KCalculator::slotPowerclicked(void)
{
	if (inverse)
	{
		core.InvPower(calc_display->getAmount());
		pbInv->setOn(false);
	}
	else
	{
		core.Power(calc_display->getAmount());
	}
	// temp. work-around
	CALCAMNT tmp_num = calc_display->getAmount();
	calc_display->Reset();
	calc_display->setAmount(tmp_num);
	UpdateDisplay(false);
}

void KCalculator::slotMCclicked(void)
{
	memory_num		= 0;
}

void KCalculator::slotClearclicked(void)
{
	calc_display->clearLastInput();
}

void KCalculator::slotACclicked(void)
{
	core.Reset();
	calc_display->Reset();

	UpdateDisplay(true);
}

void KCalculator::slotParenOpenclicked(void)
{
	core.ParenOpen(calc_display->getAmount());

	// What behavior, if e.g.: "12(6*6)"??
	//UpdateDisplay(true);
}

void KCalculator::slotParenCloseclicked(void)
{
	core.ParenClose(calc_display->getAmount());

	UpdateDisplay(true);
}

void KCalculator::slotANDclicked(void)
{
	core.And(calc_display->getAmount());

	UpdateDisplay(true);
}

void KCalculator::slotXclicked(void)
{
	core.Multiply(calc_display->getAmount());

	UpdateDisplay(true);
}

void KCalculator::slotDivisionclicked(void)
{
	core.Divide(calc_display->getAmount());

	UpdateDisplay(true);
}

void KCalculator::slotORclicked(void)
{
	core.Or(calc_display->getAmount());

	UpdateDisplay(true);
}

void KCalculator::slotXORclicked(void)
{
	core.Xor(calc_display->getAmount());

	UpdateDisplay(true);
}

void KCalculator::slotPlusclicked(void)
{
	core.Plus(calc_display->getAmount());

	UpdateDisplay(true);
}

void KCalculator::slotMinusclicked(void)
{
	core.Minus(calc_display->getAmount());

	UpdateDisplay(true);
}

void KCalculator::slotLeftShiftclicked(void)
{
	core.ShiftLeft(calc_display->getAmount());

	UpdateDisplay(true);
}

void KCalculator::slotRightShiftclicked(void)
{
	core.ShiftRight(calc_display->getAmount());

	UpdateDisplay(true);
}

void KCalculator::slotPeriodclicked(void)
{
	calc_display->newCharacter('.');
}

void KCalculator::EnterEqual()
{
	core.Equal(calc_display->getAmount());

	UpdateDisplay(true, true);
}

void KCalculator::slotEqualclicked(void)
{
	EnterEqual();
}

void KCalculator::slotPercentclicked(void)
{
	core.Percent(calc_display->getAmount());

	UpdateDisplay(true);
}

void KCalculator::slotNegateclicked(void)
{
	core.Complement(calc_display->getAmount());

	UpdateDisplay(true);
}

void KCalculator::slotModclicked(void)
{
	if (inverse)
		core.InvMod(calc_display->getAmount());
	else
		core.Mod(calc_display->getAmount());

	UpdateDisplay(true);
}

void KCalculator::slotStatNumclicked(void)
{
	if(!inverse)
	{
		core.StatCount(0);
	}
	else
	{
		pbInv->setOn(false);
		core.StatSum(0);
	}

	UpdateDisplay(true);
}

void KCalculator::slotStatMeanclicked(void)
{
	if(!inverse)
		core.StatMean(0);
	else
	{
		pbInv->setOn(false);
		core.StatSumSquares(0);
	}

	UpdateDisplay(true);
}

void KCalculator::slotStatStdDevclicked(void)
{
	if(!inverse)
	{
		// std (n-1)
		core.StatStdDeviation(0);
	}
	else
	{
		// std (n)
		pbInv->setOn(false);
		core.StatStdSample(0);
	}

	UpdateDisplay(true);
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
		pbInv->setOn(false);
	}
	// it seems two different modes should be implemented, but...?
	UpdateDisplay(true);
}

void KCalculator::slotStatDataInputclicked(void)
{
	if(!inverse)
	{
		core.StatDataNew(calc_display->getAmount());
	}
	else
	{
		pbInv->setOn(false);
		core.StatDataDel(0);
		statusBar()->message(i18n("Last stat item erased"), 3000);
	}

	UpdateDisplay(true);
}

void KCalculator::slotStatClearDataclicked(void)
{
        if(!inverse)
	{
		core.StatClearAll(0);
		statusBar()->message(i18n("Stat mem cleared"), 3000);
	}
	else
	{
		pbInv->setOn(false);
		UpdateDisplay(false);
	}
}

void KCalculator::slotC1clicked(void)
{
	//set the display to the configured value of Constant C1
	calc_display->setAmount(KCalcSettings::valueConstant1().toDouble());

	UpdateDisplay(false);
}
void KCalculator::slotC2clicked(void)
{
	//set the display to the configured value of Constant C2
	calc_display->setAmount( KCalcSettings::valueConstant2().toDouble());

	UpdateDisplay(false);
}
void KCalculator::slotC3clicked(void)
{
	//set the display to the configured value of Constant C3
	calc_display->setAmount(KCalcSettings::valueConstant3().toDouble());

	UpdateDisplay(false);
}
void KCalculator::slotC4clicked(void)
{
	//set the display to the configured value of Constant C4
	calc_display->setAmount(KCalcSettings::valueConstant4().toDouble());

	UpdateDisplay(false);
}
void KCalculator::slotC5clicked(void)
{
	//set the display to the configured value of Constant C5
	calc_display->setAmount(KCalcSettings::valueConstant5().toDouble());

	UpdateDisplay(false);
}
void KCalculator::slotC6clicked(void)
{;
	//set the display to the configured value of Constant C6
	calc_display->setAmount(KCalcSettings::valueConstant6().toDouble());

	UpdateDisplay(false);
}

void KCalculator::showSettings()
{
	// Check if there is already a dialog and if so bring
	// it to the foreground.
	if(KConfigDialog::showDialog("settings"))
		return;

	// Create a new dialog with the same name as the above checking code.
	KConfigDialog *dialog = new KConfigDialog(this, "settings", KCalcSettings::self());
	dialog->enableButtonSeparator( true );

	// Add the general page.  Store the settings in the General group and
	// use the icon package_settings.
	General *general = new General(0, "General");
	#ifdef HAVE_LONG_DOUBLE
	int maxprec = 16;
	#else
	int maxprec = 12 ;
	#endif
	general->kcfg_Precision->setMaxValue(maxprec);
	dialog->addPage(general, i18n("General"), "package_settings", i18n("General Settings"));

	QWidget *fontWidget = new QWidget(0,"Font");
	QVBoxLayout *fontLayout = new QVBoxLayout(fontWidget);
	KFontChooser *mFontChooser =
		new KFontChooser(fontWidget, "kcfg_Font", false, QStringList(), false, 6);
	fontLayout->addWidget(mFontChooser);
	dialog->addPage(fontWidget, i18n("Font"), "fonts", i18n("Select Display Font"));

	Colors *color = new Colors(0, "Color");

	dialog->addPage(color, i18n("Colors"), "colors", i18n("Button & Display Colors"));

	Constants *constant = new Constants(0, "Constant" );
        KDoubleValidator *valid = new KDoubleValidator( this );
        constant->kcfg_valueConstant1->setValidator( valid );
        constant->kcfg_valueConstant2->setValidator( valid );
        constant->kcfg_valueConstant3->setValidator( valid );
        constant->kcfg_valueConstant4->setValidator( valid );
        constant->kcfg_valueConstant5->setValidator( valid );
        constant->kcfg_valueConstant6->setValidator( valid );

	dialog->addPage(constant, "Constants", i18n("Constants"));

	// When the user clicks OK or Apply we want to update our settings.
	connect(dialog, SIGNAL(settingsChanged()), SLOT(updateSettings()));

	// Display the dialog.
	dialog->show();
}

void KCalculator::slotStatshow(bool toggled)
{
	if(toggled)
	{
		pbStat["NumData"]->show();
		pbStat["Mean"]->show();
		pbStat["StandardDeviation"]->show();
		pbStat["Median"]->show();
		pbStat["InputData"]->show();
		pbStat["ClearData"]->show();
	}
	else
	{
		pbStat["NumData"]->hide();
		pbStat["Mean"]->hide();
		pbStat["StandardDeviation"]->hide();
		pbStat["Median"]->hide();
		pbStat["InputData"]->hide();
		pbStat["ClearData"]->hide();
	}
	adjustSize();
	setFixedSize(sizeHint());
	KCalcSettings::setShowStat(toggled);
}

void KCalculator::slotTrigshow(bool toggled)
{
	if(toggled)
	{
	        pbTrig["HypMode"]->show();
		pbTrig["Sine"]->show();
		pbTrig["Cosine"]->show();
		pbTrig["Tangent"]->show();
		pbAngleChoose->show();
		if(!statusBar()->hasItem(2))
			statusBar()->insertFixedItem(" DEG ", 2, true);
		statusBar()->setItemAlignment(2, AlignCenter);
		slotAngleSelected(0);
	}
	else
	{
	        pbTrig["HypMode"]->hide();
		pbTrig["Sine"]->hide();
		pbTrig["Cosine"]->hide();
		pbTrig["Tangent"]->hide();
		pbAngleChoose->hide();
		if(statusBar()->hasItem(2))
			statusBar()->removeItem(2);
	}
	adjustSize();
	setFixedSize(sizeHint());
	KCalcSettings::setShowTrig(toggled);
}

void KCalculator::slotExpLogshow(bool toggled)
{
	if(toggled)
	{
		pbExp["Log10"]->show();
		pbExp["LogNatural"]->show();
	}
	else
	{
		pbExp["Log10"]->hide();
		pbExp["LogNatural"]->hide();
	}
	adjustSize();
	setFixedSize(sizeHint());
	KCalcSettings::setShowExpLog(toggled);
}

void KCalculator::slotLogicshow(bool toggled)
{
	if(toggled)
	{
	        pbLogic["AND"]->show();
		pbLogic["OR"]->show();
		pbLogic["XOR"]->show();
		pbLogic["One-Complement"]->show();
		pbLogic["LeftShift"]->show();
		pbLogic["RightShift"]->show();
		if(!statusBar()->hasItem(1))
			statusBar()->insertFixedItem(" HEX ", 1, true);
		statusBar()->setItemAlignment(1, AlignCenter);
		resetBase();
		BaseChooseGroup->show();
		for (int i=10; i<16; i++)
			(NumButtonGroup->find(i))->show();
	}
	else
	{
	    pbLogic["AND"]->hide();
		pbLogic["OR"]->hide();
		pbLogic["XOR"]->hide();
		pbLogic["One-Complement"]->hide();
		pbLogic["LeftShift"]->hide();
		pbLogic["RightShift"]->hide();
		// Hide Hex-Buttons, but first switch back to decimal
		resetBase();
		BaseChooseGroup->hide();
		if(statusBar()->hasItem(1))
			statusBar()->removeItem(1);
		for (int i=10; i<16; i++)
			(NumButtonGroup->find(i))->hide();
	}
	adjustSize();
	setFixedSize(sizeHint());
	KCalcSettings::setShowLogic(toggled);
}

void KCalculator::slotConstantsShow(bool toggled)
{
	if(toggled)
	{
		pbConstant["C1"]->show();
		pbConstant["C2"]->show();
		pbConstant["C3"]->show();
		pbConstant["C4"]->show();
		pbConstant["C5"]->show();
		pbConstant["C6"]->show();

	}
	else
	{
		pbConstant["C1"]->hide();
		pbConstant["C2"]->hide();
		pbConstant["C3"]->hide();
		pbConstant["C4"]->hide();
		pbConstant["C5"]->hide();
		pbConstant["C6"]->hide();
	}
	adjustSize();
	setFixedSize(sizeHint());
	KCalcSettings::setShowConstants(toggled);
}
// This function is for setting the constant names configured in the kcalc settings menue. If the user doesn't
// enter a name for the constant C1 to C6 is used.
void KCalculator::changeButtonNames()
{
	if(KCalcSettings::nameConstant1().isEmpty() )
	{
		pbConstant["C1"]->setText("C1");
	}
	else
	{
		pbConstant["C1"]->setText(KCalcSettings::nameConstant1());
	}

	if(KCalcSettings::nameConstant2().isEmpty() )
	{
		pbConstant["C2"]->setText("C2");
	}
	else
	{
		pbConstant["C2"]->setText(KCalcSettings::nameConstant2());
	}

	if(KCalcSettings::nameConstant3().isEmpty() )
	{
		pbConstant["C3"]->setText("C3");
	}
	else
	{
		pbConstant["C3"]->setText(KCalcSettings::nameConstant3());
	}

	if(KCalcSettings::nameConstant4().isEmpty() )
	{
		pbConstant["C4"]->setText("C4");
	}
	else
	{
		pbConstant["C4"]->setText(KCalcSettings::nameConstant4());
	}

	if(KCalcSettings::nameConstant5().isEmpty() )
	{
		pbConstant["C5"]->setText("C5");
	}
	else
	{
		pbConstant["C5"]->setText(KCalcSettings::nameConstant5());
	}

	if(KCalcSettings::nameConstant6().isEmpty() )
	{
		pbConstant["C6"]->setText("C6");
	}
	else
	{
		pbConstant["C6"]->setText(KCalcSettings::nameConstant6());
	}
}

void KCalculator::slotShowAll(void)
{
	// I wonder why "setChecked" does not emit "toggled"
	if(!actionStatshow->isChecked()) actionStatshow->activate();
	if(!actionTrigshow->isChecked()) actionTrigshow->activate();
	if(!actionExpLogshow->isChecked()) actionExpLogshow->activate();
	if(!actionLogicshow->isChecked()) actionLogicshow->activate();
	if(!actionConstantsShow->isChecked()) actionConstantsShow->activate();
}

void KCalculator::slotHideAll(void)
{
	// I wonder why "setChecked" does not emit "toggled"
	if(actionStatshow->isChecked()) actionStatshow->activate();
	if(actionTrigshow->isChecked()) actionTrigshow->activate();
	if(actionExpLogshow->isChecked()) actionExpLogshow->activate();
	if(actionLogicshow->isChecked()) actionLogicshow->activate();
	if(actionConstantsShow->isChecked()) actionConstantsShow->activate();
}

void KCalculator::RefreshCalculator()
{
	pbInv->setOn(false);
	calc_display->Reset();
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
		setCaption(QString::null);
	}
	calc_display->changeSettings();

	updateGeometry();
	resize(minimumSize());

	//
	// 1999-10-31 Espen Sand: Don't ask me why ;)
	//
	kapp->processOneEvent();
	setFixedHeight(minimumHeight());
}

void KCalculator::UpdateDisplay(bool get_amount_from_core,
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

	pbInv->setOn(false);

}

void KCalculator::set_colors()
{
	QPushButton *p = NULL;

	calc_display->changeSettings();

	QColor bg = palette().active().background();

	QPalette numPal(KCalcSettings::numberButtonsColor(), bg);
	for(int i=0; i<10; i++)
	{
		(NumButtonGroup->find(i))->setPalette(numPal);
	}

	QPalette funcPal(KCalcSettings::functionButtonsColor(), bg);
	for(p = mFunctionButtonList.first(); p;
		p=mFunctionButtonList.next())
	{
		p->setPalette(funcPal);
	}

	QPalette statPal(KCalcSettings::statButtonsColor(), bg);
	for(p = mStatButtonList.first(); p; p=mStatButtonList.next())
	{
		p->setPalette(statPal);
	}

	QPalette hexPal(KCalcSettings::hexButtonsColor(), bg);
	for(int i=10; i<16; i++)
	{
		(NumButtonGroup->find(i))->setPalette(hexPal);
	}

	QPalette memPal(KCalcSettings::memoryButtonsColor(), bg);
	for(p = mMemButtonList.first(); p; p=mMemButtonList.next())
	{
		p->setPalette(memPal);
	}

	QPalette opPal(KCalcSettings::operationButtonsColor(), bg);
	for(p = mOperationButtonList.first(); p;
	p=mOperationButtonList.next())
	{
		p->setPalette(opPal);
	}
}

void KCalculator::set_precision()
{
	// TODO: make this do something!!
	UpdateDisplay(false);
}

void KCalculator::history_next()
{
	if(!calc_display->history_next()  &&  KCalcSettings::beep())
		KNotifyClient::beep();
}

void KCalculator::history_prev()
{
	if(!calc_display->history_prev()  &&  KCalcSettings::beep())
		KNotifyClient::beep();
}

void KCalculator::slotConfigureKeys()
{
  KKeyDialog::configure( actionCollection(), this );
}


bool KCalculator::eventFilter(QObject *o, QEvent *e)
{
	if(e->type() == QEvent::DragEnter)
	{
		QDragEnterEvent *ev = (QDragEnterEvent *)e;
		ev->accept(KColorDrag::canDecode(ev));
		return true;
	}
	else if(e->type() == QEvent::DragLeave)
	{
		return true;
	}
	else if(e->type() == QEvent::Drop)
	{
		if(!o->isA("KCalcButton"))
			return false;

		QColor c;
		QDropEvent *ev = (QDropEvent *)e;
		if( KColorDrag::decode(ev, c))
		{
		        QPtrList<KCalcButton> *list;
			int num_but;
			if((num_but = NumButtonGroup->id((KCalcButton*)o))
			   != -1)
			{
			  QPalette pal(c, palette().active().background());

			  // Was it hex-button or normal digit??
			  if (num_but <10)
			    for(int i=0; i<10; i++)
			      (NumButtonGroup->find(i))->setPalette(pal);
			  else
			    for(int i=10; i<16; i++)
			      (NumButtonGroup->find(i))->setPalette(pal);

			  return true;
			}
			else if( mFunctionButtonList.findRef((KCalcButton*)o) != -1)
			{
				list = &mFunctionButtonList;
			}
			else if( mStatButtonList.findRef((KCalcButton*)o) != -1)
			{
				list = &mStatButtonList;
			}
			else if( mMemButtonList.findRef((KCalcButton*)o) != -1)
			{
				list = &mMemButtonList;
			}
			else if( mOperationButtonList.findRef((KCalcButton*)o) != -1)
			{
				list = &mOperationButtonList;
			}
			else
				return false;

			QPalette pal(c, palette().active().background());

			for(KCalcButton *p = list->first(); p; p=list->next())
				p->setPalette(pal);
		}

		return true;
	}
	else
	{
		return KMainWindow::eventFilter(o, e);
	}
}




////////////////////////////////////////////////////////////////
// Include the meta-object code for classes in this file
//

#include "kcalc.moc"


extern "C" int kdemain(int argc, char *argv[])
{
        QString precisionStatement;

#ifdef HAVE_LONG_DOUBLE
        precisionStatement = QString(I18N_NOOP("Built with %1 bit (long double) precision"))
                                     .arg(sizeof(long double) * 8);
#else
        precisionStatement =  QString(I18N_NOOP("Built with %1 bit precision"
                                         "\n\nNote: Due to a broken C library, KCalc's precision \n"
                                         "was conditionally reduced at compile time from\n"
                                         "'long double' to 'double'. \n\n"
                                         "Owners of systems with a working libc may \n"
                                         "want to recompile KCalc with 'long double' \n"
                                         "precision enabled. See the README for details."))
                                     .arg(sizeof(long) * 8);
#endif

	KAboutData aboutData( "kcalc", I18N_NOOP("KCalc"),
                          version, description, KAboutData::License_GPL,
                          I18N_NOOP("(c) 1996-2000, Bernd Johannes Wuebben\n"
                                    "(c) 2000-2003, The KDE Team"),
                          precisionStatement.latin1());

	aboutData.addAuthor("Bernd Johannes Wuebben", 0, "wuebben@kde.org");
	aboutData.addAuthor("Evan Teran", 0, "emt3734@rit.edu");
	aboutData.addAuthor("Espen Sand", 0, "espen@kde.org");
	aboutData.addAuthor("Chris Howells", 0, "howells@kde.org");
    aboutData.addAuthor("Aaron J. Seigo", 0, "aseigo@olympusproject.org");
    aboutData.addAuthor("Charles Samuels", 0, "charles@altair.dhs.org");
    aboutData.addAuthor("Klaus NiederkrÃ¼ger", 0, "kniederk@math.uni-koeln.de");
	KCmdLineArgs::init(argc, argv, &aboutData);

	KApplication app;
#if 0
	app->enableSessionManagement(true);
	app->setWmCommand(argv[0]);
#endif

	KCalculator *calc = new KCalculator;
	app.setTopWidget(calc);
	calc->setCaption(QString::null);
	calc->show();

	int exitCode = app.exec();

	return(exitCode);
}

