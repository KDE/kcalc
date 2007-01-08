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

#include "../config.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include <QButtonGroup>
#include <QFont>
#include <QGroupBox>
#include <QKeyEvent>
#include <QLayout>
#include <QObjectList>
#include <QRadioButton>
#include <QShortcut>
#include <QSpinBox>
#include <QStyle>
#include <QToolTip>



#include <kaboutdata.h>
#include <kactioncollection.h>
#include <kacceleratormanager.h>
#include <kaction.h>
#include <kapplication.h>
#include <ktoolbar.h>
#include <kcmdlineargs.h>
#include <kcolorbutton.h>
#include <kcolormimedata.h>
#include <kconfig.h>
#include <kconfigdialog.h>
#include <kdialog.h>
#include <kfontdialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kkeydialog.h>
#include <kmenu.h>
#include <kmenubar.h>
#include <knumvalidator.h>
#include <kpushbutton.h>
#include <kstatusbar.h>
#include <kstandardaction.h>
#include <kxmlguifactory.h>
#include <ktoggleaction.h>

#include "dlabel.h"
#include "kcalc.h"
#include "kcalc_const_menu.h"
#include "version.h"
#include "general.h"
#include "kcalc_settings.h"
#include "kcalc_bitset.h"


static const char description[] = I18N_NOOP("KDE Calculator");
static const char version[] = KCALCVERSION;


