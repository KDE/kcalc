/*
    $Id$

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

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <qlayout.h>
#include <qobjectlist.h>
#include <qaccel.h>
#include <qbuttongroup.h>
#include <qclipboard.h>
#include <qradiobutton.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qstyle.h>

#include <kapplication.h>
#include <kcolordrag.h>
#include <kconfig.h>
#include <kcmdlineargs.h>
#include <knotifyclient.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kpushbutton.h>
#include <kpopupmenu.h>
#include <kglobalsettings.h>

#include "dlabel.h"
#include "kcalc.h"
#include "optiondialog.h"
#include "version.h"

// Initialize static members
const CALCAMNT QtCalculator::pi = (ASIN(1L) * 2L);


static const char *description = I18N_NOOP("KDE Calculator");
static const char *version = KCALCVERSION;

extern CALCAMNT	display_data;
extern bool		display_error;

//
// * ported to QLayout (mosfet 10/28/99)
//
// * 1999-10-31 Espen Sand: More modifications.
//   All fixed sizes removed.
//   New config dialog.
//   To exit: Key_Q+ALT => Key_Q+CTRL,  Key_X+ALT => Key_X+CTRL
//   Look in updateGeometry() for size settings.
//


QtCalculator::QtCalculator(QWidget *parent, const char *name)
	: KMainWindow(parent, name), inverse(false), hyp_mode(false), eestate(false),
	refresh_display(false), display_size(DEC_SIZE),  angle_mode(ANG_DEGREE),
	input_limit(0), input_count(0), decimal_point(0), precedence_base(0),
	current_base(NB_DECIMAL), memory_num(0.0), last_input(DIGIT),
	history_index(0), selection_timer(new QTimer),
	mInternalSpacing(4), status_timer(new QTimer), mConfigureDialog(0)
{
	/* central widget to contain all the elements */
	QWidget *central = new QWidget(this);
	setCentralWidget(central);

	// make sure the display_str is NULL terminated so we can
	// use library string functions
	display_str[0] = '\0';

	connect(status_timer, SIGNAL(timeout()),
		this, SLOT(clear_status_label()));

	connect(selection_timer, SIGNAL(timeout()),
		this, SLOT(selection_timed_out()));

	readSettings();

	// Detect color change
	connect(kapp,SIGNAL(kdisplayPaletteChanged()), this, SLOT(set_colors()));

	// Accelerators to exit the program
	QAccel *accel = new QAccel(central);
	accel->connectItem(accel->insertItem(Key_Q+CTRL), this, SLOT(quitCalc()));
	accel->connectItem(accel->insertItem(Key_X+CTRL), this, SLOT(quitCalc()));

	// Create uppermost bar with buttons and numberdisplay
	mConfigButton = new KPushButton(KGuiItem( i18n("Config&ure"), "configure" ),
            central, "configbutton");
	mConfigButton->setAutoDefault(false);
	QToolTip::add(mConfigButton, i18n("Click to configure KCalc"));
	connect(mConfigButton, SIGNAL(clicked()), this, SLOT(configclicked()));
	if (KGlobal::config()->isImmutable())
	   mConfigButton->hide();

	mHelpMenu = new KHelpMenu(central, KGlobal::instance()->aboutData());

	mHelpButton = new KPushButton(KStdGuiItem::help(), central);
	mHelpButton->setAutoDefault(false);
	mHelpButton->setPopup(mHelpMenu->menu());

	calc_display = new DLabel(central, "display");
	calc_display->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
	calc_display->setAlignment(AlignRight | AlignVCenter);
	calc_display->setFocus();
	calc_display->setFocusPolicy(QWidget::StrongFocus);

	connect(calc_display, SIGNAL(clicked()), this, SLOT(display_selected()));

	// Status bar contents
	statusINVLabel = new QLabel( central, "INV" );
	Q_CHECK_PTR(statusINVLabel);
	statusINVLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	statusINVLabel->setAlignment(AlignCenter);
	statusINVLabel->setText("NORM");

	statusHYPLabel = new QLabel(central, "HYP");
	Q_CHECK_PTR(statusHYPLabel);
	statusHYPLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	statusHYPLabel->setAlignment(AlignCenter);

	statusERRORLabel = new QLabel(central, "ERROR");
	Q_CHECK_PTR(statusERRORLabel);
	statusERRORLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	statusERRORLabel->setAlignment(AlignLeft|AlignVCenter);

	// Create Number Base Button Group
	QButtonGroup *base_group = new QButtonGroup(4, Horizontal,  central, "base");
	base_group->setTitle(i18n("Base"));
	connect(base_group, SIGNAL(clicked(int)), SLOT(Base_Selected(int)));

	basebutton[0] = new QRadioButton(base_group);
	basebutton[0]->setText("He&x");
	QToolTip::add(basebutton[0], i18n("Hexadecimal"));
	accel->connectItem(accel->insertItem(Key_X+ALT),
		this, SLOT(Hex_Selected()));

	basebutton[1] = new QRadioButton(base_group);
	basebutton[1]->setText("&Dec");
	QToolTip::add(basebutton[1], i18n("Decimal"));
	basebutton[1]->setChecked(true);
	accel->connectItem(accel->insertItem(Key_D+ALT),
		this, SLOT(Dec_Selected()));

	basebutton[2] = new QRadioButton(base_group);
	basebutton[2]->setText("&Oct");
	QToolTip::add(basebutton[2], i18n("Octal"));
	accel->connectItem(accel->insertItem(Key_O+ALT),
		this, SLOT(Oct_Selected()));

	basebutton[3] = new QRadioButton(base_group);
	basebutton[3]->setText("&Bin");
	QToolTip::add(basebutton[3], i18n("Binary"));
	accel->connectItem(accel->insertItem(Key_B+ALT),
		this, SLOT(Bin_Selected()));

	// create angle button group
	angle_group = new QButtonGroup(3, Horizontal, central, "angle");
	angle_group->setTitle(i18n( "Angle") );
	connect( angle_group, SIGNAL(clicked(int)), SLOT(angle_selected(int)));

	anglebutton[0] = new QRadioButton(angle_group);
	anglebutton[0]->setText("D&eg");
	QToolTip::add(anglebutton[0], i18n("Degrees"));
	anglebutton[0]->setChecked(true);
	accel->connectItem(accel->insertItem(Key_E+ALT),
		this, SLOT(Deg_Selected()));

	anglebutton[1] = new QRadioButton(angle_group);
	anglebutton[1]->setText("&Rad");
	QToolTip::add(anglebutton[1], i18n("Radians"));
	accel->connectItem(accel->insertItem(Key_R+ALT),
		this, SLOT(Rad_Selected()));

	anglebutton[2] = new QRadioButton(angle_group);
	anglebutton[2]->setText("&Gra");
	QToolTip::add(anglebutton[2], i18n("Gradians"));
	accel->connectItem(accel->insertItem(Key_G+ALT),
		this, SLOT(Gra_Selected()));

	//
	//  Create Calculator Buttons
	//

	// First the widgets that are the parents of the buttons
	mSmallPage = new QWidget(central);
	mLargePage = new QWidget(central);

	pbHyp = new QPushButton("Hyp", mSmallPage, "Hyp-Button");
	QToolTip::add(pbHyp, i18n("Hyperbolic mode"));
	pbHyp->setAutoDefault(false);
	connect(pbHyp, SIGNAL(toggled(bool)), SLOT(slotHyptoggled(bool)));
	pbHyp->setToggleButton(true);

	pbStatNum = new QPushButton("N", mSmallPage, "Stat.NumData-Button");
	QToolTip::add(pbStatNum, i18n("Number of data entered"));
 	pbStatNum->setAutoDefault(false);
	connect(pbStatNum, SIGNAL(clicked(void)), SLOT(slotStatNumclicked(void)));

	pbInv = new QPushButton("Inv", mSmallPage, "Inverse-Button");
	QToolTip::add(pbInv, i18n("Inverse mode"));
	pbInv->setAutoDefault(false);
	connect(pbInv, SIGNAL(toggled(bool)), SLOT(slotInvtoggled(bool)));
	pbInv->setToggleButton(true);

	pbSin = new QPushButton("Sin ", mSmallPage, "Sin-Button");
	QToolTip::add(pbSin, i18n("Sine"));
	pbSin->setAutoDefault(false);
	connect(pbSin, SIGNAL(clicked(void)), SLOT(slotSinclicked(void)));

	pbStatMean = new QPushButton("Mea", mSmallPage, "Stat.Mean-Button");
	QToolTip::add(pbStatMean, i18n("Mean"));
	pbStatMean->setAutoDefault(false);
	connect(pbStatMean, SIGNAL(clicked(void)), SLOT(slotStatMeanclicked(void)));

	pbPlusMinus = new QPushButton("+/-", mSmallPage, "Sign-Button");
	QToolTip::add(pbPlusMinus, i18n("Change sign"));
	pbPlusMinus->setAutoDefault(false);
	connect(pbPlusMinus, SIGNAL(clicked(void)), SLOT(slotPlusMinusclicked(void)));

	pbCos = new QPushButton("Cos ", mSmallPage, "Cos-Button");
	QToolTip::add(pbCos, i18n("Cosine"));
	pbCos->setAutoDefault(false);
	connect(pbCos, SIGNAL(clicked(void)), SLOT(slotCosclicked(void)));

	pbStatStdDev = new QPushButton("Std", mSmallPage,
				       "Stat.StandardDeviation-Button");
	QToolTip::add(pbStatStdDev, i18n("Standard deviation"));
	pbStatStdDev->setAutoDefault(false);
	connect(pbStatStdDev, SIGNAL(clicked(void)), SLOT(slotStatStdDevclicked(void)));

	pbReci = new QPushButton("1/x", mSmallPage, "Reciprocal-Button");
	QToolTip::add(pbReci, i18n("Reciprocal"));
	pbReci->setAutoDefault(false);
	connect(pbReci, SIGNAL(clicked(void)), SLOT(slotReciclicked(void)));

	pbTan = new QPushButton("Tan ", mSmallPage, "Tan-Button");
	QToolTip::add(pbTan, i18n("Tangent"));
	pbTan->setAutoDefault(false);
	connect(pbTan, SIGNAL(clicked(void)),SLOT(slotTanclicked(void)));

	pbStatMedian = new QPushButton("Med", mSmallPage, "Stat.Median-Button");
	QToolTip::add(pbStatMedian, i18n("Median"));
	pbStatMedian->setAutoDefault(false);
	connect(pbStatMedian, SIGNAL(clicked(void)),SLOT(slotStatMedianclicked(void)));

	pbFactorial = new QPushButton("x!", mSmallPage, "Factorial-Button");
	QToolTip::add(pbFactorial, i18n("Factorial"));
	pbFactorial->setAutoDefault(false);
	connect(pbFactorial, SIGNAL(clicked(void)),SLOT(slotFactorialclicked(void)));

	pbLog = new QPushButton("Log", mSmallPage, "Log-Button");
	QToolTip::add(pbLog, i18n("Logarithm to base 10"));
	pbLog->setAutoDefault(false);
	connect(pbLog, SIGNAL(clicked(void)), SLOT(slotLogclicked(void)));

	pbStatDataInput = new QPushButton("Dat", mSmallPage, "Stat.DataInput-Button");
	QToolTip::add(pbStatDataInput, i18n("Enter data"));
	pbStatDataInput->setAutoDefault(false);
	connect(pbStatDataInput, SIGNAL(clicked(void)), SLOT(slotStatDataInputclicked(void)));

	pbSquare = new QPushButton("x^2", mSmallPage, "Square-Button");
	QToolTip::add(pbSquare, i18n("Square"));
	pbSquare->setAutoDefault(false);
	connect(pbSquare, SIGNAL(clicked(void)), SLOT(slotSquareclicked(void)));

	pbLn = new QPushButton("Ln", mSmallPage, "Ln-Button");
	QToolTip::add(pbLn, i18n("Natural log"));
	pbLn->setAutoDefault(false);
	connect(pbLn, SIGNAL(clicked(void)), SLOT(slotLnclicked(void)));

	pbStatClearData = new QPushButton("CSt", mSmallPage, "Stat.ClearData-Button");
	QToolTip::add(pbStatClearData, i18n("Clear data store"));
	pbStatClearData->setAutoDefault(false);
	connect(pbStatClearData, SIGNAL(clicked(void)), SLOT(slotStatClearDataclicked(void)));

	pbPower = new QPushButton("x^y", mSmallPage, "Power-Button");
	pbPower->setAutoDefault(false);
	QToolTip::add(pbPower, i18n("x to the power of y"));
	connect(pbPower, SIGNAL(clicked(void)), SLOT(slotPowerclicked(void)));

	pbEE = new QPushButton("EE", mLargePage, "EE-Button");
	QToolTip::add(pbEE, i18n("Exponent"));
	pbEE->setAutoDefault(false);
	connect(pbEE, SIGNAL(clicked(void)), SLOT(slotEEclicked(void)));

	pbMR = new QPushButton("MR", mLargePage, "MemRecall-Button");
	QToolTip::add(pbMR, i18n("Memory recall"));
	pbMR->setAutoDefault(false);
	connect(pbMR, SIGNAL(clicked(void)), SLOT(slotMRclicked(void)));

	pbMPlusMinus = new QPushButton("M+-", mLargePage, "MPlusMinus-Button");
	pbMPlusMinus->setAutoDefault(false);
	connect(pbMPlusMinus,SIGNAL(clicked(void)),SLOT(slotMPlusMinusclicked(void)));

	pbMC = new QPushButton("MC", mLargePage, "MemClear-Button");
	QToolTip::add(pbMC, i18n("Clear memory"));
	pbMC->setAutoDefault(false);
	connect(pbMC, SIGNAL(clicked(void)), SLOT(slotMCclicked(void)));

	pbClear = new QPushButton("C", mLargePage, "Clear-Button");
	QToolTip::add(pbClear, i18n("Clear"));
	pbClear->setAutoDefault(false);
	connect(pbClear, SIGNAL(clicked(void)), SLOT(slotClearclicked(void)));

	pbAC = new QPushButton("AC", mLargePage, "AC-Button");
	QToolTip::add(pbAC, i18n("Clear all"));
	pbAC->setAutoDefault(false);
	connect(pbAC, SIGNAL(clicked(void)), SLOT(slotACclicked(void)));

	pbParenOpen = new QPushButton("(", mLargePage, "ParenOpen-Button");
	pbParenOpen->setAutoDefault(false);
	connect(pbParenOpen, SIGNAL(clicked(void)),SLOT(slotParenOpenclicked(void)));

	pbParenClose = new QPushButton(")", mLargePage, "ParenClose-Button");
	pbParenClose->setAutoDefault(false);
	connect(pbParenClose,SIGNAL(clicked(void)),SLOT(slotParenCloseclicked(void)));

	pbAND = new QPushButton("And", mLargePage, "AND-Button");
	QToolTip::add(pbAND, i18n("Bitwise AND"));
	pbAND->setAutoDefault(false);
	connect(pbAND, SIGNAL(clicked(void)), SLOT(slotANDclicked(void)));

	pbX = new QPushButton("X", mLargePage, "Multiply-Button");
	QToolTip::add(pbX, i18n("Multiplication"));
	pbX->setAutoDefault(false);
	connect(pbX, SIGNAL(clicked(void)), SLOT(slotXclicked(void)));

	pbDivision = new QPushButton("/", mLargePage, "Division-Button");
	QToolTip::add(pbDivision, i18n("Division"));
	pbDivision->setAutoDefault(false);
	connect(pbDivision, SIGNAL(clicked(void)), SLOT(slotDivisionclicked(void)));

	pbOR = new QPushButton("Or", mLargePage, "OR-Button");
	QToolTip::add(pbOR, i18n("Bitwise OR"));
	pbOR->setAutoDefault(false);
	connect(pbOR, SIGNAL(clicked(void)), SLOT(slotORclicked(void)));

	QPushButton *tmp_pb;

	NumButtonGroup = new QButtonGroup(0, "Num-Button-Group");
	connect(NumButtonGroup, SIGNAL(clicked(int)),
		SLOT(slotNumberclicked(int)));

	tmp_pb = new QPushButton("0", mLargePage, "0-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 0);

	tmp_pb = new QPushButton("1", mLargePage, "1-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 1);

	tmp_pb = new QPushButton("2", mLargePage, "2-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 2);

	tmp_pb = new QPushButton("3", mLargePage, "3-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 3);

	tmp_pb = new QPushButton("4", mLargePage, "4-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 4);

	tmp_pb = new QPushButton("5", mLargePage, "5-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 5);

	tmp_pb = new QPushButton("6", mLargePage, "6-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 6);

	tmp_pb = new QPushButton("7", mLargePage, "7-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 7);

	tmp_pb = new QPushButton("8", mLargePage, "8-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 8);

	tmp_pb = new QPushButton("9", mLargePage, "9-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 9);

	tmp_pb = new QPushButton("A", mSmallPage, "A-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 0xA);

	tmp_pb = new QPushButton("B", mSmallPage, "B-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 0xB);

	tmp_pb = new QPushButton("C", mSmallPage, "C-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 0xC);

	tmp_pb = new QPushButton("D", mSmallPage, "D-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 0xD);

	tmp_pb = new QPushButton("E", mSmallPage, "E-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 0xE);

	tmp_pb = new QPushButton("F", mSmallPage, "F-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 0xF);

	pbPlus = new QPushButton("+", mLargePage, "Plus-Button");
	QToolTip::add(pbPlus, i18n("Addition"));
	pbPlus->setAutoDefault(false);
	connect(pbPlus, SIGNAL(clicked(void)), SLOT(slotPlusclicked(void)));

	pbMinus = new QPushButton("-", mLargePage, "Minus-Button");
	QToolTip::add(pbMinus, i18n("Subtraction"));
	pbMinus->setAutoDefault(false);
	connect(pbMinus, SIGNAL(clicked(void)), SLOT(slotMinusclicked(void)));

	pbShift = new QPushButton("Lsh", mLargePage, "Shift-Button");
	QToolTip::add(pbShift, i18n("Bit shift"));
	pbShift->setAutoDefault(false);
	connect(pbShift, SIGNAL(clicked(void)), SLOT(slotShiftclicked(void)));

	pbPeriod = new QPushButton(".", mLargePage, "Period-Button");
	QToolTip::add(pbPeriod, i18n("Decimal point"));
	pbPeriod->setAutoDefault(false);
	connect(pbPeriod, SIGNAL(clicked(void)), SLOT(slotPeriodclicked(void)));

	pbEqual = new QPushButton("=", mLargePage, "Equal-Button");
	QToolTip::add(pbEqual, i18n("Result"));
	pbEqual->setAutoDefault(false);
	connect(pbEqual, SIGNAL(clicked(void)), SLOT(slotEqualclicked(void)));

	pbPercent = new QPushButton("%", mLargePage, "Percent-Button");
	QToolTip::add(pbPercent, i18n("Percent"));
	pbPercent->setAutoDefault(false);
	connect(pbPercent, SIGNAL(clicked(void)), SLOT(slotPercentclicked(void)));

	pbNegate = new QPushButton("Cmp", mLargePage, "OneComplement-Button");
	QToolTip::add(pbNegate, i18n("One's complement"));
	pbNegate->setAutoDefault(false);
	connect(pbNegate, SIGNAL(clicked(void)), SLOT(slotNegateclicked(void)));

	pbMod = new QPushButton("Mod", mLargePage, "Modulo-Button");
	QToolTip::add(pbMod, i18n("Modulo"));
	pbMod->setAutoDefault(false);
	connect(pbMod, SIGNAL(clicked(void)), SLOT(slotModclicked(void)));

	//
	// All these layouts are needed because all the groups have their
	// own size per row so we can't use one huge QGridLayout (mosfet)
	//
	// 1999-10-31 Espen Sand:
	// Some more modifications to the improved layout. Note: No need
	// to "activate()" the layout at the end.
	//
	//
	QGridLayout *smallBtnLayout = new QGridLayout(mSmallPage, 6, 3, 0,
		mInternalSpacing);
	QGridLayout *largeBtnLayout = new QGridLayout(mLargePage, 5, 6, 0,
		mInternalSpacing);

	QHBoxLayout *topLayout		= new QHBoxLayout();
	QHBoxLayout *radioLayout	= new QHBoxLayout();
	QHBoxLayout *btnLayout		= new QHBoxLayout();
	QHBoxLayout *statusLayout	= new QHBoxLayout();

	// bring them all together
	QVBoxLayout *mainLayout = new QVBoxLayout(central, mInternalSpacing,
		mInternalSpacing);

	mainLayout->addLayout(topLayout );
	mainLayout->addLayout(radioLayout, 1);
	mainLayout->addLayout(btnLayout);
	mainLayout->addLayout(statusLayout);

	// button layout
	btnLayout->addWidget(mSmallPage,0,AlignTop);
	btnLayout->addSpacing(mInternalSpacing);
	btnLayout->addWidget(mLargePage,0,AlignTop);

	// small button layout
	smallBtnLayout->addWidget(pbHyp, 0, 0);
	smallBtnLayout->addWidget(pbStatNum, 0, 0);
	smallBtnLayout->addWidget(pbInv, 0, 1);
	smallBtnLayout->addWidget(NumButtonGroup->find(0xA), 0, 2);

	smallBtnLayout->addWidget(pbSin, 1, 0);
	smallBtnLayout->addWidget(pbStatMean, 1, 0);
	smallBtnLayout->addWidget(pbPlusMinus, 1, 1);
	smallBtnLayout->addWidget(NumButtonGroup->find(0xB), 1, 2);

	smallBtnLayout->addWidget(pbCos, 2, 0);
	smallBtnLayout->addWidget(pbStatStdDev, 2, 0);
	smallBtnLayout->addWidget(pbReci, 2, 1);
	smallBtnLayout->addWidget(NumButtonGroup->find(0xC), 2, 2);

	smallBtnLayout->addWidget(pbTan, 3, 0);
	smallBtnLayout->addWidget(pbStatMedian, 3, 0);
	smallBtnLayout->addWidget(pbFactorial, 3, 1);
	smallBtnLayout->addWidget(NumButtonGroup->find(0xD), 3, 2);

	smallBtnLayout->addWidget(pbLog, 4, 0);
	smallBtnLayout->addWidget(pbStatDataInput, 4, 0);
	smallBtnLayout->addWidget(pbSquare, 4, 1);
	smallBtnLayout->addWidget(NumButtonGroup->find(0xE), 4, 2);

	smallBtnLayout->addWidget(pbLn, 5, 0);
	smallBtnLayout->addWidget(pbStatClearData, 5, 0);
	smallBtnLayout->addWidget(pbPower, 5, 1);
	smallBtnLayout->addWidget(NumButtonGroup->find(0xF), 5, 2);

	smallBtnLayout->setRowStretch(0, 0);
	smallBtnLayout->setRowStretch(1, 0);
	smallBtnLayout->setRowStretch(2, 0);
	smallBtnLayout->setRowStretch(3, 0);
	smallBtnLayout->setRowStretch(4, 0);
	smallBtnLayout->setRowStretch(5, 0);

	// large button layout
	largeBtnLayout->addWidget(pbEE, 0, 0);
	largeBtnLayout->addWidget(pbMR, 0, 1);
	largeBtnLayout->addWidget(pbMPlusMinus, 0, 2);
	largeBtnLayout->addWidget(pbMC, 0, 3);
	largeBtnLayout->addWidget(pbClear, 0, 4);
	largeBtnLayout->addWidget(pbAC, 0, 5);

	largeBtnLayout->addWidget(NumButtonGroup->find(7), 1, 0);
	largeBtnLayout->addWidget(NumButtonGroup->find(8), 1, 1);
	largeBtnLayout->addWidget(NumButtonGroup->find(9), 1, 2);
	largeBtnLayout->addWidget(pbParenOpen, 1, 3);
	largeBtnLayout->addWidget(pbParenClose, 1, 4);
	largeBtnLayout->addWidget(pbAND, 1, 5);

	largeBtnLayout->addWidget(NumButtonGroup->find(4), 2, 0);
	largeBtnLayout->addWidget(NumButtonGroup->find(5), 2, 1);
	largeBtnLayout->addWidget(NumButtonGroup->find(6), 2, 2);
	largeBtnLayout->addWidget(pbX, 2, 3);
	largeBtnLayout->addWidget(pbDivision, 2, 4);
	largeBtnLayout->addWidget(pbOR, 2, 5);

	largeBtnLayout->addWidget(NumButtonGroup->find(1), 3, 0);
	largeBtnLayout->addWidget(NumButtonGroup->find(2), 3, 1);
	largeBtnLayout->addWidget(NumButtonGroup->find(3), 3, 2);
	largeBtnLayout->addWidget(pbPlus, 3, 3);
	largeBtnLayout->addWidget(pbMinus, 3, 4);
	largeBtnLayout->addWidget(pbShift, 3, 5);

        largeBtnLayout->addWidget(NumButtonGroup->find(0), 4, 0);
	largeBtnLayout->addWidget(pbPeriod, 4, 1);
	largeBtnLayout->addWidget(pbEqual, 4, 2);
	largeBtnLayout->addWidget(pbPercent, 4, 3);
	largeBtnLayout->addWidget(pbNegate, 4, 4);
	largeBtnLayout->addWidget(pbMod, 4, 5);

	largeBtnLayout->addColSpacing(0,10);
	largeBtnLayout->addColSpacing(1,10);
	largeBtnLayout->addColSpacing(2,10);
	largeBtnLayout->addColSpacing(3,10);
	largeBtnLayout->addColSpacing(4,10);


	// top layout
	topLayout->addWidget(mConfigButton);
	topLayout->addWidget(mHelpButton);
	topLayout->addWidget(calc_display, 10);

	// radiobutton layout
	radioLayout->addWidget(base_group);
	radioLayout->addWidget(angle_group);

	// status layout
	statusLayout->addWidget(statusINVLabel);
	statusLayout->addWidget(statusHYPLabel);
	statusLayout->addWidget(statusERRORLabel, 10);

	mFunctionButtonList.append(pbHyp);
	mFunctionButtonList.append(pbInv);
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

	mStatButtonList.append(pbStatNum);
	mStatButtonList.append(pbStatMean);
	mStatButtonList.append(pbStatStdDev);
	mStatButtonList.append(pbStatMedian);
	mStatButtonList.append(pbStatDataInput);
	mStatButtonList.append(pbStatClearData);

	mMemButtonList.append(pbEE);
	mMemButtonList.append(pbMR);
	mMemButtonList.append(pbMPlusMinus);
	mMemButtonList.append(pbMC);
	mMemButtonList.append(pbClear);
	mMemButtonList.append(pbAC);

	mOperationButtonList.append(pbX);
	mOperationButtonList.append(pbParenOpen);
	mOperationButtonList.append(pbParenClose);
	mOperationButtonList.append(pbAND);
	mOperationButtonList.append(pbDivision);
	mOperationButtonList.append(pbOR);
	mOperationButtonList.append(pbPlus);
	mOperationButtonList.append(pbMinus);
	mOperationButtonList.append(pbShift);
	mOperationButtonList.append(pbPeriod);
	mOperationButtonList.append(pbEqual);
	mOperationButtonList.append(pbPercent);
	mOperationButtonList.append(pbNegate);
	mOperationButtonList.append(pbMod);

	set_colors();
	set_display_font();
	set_precision();
	set_style();
	Base_Selected(1);
	InitializeCalculator();

	updateGeometry();
	setFixedSize(minimumSize());
}

QtCalculator::~QtCalculator()
{
	delete mConfigureDialog;
	delete selection_timer;
	delete status_timer;
}

void QtCalculator::updateGeometry()
{
    QObjectList *l;
    QSize s;
    int margin;

    //
    // Calculator buttons
    //
    s.setWidth(mSmallPage->fontMetrics().width("MMM"));
    s.setHeight(mSmallPage->fontMetrics().lineSpacing());

    l = (QObjectList*)mSmallPage->children(); // silence please

    for(uint i=0; i < l->count(); i++)
    {
        QObject *o = l->at(i);
        if( o->isWidgetType() )
        {
            margin = QApplication::style().
                pixelMetric(QStyle::PM_ButtonMargin, ((QWidget *)o))*2;
            ((QWidget*)o)->setMinimumSize(s.width()+margin, s.height()+margin);
            ((QWidget*)o)->installEventFilter( this );
            ((QWidget*)o)->setAcceptDrops(true);
        }
    }

    l = (QObjectList*)mLargePage->children(); // silence please

    int h1 = (NumButtonGroup->find(0xF))->minimumSize().height();
    int h2 = (int)((((float)h1 + 4.0) / 5.0));
    s.setWidth(mLargePage->fontMetrics().width("MMM") +
               QApplication::style().
               pixelMetric(QStyle::PM_ButtonMargin, NumButtonGroup->find(0xF))*2);
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

    //
    // The status bar
    //
    s.setWidth( statusINVLabel->fontMetrics().width("NORM") +
                statusINVLabel->frameWidth() * 2 + 10);
    statusINVLabel->setMinimumWidth(s.width());
    statusHYPLabel->setMinimumWidth(s.width());

    //setFixedSize(minimumSize());
}

void QtCalculator::Base_Selected(int base)
{
	// Call down to the core
	base_selected(base);

	// Enable the buttons not available in this base
	for (int i=0; i<current_base; i++)
	  NumButtonGroup->find(i)->setEnabled (true);

	// Disable the buttons not available in this base
	for (int i=current_base; i<16; i++)
	  NumButtonGroup->find(i)->setEnabled (false);

	// Only enable the decimal point in decimal
	pbPeriod->setEnabled(current_base == NB_DECIMAL);
}

void QtCalculator::Hex_Selected()
{
	basebutton[0]->setChecked(true);
	basebutton[1]->setChecked(false);
	basebutton[2]->setChecked(false);
	basebutton[3]->setChecked(false);
	Base_Selected(0);
}

void QtCalculator::Dec_Selected()
{
	basebutton[0]->setChecked(false);
	basebutton[1]->setChecked(true);
	basebutton[2]->setChecked(false);
	basebutton[3]->setChecked(false);
	Base_Selected(1);
}

void QtCalculator::Oct_Selected()
{
	basebutton[0]->setChecked(false);
	basebutton[1]->setChecked(false);
	basebutton[2]->setChecked(true);
	basebutton[3]->setChecked(false);
	Base_Selected(2);
}

void QtCalculator::Bin_Selected()
{
	basebutton[0]->setChecked(false);
	basebutton[1]->setChecked(false);
	basebutton[2]->setChecked(false);
	basebutton[3]->setChecked(true);
	Base_Selected(3);
}

void QtCalculator::Deg_Selected()
{
	anglebutton[0]->setChecked(true);
	anglebutton[1]->setChecked(false);
	anglebutton[2]->setChecked(false);
	angle_selected(0);
}

void QtCalculator::Rad_Selected()
{
	anglebutton[0]->setChecked(false);
	anglebutton[1]->setChecked(true);
	anglebutton[2]->setChecked(false);
	angle_selected(1);
}

void QtCalculator::Gra_Selected()
{
	anglebutton[0]->setChecked(false);
	anglebutton[1]->setChecked(false);
	anglebutton[2]->setChecked(true);
	angle_selected(2);
}

void QtCalculator::configurationChanged(const DefStruct &state)
{
	kcalcdefaults = state;

	set_colors();
	set_precision();
	set_display_font();
	set_style();

	updateGeometry();
	resize(minimumSize());

	//
	// 1999-10-31 Espen Sand: Don't ask me why ;)
	//
	kapp->processOneEvent();
	setFixedHeight(minimumHeight());
}

void QtCalculator::keyPressEvent(QKeyEvent *e)
{
    if ( ( e->state() & KeyButtonMask ) == 0 || ( e->state() & ShiftButton ) ) {
	switch (e->key())
	{
	case Key_F1:
		kapp->invokeHelp();
		break;
	case Key_F2:
		configclicked();
		break;
	case Key_F3:
		kcalcdefaults.style = !kcalcdefaults.style;
		set_style();
		break;
	case Key_Up:
		history_prev();
		break;
	case Key_Down:
		history_next();
		break;
	case Key_Next:
		pbAC->animateClick();
		break;
	case Key_Prior:
		pbClear->animateClick();
		break;
	case Key_H:
		pbHyp->toggle();
		break;
	case Key_I:
		pbInv->toggle();
		break;
	case Key_A:
	        (NumButtonGroup->find(0xA))->animateClick();
		break;
	case Key_E:
		if (current_base == NB_HEX)
			(NumButtonGroup->find(0xE))->animateClick();
		else
			pbEE->animateClick();
		break;
	case Key_Escape:
		pbClear->animateClick();
		break;
	case Key_Delete:
		pbAC->animateClick();
		break;
	case Key_S:
		pbSin->animateClick();
		break;
	case Key_Backslash:
		pbPlusMinus->animateClick();
		break;
	case Key_B:
		(NumButtonGroup->find(0xB))->animateClick();
		break;
	case Key_7:
		(NumButtonGroup->find(7))->animateClick();
		break;
	case Key_8:
		(NumButtonGroup->find(8))->animateClick();
		break;
	case Key_9:
		(NumButtonGroup->find(9))->animateClick();
		break;
	case Key_ParenLeft:
		pbParenOpen->animateClick();
		break;
	case Key_ParenRight:
		pbParenClose->animateClick();
		break;
	case Key_Ampersand:
		pbAND->animateClick();
		break;
	case Key_C:
		if (current_base == NB_HEX)
			(NumButtonGroup->find(0xC))->animateClick();
		else
			pbCos->animateClick();
		break;
	case Key_4:
		(NumButtonGroup->find(4))->animateClick();
		break;
	case Key_5:
		(NumButtonGroup->find(5))->animateClick();
		break;
	case Key_6:
		(NumButtonGroup->find(6))->animateClick();
		break;
	case Key_Asterisk:
        case Key_multiply:
		pbX->animateClick();
		break;
	case Key_Slash:
        case Key_division:
		pbDivision->animateClick();
		break;
	case Key_O:
		pbOR->animateClick();
		break;
	case Key_T:
		pbTan->animateClick();
		break;
	case Key_Exclam:
		pbFactorial->animateClick();
		break;
	case Key_D:
		if(kcalcdefaults.style == 0)
			(NumButtonGroup->find(0xD))->animateClick(); // trig mode
		else
			pbStatDataInput->animateClick(); // stat mode
		break;
	case Key_1:
		(NumButtonGroup->find(1))->animateClick();
		break;
	case Key_2:
		(NumButtonGroup->find(2))->animateClick();
		break;
	case Key_3:
		(NumButtonGroup->find(3))->animateClick();
		break;
	case Key_Plus:
		pbPlus->animateClick();
		break;
	case Key_Minus:
		pbMinus->animateClick();
		break;
	case Key_Less:
		pbShift->animateClick();
		break;
	case Key_N:
		pbLn->animateClick();
		break;
	case Key_L:
		pbLog->animateClick();
		break;
	case Key_AsciiCircum:
		pbPower->animateClick();
		break;
	case Key_F:
		(NumButtonGroup->find(0xF))->animateClick();
		break;
	case Key_Period:
	case Key_Comma:
		pbPeriod->animateClick();
		break;
	case Key_0:
		(NumButtonGroup->find(0))->animateClick();
		break;
	case Key_Equal:
	case Key_Return:
	case Key_Enter:
		pbEqual->animateClick();
		break;
	case Key_Percent:
		pbPercent->animateClick();
		break;
	case Key_AsciiTilde:
		pbNegate->animateClick();
		break;
	case Key_Colon:
		pbMod->animateClick();
		break;
	case Key_BracketLeft:
        case Key_twosuperior:
		pbSquare->animateClick();
		break;
	case Key_Backspace:
		SubtractDigit();
		// pbAC->animateClick();
		break;
	case Key_R:
		pbReci->animateClick();
		break;
	}
    }
}

void QtCalculator::slotEEclicked(void)
{
	if(!display_error)
	{
		EE();
	}
	else
		KNotifyClient::beep();
}

void QtCalculator::slotInvtoggled(bool myboolean)
{
	SetInverse(myboolean);
}

void QtCalculator::slotHyptoggled(bool myboolean)
{
	EnterHyp(myboolean);
	if(myboolean)
	{
		pbSin->setText("Sinh");
		QToolTip::remove(pbSin);
		QToolTip::add(pbSin, i18n("Hyperbolic Sine"));
		pbCos->setText("Cosh");
		QToolTip::remove(pbCos);
		QToolTip::add(pbCos, i18n("Hyperbolic Cosine"));
		pbTan->setText("Tanh");
		QToolTip::remove(pbTan);
		QToolTip::add(pbTan, i18n("Hyperbolic Tangent"));
	}
	else
	{
		pbSin->setText("Sin");
		QToolTip::remove(pbSin);
		QToolTip::add(pbSin, i18n("Sine"));
		pbCos->setText("Cos");
		QToolTip::remove(pbCos);
		QToolTip::add(pbCos, i18n("Cosine"));
		pbTan->setText("Tan");
		QToolTip::remove(pbTan);
		QToolTip::add(pbTan, i18n("Tangent"));
	}
}

void QtCalculator::deactivateInvButton()
{
	pbInv->setOn(false);
}

void QtCalculator::slotMRclicked(void)
{
	MR();
}

void QtCalculator::slotNumberclicked(int number_clicked)
{
  Q_ASSERT(number_clicked < current_base);

  EnterDigit(number_clicked);
}

void QtCalculator::slotSinclicked(void)
{
	ComputeSin();
}

void QtCalculator::slotPlusMinusclicked(void)
{
	EnterNegate();
}

void QtCalculator::slotMPlusMinusclicked(void)
{
	Mplusminus();
}

void QtCalculator::slotCosclicked(void)
{
	ComputeCos();
}

void QtCalculator::slotReciclicked(void)
{
	EnterRecip();
}

void QtCalculator::slotTanclicked(void)
{
	ComputeTan();
}

void QtCalculator::slotFactorialclicked(void)
{
	EnterFactorial();
}

void QtCalculator::slotLogclicked(void)
{
	ComputeLog10();
}

void QtCalculator::slotSquareclicked(void)
{
	EnterSquare();
}

void QtCalculator::slotLnclicked(void)
{
	ComputeNaturalLog();
}

void QtCalculator::slotPowerclicked(void)
{
	Power();
}

void QtCalculator::slotMCclicked(void)
{
	MC();
}

void QtCalculator::slotClearclicked(void)
{
	Clear();
}

void QtCalculator::slotACclicked(void)
{
	ClearAll();
}

void QtCalculator::slotParenOpenclicked(void)
{
	EnterOpenParen();
}

void QtCalculator::slotParenCloseclicked(void)
{
	EnterCloseParen();
}

void QtCalculator::slotANDclicked(void)
{
	And();
}

void QtCalculator::slotXclicked(void)
{
	Multiply();
}

void QtCalculator::slotDivisionclicked(void)
{
	Divide();
}

void QtCalculator::slotORclicked(void)
{
	Or();
}

void QtCalculator::slotPlusclicked(void)
{
	Plus();
}

void QtCalculator::slotMinusclicked(void)
{
	Minus();
}

void QtCalculator::slotShiftclicked(void)
{
	Shift();
}

void QtCalculator::slotPeriodclicked(void)
{
	EnterDecimal();
}

void QtCalculator::slotEqualclicked(void)
{
	EnterEqual();
}

void QtCalculator::slotPercentclicked(void)
{
	EnterPercent();
}

void QtCalculator::slotNegateclicked(void)
{
	EnterNotCmp();
}

void QtCalculator::slotModclicked(void)
{
	Mod();
}

void QtCalculator::slotStatNumclicked(void)
{
	DisplayNumData();
}

void QtCalculator::slotStatMeanclicked(void)
{
	ComputeMean();
}

void QtCalculator::slotStatStdDevclicked(void)
{
	ComputeStd();
}

void QtCalculator::slotStatMedianclicked(void)
{
	ComputeMedian();
}

void QtCalculator::slotStatDataInputclicked(void)
{
	EnterStatData();
}

void QtCalculator::slotStatClearDataclicked(void)
{
	ClearStatMem();
}

//-------------------------------------------------------------------------
// Name: configclicked()
//-------------------------------------------------------------------------
void QtCalculator::configclicked()
{
	if(mConfigureDialog == 0)
	{
		mConfigureDialog = new ConfigureDialog( 0, 0, false );
		mConfigureDialog->setState( kcalcdefaults );

		connect( mConfigureDialog, SIGNAL( valueChanged(const DefStruct &)),
			this, SLOT(configurationChanged(const DefStruct &)));
	}
	mConfigureDialog->show();
        mConfigureDialog->raise();
}


//-------------------------------------------------------------------------
// Name: set_style()
//-------------------------------------------------------------------------
void QtCalculator::set_style()
{
	switch(kcalcdefaults.style)
	{
	case  0:
	        pbHyp->show();
		pbSin->show();
		pbCos->show();
		pbTan->show();
		pbLog->show();
		pbLn->show();
		pbStatNum->hide();
		pbStatMean->hide();
		pbStatStdDev->hide();
		pbStatMedian->hide();
		pbStatDataInput->hide();
		pbStatClearData->hide();
		break;

	case 1:
	        pbHyp->hide();
		pbSin->hide();
		pbCos->hide();
		pbTan->hide();
		pbLog->hide();
		pbLn ->hide();
		pbStatNum->show();
		pbStatMean->show();
		pbStatStdDev->show();
		pbStatMedian->show();
		pbStatDataInput->show();
		pbStatClearData->show();
		break;

	default:
		break;
	}

	angle_group->setEnabled(kcalcdefaults.style == 0);
}

//-------------------------------------------------------------------------
// Name: readSettings()
//-------------------------------------------------------------------------
void QtCalculator::readSettings()
{
	QString str;

	KConfig *config = KGlobal::config();
	config->setGroup("Font");
	QFont tmpFont(KGlobalSettings::generalFont().family() ,14 ,QFont::Bold);
	kcalcdefaults.font = config->readFontEntry("Font",&tmpFont);

	config->setGroup("Colors");
	QColor tmpC(189, 255, 180);
	QColor blackC(0, 0, 0);
	QColor defaultButtonColor = palette().active().background();

	kcalcdefaults.forecolor = config->readColorEntry("ForeColor", &blackC);
	kcalcdefaults.backcolor = config->readColorEntry("BackColor", &tmpC);
	kcalcdefaults.numberButtonColor  =
		config->readColorEntry("NumberButtonsColor",&defaultButtonColor);
	kcalcdefaults.functionButtonColor =
		config->readColorEntry("FunctionButtonsColor", &defaultButtonColor);
	kcalcdefaults.statButtonColor =
		config->readColorEntry("StatButtonsColor", &defaultButtonColor);
	kcalcdefaults.hexButtonColor =
		config->readColorEntry("HexButtonsColor",&defaultButtonColor);
	kcalcdefaults.memoryButtonColor =
		config->readColorEntry("MemoryButtonsColor",&defaultButtonColor);
	kcalcdefaults.operationButtonColor =
		config->readColorEntry("OperationButtonsColor",&defaultButtonColor);

	config->setGroup("Precision");

#ifdef HAVE_LONG_DOUBLE
	kcalcdefaults.precision	= config->readNumEntry("precision", (int)14);
#else
	kcalcdefaults.precision	= config->readNumEntry("precision", (int)10);
#endif

	kcalcdefaults.fixedprecision =
		config->readNumEntry("fixedprecision", (int)2);
	kcalcdefaults.fixed = config->readBoolEntry("fixed", false);

	config->setGroup("General");
	kcalcdefaults.style	= config->readNumEntry("style", (int)0);
	kcalcdefaults.beep	= config->readBoolEntry("beep", true);
	kcalcdefaults.capres	= config->readBoolEntry("captionresult",false);
}

//-------------------------------------------------------------------------
// Name: writeSettings()
//-------------------------------------------------------------------------
void QtCalculator::writeSettings()
{
	KConfig *config = KGlobal::config();

	config->setGroup("Font");
	config->writeEntry("Font", kcalcdefaults.font);

	config->setGroup("Colors");
	config->writeEntry("ForeColor",kcalcdefaults.forecolor);
	config->writeEntry("BackColor",kcalcdefaults.backcolor);
	config->writeEntry("NumberButtonsColor",
		(NumButtonGroup->find(0))->palette().active().button());
	config->writeEntry("FunctionButtonsColor",
		mFunctionButtonList.first()->palette().active().button());
	config->writeEntry("StatButtonsColor",
		mStatButtonList.first()->palette().active().button());
	config->writeEntry("HexButtonsColor",
		(NumButtonGroup->find(0xA))->palette().active().button());
	config->writeEntry("MemoryButtonsColor",
		mMemButtonList.first()->palette().active().button());
	config->writeEntry("OperationButtonsColor",
		mOperationButtonList.first()->palette().active().button());

	config->setGroup("Precision");
	config->writeEntry("precision",  kcalcdefaults.precision);
	config->writeEntry("fixedprecision",  kcalcdefaults.fixedprecision);
	config->writeEntry("fixed",  kcalcdefaults.fixed);

	config->setGroup("General");
	config->writeEntry("style",(int)kcalcdefaults.style);
	config->writeEntry("beep", kcalcdefaults.beep);
	config->writeEntry("captionresult", kcalcdefaults.capres);

	config->sync();
}

//-------------------------------------------------------------------------
// Name: display_selected()
//-------------------------------------------------------------------------
void QtCalculator::display_selected()
{
	if(calc_display->Button() == LeftButton) {

		if(calc_display->isLit()) {
			QClipboard *cb = QApplication::clipboard();
			bool oldMode = cb->selectionModeEnabled();
			cb->setSelectionMode(true);
			cb->setText(calc_display->text());
			cb->setSelectionMode(oldMode);
			selection_timer->start(100);
		} else {
			selection_timer->stop();
		}

		invertColors();
	} else {
		QClipboard *cb = QApplication::clipboard();
		bool oldMode = cb->selectionModeEnabled();
		cb->setSelectionMode(true);

		CALCAMNT result;
		bool was_ok;
		result = (CALCAMNT) cb->text().toDouble(&was_ok);
		cb->setSelectionMode(oldMode);

		if (!was_ok)
			result = (CALCAMNT) (0);

		last_input = PASTE;
		DISPLAY_AMOUNT = result;
		UpdateDisplay();
	}
}

//-------------------------------------------------------------------------
// Name: selection_timed_out()
//-------------------------------------------------------------------------
void QtCalculator::selection_timed_out()
{
	selection_timer->stop();
	calc_display->setLit(false);
	invertColors();
}

//-------------------------------------------------------------------------
// Name: clear_status_label()
//-------------------------------------------------------------------------
void QtCalculator::clear_status_label()
{
	statusERRORLabel->clear();
	status_timer->stop();
}

//-------------------------------------------------------------------------
// Name: setStatusLabel(const QString& string)
//-------------------------------------------------------------------------
void QtCalculator::setStatusLabel(const QString& string)
{
	statusERRORLabel->setText(string);
	status_timer->start(3000, true);
}


//-------------------------------------------------------------------------
// Name: invertColors()
//-------------------------------------------------------------------------
void QtCalculator::invertColors()
{
	QColor tmpcolor;

	if(calc_display->isLit())
	{
		tmpcolor = kcalcdefaults.backcolor;
		kcalcdefaults.backcolor = kcalcdefaults.forecolor;
		kcalcdefaults.forecolor = tmpcolor;

		set_colors();

		tmpcolor = kcalcdefaults.backcolor;
		kcalcdefaults.backcolor = kcalcdefaults.forecolor;
		kcalcdefaults.forecolor = tmpcolor;
	}
	else
		set_colors();
}

//-------------------------------------------------------------------------
// Name: closeEvent(QCloseEvent *)
//-------------------------------------------------------------------------
void QtCalculator::closeEvent(QCloseEvent *)
{
	quitCalc();
}


//-------------------------------------------------------------------------
// Name: quitCalc()
//-------------------------------------------------------------------------
void QtCalculator::quitCalc()
{
	writeSettings();
	qApp->quit();
}


//-------------------------------------------------------------------------
// Name: set_colors()
//-------------------------------------------------------------------------
void QtCalculator::set_colors()
{
	QPalette pal = calc_display->palette();
	QPushButton *p = NULL;

	pal.setColor(QColorGroup::Text, kcalcdefaults.forecolor);
	pal.setColor(QColorGroup::Foreground, kcalcdefaults.forecolor);
	pal.setColor(QColorGroup::Background, kcalcdefaults.backcolor);

	calc_display->setPalette(pal);
	calc_display->setBackgroundColor(kcalcdefaults.backcolor);

	/*
	QPalette mypalette = (calc_display->palette()).copy();

	QColorGroup cgrp = mypalette.normal();
	QColorGroup ncgrp(kcalcdefaults.forecolor,
	cgrp.background(),
	cgrp.light(),
	cgrp.dark(),
	cgrp.mid(),
	kcalcdefaults.forecolor,
	kcalcdefaults.backcolor);

	mypalette.setNormal(ncgrp);
	mypalette.setDisabled(ncgrp);
	mypalette.setActive(ncgrp);

	calc_display->setPalette(mypalette);
	calc_display->setBackgroundColor(kcalcdefaults.backcolor);
	*/

	QColor bg = palette().active().background();

	QPalette numPal(kcalcdefaults.numberButtonColor, bg);
	for(int i=0; i<10; i++)
	{
		(NumButtonGroup->find(i))->setPalette(numPal);
	}

	QPalette funcPal(kcalcdefaults.functionButtonColor, bg);
	for(p = mFunctionButtonList.first(); p;
		p=mFunctionButtonList.next())
	{
		p->setPalette(funcPal);
	}

	QPalette statPal(kcalcdefaults.statButtonColor, bg);
	for(p = mStatButtonList.first(); p;
		p=mStatButtonList.next())
	{
		p->setPalette(statPal);
	}

	QPalette hexPal(kcalcdefaults.hexButtonColor, bg);
	for(int i=10; i<16; i++)
	{
		(NumButtonGroup->find(i))->setPalette(hexPal);
	}

	QPalette memPal(kcalcdefaults.memoryButtonColor, bg);
	for(p = mMemButtonList.first(); p; p=mMemButtonList.next())
	{
		p->setPalette(memPal);
	}

	QPalette opPal(kcalcdefaults.operationButtonColor, bg);
	for(p = mOperationButtonList.first(); p;
	p=mOperationButtonList.next())
	{
		p->setPalette(opPal);
	}
}

//-------------------------------------------------------------------------
// Name: set_precision()
//-------------------------------------------------------------------------
void QtCalculator::set_precision()
{
	// TODO: make this do somthing!!
	UpdateDisplay();
}

//-------------------------------------------------------------------------
// Name: set_display_font()
//-------------------------------------------------------------------------
void QtCalculator::set_display_font()
{
	calc_display->setFont(kcalcdefaults.font);
}

//-------------------------------------------------------------------------
// Name: history_next()
//-------------------------------------------------------------------------
void QtCalculator::history_next()
{
  	if((history_list.empty()) || (history_index <= 0))
	{
		KNotifyClient::beep();
		return;
	}

    last_input = RECALL;
    DISPLAY_AMOUNT = history_list[--history_index];
    UpdateDisplay();
}

//-------------------------------------------------------------------------
// Name: history_prev()
//-------------------------------------------------------------------------
void QtCalculator::history_prev()
{

	if((history_list.empty()) || (history_index >= ((int)history_list.size() - 1)))
	{
		KNotifyClient::beep();
		return;
	}

	last_input = RECALL;
	DISPLAY_AMOUNT = history_list[++history_index];
	UpdateDisplay();
}

//-------------------------------------------------------------------------
// Name: eventFilter(QObject *o, QEvent *e)
//-------------------------------------------------------------------------
bool QtCalculator::eventFilter(QObject *o, QEvent *e)
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
		if(!o->isA("QPushButton"))
			return false;

		QColor c;
		QDropEvent *ev = (QDropEvent *)e;
		if( KColorDrag::decode(ev, c))
		{
		        QPtrList<QPushButton> *list;
			int num_but;
			if((num_but = NumButtonGroup->id((QPushButton*)o))
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
			else if( mFunctionButtonList.findRef((QPushButton*)o) != -1)
			{
				list = &mFunctionButtonList;
			}
			else if( mStatButtonList.findRef((QPushButton*)o) != -1)
			{
				list = &mStatButtonList;
			}
			else if( mMemButtonList.findRef((QPushButton*)o) != -1)
			{
				list = &mMemButtonList;
			}
			else if( mOperationButtonList.findRef((QPushButton*)o) != -1)
			{
				list = &mOperationButtonList;
			}
			else
				return false;

			QPalette pal(c, palette().active().background());

			for(QPushButton *p = list->first(); p; p=list->next())
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

//-------------------------------------------------------------------------
// Name: main(int argc, char *argv[])
//-------------------------------------------------------------------------
int main(int argc, char *argv[])
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
		"(c) 1996-2000, Bernd Johannes Wuebben\n"
		"(c) 2000-2002, The KDE Team",
                precisionStatement.latin1());

	aboutData.addAuthor("Bernd Johannes Wuebben", 0, "wuebben@kde.org");
	aboutData.addAuthor("Evan Teran", 0, "emt3734@rit.edu");
	aboutData.addAuthor("Espen Sand", 0, "espen@kde.org");
	aboutData.addAuthor("Chris Howells", 0, "howells@kde.org");
        aboutData.addAuthor("Aaron J. Seigo", 0, "aseigo@olympusproject.org");
        aboutData.addAuthor("Charles Samuels", 0, "charles@altair.dhs.org");
	KCmdLineArgs::init(argc, argv, &aboutData);

	KApplication app;

#if 0
	app->enableSessionManagement(true);
	app->setWmCommand(argv[0]);
#endif

	QtCalculator *calc = new QtCalculator;
	app.setTopWidget(calc);
	calc->setCaption(QString::null);
	calc->show();

	int exitCode = app.exec();
	delete calc;

	return(exitCode);
}
