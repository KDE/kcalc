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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <qlayout.h>
#include <qobjectlist.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qtooltip.h>
#include <qstyle.h>

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

#include "dlabel.h"
#include "kcalc.h"
#include "version.h"
#include "general.h"
#include "colors.h"

#include "kcalc_settings.h"

const CALCAMNT KCalculator::pi = (ASIN(1L) * 2L);


static const char *description = I18N_NOOP("KDE Calculator");
static const char *version = KCALCVERSION;

//
// * ported to QLayout (mosfet 10/28/99)
//
// * 1999-10-31 Espen Sand: More modifications.
//   All fixed sizes removed.
//   New config dialog.
//   Look in updateGeometry() for size settings.
//


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
	connect(kapp,SIGNAL(kdisplayPaletteChanged()), this, SLOT(set_colors()));

	calc_display = new DispLogic(central, "display");

	// Status bar contents
	statusBar()->insertFixedItem(" NORM ", 0, true);
	statusBar()->setItemAlignment(0, AlignCenter);

	setupMainActions();

	createGUI();

	// How can I make the toolBar not appear at all?
	// This is not a nice solution.
	toolBar()->close();

	// Create Number Base Button Group
	base_group = new QButtonGroup(4, Horizontal,  central, "base");
	base_group->setTitle(i18n("Base"));
	connect(base_group, SIGNAL(clicked(int)), SLOT(slotBaseSelected(int)));

	basebutton[0] = new QRadioButton(base_group);
	basebutton[0]->setText("He&x");
	QToolTip::add(basebutton[0], i18n("Hexadecimal"));

	basebutton[1] = new QRadioButton(base_group);
	basebutton[1]->setText("&Dec");
	QToolTip::add(basebutton[1], i18n("Decimal"));
	basebutton[1]->setChecked(true);

	basebutton[2] = new QRadioButton(base_group);
	basebutton[2]->setText("&Oct");
	QToolTip::add(basebutton[2], i18n("Octal"));

	basebutton[3] = new QRadioButton(base_group);
	basebutton[3]->setText("&Bin");
	QToolTip::add(basebutton[3], i18n("Binary"));

	// create angle button group
	angle_group = new QButtonGroup(3, Horizontal, central, "angle");
	angle_group->setTitle(i18n( "Angle") );
	connect(angle_group, SIGNAL(clicked(int)), SLOT(slotAngleSelected(int)));

	anglebutton[0] = new QRadioButton(angle_group);
	anglebutton[0]->setText("D&eg");
	QToolTip::add(anglebutton[0], i18n("Degrees"));
	anglebutton[0]->setChecked(true);

	anglebutton[1] = new QRadioButton(angle_group);
	anglebutton[1]->setText("&Rad");
	QToolTip::add(anglebutton[1], i18n("Radians"));

	anglebutton[2] = new QRadioButton(angle_group);
	anglebutton[2]->setText("&Gra");
	QToolTip::add(anglebutton[2], i18n("Gradians"));

	pbInv = new QPushButton("Inv", central, "Inverse-Button");
	QToolTip::add(pbInv, i18n("Inverse mode"));
	pbInv->setAutoDefault(false);
	accel()->insert("Switch Inverse", i18n("Pressed Inverse-Button"),
			0, Qt::Key_I, pbInv, SLOT(animateClick()));
	connect(pbInv, SIGNAL(toggled(bool)), SLOT(slotInvtoggled(bool)));
	pbInv->setToggleButton(true);

	//
	//  Create Calculator Buttons
	//

	// First the widgets that are the parents of the buttons
	mSmallPage = new QWidget(central);
	mLargePage = new QWidget(central);

	pbPi = new QPushButton(QString::fromUtf8("π", -1), // Pi in utf8
			       mSmallPage, "Pi-Button");
	QToolTip::add(pbPi, i18n("Pi=3.1415..."));
	pbPi->setAutoDefault(false);
	connect(pbPi, SIGNAL(clicked(void)), SLOT(slotPiclicked(void)));

	pbSin = new QPushButton("Sin ", mSmallPage, "Sin-Button");
	QToolTip::add(pbSin, i18n("Sine"));
	pbSin->setAutoDefault(false);
	accel()->insert("Apply Sine", i18n("Pressed Sin-Button"),
			0, Qt::Key_S, pbSin, SLOT(animateClick()));
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
	accel()->insert("Apply Cosine", i18n("Pressed Cos-Button"),
			0, Qt::Key_C, pbCos, SLOT(animateClick()));
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
	accel()->insert("Apply Tangent", i18n("Pressed Tan-Button"),
			0, Qt::Key_T, pbTan, SLOT(animateClick()));
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
	accel()->insert("Apply Logarithm", i18n("Pressed Log-Button"),
			0, Qt::Key_L, pbLog, SLOT(animateClick()));
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
	accel()->insert("Apply Natural Log", i18n("Pressed Ln-Button"),
			0, Qt::Key_N, pbLn, SLOT(animateClick()));
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
	accel()->insert("Entered 0", i18n("Pressed 0-Button"),
			0, Qt::Key_0, tmp_pb, SLOT(animateClick()));

	tmp_pb = new QPushButton("1", mLargePage, "1-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 1);
	accel()->insert("Entered 1", i18n("Pressed 1-Button"),
			0, Qt::Key_1, tmp_pb, SLOT(animateClick()));

	tmp_pb = new QPushButton("2", mLargePage, "2-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 2);
	accel()->insert("Entered 2", i18n("Pressed 2-Button"),
			0, Qt::Key_2, tmp_pb, SLOT(animateClick()));


	tmp_pb = new QPushButton("3", mLargePage, "3-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 3);
	accel()->insert("Entered 3", i18n("Pressed 3-Button"),
			0, Qt::Key_3, tmp_pb, SLOT(animateClick()));

	tmp_pb = new QPushButton("4", mLargePage, "4-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 4);
	accel()->insert("Entered 4", i18n("Pressed 4-Button"),
			0, Qt::Key_4, tmp_pb, SLOT(animateClick()));

	tmp_pb = new QPushButton("5", mLargePage, "5-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 5);
	accel()->insert("Entered 5", i18n("Pressed 5-Button"),
			0, Qt::Key_5, tmp_pb, SLOT(animateClick()));

	tmp_pb = new QPushButton("6", mLargePage, "6-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 6);
	accel()->insert("Entered 6", i18n("Pressed 6-Button"),
			0, Qt::Key_6, tmp_pb, SLOT(animateClick()));

	tmp_pb = new QPushButton("7", mLargePage, "7-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 7);
	accel()->insert("Entered 7", i18n("Pressed 7-Button"),
			0, Qt::Key_7, tmp_pb, SLOT(animateClick()));

	tmp_pb = new QPushButton("8", mLargePage, "8-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 8);
	accel()->insert("Entered 8", i18n("Pressed 8-Button"),
			0, Qt::Key_8, tmp_pb, SLOT(animateClick()));

	tmp_pb = new QPushButton("9", mLargePage, "9-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 9);
	accel()->insert("Entered 9", i18n("Pressed 9-Button"),
			0, Qt::Key_9, tmp_pb, SLOT(animateClick()));

	tmp_pb = new QPushButton("A", mSmallPage, "A-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 0xA);
	accel()->insert("Entered A", i18n("Pressed A-Button"),
			0, Qt::Key_A, tmp_pb, SLOT(animateClick()));

	tmp_pb = new QPushButton("B", mSmallPage, "B-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 0xB);
	accel()->insert("Entered B", i18n("Pressed B-Button"),
			0, Qt::Key_B, tmp_pb, SLOT(animateClick()));

	tmp_pb = new QPushButton("C", mSmallPage, "C-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 0xC);
	accel()->insert("Entered C", i18n("Pressed C-Button"),
			0, Qt::Key_C, tmp_pb, SLOT(animateClick()));

	tmp_pb = new QPushButton("D", mSmallPage, "D-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 0xD);
	accel()->insert("Entered D", i18n("Pressed D-Button"),
			0, Qt::Key_D, tmp_pb, SLOT(animateClick()));

	tmp_pb = new QPushButton("E", mSmallPage, "E-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 0xE);
	accel()->insert("Entered E", i18n("Pressed E-Button"),
			0, Qt::Key_E, tmp_pb, SLOT(animateClick()));

	tmp_pb = new QPushButton("F", mSmallPage, "F-Button");
	tmp_pb->setAutoDefault(false);
        NumButtonGroup->insert(tmp_pb, 0xF);
	accel()->insert("Entered F", i18n("Pressed F-Button"),
			0, Qt::Key_F, tmp_pb, SLOT(animateClick()));

	pbPlus = new QPushButton("+", mLargePage, "Plus-Button");
	QToolTip::add(pbPlus, i18n("Addition"));
	pbPlus->setAutoDefault(false);
	accel()->insert("Pressed '+'", i18n("Pressed Plus-Button"),
			0, Qt::Key_Plus, pbPlus, SLOT(animateClick()));
	connect(pbPlus, SIGNAL(clicked(void)), SLOT(slotPlusclicked(void)));

	pbMinus = new QPushButton("-", mLargePage, "Minus-Button");
	QToolTip::add(pbMinus, i18n("Subtraction"));
	pbMinus->setAutoDefault(false);
	accel()->insert("Pressed '-'", i18n("Pressed Minus-Button"),
			0, Qt::Key_Minus, pbMinus, SLOT(animateClick()));
	connect(pbMinus, SIGNAL(clicked(void)), SLOT(slotMinusclicked(void)));

	pbShift = new QPushButton("Lsh", mLargePage, "Bitshift-Button");
	QToolTip::add(pbShift, i18n("Bit shift"));
	pbShift->setAutoDefault(false);
	connect(pbShift, SIGNAL(clicked(void)), SLOT(slotShiftclicked(void)));

	pbPeriod = new QPushButton(KGlobal::locale()->decimalSymbol(), mLargePage, "Period-Button");
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
	QGridLayout *smallBtnLayout = new QGridLayout(mSmallPage, 6, 2, 0,
		mInternalSpacing);
	QGridLayout *largeBtnLayout = new QGridLayout(mLargePage, 5, 6, 0,
		mInternalSpacing);

	QHBoxLayout *topLayout		= new QHBoxLayout();
	QHBoxLayout *radioLayout	= new QHBoxLayout();
	QHBoxLayout *btnLayout		= new QHBoxLayout();

	// bring them all together
	QVBoxLayout *mainLayout = new QVBoxLayout(central, mInternalSpacing,
		mInternalSpacing);

	mainLayout->addLayout(topLayout );
	mainLayout->addLayout(radioLayout, 1);
	mainLayout->addLayout(btnLayout);

	// button layout
	btnLayout->addWidget(mSmallPage,0,AlignTop);
	btnLayout->addSpacing(mInternalSpacing);
	btnLayout->addWidget(mLargePage,0,AlignTop);

	// small button layout
	smallBtnLayout->addWidget(pbPi, 0, 0);
	smallBtnLayout->addWidget(NumButtonGroup->find(0xA), 0, 1);

	smallBtnLayout->addWidget(pbPlusMinus, 1, 0);
	smallBtnLayout->addWidget(NumButtonGroup->find(0xB), 1, 1);

	smallBtnLayout->addWidget(pbReci, 2, 0);
	smallBtnLayout->addWidget(NumButtonGroup->find(0xC), 2, 1);

	smallBtnLayout->addWidget(pbFactorial, 3, 0);
	smallBtnLayout->addWidget(NumButtonGroup->find(0xD), 3, 1);

	smallBtnLayout->addWidget(pbSquare, 4, 0);
	smallBtnLayout->addWidget(NumButtonGroup->find(0xE), 4, 1);

	smallBtnLayout->addWidget(pbPower, 5, 0);
	smallBtnLayout->addWidget(NumButtonGroup->find(0xF), 5, 1);

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
	largeBtnLayout->addWidget(pbMod, 4, 4);
	largeBtnLayout->addWidget(pbNegate, 4, 5);


	largeBtnLayout->addColSpacing(0,10);
	largeBtnLayout->addColSpacing(1,10);
	largeBtnLayout->addColSpacing(2,10);
	largeBtnLayout->addColSpacing(3,10);
	largeBtnLayout->addColSpacing(4,10);


	// top layout
	topLayout->addWidget(calc_display, 10);

	// radiobutton layout
	radioLayout->addWidget(base_group);
	radioLayout->addWidget(angle_group);
	radioLayout->addWidget(pbInv);

	mFunctionButtonList.append(pbHyp);
	mFunctionButtonList.append(pbInv);
	mFunctionButtonList.append(pbPi);
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
	// Show the result in the app's caption in taskbar (wishlist - bug #52858)
	if (KCalcSettings::captionResult() == true)
		connect(calc_display,
			SIGNAL(changedText(const QString &)),
			SLOT(setCaption(const QString &)));
	calc_display->changeSettings();
	set_precision();

	basebutton[1]->animateClick();
	anglebutton[0]->animateClick();

	updateGeometry();
	setFixedSize(minimumSize());

	UpdateDisplay(true);
}

KCalculator::~KCalculator()
{
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
	connect(actionStatshow, SIGNAL(toggled(bool)), this,
		SLOT(slotStatshow(bool)));

	actionExpLogshow = new KToggleAction(i18n("&Exp/Log-Buttons"), 0,
					     actionCollection(),
					     "show_explog");
	connect(actionExpLogshow, SIGNAL(toggled(bool)),
		this, SLOT(slotExpLogshow(bool)));
	
	actionTrigshow = new KToggleAction(i18n("&Trigonometric Buttons"), 0,
					   actionCollection(), "show_trig");
	connect(actionTrigshow, SIGNAL(toggled(bool)),
		this, SLOT(slotTrigshow(bool)));

	actionLogicshow = new KToggleAction(i18n("&Logic Buttons"), 0,
					    actionCollection(), "show_logic");
	connect(actionLogicshow, SIGNAL(toggled(bool)),
		this, SLOT(slotLogicshow(bool)));


	(void) new KAction(i18n("&Show All"), 0, this, SLOT(slotShowAll()),
			   actionCollection(), "show_all");

	(void) new KAction(i18n("&Hide All"), 0, this, SLOT(slotHideAll()),
			   actionCollection(), "hide_all");

	KStdAction::preferences(this, SLOT(showSettings()), actionCollection());
}


void KCalculator::updateGeometry(void)
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

}

void KCalculator::slotBaseSelected(int base)
{
	// set_display
	int current_base = calc_display->set_base(base);

	// Enable the buttons not available in this base
	for (int i=0; i<current_base; i++)
	  NumButtonGroup->find(i)->setEnabled (true);

	// Disable the buttons not available in this base
	for (int i=current_base; i<16; i++)
	  NumButtonGroup->find(i)->setEnabled (false);

	// Only enable the decimal point in decimal
	pbPeriod->setEnabled(current_base == NB_DECIMAL);
}

void KCalculator::keyPressEvent(QKeyEvent *e)
{
    if ( ( e->state() & KeyButtonMask ) == 0 || ( e->state() & ShiftButton ) ) {
	switch (e->key())
	{
	case Key_F2:
		showSettings();
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
	case Key_E:
	  //if (current_base == NB_HEX)
	  //		(NumButtonGroup->find(0xE))->animateClick();
			//else
			pbEE->animateClick();
		break;
	case Key_Escape:
		pbClear->animateClick();
		break;
	case Key_Delete:
		pbAC->animateClick();
		break;
	case Key_Backslash:
		pbPlusMinus->animateClick();
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
	case Key_Exclam:
		pbFactorial->animateClick();
		break;
	case Key_D:
	  //if(kcalcdefaults.style == 0)
	  //	(NumButtonGroup->find(0xD))->animateClick(); // trig mode
	  //	else
			pbStatDataInput->animateClick(); // stat mode
		break;
	case Key_Less:
		pbShift->animateClick();
		break;
	case Key_AsciiCircum:
		pbPower->animateClick();
		break;
	case Key_Period:
	case Key_Comma:
		pbPeriod->animateClick();
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
		calc_display->deleteLastDigit();
		// pbAC->animateClick();
		break;
	case Key_R:
		pbReci->animateClick();
		break;
	}
    }
}



void KCalculator::slotAngleSelected(int number)
{
	switch(number)
	{
	case 0:
		core.SetAngleMode(ANG_DEGREE);
		break;
	case 1:
		core.SetAngleMode(ANG_RADIAN);
		break;
	case 2:
		core.SetAngleMode(ANG_GRADIENT);
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

	if (inverse)	statusBar()->changeItem("INV", 0);
	else		statusBar()->changeItem("NORM", 0);
}

void KCalculator::slotHyptoggled(bool flag)
{
	// toggle between hyperbolic and standart trig functions
	hyp_mode = flag;

	if(flag)
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
		// sinh or arcsinh
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
		// cosh or arccosh
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
		// tanh or arctanh
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

	// What behaviour, if e.g.: "12(6*6)"??
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
	if (inverse)
		core.Xor(calc_display->getAmount());
	else
		core.Or(calc_display->getAmount());

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

void KCalculator::slotShiftclicked(void)
{
	if (inverse)
		core.ShiftRight(calc_display->getAmount());
	else
		core.ShiftLeft(calc_display->getAmount());

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

void KCalculator::showSettings()
{
	// Check if there is already a dialog and if so bring
	// it to the foreground.
	if(KConfigDialog::showDialog("settings"))
		return;
  
	// Create a new dialog with the same name as the above checking code.
	KConfigDialog *dialog = new KConfigDialog(this, "settings", KCalcSettings::self());
  
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

	QWidget *font = new QWidget();
	QVBoxLayout *topLayout = new QVBoxLayout(font, 0, KDialog::spacingHint());
	KFontChooser *mFontChooser = 
		new KFontChooser(font, "kcfg_Font", false, QStringList(), false, 6);
	QFont tmpFont(KGlobalSettings::generalFont().family() ,14 ,QFont::Bold);
	mFontChooser->setFont(tmpFont);
	topLayout->addWidget(mFontChooser);
	dialog->addPage(font, i18n("Font"), "fonts", i18n("Select Display Font"));
 
	Colors *color = new Colors(0, "Color");

	dialog->addPage(color, i18n("Colors"), "colors", i18n("Button & Display Colors"));
 
	// When the user clicks OK or Apply we want to update our settings.
	connect(dialog, SIGNAL(settingsChanged()), this, SLOT(updateSettings()));
	
	// Display the dialog.
	dialog->show();
}

void KCalculator::slotStatshow(bool toggled)
{
	if(toggled)
	{
		pbStatNum->show();
		pbStatMean->show();
		pbStatStdDev->show();
		pbStatMedian->show();
		pbStatDataInput->show();
		pbStatClearData->show();
	}
	else
	{
		pbStatNum->hide();
		pbStatMean->hide();
		pbStatStdDev->hide();
		pbStatMedian->hide();
		pbStatDataInput->hide();
		pbStatClearData->hide();
	}

}

void KCalculator::slotTrigshow(bool toggled)
{
	if(toggled)
	{
	        pbHyp->show();
		pbSin->show();
		pbCos->show();
		pbTan->show();
		angle_group->show();
	}
	else
	{
	        pbHyp->hide();
		pbSin->hide();
		pbCos->hide();
		pbTan->hide();
		angle_group->hide();
	}
}

void KCalculator::slotExpLogshow(bool toggled)
{
	if(toggled)
	{
		pbLog->show();
		pbLn->show();
	}
	else
	{
		pbLog->hide();
		pbLn ->hide();
	}
}

void KCalculator::slotLogicshow(bool toggled)
{
	if(toggled)
	{
	        pbAND->show();
		pbOR->show();
		pbNegate->show();
		pbShift->show();
		base_group->show();
		for (int i=10; i<16; i++)
			(NumButtonGroup->find(i))->show();
	}
	else
	{
	        pbAND->hide();
		pbOR->hide();
		pbNegate->hide();
		pbShift->hide();
		// Hide Hex-Buttons, but first switch back to decimal
		(base_group->find(1))->animateClick();
		base_group->hide();
		for (int i=10; i<16; i++)
			(NumButtonGroup->find(i))->hide();
	}
}

void KCalculator::slotShowAll(void)
{
	// I wonder why "setChecked" does not emit "toggled"
	if(!actionStatshow->isChecked()) actionStatshow->activate();
	if(!actionTrigshow->isChecked()) actionTrigshow->activate();
	if(!actionExpLogshow->isChecked()) actionExpLogshow->activate();
	if(!actionLogicshow->isChecked()) actionLogicshow->activate();
}

void KCalculator::slotHideAll(void)
{
	// I wonder why "setChecked" does not emit "toggled"
	if(actionStatshow->isChecked()) actionStatshow->activate();
	if(actionTrigshow->isChecked()) actionTrigshow->activate();
	if(actionExpLogshow->isChecked()) actionExpLogshow->activate();
	if(actionLogicshow->isChecked()) actionLogicshow->activate();
}

void KCalculator::RefreshCalculator()
{
	pbInv->setOn(false);
	calc_display->Reset();
}

void KCalculator::updateSettings()
{
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
	for(p = mStatButtonList.first(); p;
		p=mStatButtonList.next())
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
		"(c) 1996-2000, Bernd Johannes Wuebben\n"
		"(c) 2000-2003, The KDE Team",
                precisionStatement.latin1());

	aboutData.addAuthor("Bernd Johannes Wuebben", 0, "wuebben@kde.org");
	aboutData.addAuthor("Evan Teran", 0, "emt3734@rit.edu");
	aboutData.addAuthor("Espen Sand", 0, "espen@kde.org");
	aboutData.addAuthor("Chris Howells", 0, "howells@kde.org");
        aboutData.addAuthor("Aaron J. Seigo", 0, "aseigo@olympusproject.org");
        aboutData.addAuthor("Charles Samuels", 0, "charles@altair.dhs.org");
        aboutData.addAuthor("Klaus Niederkrüger", 0, "kniederk@math.uni-koeln.de");
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