KCalculator::KCalculator(QWidget *parent)
	: KMainWindow(parent), inverse(false), hyp_mode(false),
	  memory_num(0.0), calc_display(NULL),
	  mInternalSpacing(4), core()
{
	/* central widget to contain all the elements */
	QWidget *central = new QWidget(this);
	setCentralWidget(central);
	KAcceleratorManager::setNoAccel( central );

	// load science constants from xml-file
	KCalcConstMenu::init_consts();

	// Detect color change
	connect(KGlobalSettings::self(),SIGNAL(kdisplayPaletteChanged()), SLOT(set_colors()));

	calc_display = new DispLogic(central, actionCollection());

	setupMainActions();

	setupStatusbar();

	createGUI();

	// How can I make the toolBar not appear at all?
	// This is not a nice solution.
	toolBar()->hide();

	// Create Button to select BaseMode
	BaseChooseGroup = new QGroupBox(i18n("Base"), central);

	//BaseChooseGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed, false);
	QHBoxLayout * base_but_layout = new QHBoxLayout(BaseChooseGroup);

	pbBaseChoose[0] =  new QRadioButton(i18n("He&x"), BaseChooseGroup);
	connect(pbBaseChoose[0], SIGNAL(clicked(void)), SLOT(slotBaseHex(void)));
	base_but_layout->addWidget(pbBaseChoose[0]);
	pbBaseChoose[0]->setToolTip( i18n("Switch base to hexadecimal."));

	pbBaseChoose[1] =  new QRadioButton(i18n("&Dec"), BaseChooseGroup);
	connect(pbBaseChoose[1], SIGNAL(clicked(void)), SLOT(slotBaseDec(void)));
	base_but_layout->addWidget(pbBaseChoose[1]);
	pbBaseChoose[1]->setToolTip( i18n("Switch base to decimal."));

	pbBaseChoose[2] =  new QRadioButton(i18n("&Oct"), BaseChooseGroup);
	connect(pbBaseChoose[2], SIGNAL(clicked(void)), SLOT(slotBaseOct(void)));
	base_but_layout->addWidget(pbBaseChoose[2]);
	pbBaseChoose[2]->setToolTip( i18n("Switch base to octal."));

	pbBaseChoose[3] =  new QRadioButton(i18n("&Bin"), BaseChooseGroup);
	connect(pbBaseChoose[3], SIGNAL(clicked(void)), SLOT(slotBaseBin(void)));
	base_but_layout->addWidget(pbBaseChoose[3]);
	pbBaseChoose[3]->setToolTip( i18n("Switch base to binary."));


	// Create Button to select AngleMode
	pbAngleChoose =  new QPushButton(i18n("&Angle"), central);
	pbAngleChoose->setToolTip( i18n("Choose the unit for the angle measure"));
	pbAngleChoose->setAutoDefault(false);

	KMenu *angle_menu = new KMenu(pbAngleChoose);
	angle_menu->insertItem(i18n("Degrees"), 0);
	angle_menu->insertItem(i18n("Radians"), 1);
	angle_menu->insertItem(i18n("Gradians"), 2);

	angle_menu->setCheckable(true);
	connect(angle_menu, SIGNAL(activated(int)), SLOT(slotAngleSelected(int)));
	pbAngleChoose->setMenu(angle_menu);



	pbInv = new KCalcButton("Inv", central,	i18n("Inverse mode"));
	pbInv->setShortcut(QKeySequence(Qt::Key_I));
	connect(pbInv, SIGNAL(toggled(bool)), SLOT(slotInvtoggled(bool)));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbInv, SLOT(slotSetAccelDisplayMode(bool)));
	pbInv->setCheckable(true);

	// Create Bitset Display
	mBitset = new KCalcBitset(central);
	mBitset->setToolTip(i18n("Click on a Bit to toggle it."));

	//
	//  Create Calculator Buttons
	//

	// First the widgets that are the parents of the buttons
	mSmallPage = new QWidget(central);
	mLargePage = new QWidget(central);
	mNumericPage = 	setupNumericKeys(central);

	setupLogicKeys(mSmallPage);
	setupStatisticKeys(mSmallPage);
	setupScientificKeys(mSmallPage);
	setupConstantsKeys(mSmallPage);


	pbMod = new KCalcButton(mSmallPage);
	pbMod->addMode(ModeNormal, "Mod", i18n("Modulo"));
	pbMod->addMode(ModeInverse, "IntDiv", i18n("Integer division"));
	pbMod->setShortcut(QKeySequence(Qt::Key_Colon));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		pbMod, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbMod, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbMod, SIGNAL(clicked(void)), SLOT(slotModclicked(void)));

	pbReci = new KCalcButton(mSmallPage);
	pbReci->addMode(ModeNormal, "1/x", i18n("Reciprocal"));
	pbReci->setShortcut(QKeySequence(Qt::Key_R));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbReci, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbReci, SIGNAL(clicked(void)), SLOT(slotReciclicked(void)));

	pbFactorial = new KCalcButton(mSmallPage);
	pbFactorial->addMode(ModeNormal, "x!", i18n("Factorial"));
	pbFactorial->setShortcut(QKeySequence(Qt::Key_Exclam));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbFactorial, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbFactorial, SIGNAL(clicked(void)),SLOT(slotFactorialclicked(void)));

	// Representation of x^2 is moved to the function
	// changeRepresentation() that paints the letters When
	// pressing the INV Button a sqrt symbol will be drawn on that
	// button
	pbSquare = new KCalcButton(mSmallPage);
	pbSquare->addMode(ModeNormal, "x<sup>2</sup>", i18n("Square"), true);
	pbSquare->addMode(ModeInverse, "x<sup>3</sup>", i18n("Third power"), true);
	connect(this, SIGNAL(switchShowAccels(bool)),
	        pbSquare, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
	        pbSquare, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbSquare, SIGNAL(clicked(void)), SLOT(slotSquareclicked(void)));

	pbRoot = new KSquareButton(mSmallPage);
	pbRoot->addMode(ModeNormal, QString::null, i18n("Square root"));
	pbRoot->addMode(ModeInverse, QString::null, i18n("Cube root"));
	connect(this, SIGNAL(switchShowAccels(bool)),
	        pbRoot, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
	        pbRoot, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(pbRoot, SIGNAL(clicked(void)), SLOT(slotRootclicked(void)));


	// Representation of x^y is moved to the function
	// changeRepresentation() that paints the letters When
	// pressing the INV Button y^x will be drawn on that button
	pbPower = new KCalcButton(mSmallPage);
	pbPower->addMode(ModeNormal, "x<sup>y</sup>", i18n("x to the power of y"), true);
	pbPower->addMode(ModeInverse, "x<sup>1/y</sup>", i18n("x to the power of 1/y"), true);
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbPower, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		pbPower, SLOT(slotSetMode(ButtonModeFlags,bool)));
	pbPower->setShortcut(QKeySequence(Qt::Key_AsciiCircum));
	connect(pbPower, SIGNAL(clicked(void)), SLOT(slotPowerclicked(void)));


	//
	// All these layouts are needed because all the groups have their
	// own size per row so we can't use one huge QGridLayout (mosfet)
	//
	QGridLayout *smallBtnLayout = new QGridLayout(mSmallPage);
	QGridLayout *largeBtnLayout = new QGridLayout(mLargePage);

	QHBoxLayout *topLayout		= new QHBoxLayout();
	QHBoxLayout *btnLayout		= new QHBoxLayout();

	// bring them all together
	QVBoxLayout *mainLayout = new QVBoxLayout(central);

	mainLayout->addLayout(topLayout);
	mainLayout->addWidget(mBitset);
	mainLayout->addLayout(btnLayout);

	// button layout
	btnLayout->addWidget(mSmallPage, 0, Qt::AlignTop);
	btnLayout->addSpacing(2*mInternalSpacing);
	btnLayout->addWidget(mNumericPage, 0, Qt::AlignTop);
	btnLayout->addSpacing(2*mInternalSpacing);
	btnLayout->addWidget(mLargePage, 0, Qt::AlignTop);

	// small button layout
	smallBtnLayout->addWidget(pbStat["NumData"], 0, 0);
	smallBtnLayout->addWidget(pbScientific["HypMode"], 0, 1);
	smallBtnLayout->addWidget(pbLogic["AND"], 0, 2);
	smallBtnLayout->addWidget(pbMod, 0, 3);
	smallBtnLayout->addWidget(NumButtonGroup->button(0xA), 0, 4);
	smallBtnLayout->addWidget(pbConstant[0], 0, 5);

	smallBtnLayout->addWidget(pbStat["Mean"], 1, 0);
	smallBtnLayout->addWidget(pbScientific["Sine"], 1, 1);
	smallBtnLayout->addWidget(pbLogic["OR"], 1, 2);
	smallBtnLayout->addWidget(pbReci, 1, 3);
	smallBtnLayout->addWidget(NumButtonGroup->button(0xB), 1, 4);
	smallBtnLayout->addWidget(pbConstant[1], 1, 5);

	smallBtnLayout->addWidget(pbStat["StandardDeviation"], 2, 0);
	smallBtnLayout->addWidget(pbScientific["Cosine"], 2, 1);
	smallBtnLayout->addWidget(pbLogic["XOR"], 2, 2);
	smallBtnLayout->addWidget(pbFactorial, 2, 3);
	smallBtnLayout->addWidget(NumButtonGroup->button(0xC), 2, 4);
	smallBtnLayout->addWidget(pbConstant[2], 2, 5);

	smallBtnLayout->addWidget(pbStat["Median"], 3, 0);
	smallBtnLayout->addWidget(pbScientific["Tangent"], 3, 1);
	smallBtnLayout->addWidget(pbLogic["LeftShift"], 3, 2);
	smallBtnLayout->addWidget(pbSquare, 3, 3);
	smallBtnLayout->addWidget(NumButtonGroup->button(0xD), 3, 4);
	smallBtnLayout->addWidget(pbConstant[3], 3, 5);

	smallBtnLayout->addWidget(pbStat["InputData"], 4, 0);
	smallBtnLayout->addWidget(pbScientific["Log10"], 4, 1);
	smallBtnLayout->addWidget(pbLogic["RightShift"], 4, 2);
	smallBtnLayout->addWidget(pbRoot, 4, 3);
	smallBtnLayout->addWidget(NumButtonGroup->button(0xE), 4, 4);
	smallBtnLayout->addWidget(pbConstant[4], 4, 5);

	smallBtnLayout->addWidget(pbStat["ClearData"], 5, 0);
	smallBtnLayout->addWidget(pbScientific["LogNatural"], 5, 1);
	smallBtnLayout->addWidget(pbLogic["One-Complement"], 5, 2);
	smallBtnLayout->addWidget(pbPower, 5, 3);
	smallBtnLayout->addWidget(NumButtonGroup->button(0xF), 5, 4);
	smallBtnLayout->addWidget(pbConstant[5], 5, 5);

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

	largeBtnLayout->addWidget(pbMemRecall, 2, 0);
	largeBtnLayout->addWidget(pbMemStore, 2, 1);

	largeBtnLayout->addWidget(pbMemPlusMinus, 3, 0);
	largeBtnLayout->addWidget(pbMC, 3, 1);

	largeBtnLayout->addWidget(pbPercent, 4, 0);
	largeBtnLayout->addWidget(pbPlusMinus, 4, 1);

	// top layout
	topLayout->addWidget(pbAngleChoose);
	topLayout->addWidget(BaseChooseGroup);
	topLayout->addWidget(pbInv);
	topLayout->addWidget(calc_display, 10);

	mFunctionButtonList.append(pbScientific["HypMode"]);
	mFunctionButtonList.append(pbInv);
	mFunctionButtonList.append(pbRoot);
	mFunctionButtonList.append(pbScientific["Sine"]);
	mFunctionButtonList.append(pbPlusMinus);
	mFunctionButtonList.append(pbScientific["Cosine"]);
	mFunctionButtonList.append(pbReci);
	mFunctionButtonList.append(pbScientific["Tangent"]);
	mFunctionButtonList.append(pbFactorial);
	mFunctionButtonList.append(pbScientific["Log10"]);
	mFunctionButtonList.append(pbSquare);
	mFunctionButtonList.append(pbScientific["LogNatural"]);
	mFunctionButtonList.append(pbPower);

	mMemButtonList.append(pbEE);
	mMemButtonList.append(pbMemRecall);
	mMemButtonList.append(pbMemPlusMinus);
	mMemButtonList.append(pbMemStore);
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

	actionScientificshow->setChecked(KCalcSettings::showScientific());
	slotScientificshow(KCalcSettings::showScientific());

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
	KStandardAction::quit(this, SLOT(close()), actionCollection());

	// edit menu
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


        QAction*showAct = actionCollection()->addAction( "show_all" );
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

QWidget* KCalculator::setupNumericKeys(QWidget *parent)
{
	Q_CHECK_PTR(mSmallPage);
	Q_CHECK_PTR(mLargePage);

	QWidget *thisPage = new QWidget(parent);

	KCalcButton *tmp_pb;

	NumButtonGroup = new QButtonGroup(this);
	connect(NumButtonGroup, SIGNAL(buttonClicked(int)),
		SLOT(slotNumberclicked(int)));

	tmp_pb = new KCalcButton("0", thisPage);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_0));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(tmp_pb, 0);

	tmp_pb = new KCalcButton("1", thisPage);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_1));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(tmp_pb, 1);

	tmp_pb = new KCalcButton("2", thisPage);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_2));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(tmp_pb, 2);

	tmp_pb = new KCalcButton("3", thisPage);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_3));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(tmp_pb, 3);

	tmp_pb = new KCalcButton("4", thisPage);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_4));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(tmp_pb, 4);

	tmp_pb = new KCalcButton("5", thisPage);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_5));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(tmp_pb, 5);

	tmp_pb = new KCalcButton("6", thisPage);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_6));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(tmp_pb, 6);

	tmp_pb = new KCalcButton("7", thisPage);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_7));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(tmp_pb, 7);

	tmp_pb = new KCalcButton("8", thisPage);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_8));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(tmp_pb, 8);

	tmp_pb = new KCalcButton("9", thisPage);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_9));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(tmp_pb, 9);

	pbEE = new KCalcButton(thisPage);
	pbEE->addMode(ModeNormal, "x<small>" "\xb7" "10</small><sup>y</sup>",
			i18n("Exponent"), true);
	pbEE->setShortcut(QKeySequence(Qt::Key_E));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbEE, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbEE, SIGNAL(clicked(void)), SLOT(slotEEclicked(void)));

	pbParenClose = new KCalcButton(")", mLargePage);
	pbParenClose->setShortcut(QKeySequence(Qt::Key_ParenRight));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbParenClose, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbParenClose,SIGNAL(clicked(void)),SLOT(slotParenCloseclicked(void)));

	pbX = new KCalcButton("X", thisPage, i18n("Multiplication"));
	pbX->setShortcut(QKeySequence(Qt::Key_multiply));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbX, SLOT(slotSetAccelDisplayMode(bool)));
	new QShortcut( Qt::Key_Asterisk, pbX, SLOT(animateClick()) );
#if 0
	accel()->insert("Pressed '*'", i18n("Pressed Multiplication-Button"),
			0, Qt::Key_Asterisk, pbX, SLOT(animateClick()));
#endif
	connect(pbX, SIGNAL(clicked(void)), SLOT(slotXclicked(void)));

	pbDivision = new KCalcButton("/", thisPage, i18n("Division"));
	pbDivision->setShortcut(QKeySequence(Qt::Key_Slash));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbDivision, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbDivision, SIGNAL(clicked(void)), SLOT(slotDivisionclicked(void)));

	pbPlus = new KCalcButton("+", thisPage, i18n("Addition"));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbPlus, SLOT(slotSetAccelDisplayMode(bool)));
	pbPlus->setShortcut(QKeySequence(Qt::Key_Plus));
	connect(pbPlus, SIGNAL(clicked(void)), SLOT(slotPlusclicked(void)));

	pbMinus = new KCalcButton("-", thisPage, i18n("Subtraction"));
	pbMinus->setShortcut(QKeySequence(Qt::Key_Minus));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbMinus, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbMinus, SIGNAL(clicked(void)), SLOT(slotMinusclicked(void)));

	pbPeriod = new KCalcButton(KGlobal::locale()->decimalSymbol(), thisPage,
				   i18n("Decimal point"));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbPeriod, SLOT(slotSetAccelDisplayMode(bool)));
	pbPeriod->setShortcut(QKeySequence(Qt::Key_Period));
	new QShortcut( Qt::Key_Comma, pbPeriod, SLOT(animateClick()) );
#if 0
	accel()->insert("Decimal Point (Period)", i18n("Pressed Decimal Point"),
			0, Qt::Key_Period, pbPeriod, SLOT(animateClick()));
	accel()->insert("Decimal Point (Comma)", i18n("Pressed Decimal Point"),
			0, Qt::Key_Comma, pbPeriod, SLOT(animateClick()));
#endif
	connect(pbPeriod, SIGNAL(clicked(void)), SLOT(slotPeriodclicked(void)));

	pbEqual = new KCalcButton("=", thisPage, i18n("Result"));
	pbEqual->setShortcut(QKeySequence(Qt::Key_Enter));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbEqual, SLOT(slotSetAccelDisplayMode(bool)));
	new QShortcut( Qt::Key_Equal, pbEqual, SLOT(animateClick()) );
	new QShortcut( Qt::Key_Return, pbEqual, SLOT(animateClick()) );
#if 0
	accel()->insert("Entered Equal", i18n("Pressed Equal-Button"),
			0, Qt::Key_Equal, pbEqual, SLOT(animateClick()));
	accel()->insert("Entered Return", i18n("Pressed Equal-Button"),
			0, Qt::Key_Return, pbEqual, SLOT(animateClick()));
#endif
	connect(pbEqual, SIGNAL(clicked(void)), SLOT(slotEqualclicked(void)));


	QGridLayout *thisLayout = new QGridLayout(thisPage);

	// large button layout
	thisLayout->addWidget(pbEE, 0, 0);
	thisLayout->addWidget(pbDivision, 0, 1);
	thisLayout->addWidget(pbX, 0, 2);
	thisLayout->addWidget(pbMinus, 0, 3);

	thisLayout->addWidget(NumButtonGroup->buttons()[7], 1, 0);
	thisLayout->addWidget(NumButtonGroup->buttons()[8], 1, 1);
	thisLayout->addWidget(NumButtonGroup->buttons()[9], 1, 2);
	thisLayout->addWidget(pbPlus, 1, 3, 2, 1);

	thisLayout->addWidget(NumButtonGroup->buttons()[4], 2, 0);
	thisLayout->addWidget(NumButtonGroup->buttons()[5], 2, 1);
	thisLayout->addWidget(NumButtonGroup->buttons()[6], 2, 2);
	// thisLayout->addWidget(pbPlus, 1, 3, 2, 1);

	thisLayout->addWidget(NumButtonGroup->buttons()[1], 3, 0);
	thisLayout->addWidget(NumButtonGroup->buttons()[2], 3, 1);
	thisLayout->addWidget(NumButtonGroup->buttons()[3], 3, 2);
	thisLayout->addWidget(pbEqual, 3, 3, 2, 1);

	thisLayout->addWidget(NumButtonGroup->buttons()[0], 4, 0, 1, 2);
	thisLayout->addWidget(pbPeriod, 4, 2);
	// thisLayout->addWidget(pbEqual, 3, 3, 2, 1);

	thisLayout->addItem( new QSpacerItem(10, 0 ), 0, 0 );
	thisLayout->addItem( new QSpacerItem(10, 0 ), 0, 1 );
	thisLayout->addItem( new QSpacerItem(10, 0 ), 0, 2 );
	thisLayout->addItem( new QSpacerItem(10, 0 ), 0, 3 );
	thisLayout->addItem( new QSpacerItem(10, 0 ), 0, 4 );


	pbMemRecall = new KCalcButton("MR", mLargePage, i18n("Memory recall"));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbMemRecall, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbMemRecall, SIGNAL(clicked(void)),
		SLOT(slotMemRecallclicked(void)));
	pbMemRecall->setDisabled(true); // At start, there is nothing in memory

	pbMemPlusMinus = new KCalcButton(mLargePage);
	pbMemPlusMinus->addMode(ModeNormal, "M+", i18n("Add display to memory"));
	pbMemPlusMinus->addMode(ModeInverse, "M-", i18n("Subtract from memory"));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		pbMemPlusMinus, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbMemPlusMinus, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbMemPlusMinus,SIGNAL(clicked(void)),
		SLOT(slotMemPlusMinusclicked(void)));

	pbMemStore = new KCalcButton("MS", mLargePage,
				     i18n("Memory store"));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbMemStore, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbMemStore, SIGNAL(clicked(void)), SLOT(slotMemStoreclicked(void)));


	pbMC = new KCalcButton("MC", mLargePage, i18n("Clear memory"));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbMC, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbMC, SIGNAL(clicked(void)), SLOT(slotMCclicked(void)));

	pbClear = new KCalcButton("C", mLargePage, i18n("Clear"));
	pbClear->setShortcut(QKeySequence(Qt::Key_Prior));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbClear, SLOT(slotSetAccelDisplayMode(bool)));
	new QShortcut( Qt::Key_Escape, pbClear, SLOT(animateClick()) );
#if 0
	accel()->insert("Entered 'ESC'", i18n("Pressed ESC-Button"), 0,
			Qt::Key_Escape, pbClear, SLOT(animateClick()));
#endif
	connect(pbClear, SIGNAL(clicked(void)), SLOT(slotClearclicked(void)));

	pbAC = new KCalcButton("AC", mLargePage, i18n("Clear all"));
	pbAC->setShortcut(QKeySequence(Qt::Key_Delete));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbAC, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbAC, SIGNAL(clicked(void)), SLOT(slotACclicked(void)));

	pbParenOpen = new KCalcButton("(", mLargePage);
	pbParenOpen->setShortcut(QKeySequence(Qt::Key_ParenLeft));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbParenOpen, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbParenOpen, SIGNAL(clicked(void)),SLOT(slotParenOpenclicked(void)));

	pbPercent = new KCalcButton("%", mLargePage, i18n("Percent"));
	pbPercent->setShortcut(QKeySequence(Qt::Key_Percent));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbPercent, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbPercent, SIGNAL(clicked(void)), SLOT(slotPercentclicked(void)));

	pbPlusMinus = new KCalcButton("\xb1", mLargePage, i18n("Change sign"));
	pbPlusMinus->setShortcut(QKeySequence(Qt::Key_Backslash));
	connect(this, SIGNAL(switchShowAccels(bool)),
		pbPlusMinus, SLOT(slotSetAccelDisplayMode(bool)));
	connect(pbPlusMinus, SIGNAL(clicked(void)), SLOT(slotPlusMinusclicked(void)));


	tmp_pb = new KCalcButton("A", mSmallPage);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_A));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(tmp_pb, 0xA);

	tmp_pb = new KCalcButton("B", mSmallPage);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_B));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(tmp_pb, 0xB);

	tmp_pb = new KCalcButton("C", mSmallPage);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_C));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(tmp_pb, 0xC);

	tmp_pb = new KCalcButton("D", mSmallPage);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_D));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(tmp_pb, 0xD);

	tmp_pb = new KCalcButton("E", mSmallPage);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_E));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(tmp_pb, 0xE);

	tmp_pb = new KCalcButton("F", mSmallPage);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_F));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	NumButtonGroup->addButton(tmp_pb, 0xF);

	return thisPage;
}

void KCalculator::setupLogicKeys(QWidget *parent)
{
	Q_CHECK_PTR(parent);

	KCalcButton *tmp_pb;

	tmp_pb = new KCalcButton("AND", parent, i18n("Bitwise AND"));
	pbLogic.insert("AND", tmp_pb);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_Ampersand));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotANDclicked(void)));

	tmp_pb = new KCalcButton("OR", parent, i18n("Bitwise OR"));
	pbLogic.insert("OR", tmp_pb);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_Bar));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotORclicked(void)));

	tmp_pb = new KCalcButton("XOR", parent, i18n("Bitwise XOR"));
	pbLogic.insert("XOR", tmp_pb);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotXORclicked(void)));

	tmp_pb = new KCalcButton("Cmp", parent,	i18n("One's complement"));
	pbLogic.insert("One-Complement", tmp_pb);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_AsciiTilde));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotNegateclicked(void)));

	tmp_pb = new KCalcButton("Lsh", parent,	i18n("Left bit shift"));
	tmp_pb->setShortcut(QKeySequence(Qt::Key_Less));
	pbLogic.insert("LeftShift", tmp_pb);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)),
		SLOT(slotLeftShiftclicked(void)));

	tmp_pb = new KCalcButton("Rsh", parent,	i18n("Right bit shift"));
	tmp_pb->setShortcut(QKeySequence(Qt::Key_Greater));
	pbLogic.insert("RightShift", tmp_pb);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)),
		SLOT(slotRightShiftclicked(void)));
}

void KCalculator::setupScientificKeys(QWidget *parent)
{
	Q_CHECK_PTR(parent);

	KCalcButton *tmp_pb;

	tmp_pb = new KCalcButton("Hyp", parent, i18n("Hyperbolic mode"));
	pbScientific.insert("HypMode", tmp_pb);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_H));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(toggled(bool)), SLOT(slotHyptoggled(bool)));
	tmp_pb->setCheckable(true);

	tmp_pb = new KCalcButton(parent);
	pbScientific.insert("Sine", tmp_pb);
	tmp_pb->addMode(ModeNormal, "Sin", i18n("Sine"));
	tmp_pb->addMode(ModeInverse, "Asin", i18n("Arc sine"));
	tmp_pb->addMode(ModeHyperbolic, "Sinh", i18n("Hyperbolic sine"));
	tmp_pb->addMode(ButtonModeFlags(ModeInverse | ModeHyperbolic),
			"Asinh", i18n("Inverse hyperbolic sine"));
	tmp_pb->setShortcut(QKeySequence(Qt::Key_S));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotSinclicked(void)));

	tmp_pb = new KCalcButton(parent);
	pbScientific.insert("Cosine", tmp_pb);
	tmp_pb->addMode(ModeNormal, "Cos", i18n("Cosine"));
	tmp_pb->addMode(ModeInverse, "Acos", i18n("Arc cosine"));
	tmp_pb->addMode(ModeHyperbolic, "Cosh", i18n("Hyperbolic cosine"));
	tmp_pb->addMode(ButtonModeFlags(ModeInverse | ModeHyperbolic),
			"Acosh", i18n("Inverse hyperbolic cosine"));
	tmp_pb->setShortcut(QKeySequence(Qt::Key_C));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotCosclicked(void)));

	tmp_pb = new KCalcButton(parent);
	pbScientific.insert("Tangent", tmp_pb);
	tmp_pb->addMode(ModeNormal, "Tan", i18n("Tangent"));
	tmp_pb->addMode(ModeInverse, "Atan", i18n("Arc tangent"));
	tmp_pb->addMode(ModeHyperbolic, "Tanh", i18n("Hyperbolic tangent"));
	tmp_pb->addMode(ButtonModeFlags(ModeInverse | ModeHyperbolic),
			"Atanh", i18n("Inverse hyperbolic tangent"));
	tmp_pb->setShortcut(QKeySequence(Qt::Key_T));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(tmp_pb, SIGNAL(clicked(void)),SLOT(slotTanclicked(void)));

	tmp_pb = new KCalcButton(parent);
	tmp_pb->addMode(ModeNormal, "Ln", i18n("Natural log"));
	tmp_pb->addMode(ModeInverse, "e<sup> x </sup>", i18n("Exponential function"),
			true);
	pbScientific.insert("LogNatural", tmp_pb);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_N));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotLnclicked(void)));

	tmp_pb = new KCalcButton(parent);
	tmp_pb->addMode(ModeNormal, "Log", i18n("Logarithm to base 10"));
	tmp_pb->addMode(ModeInverse, "10<sup> x </sup>", i18n("10 to the power of x"),
			true);
	pbScientific.insert("Log10", tmp_pb);
	tmp_pb->setShortcut(QKeySequence(Qt::Key_L));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotLogclicked(void)));

}

void KCalculator::setupStatisticKeys(QWidget *parent)
{
	Q_CHECK_PTR(parent);

	KCalcButton *tmp_pb;

	tmp_pb = new KCalcButton(parent);
	tmp_pb->addMode(ModeNormal, "N", i18n("Number of data entered"));
	tmp_pb->addMode(ModeInverse, QString::fromUtf8("\xce\xa3")
			+ 'x', i18n("Sum of all data items"));
	pbStat.insert("NumData", tmp_pb);
	mStatButtonList.append(tmp_pb);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotStatNumclicked(void)));

	tmp_pb = new KCalcButton(parent);
	tmp_pb->addMode(ModeNormal, "Med", i18n("Median"));
	pbStat.insert("Median", tmp_pb);
	mStatButtonList.append(tmp_pb);
	tmp_pb->setToolTip( i18n("Median"));
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotStatMedianclicked(void)));

	tmp_pb = new KCalcButton(parent);
	tmp_pb->addMode(ModeNormal, "Mea", i18n("Mean"));
	tmp_pb->addMode(ModeInverse, QString::fromUtf8("\xce\xa3")
			+ "x<sup>2</sup>",
			i18n("Sum of all data items squared"), true);
	pbStat.insert("Mean", tmp_pb);
	mStatButtonList.append(tmp_pb);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotStatMeanclicked(void)));

	tmp_pb = new KCalcButton(parent);
	tmp_pb->addMode(ModeNormal, QString::fromUtf8("σ",-1) +  "<sub>N-1</sub>",
			i18n("Sample standard deviation"), true);
	tmp_pb->addMode(ModeInverse, QString::fromUtf8("σ",-1) +  "<sub>N</sub>",
			i18n("Standard deviation"), true);
	pbStat.insert("StandardDeviation", tmp_pb);
	mStatButtonList.append(tmp_pb);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotStatStdDevclicked(void)));

	tmp_pb = new KCalcButton(parent);
	tmp_pb->addMode(ModeNormal, "Dat", i18n("Enter data"));
	tmp_pb->addMode(ModeInverse, "CDat", i18n("Delete last data item"));
	pbStat.insert("InputData", tmp_pb);
	mStatButtonList.append(tmp_pb);
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	connect(tmp_pb, SIGNAL(clicked(void)), SLOT(slotStatDataInputclicked(void)));

	tmp_pb = new KCalcButton(parent);
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

	ConstButtonGroup = new QButtonGroup(this);
	connect(ConstButtonGroup, SIGNAL(buttonClicked(int)), SLOT(slotConstclicked(int)));


	KCalcConstButton *tmp_pb;
	tmp_pb = new KCalcConstButton(parent, 0);
	tmp_pb->setShortcut(QKeySequence(Qt::ALT + Qt::Key_1));
	pbConstant[0] = tmp_pb;
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	ConstButtonGroup->addButton(tmp_pb, 0);

	tmp_pb = new KCalcConstButton(parent, 1);
	tmp_pb->setShortcut(QKeySequence(Qt::ALT + Qt::Key_2));
	pbConstant[1] = tmp_pb;
	connect(this, SIGNAL(switchShowAccels(bool)),
		tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
		tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	ConstButtonGroup->addButton(tmp_pb, 1);

	tmp_pb = new KCalcConstButton(parent, 2);
	tmp_pb->setShortcut(QKeySequence(Qt::ALT + Qt::Key_3));
	pbConstant[2] = tmp_pb;
	connect(this, SIGNAL(switchShowAccels(bool)),
			tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	ConstButtonGroup->addButton(tmp_pb, 2);

	tmp_pb = new KCalcConstButton(parent, 3);
	tmp_pb->setShortcut(QKeySequence(Qt::ALT + Qt::Key_4));
	pbConstant[3] = tmp_pb;
	connect(this, SIGNAL(switchShowAccels(bool)),
			tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	ConstButtonGroup->addButton(tmp_pb, 3);

	tmp_pb = new KCalcConstButton(parent, 4);
	tmp_pb->setShortcut(QKeySequence(Qt::ALT + Qt::Key_5));
	pbConstant[4] = tmp_pb;
	connect(this, SIGNAL(switchShowAccels(bool)),
			tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	ConstButtonGroup->addButton(tmp_pb, 4);

	tmp_pb = new KCalcConstButton(parent, 5);
	tmp_pb->setShortcut(QKeySequence(Qt::ALT + Qt::Key_6));
	pbConstant[5]  = tmp_pb;
	connect(this, SIGNAL(switchShowAccels(bool)),
			tmp_pb, SLOT(slotSetAccelDisplayMode(bool)));
	connect(this, SIGNAL(switchMode(ButtonModeFlags,bool)),
			tmp_pb, SLOT(slotSetMode(ButtonModeFlags,bool)));
	ConstButtonGroup->addButton(tmp_pb, 5);

	changeButtonNames();

	// add menu with scientific constants
	KCalcConstMenu *tmp_menu = new KCalcConstMenu(i18n("&Constants"),
						      this);
	#warning order of menu initialization is fundamental!!
	menuBar()->insertMenu((menuBar()->actions)()[2], tmp_menu);
	connect(tmp_menu,
		SIGNAL(triggeredConstant(science_constant const &)),
		this,
		SLOT(slotConstantToDisplay(science_constant const &)));

}

void KCalculator::slotConstantToDisplay(struct science_constant const &const_chosen)
{
	calc_display->setAmount(const_chosen.value);

	UpdateDisplay(false);
}

void KCalculator::updateGeometry(void)
{
    QObjectList const *l;
    QSize s;
    int margin;

    //
    // Calculator buttons
    //
    s.setWidth(mSmallPage->fontMetrics().width("MMMM"));
    s.setHeight(mSmallPage->fontMetrics().lineSpacing());

    l =  &(mSmallPage->children());

    for(int i=0; i < l->size(); i++)
    {
		QObject *o = l->at(i);
        if( o->isWidgetType() )
        {
            QWidget *tmp_widget = static_cast<QWidget *>(o);
            margin = QApplication::style()->
	      pixelMetric(QStyle::PM_ButtonMargin, 0, tmp_widget)*2;
            tmp_widget->setFixedSize(s.width()+margin, s.height()+margin);
            //tmp_widget->setMinimumSize(s.width()+margin, s.height()+margin);
            tmp_widget->installEventFilter( this );
            tmp_widget->setAcceptDrops(true);
        }
    }

    l = &(mLargePage->children());

    int h1 = (NumButtonGroup->buttons()[0x0F])->minimumSize().height();
    int h2 = static_cast<int>( (static_cast<float>(h1) + 4.0) / 5.0 );
    s.setWidth(mLargePage->fontMetrics().width("MMM") +
               QApplication::style()->
               pixelMetric(QStyle::PM_ButtonMargin, 0,
			   (NumButtonGroup->buttons()[0x0F]))*2);
    s.setHeight(h1 + h2);

    for(int i = 0; i < l->size(); i++)
    {
        QObject *o = l->at(i);
        if(o->isWidgetType())
        {
            QWidget *tmp_widget = static_cast<QWidget *>(o);
            tmp_widget->setFixedSize(s);
            tmp_widget->installEventFilter(this);
            tmp_widget->setAcceptDrops(true);
        }
    }

    pbInv->setFixedSize(s);
    pbInv->installEventFilter(this);
    pbInv->setAcceptDrops(true);



    l = &(mNumericPage->children()); // silence please

    h1 = (NumButtonGroup->buttons()[0x0F])->minimumSize().height();
    h2 = (int)((((float)h1 + 4.0) / 5.0));
    s.setWidth(mLargePage->fontMetrics().width("MMM") +
               QApplication::style()->
               pixelMetric(QStyle::PM_ButtonMargin, 0,
			   NumButtonGroup->buttons()[0x0F])*2);
    s.setHeight(h1 + h2);

    for(int i = 0; i < l->count(); i++)
    {
        QObject *o = l->at(i);
        if(o->isWidgetType())
        {
            QWidget *tmp_widget = static_cast<QWidget *>(o);
            tmp_widget->setFixedSize(s);
            tmp_widget->installEventFilter(this);
            tmp_widget->setAcceptDrops(true);
        }
    }

    // Set Buttons of double size
    QSize t(s);
	       t.setWidth(2*s.width());
	       (NumButtonGroup->buttons()[0x00])->setFixedSize(t);
    t = s;
    t.setHeight(2*s.height());
    pbEqual->setFixedSize(t);
    pbPlus->setFixedSize(t);
}

void KCalculator::slotBaseHex(void)
{
	// set display & statusbar (if item exist in statusbar)
	calc_display->setBase(NumBase(16));
	if (statusBar()->hasItem(1)) statusBar()->changeItem("HEX",1);
	calc_display->setStatusText(1, "Hex");


	// Enable the buttons not available in this base
	for (int i=0; i<16; i++)
	  (NumButtonGroup->buttons()[i])->setEnabled (true);

	// Only enable the decimal point, x*10^y button in decimal
	pbPeriod->setEnabled(false);
	pbEE->setEnabled(false);

	// Disable buttons that make only sense with floating point
	// numbers
	pbScientific["HypMode"]->setEnabled(false);
	pbScientific["Sine"]->setEnabled(false);
	pbScientific["Cosine"]->setEnabled(false);
	pbScientific["Tangent"]->setEnabled(false);
	pbScientific["LogNatural"]->setEnabled(false);
	pbScientific["Log10"]->setEnabled(false);
}

void KCalculator::slotBaseDec(void)
{
	// set display & statusbar (if item exist in statusbar)
	calc_display->setBase(NumBase(10));
	if (statusBar()->hasItem(1)) statusBar()->changeItem("DEC",1);
	calc_display->setStatusText(1, "Dec");


	// Enable the buttons not available in this base
	for (int i=0; i<10; i++)
	  (NumButtonGroup->buttons()[i])->setEnabled (true);

	// Disable the buttons not available in this base
	for (int i=10; i<16; i++)
	  (NumButtonGroup->buttons()[i])->setEnabled (false);

	// Only enable the decimal point, x*10^y button in decimal
	pbPeriod->setEnabled(true);
	pbEE->setEnabled(true);

	pbScientific["HypMode"]->setEnabled(true);
	pbScientific["Sine"]->setEnabled(true);
	pbScientific["Cosine"]->setEnabled(true);
	pbScientific["Tangent"]->setEnabled(true);
	pbScientific["LogNatural"]->setEnabled(true);
	pbScientific["Log10"]->setEnabled(true);
}

void KCalculator::slotBaseOct(void)
{
	// set display & statusbar (if item exist in statusbar)
	  calc_display->setBase(NumBase(8));
	  if (statusBar()->hasItem(1)) statusBar()->changeItem("OCT",1);
	  calc_display->setStatusText(1, "Oct");

	// Enable the buttons not available in this base
	for (int i=0; i<8; i++)
	  (NumButtonGroup->buttons()[i])->setEnabled (true);

	// Disable the buttons not available in this base
	for (int i=8; i<16; i++)
	  (NumButtonGroup->buttons()[i])->setEnabled (false);

	// Only enable the decimal point, x*10^y button in decimal
	pbPeriod->setEnabled(false);
	pbEE->setEnabled(false);

	// Disable buttons that make only sense with floating point
	// numbers
	pbScientific["HypMode"]->setEnabled(false);
	pbScientific["Sine"]->setEnabled(false);
	pbScientific["Cosine"]->setEnabled(false);
	pbScientific["Tangent"]->setEnabled(false);
	pbScientific["LogNatural"]->setEnabled(false);
	pbScientific["Log10"]->setEnabled(false);
}

void KCalculator::slotBaseBin(void)
{
	// set display & statusbar (if item exist in statusbar)
	calc_display->setBase(NumBase(2));
	if (statusBar()->hasItem(1)) statusBar()->changeItem("BIN",1);
	calc_display->setStatusText(1, "Bin");

	// Enable the buttons not available in this base
	for (int i=0; i<2; i++)
	  (NumButtonGroup->buttons()[i])->setEnabled (true);

	// Disable the buttons not available in this base
	for (int i=2; i<16; i++)
	  (NumButtonGroup->buttons()[i])->setEnabled (false);

	// Only enable the decimal point, x*10^y button in decimal
	pbPeriod->setEnabled(false);
	pbEE->setEnabled(false);

	// Disable buttons that make only sense with floating point
	// numbers
	pbScientific["HypMode"]->setEnabled(false);
	pbScientific["Sine"]->setEnabled(false);
	pbScientific["Cosine"]->setEnabled(false);
	pbScientific["Tangent"]->setEnabled(false);
	pbScientific["LogNatural"]->setEnabled(false);
	pbScientific["Log10"]->setEnabled(false);
}

void KCalculator::keyPressEvent(QKeyEvent *e)
{
  if ( ( e->modifiers() & Qt::NoModifier ) == 0 || ( e->modifiers() & Qt::ShiftModifier) ) {
	switch (e->key())
	{
	case Qt::Key_Next:
		pbAC->animateClick();
		break;
	case Qt::Key_Slash:
        case Qt::Key_division:
		pbDivision->animateClick();
		break;
 	case Qt::Key_D:
		pbStat["InputData"]->animateClick(); // stat mode
		break;
	case Qt::Key_BracketLeft:
        case Qt::Key_twosuperior:
		pbSquare->animateClick();
		break;
	case Qt::Key_Backspace:
		calc_display->deleteLastDigit();
		// pbAC->animateClick();
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
	pbAngleChoose->menu()->setItemChecked(0, false);
	pbAngleChoose->menu()->setItemChecked(1, false);
	pbAngleChoose->menu()->setItemChecked(2, false);

	switch(number)
	{
	case 0:
		_angle_mode = DegMode;
		statusBar()->changeItem("DEG", 2);
		pbAngleChoose->menu()->setItemChecked(0, true);
		calc_display->setStatusText(2, "Deg");
		break;
	case 1:
		_angle_mode = RadMode;
		statusBar()->changeItem("RAD", 2);
		pbAngleChoose->menu()->setItemChecked(1, true);
		calc_display->setStatusText(2, "Rad");
		break;
	case 2:
		_angle_mode = GradMode;
		statusBar()->changeItem("GRA", 2);
		pbAngleChoose->menu()->setItemChecked(2, true);
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
		calc_display->setStatusText(0, QString::null);
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
	UpdateDisplay(false);
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

	UpdateDisplay(true);
}

void KCalculator::slotPlusMinusclicked(void)
{
	// display can only change sign, when in input mode, otherwise we
	// need the core to do this.
	if (!calc_display->sendEvent(KCalcDisplay::EventChangeSign))
	{
	    core.InvertSign(calc_display->getAmount());
	    UpdateDisplay(true);
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
		core.Cube(calc_display->getAmount());

	UpdateDisplay(true);
}

void KCalculator::slotRootclicked(void)
{
	if (!inverse)
		core.SquareRoot(calc_display->getAmount());
	else
		core.CubeRoot(calc_display->getAmount());

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
	UpdateDisplay(false);
}

void KCalculator::slotMCclicked(void)
{
	memory_num		= 0;
	statusBar()->changeItem(" \xa0\xa0 ",3);
	calc_display->setStatusText(3, QString::null);
	pbMemRecall->setDisabled(true);
}

void KCalculator::slotClearclicked(void)
{
	calc_display->sendEvent(KCalcDisplay::EventClear);
}

void KCalculator::slotACclicked(void)
{
	core.Reset();
	calc_display->sendEvent(KCalcDisplay::EventReset);

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
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_AND);

	UpdateDisplay(true);
}

void KCalculator::slotXclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_MULTIPLY);

	UpdateDisplay(true);
}

void KCalculator::slotDivisionclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_DIVIDE);

	UpdateDisplay(true);
}

void KCalculator::slotORclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_OR);

	UpdateDisplay(true);
}

void KCalculator::slotXORclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_XOR);

	UpdateDisplay(true);
}

void KCalculator::slotPlusclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_ADD);

	UpdateDisplay(true);
}

void KCalculator::slotMinusclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_SUBTRACT);

	UpdateDisplay(true);
}

void KCalculator::slotLeftShiftclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_LSH);

	UpdateDisplay(true);
}

void KCalculator::slotRightShiftclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_RSH);

	UpdateDisplay(true);
}

void KCalculator::slotPeriodclicked(void)
{
	calc_display->newCharacter('.');
}

void KCalculator::EnterEqual()
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_EQUAL);

	UpdateDisplay(true, true);
}

void KCalculator::slotEqualclicked(void)
{
	EnterEqual();
}

void KCalculator::slotPercentclicked(void)
{
	core.enterOperation(calc_display->getAmount(),
			    CalcEngine::FUNC_PERCENT);

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
		core.enterOperation(calc_display->getAmount(),
				    CalcEngine::FUNC_INTDIV);
	else
		core.enterOperation(calc_display->getAmount(),
				    CalcEngine::FUNC_MOD);

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
		pbInv->setChecked(false);
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
		pbInv->setChecked(false);
		core.StatSumSquares(0);
	}

	UpdateDisplay(true);
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
		pbInv->setChecked(false);
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
		pbInv->setChecked(false);
		core.StatDataDel(0);
		statusBar()->showMessage(i18n("Last stat item erased"), 3000);
	}

	UpdateDisplay(true);
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
		UpdateDisplay(false);
	}
}

void KCalculator::slotConstclicked(int button)
{
	if(!inverse)
	{
		//set the display to the configured value of Constant
		//Button
		calc_display->setAmount(pbConstant[button]->constant());
	}
	else
	{
		pbInv->setChecked(false);
		KCalcSettings::setValueConstant(button, calc_display->text());
		// below set new tooltip
		pbConstant[button]->setLabelAndTooltip();
		// work around: after storing a number, pressing a digit should start
		// a new number
		calc_display->setAmount(calc_display->getAmount());
	}

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
	dialog->showButtonSeparator( true );

	// Add the general page.  Store the settings in the General group and
	// use the icon package_settings.
	General *general = new General(0, "General");
	int maxprec = 1000;
	general->kcfg_Precision->setMaximum(maxprec);
	dialog->addPage(general, i18n("General"), "package_settings", i18n("General Settings"));

	QWidget *fontWidget = new QWidget(0);
	QVBoxLayout *fontLayout = new QVBoxLayout(fontWidget);
	KFontChooser *mFontChooser =
		new KFontChooser(fontWidget, false, QStringList(), false, 6);
	mFontChooser->setObjectName("kcfg_Font");

	fontLayout->addWidget(mFontChooser);
	dialog->addPage(fontWidget, i18n("Font"), "fonts", i18n("Select Display Font"));

	// color settings

	Colors *color = new Colors(0);

	dialog->addPage(color, i18n("Colors"), "colors", i18n("Button & Display Colors"));

	// constant settings

	Constants *constant = new Constants(0);
	tmp_const = constant;

	KCalcConstMenu *tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu,
		SIGNAL(triggeredConstant(science_constant const &)),
		this,
		SLOT(slotChooseScientificConst0(science_constant const &)));
	(constant->kPushButton0)->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu,
		SIGNAL(triggeredConstant(science_constant const &)),
		this,
		SLOT(slotChooseScientificConst1(science_constant const &)));
	(constant->kPushButton1)->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu,
		SIGNAL(triggeredConstant(science_constant const &)),
		this,
		SLOT(slotChooseScientificConst2(science_constant const &)));
	(constant->kPushButton2)->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu,
		SIGNAL(triggeredConstant(science_constant const &)),
		this,
		SLOT(slotChooseScientificConst3(science_constant const &)));
	(constant->kPushButton3)->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu,
		SIGNAL(triggeredConstant(science_constant const &)),
		this,
		SLOT(slotChooseScientificConst4(science_constant const &)));
	(constant->kPushButton4)->setMenu(tmp_menu);

	tmp_menu = new KCalcConstMenu(this);
	connect(tmp_menu,
		SIGNAL(triggeredConstant(science_constant const &)),
		this,
		SLOT(slotChooseScientificConst5(science_constant const &)));
	(constant->kPushButton5)->setMenu(tmp_menu);

	dialog->addPage(constant, i18n("Constants"), "constants",i18n("Define constants"));


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
  (tmp_const->kcfg_valueConstant0)->setText(chosen_const.value);

  (tmp_const->kcfg_nameConstant0)->setText(chosen_const.label);
}

void KCalculator::slotChooseScientificConst1(struct science_constant const & chosen_const)
{
  (tmp_const->kcfg_valueConstant1)->setText(chosen_const.value);

  (tmp_const->kcfg_nameConstant1)->setText(chosen_const.label);
}

void KCalculator::slotChooseScientificConst2(struct science_constant const & chosen_const)
{
  (tmp_const->kcfg_valueConstant2)->setText(chosen_const.value);

  (tmp_const->kcfg_nameConstant2)->setText(chosen_const.label);
}

void KCalculator::slotChooseScientificConst3(struct science_constant const & chosen_const)
{
  (tmp_const->kcfg_valueConstant3)->setText(chosen_const.value);

  (tmp_const->kcfg_nameConstant3)->setText(chosen_const.label);
}

void KCalculator::slotChooseScientificConst4(struct science_constant const & chosen_const)
{
  (tmp_const->kcfg_valueConstant4)->setText(chosen_const.value);

  (tmp_const->kcfg_nameConstant4)->setText(chosen_const.label);
}

void KCalculator::slotChooseScientificConst5(struct science_constant const & chosen_const)
{
  (tmp_const->kcfg_valueConstant5)->setText(chosen_const.value);

  (tmp_const->kcfg_nameConstant5)->setText(chosen_const.label);
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

void KCalculator::slotScientificshow(bool toggled)
{
	if(toggled)
	{
	        pbScientific["HypMode"]->show();
		pbScientific["Sine"]->show();
		pbScientific["Cosine"]->show();
		pbScientific["Tangent"]->show();
		pbScientific["Log10"]->show();
		pbScientific["LogNatural"]->show();
		pbAngleChoose->show();
		if(!statusBar()->hasItem(2))
			statusBar()->insertFixedItem(" DEG ", 2);
		statusBar()->setItemAlignment(2, Qt::AlignCenter);
		calc_display->setStatusText(2, "Deg");
		slotAngleSelected(0);
	}
	else
	{
	        pbScientific["HypMode"]->hide();
		pbScientific["Sine"]->hide();
		pbScientific["Cosine"]->hide();
		pbScientific["Tangent"]->hide();
		pbScientific["Log10"]->hide();
		pbScientific["LogNatural"]->hide();
		pbAngleChoose->hide();
		if(statusBar()->hasItem(2))
			statusBar()->removeItem(2);
		calc_display->setStatusText(2, QString::null);
	}
	adjustSize();
	setFixedSize(sizeHint());
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

		pbLogic["AND"]->show();
		pbLogic["OR"]->show();
		pbLogic["XOR"]->show();
		pbLogic["One-Complement"]->show();
		pbLogic["LeftShift"]->show();
		pbLogic["RightShift"]->show();
		if(!statusBar()->hasItem(1))
			statusBar()->insertFixedItem(" HEX ", 1);
		statusBar()->setItemAlignment(1, Qt::AlignCenter);
		calc_display->setStatusText(1, "Hex");
		resetBase();
		BaseChooseGroup->show();
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
		calc_display->setStatusText(1, QString::null);
		for (int i=10; i<16; i++)
			(NumButtonGroup->button(i))->hide();
	}
	adjustSize();
	setFixedSize(sizeHint());
	KCalcSettings::setShowLogic(toggled);
}

void KCalculator::slotConstantsShow(bool toggled)
{
	if(toggled)
	{
		pbConstant[0]->show();
		pbConstant[1]->show();
		pbConstant[2]->show();
		pbConstant[3]->show();
		pbConstant[4]->show();
		pbConstant[5]->show();

	}
	else
	{
		pbConstant[0]->hide();
		pbConstant[1]->hide();
		pbConstant[2]->hide();
		pbConstant[3]->hide();
		pbConstant[4]->hide();
		pbConstant[5]->hide();
	}
	adjustSize();
	setFixedSize(sizeHint());
	KCalcSettings::setShowConstants(toggled);
}

// This function is for setting the constant names configured in the
// kcalc settings menu. If the user doesn't enter a name for the
// constant C1 to C6 is used.
void KCalculator::changeButtonNames()
{
	pbConstant[0]->setLabelAndTooltip();
	pbConstant[1]->setLabelAndTooltip();
	pbConstant[2]->setLabelAndTooltip();
	pbConstant[3]->setLabelAndTooltip();
	pbConstant[4]->setLabelAndTooltip();
	pbConstant[5]->setLabelAndTooltip();
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
	UpdateDisplay(false);
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
		setCaption(QString::null);
	}
	calc_display->changeSettings();

	updateGeometry();
	resize(minimumSize());

	//
	// 1999-10-31 Espen Sand: Don't ask me why ;)
	// 2006-02-26 David Faure: time to find out why -> commented out
	//kapp->processOneEvent();

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
	for(int i = 0; i < mFunctionButtonList.size(); i++)
	{
		mFunctionButtonList[i]->setPalette(funcPal);
	}

	QPalette statPal(KCalcSettings::statButtonsColor(), bg);
	for(int i = 0; i < mStatButtonList.size(); i++)
	{
		mStatButtonList[i]->setPalette(statPal);
	}

	QPalette hexPal(KCalcSettings::hexButtonsColor(), bg);
	for(int i=10; i<16; i++)
	{
		(NumButtonGroup->button(i))->setPalette(hexPal);
	}

	QPalette memPal(KCalcSettings::memoryButtonsColor(), bg);
	for(int i = 0; i < mMemButtonList.size(); i++)
	{
		mMemButtonList[i]->setPalette(memPal);
	}

	QPalette opPal(KCalcSettings::operationButtonsColor(), bg);
	for(int i = 0; i < mOperationButtonList.size(); i++)
	{
		mOperationButtonList[i]->setPalette(opPal);
	}
}

void KCalculator::set_precision()
{
	KNumber:: setDefaultFloatPrecision(KCalcSettings::precision());
	UpdateDisplay(false);
}

bool KCalculator::eventFilter(QObject *o, QEvent *e)
{
	switch(e->type())
	{
	case QEvent::DragEnter:
	{
		QDragEnterEvent *ev = (QDragEnterEvent *)e;
		ev->accept( KColorMimeData::canDecode( ev->mimeData() ) );
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
			QList<KCalcButton*> *list;
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
		return KMainWindow::eventFilter(o, e);
	}
}




////////////////////////////////////////////////////////////////
// Include the meta-object code for classes in this file
//

#include "kcalc.moc"


extern "C" KDE_EXPORT int kdemain(int argc, char *argv[])
{
	KAboutData aboutData( "kcalc", I18N_NOOP("KCalc"),
                          version, description, KAboutData::License_GPL,
                          I18N_NOOP("(c) 2003-2005, Klaus Niederkr" "\xc3\xbc" "ger\n"
				    "(c) 1996-2000, Bernd Johannes Wuebben\n"
                                    "(c) 2000-2005, The KDE Team"));

	/* Klaus Niederkrueger */
	aboutData.addAuthor("Klaus Niederkr" "\xc3\xbc" "ger", 0, "kniederk@math.uni-koeln.de");
	aboutData.addAuthor("Bernd Johannes Wuebben", 0, "wuebben@kde.org");
	aboutData.addAuthor("Evan Teran", 0, "eteran@alum.rit.edu");
	aboutData.addAuthor("Espen Sand", 0, "espen@kde.org");
	aboutData.addAuthor("Chris Howells", 0, "howells@kde.org");
	aboutData.addAuthor("Aaron J. Seigo", 0, "aseigo@olympusproject.org");
	aboutData.addAuthor("Charles Samuels", 0, "charles@altair.dhs.org");
	/* Rene Merou */
	aboutData.addAuthor("Ren" "\xc3\xa9" " M" "\xc3\xa9"  "rou", 0, "ochominutosdearco@yahoo.es");
	aboutData.addAuthor("Michel Marti", 0, "mma@objectxp.com");
	KCmdLineArgs::init(argc, argv, &aboutData);

	KApplication app;

	KCalculator *calc = new KCalculator(0);
	app.setTopWidget(calc);

	calc->show();

	return app.exec();
}

