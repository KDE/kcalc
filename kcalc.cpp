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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

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
#include <qgroupbox.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qstyle.h>

#include <kapplication.h>
#include <kcolordrag.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <knotifyclient.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kpushbutton.h>
#include <kpopupmenu.h>

#include "dlabel.h"
#include "kcalc.h"
#include "optiondialog.h"
#include "version.h"

// Initalize static members
const CALCAMNT QtCalculator::pi = (ASIN(1L) * 2L);


static const char *description = I18N_NOOP("KDE Calculator");
static const char *version = KCALCVERSION;

extern item_contents	display_data;
extern bool				display_error;

//
// * ported to QLayout (mosfet 10/28/99)
//
// * 1999-10-31 Espen Sand: More modifications.
//   All fixed sizes removed.
//   New config dialog.
//   To exit: Key_Q+ALT => Key_Q+CTRL,  Key_X+ALT => Key_X+CTRL
//   Look in updateGeometry() for size settings.
//


//-------------------------------------------------------------------------
// Name: QtCalculator(QWidget *parent, const char *name)
//-------------------------------------------------------------------------
QtCalculator::QtCalculator(QWidget *parent, const char *name)
	: KDialog(parent, name), inverse(false), hyp_mode(false), eestate(false), 
	refresh_display(false), display_size(DEC_SIZE),  angle_mode(ANG_DEGREE), 
	input_limit(0), input_count(0), decimal_point(0), precedence_base(0),
	current_base(NB_DECIMAL), memory_num(0.0), last_input(DIGIT), 
	history_index(0), selection_timer(new QTimer), key_pressed(false),
	mInternalSpacing(4), status_timer(new QTimer), mConfigureDialog(0)
{
	// make sure the display_str is NULL terminated so we can
	// user library string functions
	display_str[0] = '\0';

	connect(status_timer, SIGNAL(timeout()),
		this, SLOT(clear_status_label()));

	connect(selection_timer, SIGNAL(timeout()),
		this, SLOT(selection_timed_out()));

	readSettings();

	// Detect color change
	connect(kapp,SIGNAL(kdisplayPaletteChanged()), this, SLOT(set_colors()));

	// Accelerators to exit the program
	QAccel *accel = new QAccel(this);
	accel->connectItem(accel->insertItem(Key_Q+CTRL), this, SLOT(quitCalc()));
	accel->connectItem(accel->insertItem(Key_X+CTRL), this, SLOT(quitCalc()));

	// Create uppermost bar with buttons and numberdisplay
	mConfigButton = new KPushButton(KGuiItem( i18n("Config&ure"), "configure" ),
            this, "configbutton");
	mConfigButton->setAutoDefault(false);
	QToolTip::add(mConfigButton, i18n("Click to configure KCalc"));
	connect(mConfigButton, SIGNAL(clicked()), this, SLOT(configclicked()));

	mHelpMenu = new KHelpMenu(this, KGlobal::instance()->aboutData());

	mHelpButton = new KPushButton(KStdGuiItem::help(), this);
	mHelpButton->setAutoDefault(false);
	mHelpButton->setPopup(mHelpMenu->menu());

	calc_display = new DLabel(this, "display");
	calc_display->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
	calc_display->setAlignment(AlignRight | AlignVCenter);
	calc_display->setFocus();
	calc_display->setFocusPolicy(QWidget::StrongFocus);

	connect(calc_display, SIGNAL(clicked()), this, SLOT(display_selected()));

	// Status bar contents
	statusINVLabel = new QLabel( this, "INV" );
	Q_CHECK_PTR(statusINVLabel);
	statusINVLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	statusINVLabel->setAlignment(AlignCenter);
	statusINVLabel->setText("NORM");

	statusHYPLabel = new QLabel(this, "HYP");
	Q_CHECK_PTR(statusHYPLabel);
	statusHYPLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	statusHYPLabel->setAlignment(AlignCenter);

	statusERRORLabel = new QLabel(this, "ERROR");
	Q_CHECK_PTR(statusERRORLabel);
	statusERRORLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	statusERRORLabel->setAlignment(AlignLeft|AlignVCenter);

	// Create Number Base Button Group
	QButtonGroup *base_group = new QButtonGroup(4, Horizontal,  this, "base");
	base_group->setTitle(i18n("Base"));
	connect(base_group, SIGNAL(clicked(int)), SLOT(Base_Selected(int)));

	basebutton[0] = new QRadioButton(base_group);
	basebutton[0]->setText("&Hex");
	QToolTip::add(basebutton[0], i18n("Hexadecimal"));
	accel->connectItem(accel->insertItem(Key_H+ALT),
		this, SLOT(Hex_Selected()));

	basebutton[1] = new QRadioButton(base_group);
	basebutton[1]->setText("D&ec");
	QToolTip::add(basebutton[1], i18n("Decimal"));
	basebutton[1]->setChecked(true);
	accel->connectItem(accel->insertItem(Key_E+ALT),
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
	angle_group = new QButtonGroup(3, Horizontal, this, "angle");
	angle_group->setTitle(i18n( "Angle") );
	connect( angle_group, SIGNAL(clicked(int)), SLOT(angle_selected(int)));

	anglebutton[0] = new QRadioButton(angle_group);
	anglebutton[0]->setText("&Deg");
	QToolTip::add(anglebutton[0], i18n("Degrees"));
	anglebutton[0]->setChecked(true);
	accel->connectItem(accel->insertItem(Key_D+ALT),
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
	mSmallPage = new QWidget(this);
	mLargePage = new QWidget(this);

	pbhyp = new QPushButton("Hyp", mSmallPage, "hypbutton");
	pbhyp->setAutoDefault(false);
	connect(pbhyp, SIGNAL(toggled(bool)), SLOT(pbhyptoggled(bool)));
	pbhyp->setToggleButton(true);

	pbinv = new QPushButton("Inv", mSmallPage, "InverseButton");
	QToolTip::add(pbinv, i18n("Inverse mode"));
	pbinv->setAutoDefault(false);
	connect(pbinv, SIGNAL(toggled(bool)), SLOT(pbinvtoggled(bool)));
	pbinv->setToggleButton(true);

	pbA = new QPushButton("A", mSmallPage, "Abutton");
	pbA->setAutoDefault(false);
	connect(pbA, SIGNAL(toggled(bool)), SLOT(pbAtoggled(bool)));
	pbA->setToggleButton(true);

	pbSin = new QPushButton("Sin", mSmallPage, "Sinbutton");
	pbSin->setAutoDefault(false);
	connect(pbSin, SIGNAL(toggled(bool)), SLOT(pbSintoggled(bool)));
	pbSin->setToggleButton(true);

	pbplusminus = new QPushButton("+/-", mSmallPage, "plusminusbutton");
	QToolTip::add(pbplusminus, i18n("Change sign"));
	pbplusminus->setAutoDefault(false);
	connect(pbplusminus, SIGNAL(toggled(bool)), SLOT(pbplusminustoggled(bool)));
	pbplusminus->setToggleButton(true);

	pbB = new QPushButton("B", mSmallPage, "Bbutton");
	pbB->setAutoDefault(false);
	connect(pbB, SIGNAL(toggled(bool)), SLOT(pbBtoggled(bool)));
	pbB->setToggleButton(true);

	pbCos = new QPushButton("Cos", mSmallPage, "Cosbutton");
	pbCos->setAutoDefault(false);
	connect(pbCos, SIGNAL(toggled(bool)), SLOT(pbCostoggled(bool)));
	pbCos->setToggleButton(true);

	pbreci = new QPushButton("1/x", mSmallPage, "recibutton");
	QToolTip::add(pbreci, i18n("Reciprocal"));
	pbreci->setAutoDefault(false);
	connect(pbreci, SIGNAL(toggled(bool)), SLOT(pbrecitoggled(bool)));
	pbreci->setToggleButton(true);

	pbC = new QPushButton("C", mSmallPage, "Cbutton");
	pbC->setAutoDefault(false);
	connect(pbC, SIGNAL(toggled(bool)), SLOT(pbCtoggled(bool)));
	pbC->setToggleButton(true);

	pbTan = new QPushButton("Tan", mSmallPage, "Tanbutton");
	pbTan->setAutoDefault(false);
	connect(pbTan, SIGNAL(toggled(bool)),SLOT(pbTantoggled(bool)));
	pbTan->setToggleButton(true);

	pbfactorial = new QPushButton("x!", mSmallPage, "factorialbutton");
	QToolTip::add(pbfactorial, i18n("Factorial"));
	pbfactorial->setAutoDefault(false);
	connect(pbfactorial, SIGNAL(toggled(bool)),SLOT(pbfactorialtoggled(bool)));
	pbfactorial->setToggleButton(true);

	pbD = new QPushButton("D", mSmallPage, "Dbutton");
	pbD->setAutoDefault(false);
	connect(pbD, SIGNAL(toggled(bool)), SLOT(pbDtoggled(bool)));
	pbD->setToggleButton(true);

	pblog = new QPushButton("Log", mSmallPage, "logbutton");
	pblog->setAutoDefault(false);
	connect(pblog, SIGNAL(toggled(bool)), SLOT(pblogtoggled(bool)));
	pblog->setToggleButton(true);

	pbsquare = new QPushButton("x^2", mSmallPage, "squarebutton");
	QToolTip::add(pbsquare, i18n("Square"));
	pbsquare->setAutoDefault(false);
	connect(pbsquare, SIGNAL(toggled(bool)), SLOT(pbsquaretoggled(bool)));
	pbsquare->setToggleButton(true);

	pbE = new QPushButton("E", mSmallPage, "Ebutton");
	pbE->setAutoDefault(false);
	connect(pbE, SIGNAL(toggled(bool)), SLOT(pbEtoggled(bool)));
	pbE->setToggleButton(true);

	pbln = new QPushButton("Ln", mSmallPage, "lnbutton");
	pbln->setAutoDefault(false);
	connect(pbln, SIGNAL(toggled(bool)), SLOT(pblntoggled(bool)));
	pbln->setToggleButton(true);

	pbpower = new QPushButton("x^y", mSmallPage, "powerbutton");
	pbpower->setAutoDefault(false);
	connect(pbpower, SIGNAL(toggled(bool)), SLOT(pbpowertoggled(bool)));
	pbpower->setToggleButton(true);

	pbF = new QPushButton("F", mSmallPage, "Fbutton");
	pbF->setAutoDefault(false);
	connect(pbF, SIGNAL(toggled(bool)), SLOT(pbFtoggled(bool)));
	pbF->setToggleButton(true);

	pbEE = new QPushButton("EE", mLargePage, "EEbutton");
	pbEE->setToggleButton(true);
	pbEE->setAutoDefault(false);
	connect(pbEE, SIGNAL(toggled(bool)), SLOT(EEtoggled(bool)));

	pbMR = new QPushButton("MR", mLargePage, "MRbutton");
	QToolTip::add(pbMR, i18n("Memory recall"));
	pbMR->setAutoDefault(false);
	connect(pbMR, SIGNAL(toggled(bool)), SLOT(pbMRtoggled(bool)));
	pbMR->setToggleButton(true);

	pbMplusminus = new QPushButton("M+-", mLargePage, "Mplusminusbutton");
	pbMplusminus->setAutoDefault(false);
	connect(pbMplusminus,SIGNAL(toggled(bool)),SLOT(pbMplusminustoggled(bool)));
	pbMplusminus->setToggleButton(true);

	pbMC = new QPushButton("MC", mLargePage, "MCbutton");
	QToolTip::add(pbMC, i18n("Clear memory"));
	pbMC->setAutoDefault(false);
	connect(pbMC, SIGNAL(toggled(bool)), SLOT(pbMCtoggled(bool)));
	pbMC->setToggleButton(true);

	pbClear = new QPushButton("C", mLargePage, "Clearbutton");
	QToolTip::add(pbClear, i18n("Clear"));
	pbClear->setAutoDefault(false);
	connect(pbClear, SIGNAL(toggled(bool)), SLOT(pbCleartoggled(bool)));
	pbClear->setToggleButton(true);

	pbAC = new QPushButton("AC", mLargePage, "ACbutton");
	QToolTip::add(pbAC, i18n("Clear all"));
	pbAC->setAutoDefault(false);
	connect(pbAC, SIGNAL(toggled(bool)), SLOT(pbACtoggled(bool)));
	pbAC->setToggleButton(true);

	pb7 = new QPushButton("7", mLargePage, "7button");
	pb7->setAutoDefault(false);
	connect(pb7, SIGNAL(toggled(bool)), SLOT(pb7toggled(bool)));
	pb7->setToggleButton(true);

	pb8 = new QPushButton("8", mLargePage, "8button");
	pb8->setAutoDefault(false);
	connect(pb8, SIGNAL(toggled(bool)), SLOT(pb8toggled(bool)));
	pb8->setToggleButton(true);

	pb9 = new QPushButton("9", mLargePage, "9button");
	pb9->setAutoDefault(false);
	connect(pb9, SIGNAL(toggled(bool)), SLOT(pb9toggled(bool)));
	pb9->setToggleButton(true);

	pbparenopen = new QPushButton("(", mLargePage, "parenopenbutton");
	pbparenopen->setAutoDefault(false);
	connect(pbparenopen, SIGNAL(toggled(bool)),SLOT(pbparenopentoggled(bool)));
	pbparenopen->setToggleButton(true);

	pbparenclose = new QPushButton(")", mLargePage, "parenclosebutton");
	pbparenclose->setAutoDefault(false);
	connect(pbparenclose,SIGNAL(toggled(bool)),SLOT(pbparenclosetoggled(bool)));
	pbparenclose->setToggleButton(true);

	pband = new QPushButton("And", mLargePage, "andbutton");
	QToolTip::add(pband, i18n("Bitwise AND"));
	pband->setAutoDefault(false);
	connect(pband, SIGNAL(toggled(bool)), SLOT(pbandtoggled(bool)));
	pband->setToggleButton(true);

	pb4 = new QPushButton("4", mLargePage, "4button");
	pb4->setAutoDefault(false);
	connect(pb4, SIGNAL(toggled(bool)), SLOT(pb4toggled(bool)));
	pb4->setToggleButton(true);

	pb5 = new QPushButton("5", mLargePage, "5button");
	pb5->setAutoDefault(false);
	connect(pb5, SIGNAL(toggled(bool)), SLOT(pb5toggled(bool)));
	pb5->setToggleButton(true);

	pb6 = new QPushButton("6", mLargePage, "6button");
	pb6->setAutoDefault(false);
	connect(pb6, SIGNAL(toggled(bool)), SLOT(pb6toggled(bool)));
	pb6->setToggleButton(true);

	pbX = new QPushButton("X", mLargePage, "Multiplybutton");
	QToolTip::add(pbX, i18n("Multiplication"));
	pbX->setAutoDefault(false);
	connect(pbX, SIGNAL(toggled(bool)), SLOT(pbXtoggled(bool)));
	pbX->setToggleButton(true);

	pbdivision = new QPushButton("/", mLargePage, "divisionbutton");
	QToolTip::add(pbdivision, i18n("Division"));
	pbdivision->setAutoDefault(false);
	connect(pbdivision, SIGNAL(toggled(bool)), SLOT(pbdivisiontoggled(bool)));
	pbdivision->setToggleButton(true);

	pbor = new QPushButton("Or", mLargePage, "orbutton");
	QToolTip::add(pbor, i18n("Bitwise OR"));
	pbor->setAutoDefault(false);
	connect(pbor, SIGNAL(toggled(bool)), SLOT(pbortoggled(bool)));
	pbor->setToggleButton(true);

	pb1 = new QPushButton("1", mLargePage, "1button");
	pb1->setAutoDefault(false);
	connect(pb1, SIGNAL(toggled(bool)), SLOT(pb1toggled(bool)));
	pb1->setToggleButton(true);

	pb2 = new QPushButton("2", mLargePage, "2button");
	pb2->setAutoDefault(false);
	connect(pb2, SIGNAL(toggled(bool)), SLOT(pb2toggled(bool)));
	pb2->setToggleButton(true);

	pb3 = new QPushButton("3", mLargePage, "3button");
	pb3->setAutoDefault(false);
	connect(pb3, SIGNAL(toggled(bool)), SLOT(pb3toggled(bool)));
	pb3->setToggleButton(true);

	pbplus = new QPushButton("+", mLargePage, "plusbutton");
	pbplus->setAutoDefault(false);
	connect(pbplus, SIGNAL(toggled(bool)), SLOT(pbplustoggled(bool)));
	pbplus->setToggleButton(true);

	pbminus = new QPushButton("-", mLargePage, "minusbutton");
	pbminus->setAutoDefault(false);
	connect(pbminus, SIGNAL(toggled(bool)), SLOT(pbminustoggled(bool)));
	pbminus->setToggleButton(true);

	pbshift = new QPushButton("Lsh", mLargePage, "shiftbutton");
	QToolTip::add(pbshift, i18n("Bit shift"));
	pbshift->setAutoDefault(false);
	connect(pbshift, SIGNAL(toggled(bool)), SLOT(pbshifttoggled(bool)));
	pbshift->setToggleButton(true);

	pbperiod = new QPushButton(".", mLargePage, "periodbutton");
	QToolTip::add(pbperiod, i18n("Decimal point"));
	pbperiod->setAutoDefault(false);
	connect(pbperiod, SIGNAL(toggled(bool)), SLOT(pbperiodtoggled(bool)));
	pbperiod->setToggleButton(true);

	pb0 = new QPushButton("0", mLargePage, "0button");
	pb0->setAutoDefault(false);
	connect(pb0, SIGNAL(toggled(bool)), SLOT(pb0toggled(bool)));
	pb0->setToggleButton(true);

	pbequal = new QPushButton("=", mLargePage, "equalbutton");
	pbequal->setAutoDefault(false);
	connect(pbequal, SIGNAL(toggled(bool)), SLOT(pbequaltoggled(bool)));
	pbequal->setToggleButton(true);

	pbpercent = new QPushButton("%", mLargePage, "percentbutton");
	QToolTip::add(pbpercent, i18n("Percent"));
	pbpercent->setAutoDefault(false);
	connect(pbpercent, SIGNAL(toggled(bool)), SLOT(pbpercenttoggled(bool)));
	pbpercent->setToggleButton(true);

	pbnegate = new QPushButton("Cmp", mLargePage, "OneComplementbutton");
	QToolTip::add(pbnegate, i18n("One's complement"));
	pbnegate->setAutoDefault(false);
	connect(pbnegate, SIGNAL(toggled(bool)), SLOT(pbnegatetoggled(bool)));
	pbnegate->setToggleButton(true);

	pbmod = new QPushButton("Mod", mLargePage, "modbutton");
	QToolTip::add(pbmod, i18n("Modulo"));
	pbmod->setAutoDefault(false);
	connect(pbmod, SIGNAL(toggled(bool)), SLOT(pbmodtoggled(bool)));
	pbmod->setToggleButton(true);

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
	QVBoxLayout *mainLayout = new QVBoxLayout(this, mInternalSpacing,
		mInternalSpacing );

	mainLayout->addLayout(topLayout );
	mainLayout->addLayout(radioLayout, 1);
	mainLayout->addLayout(btnLayout);
	mainLayout->addLayout(statusLayout);

	// button layout
	btnLayout->addWidget(mSmallPage,0,AlignTop);
	btnLayout->addSpacing(mInternalSpacing);
	btnLayout->addWidget(mLargePage,0,AlignTop);

	// small button layout
	smallBtnLayout->addWidget(pbhyp, 0, 0);
	smallBtnLayout->addWidget(pbinv, 0, 1);
	smallBtnLayout->addWidget(pbA, 0, 2);

	smallBtnLayout->addWidget(pbSin, 1, 0);
	smallBtnLayout->addWidget(pbplusminus, 1, 1);
	smallBtnLayout->addWidget(pbB, 1, 2);

	smallBtnLayout->addWidget(pbCos, 2, 0);
	smallBtnLayout->addWidget(pbreci, 2, 1);
	smallBtnLayout->addWidget(pbC, 2, 2);

	smallBtnLayout->addWidget(pbTan, 3, 0);
	smallBtnLayout->addWidget(pbfactorial, 3, 1);
	smallBtnLayout->addWidget(pbD, 3, 2);

	smallBtnLayout->addWidget(pblog, 4, 0);
	smallBtnLayout->addWidget(pbsquare, 4, 1);
	smallBtnLayout->addWidget(pbE, 4, 2);

	smallBtnLayout->addWidget(pbln, 5, 0);
	smallBtnLayout->addWidget(pbpower, 5, 1);
	smallBtnLayout->addWidget(pbF, 5, 2);

	smallBtnLayout->setRowStretch(0, 0);
	smallBtnLayout->setRowStretch(1, 0);
	smallBtnLayout->setRowStretch(2, 0);
	smallBtnLayout->setRowStretch(3, 0);
	smallBtnLayout->setRowStretch(4, 0);
	smallBtnLayout->setRowStretch(5, 0);

	// large button layout
	largeBtnLayout->addWidget(pbEE, 0, 0);
	largeBtnLayout->addWidget(pbMR, 0, 1);
	largeBtnLayout->addWidget(pbMplusminus, 0, 2);
	largeBtnLayout->addWidget(pbMC, 0, 3);
	largeBtnLayout->addWidget(pbClear, 0, 4);
	largeBtnLayout->addWidget(pbAC, 0, 5);

	largeBtnLayout->addWidget(pb7, 1, 0);
	largeBtnLayout->addWidget(pb8, 1, 1);
	largeBtnLayout->addWidget(pb9, 1, 2);
	largeBtnLayout->addWidget(pbparenopen, 1, 3);
	largeBtnLayout->addWidget(pbparenclose, 1, 4);
	largeBtnLayout->addWidget(pband, 1, 5);

	largeBtnLayout->addWidget(pb4, 2, 0);
	largeBtnLayout->addWidget(pb5, 2, 1);
	largeBtnLayout->addWidget(pb6, 2, 2);
	largeBtnLayout->addWidget(pbX, 2, 3);
	largeBtnLayout->addWidget(pbdivision, 2, 4);
	largeBtnLayout->addWidget(pbor, 2, 5);

	largeBtnLayout->addWidget(pb1, 3, 0);
	largeBtnLayout->addWidget(pb2, 3, 1);
	largeBtnLayout->addWidget(pb3, 3, 2);
	largeBtnLayout->addWidget(pbplus, 3, 3);
	largeBtnLayout->addWidget(pbminus, 3, 4);
	largeBtnLayout->addWidget(pbshift, 3, 5);

	largeBtnLayout->addWidget(pbperiod, 4, 0);
	largeBtnLayout->addWidget(pb0, 4, 1);
	largeBtnLayout->addWidget(pbequal, 4, 2);
	largeBtnLayout->addWidget(pbpercent, 4, 3);
	largeBtnLayout->addWidget(pbnegate, 4, 4);
	largeBtnLayout->addWidget(pbmod, 4, 5);

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

	mNumButtonList.append(pb0);
	mNumButtonList.append(pb1);
	mNumButtonList.append(pb2);
	mNumButtonList.append(pb3);
	mNumButtonList.append(pb4);
	mNumButtonList.append(pb5);
	mNumButtonList.append(pb6);
	mNumButtonList.append(pb7);
	mNumButtonList.append(pb8);
	mNumButtonList.append(pb9);

	mFunctionButtonList.append(pbhyp);
	mFunctionButtonList.append(pbinv);
	mFunctionButtonList.append(pbSin);
	mFunctionButtonList.append(pbplusminus);
	mFunctionButtonList.append(pbCos);
	mFunctionButtonList.append(pbreci);
	mFunctionButtonList.append(pbTan);
	mFunctionButtonList.append(pbfactorial);
	mFunctionButtonList.append(pblog);
	mFunctionButtonList.append(pbsquare);
	mFunctionButtonList.append(pbln);
	mFunctionButtonList.append(pbpower);

	mHexButtonList.append(pbA);
	mHexButtonList.append(pbB);
	mHexButtonList.append(pbC);
	mHexButtonList.append(pbD);
	mHexButtonList.append(pbE);
	mHexButtonList.append(pbF);

	mMemButtonList.append(pbEE);
	mMemButtonList.append(pbMR);
	mMemButtonList.append(pbMplusminus);
	mMemButtonList.append(pbMC);
	mMemButtonList.append(pbClear);
	mMemButtonList.append(pbAC);

	mOperationButtonList.append(pbX);
	mOperationButtonList.append(pbparenopen);
	mOperationButtonList.append(pbparenclose);
	mOperationButtonList.append(pband);
	mOperationButtonList.append(pbdivision);
	mOperationButtonList.append(pbor);
	mOperationButtonList.append(pbplus);
	mOperationButtonList.append(pbminus);
	mOperationButtonList.append(pbshift);
	mOperationButtonList.append(pbperiod);
	mOperationButtonList.append(pbequal);
	mOperationButtonList.append(pbpercent);
	mOperationButtonList.append(pbnegate);
	mOperationButtonList.append(pbmod);

	set_colors();
	set_display_font();
	set_precision();
	set_style();
	Base_Selected(1);
	InitializeCalculator();

	updateGeometry();
	setFixedSize(minimumSize());
}

//-------------------------------------------------------------------------
// Name: ~QtCalculator()
//-------------------------------------------------------------------------
QtCalculator::~QtCalculator()
{
	delete mConfigureDialog;
	delete selection_timer;
	delete status_timer;
}


//-------------------------------------------------------------------------
// Name: updateGeometry()
//-------------------------------------------------------------------------
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

    int h1 = pbF->minimumSize().height();
    int h2 = (int)((((float)h1 + 4.0) / 5.0));
    s.setWidth(mLargePage->fontMetrics().width("MMM") +
               QApplication::style().
               pixelMetric(QStyle::PM_ButtonMargin, pbF)*2);
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

//-------------------------------------------------------------------------
// Name: Base_Selected()
//-------------------------------------------------------------------------
void QtCalculator::Base_Selected(int base)
{
	// Enable the hexit buttons if we're in hex mode
	// (Base 0 == hexidecimal)

	for(QPushButton *p = mHexButtonList.first(); p; p=mHexButtonList.next())
	{
		p->setEnabled(base == 0);
	}

	// Enable 8 & 9 if we're in dec mode or better
	// (Base 1 == decimal)
	pb9->setEnabled(base <= 1);
	pb8->setEnabled(base <= 1);

	// Enable 2,3,4,5,6,7 if we're in oct mode or better
	// (Base 2 == decimal)
	pb7->setEnabled(base <= 2);
	pb6->setEnabled(base <= 2);
	pb5->setEnabled(base <= 2);
	pb4->setEnabled(base <= 2);
	pb3->setEnabled(base <= 2);
	pb2->setEnabled(base <= 2);

	// Only enable the decimal point in decimal
	pbperiod->setEnabled(base == 1);

	// Call down to the core
	base_selected(base);
}

//-------------------------------------------------------------------------
// Name: Hex_Selected()
//-------------------------------------------------------------------------
void QtCalculator::Hex_Selected()
{
	basebutton[0]->setChecked(true);
	basebutton[1]->setChecked(false);
	basebutton[2]->setChecked(false);
	basebutton[3]->setChecked(false);
	Base_Selected(0);
}

//-------------------------------------------------------------------------
// Name: Dec_Selected()
//-------------------------------------------------------------------------
void QtCalculator::Dec_Selected()
{
	basebutton[0]->setChecked(false);
	basebutton[1]->setChecked(true);
	basebutton[2]->setChecked(false);
	basebutton[3]->setChecked(false);
	Base_Selected(1);
}

//-------------------------------------------------------------------------
// Name: Oct_Selected()
//-------------------------------------------------------------------------
void QtCalculator::Oct_Selected()
{
	basebutton[0]->setChecked(false);
	basebutton[1]->setChecked(false);
	basebutton[2]->setChecked(true);
	basebutton[3]->setChecked(false);
	Base_Selected(2);
}

//-------------------------------------------------------------------------
// Name: Bin_Selected()
//-------------------------------------------------------------------------
void QtCalculator::Bin_Selected()
{
	basebutton[0]->setChecked(false);
	basebutton[1]->setChecked(false);
	basebutton[2]->setChecked(false);
	basebutton[3]->setChecked(true);
	Base_Selected(3);
}

//-------------------------------------------------------------------------
// Name: Deg_Selected()
//-------------------------------------------------------------------------
void QtCalculator::Deg_Selected()
{
	anglebutton[0]->setChecked(true);
	anglebutton[1]->setChecked(false);
	anglebutton[2]->setChecked(false);
	angle_selected(0);
}

//-------------------------------------------------------------------------
// Name: Rad_Selected()
//-------------------------------------------------------------------------
void QtCalculator::Rad_Selected()
{
	anglebutton[0]->setChecked(false);
	anglebutton[1]->setChecked(true);
	anglebutton[2]->setChecked(false);
	angle_selected(1);
}

//-------------------------------------------------------------------------
// Name: Gra_Selected()
//-------------------------------------------------------------------------
void QtCalculator::Gra_Selected()
{
	anglebutton[0]->setChecked(false);
	anglebutton[1]->setChecked(false);
	anglebutton[2]->setChecked(true);
	angle_selected(2);
}

//-------------------------------------------------------------------------
// Name: configurationChanged(const DefStruct &state)
//-------------------------------------------------------------------------
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

//-------------------------------------------------------------------------
// Name: keyPressEvent(QKeyEvent *e)
//-------------------------------------------------------------------------
void QtCalculator::keyPressEvent(QKeyEvent *e)
{
	switch (e->key())
	{
	case Key_F1:
		kapp->invokeHelp();
		break;
	case Key_F2:
		configclicked();
		break;
	case Key_F3:
		if(kcalcdefaults.style == 0)
			kcalcdefaults.style = 1;
		else
			kcalcdefaults.style = 0;
		set_style();
		break;
	case Key_Up:
		history_prev();
		break;
	case Key_Down:
		history_next();
		break;
	case Key_Next:
		key_pressed = true;
		pbAC->setOn(true);
		break;
	case Key_Prior:
		key_pressed = true;
		pbClear->setOn(true);
		break;
	case Key_H:
		key_pressed = true;
		pbhyp->setOn(true);
		break;
	case Key_I:
		key_pressed = true;
		pbinv->setOn(true);
		break;
	case Key_A:
		key_pressed = true;
		pbA->setOn(true);
		break;
	case Key_E:
		key_pressed = true;
		if (current_base == NB_HEX)
			pbE->setOn(true);
		else
			pbEE->setOn(true);
		break;
	case Key_Escape:
		key_pressed = true;
		pbClear->setOn(true);
		break;
	case Key_Delete:
		key_pressed = true;
		pbAC->setOn(true);
		break;
	case Key_S:
		key_pressed = true;
		pbSin->setOn(true);
		break;
	case Key_Backslash:
		key_pressed = true;
		pbplusminus->setOn(true);
		break;
	case Key_B:
		key_pressed = true;
		pbB->setOn(true);
		break;
	case Key_7:
		key_pressed = true;
		pb7->setOn(true);
		break;
	case Key_8:
		key_pressed = true;
		pb8->setOn(true);
		break;
	case Key_9:
		key_pressed = true;
		pb9->setOn(true);
		break;
	case Key_ParenLeft:
		key_pressed = true;
		pbparenopen->setOn(true);
		break;
	case Key_ParenRight:
		key_pressed = true;
		pbparenclose->setOn(true);
		break;
	case Key_Ampersand:
		key_pressed = true;
		pband->setOn(true);
		break;
	case Key_C:
		key_pressed = true;
		if (current_base == NB_HEX)
			pbC->setOn(true);
		else
			pbCos->setOn(true);
		break;
	case Key_4:
		key_pressed = true;
		pb4->setOn(true);
		break;
	case Key_5:
		key_pressed = true;
		pb5->setOn(true);
		break;
	case Key_6:
		key_pressed = true;
		pb6->setOn(true);
		break;
	case Key_Asterisk:
        case Key_multiply:
		key_pressed = true;
		pbX->setOn(true);
		break;
	case Key_Slash:
        case Key_division:
		key_pressed = true;
		pbdivision->setOn(true);
		break;
	case Key_O:
		key_pressed = true;
		pbor->setOn(true);
		break;
	case Key_T:
		key_pressed = true;
		pbTan->setOn(true);
		break;
	case Key_Exclam:
		key_pressed = true;
		pbfactorial->setOn(true);
		break;
	case Key_D:
		key_pressed = true;
		if(kcalcdefaults.style == 0)
			pbD->setOn(true); // trig mode
		else
			pblog->setOn(true); // stat mode
		break;
	case Key_1:
		key_pressed = true;
		pb1->setOn(true);
		break;
	case Key_2:
		key_pressed = true;
		pb2->setOn(true);
		break;
	case Key_3:
		key_pressed = true;
		pb3->setOn(true);
		break;
	case Key_Plus:
		key_pressed = true;
		pbplus->setOn(true);
		break;
	case Key_Minus:
		key_pressed = true;
		pbminus->setOn(true);
		break;
	case Key_Less:
		key_pressed = true;
		pbshift->setOn(true);
		break;
	case Key_N:
		key_pressed = true;
		pbln->setOn(true);
		break;
	case Key_L:
		key_pressed = true;
		pblog->setOn(true);
		break;
	case Key_AsciiCircum:
		key_pressed = true;
		pbpower->setOn(true);
		break;
	case Key_F:
		key_pressed = true;
		pbF->setOn(true);
		break;
	case Key_Period:
		key_pressed = true;
		pbperiod->setOn(true);
		break;
	case Key_Comma:
		key_pressed = true;
		pbperiod->setOn(true);
		break;
	case Key_0:
		key_pressed = true;
		pb0->setOn(true);
		break;
	case Key_Equal:
		key_pressed = true;
		pbequal->setOn(true);
		break;
	case Key_Return:
		key_pressed = true;
		pbequal->setOn(true);
		break;
	case Key_Enter:
		key_pressed = true;
		pbequal->setOn(true);
		break;
	case Key_Percent:
		key_pressed = true;
		pbpercent->setOn(true);
		break;
	case Key_AsciiTilde:
		key_pressed = true;
		pbnegate->setOn(true);
		break;
	case Key_Colon:
		key_pressed = true;
		pbmod->setOn(true);
		break;
	case Key_BracketLeft:
        case Key_twosuperior:
		key_pressed = true;
		pbsquare->setOn(true);
		break;
	case Key_Backspace:
		SubtractDigit();
		// key_pressed = true;
		// pbAC->setOn(true);
		break;
	case Key_R:
		key_pressed = true;
		pbreci->setOn(true);
		break;
	}
}

//-------------------------------------------------------------------------
// Name: keyReleaseEvent(QKeyEvent *e)
//-------------------------------------------------------------------------
void QtCalculator::keyReleaseEvent(QKeyEvent *e)
{
	bool oldKey = key_pressed;
	key_pressed = false;

	switch (e->key())
	{
	case Key_Next:
		pbAC->setOn(false);
		break;
	case Key_Prior:
		pbClear->setOn(false);
		break;
	case Key_H:
		pbhyp->setOn(false);
		break;
	case Key_I:
		pbinv->setOn(false);
		break;
	case Key_A:
		pbA->setOn(false);
		break;
	case Key_E:
		if (current_base == NB_HEX)
			pbE->setOn(false);
		else
			pbEE->setOn(false);
		break;
	case Key_Escape:
		pbClear->setOn(false);
		break;
	case Key_Delete:
		pbAC->setOn(false);
		break;
	case Key_S:
		pbSin->setOn(false);
		break;
	case Key_Backslash:
		pbplusminus->setOn(false);
		break;
	case Key_B:
		pbB->setOn(false);
		break;
	case Key_7:
		pb7->setOn(false);
		break;
	case Key_8:
		pb8->setOn(false);
		break;
	case Key_9:
		pb9->setOn(false);
		break;
	case Key_ParenLeft:
		pbparenopen->setOn(false);
		break;
	case Key_ParenRight:
		pbparenclose->setOn(false);
		break;
	case Key_Ampersand:
		pband->setOn(false);
		break;
	case Key_C:
		if (current_base == NB_HEX)
			pbC->setOn(false);
		else
			pbCos->setOn(false);
		break;
	case Key_4:
		pb4->setOn(false);
		break;
	case Key_5:
		pb5->setOn(false);
		break;
	case Key_6:
		pb6->setOn(false);
		break;
	case Key_Asterisk:
		pbX->setOn(false);
		break;
	case Key_Slash:
		pbdivision->setOn(false);
		break;
	case Key_O:
		pbor->setOn(false);
		break;
	case Key_T:
		pbTan->setOn(false);
		break;
	case Key_Exclam:
		pbfactorial->setOn(false);
		break;
	case Key_D:
		if(kcalcdefaults.style == 0)
			pbD->setOn(false);		// trig mode
		else
			pblog->setOn(false);	// stat mode
		break;
	case Key_1:
		pb1->setOn(false);
		break;
	case Key_2:
		pb2->setOn(false);
		break;
	case Key_3:
		pb3->setOn(false);
		break;
	case Key_Plus:
		pbplus->setOn(false);
		break;
	case Key_Minus:
		pbminus->setOn(false);
		break;
	case Key_Less:
		pbshift->setOn(false);
		break;
	case Key_N:
		pbln->setOn(false);
		break;
	case Key_L:
		key_pressed = false;
		pblog->setOn(false);
		break;
	case Key_AsciiCircum:
		pbpower->setOn(false);
		break;
	case Key_F:
		pbF->setOn(false);
		break;
	case Key_Period:
		pbperiod->setOn(false);
		break;
	case Key_Comma:
		pbperiod->setOn(false);
		break;
	case Key_0:
		pb0->setOn(false);
		break;
	case Key_Equal:
		pbequal->setOn(false);
		break;
	case Key_Return:
		pbequal->setOn(false);
		break;
	case Key_Enter:
		pbequal->setOn(false);
		break;
	case Key_Percent:
		key_pressed = false;
		pbpercent->setOn(false);
		break;
	case Key_AsciiTilde:
		pbnegate->setOn(false);
		break;
	case Key_Colon:
		pbmod->setOn(false);
		break;
	case Key_BracketLeft:
		pbsquare->setOn(false);
		break;
	case Key_Backspace:
		pbAC->setOn(false);
		break;
	case Key_R:
		pbreci->setOn(false);
		break;
	default:
		key_pressed = oldKey;
	}

	clear_buttons();
}

//-------------------------------------------------------------------------
// Name: clear_buttons()
//-------------------------------------------------------------------------
void QtCalculator::clear_buttons()
{
	//TODO: does this do anything?
}

//-------------------------------------------------------------------------
// Name: EEtoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::EEtoggled(bool myboolean)
{
	if(!display_error)
	{
		if(myboolean)
			EE();

		if(pbEE->isOn() && (!key_pressed))
			pbEE->setOn(false);
	}
	else
		KNotifyClient::beep();
}

//-------------------------------------------------------------------------
// Name: pbinvtoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbinvtoggled(bool myboolean)
{
	if(myboolean)
		SetInverse();

	if(pbinv->isOn() && (!key_pressed))
		pbinv->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbhyptoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbhyptoggled(bool myboolean)
{
	if(myboolean)
		EnterHyp();

	if(pbhyp->isOn() && (!key_pressed))
		pbhyp->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbMRtoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbMRtoggled(bool myboolean)
{
	if(myboolean)
		MR();

	if(pbMR->isOn() && (!key_pressed))
		pbMR->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbAtoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbAtoggled(bool myboolean)
{
	if(myboolean)
		buttonA();

	if(pbA->isOn() && (!key_pressed))
		pbA->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbSintoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbSintoggled(bool myboolean)
{
	if(myboolean)
		ExecSin();

	if(pbSin->isOn() && (!key_pressed))
		pbSin->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbplusminustoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbplusminustoggled(bool myboolean)
{
	if(myboolean)
		EnterNegate();

	if(pbplusminus->isOn() && (!key_pressed))
		pbplusminus->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbMplusminustoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbMplusminustoggled(bool myboolean)
{
	if(myboolean)
		Mplusminus();

	if(pbMplusminus->isOn() && (!key_pressed))
		pbMplusminus->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbBtoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbBtoggled(bool myboolean)
{
	if(myboolean)
		buttonB();

	if(pbB->isOn() && (!key_pressed))
		pbB->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbCostoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbCostoggled(bool myboolean)
{
	if(myboolean)
		ExecCos();

	if(pbCos->isOn() && (!key_pressed))
		pbCos->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbrecitoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbrecitoggled(bool myboolean)
{
	if(myboolean)
		EnterRecip();

	if(pbreci->isOn() && (!key_pressed))
		pbreci->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbCtoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbCtoggled(bool myboolean)
{
	if(myboolean)
		buttonC();

	if(pbC->isOn() && (!key_pressed))
		pbC->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbTantoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbTantoggled(bool myboolean)
{
	if(myboolean)
		ExecTan();

	if(pbTan->isOn() && (!key_pressed))
		pbTan->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbfactorialtoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbfactorialtoggled(bool myboolean)
{
	if(myboolean)
		EnterFactorial();

	if(pbfactorial->isOn() && (!key_pressed))
		pbfactorial->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbDtoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbDtoggled(bool myboolean)
{
	if(myboolean)
		buttonD();

	if(pbD->isOn() && (!key_pressed))
		pbD->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pblogtoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pblogtoggled(bool myboolean)
{
	if(myboolean)
		EnterLogr();

	if(pblog->isOn() && (!key_pressed))
		pblog->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbsquaretoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbsquaretoggled(bool myboolean)
{
	if(myboolean)
		EnterSquare();

	if(pbsquare->isOn() && (!key_pressed))
		pbsquare->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbEtoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbEtoggled(bool myboolean)
{
	if(!display_error)
	{
		if(myboolean)
			buttonE();

		if(pbE->isOn() && (!key_pressed))
			pbE->setOn(false);
	}
	else
		KNotifyClient::beep();
}

//-------------------------------------------------------------------------
// Name: pblntoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pblntoggled(bool myboolean)
{
	if(myboolean)
		EnterLogn();

	if(pbln->isOn() && (!key_pressed))
		pbln->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbpowertoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbpowertoggled(bool myboolean)
{
	if(myboolean)
		Power();

	if(pbpower->isOn() && (!key_pressed))
		pbpower->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbFtoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbFtoggled(bool myboolean)
{
	if(myboolean)
		buttonF();

	if(pbF->isOn() && (!key_pressed))
		pbF->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbMCtoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbMCtoggled(bool myboolean)
{
	if(myboolean)
		MC();

	if(pbMC->isOn() && (!key_pressed))
		pbMC->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbCleartoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbCleartoggled(bool myboolean)
{
	if(myboolean)
		Clear();

	if(pbClear->isOn() && (!key_pressed))
		pbClear->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbACtoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbACtoggled(bool myboolean)
{
	if(myboolean)
		ClearAll();

	if(pbAC->isOn() && (!key_pressed))
		pbAC->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pb7toggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pb7toggled(bool myboolean)
{
	if(myboolean)
		button7();

	if(pb7->isOn() && (!key_pressed))
		pb7->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pb8toggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pb8toggled(bool myboolean)
{
	if(myboolean)
		button8();

	if(pb8->isOn() && (!key_pressed))
		pb8->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pb9toggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pb9toggled(bool myboolean)
{
	if(myboolean)
		button9();

	if(pb9->isOn() && (!key_pressed))
		pb9->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbparenopentoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbparenopentoggled(bool myboolean)
{
	if(myboolean)
		EnterOpenParen();

	if(pbparenopen->isOn() && (!key_pressed))
		pbparenopen->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbparenclosetoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbparenclosetoggled(bool myboolean)
{
	if(myboolean)
		EnterCloseParen();

	if(pbparenclose->isOn() && (!key_pressed))
		pbparenclose->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbandtoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbandtoggled(bool myboolean)
{
	if(myboolean)
		And();

	if(pband->isOn() && (!key_pressed))
		pband->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pb4toggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pb4toggled(bool myboolean)
{
	if(myboolean)
		button4();

	if(pb4->isOn() && (!key_pressed))
		pb4->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pb5toggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pb5toggled(bool myboolean)
{
	if(myboolean)
		button5();

	if(pb5->isOn() && (!key_pressed))
		pb5->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pb6toggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pb6toggled(bool myboolean)
{
	if(myboolean)
		button6();

	if(pb6->isOn() && (!key_pressed))
		pb6->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbXtoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbXtoggled(bool myboolean)
{
	if(myboolean)
		Multiply();

	if(pbX->isOn() && (!key_pressed))
		pbX->setOn(false);
}

//-------------------------------------------------------------------------
// Name:
//-------------------------------------------------------------------------
void QtCalculator::pbdivisiontoggled(bool myboolean)
{
	if(myboolean)
		Divide();

	if(pbdivision->isOn() && (!key_pressed))
		pbdivision->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbortoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbortoggled(bool myboolean)
{
	if(myboolean)
		Or();

	if(pbor->isOn() && (!key_pressed))
		pbor->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pb1toggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pb1toggled(bool myboolean)
{
	if(myboolean)
		button1();

	if(pb1->isOn() && (!key_pressed))
		pb1->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pb2toggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pb2toggled(bool myboolean)
{
	if(myboolean)
		button2();

	if(pb2->isOn() && (!key_pressed))
		pb2->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pb3toggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pb3toggled(bool myboolean)
{
	if(myboolean)
		button3();

	if(pb3->isOn() && (!key_pressed))
		pb3->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbplustoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbplustoggled(bool myboolean)
{
	if(myboolean)
		Plus();

	if(pbplus->isOn() && (!key_pressed))
		pbplus->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbminustoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbminustoggled(bool myboolean)
{
	if(myboolean)
		Minus();

	if(pbminus->isOn() && (!key_pressed))
		pbminus->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbshifttoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbshifttoggled(bool myboolean)
{
	if(myboolean)
		Shift();

	if(pbshift->isOn() && (!key_pressed))
		pbshift->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbperiodtoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbperiodtoggled(bool myboolean)
{
	if(myboolean)
		EnterDecimal();

	if(pbperiod->isOn() && (!key_pressed))
		pbperiod->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pb0toggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pb0toggled(bool myboolean)
{
	if(myboolean)
		button0();

	if(pb0->isOn() && (!key_pressed))
		pb0->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbequaltoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbequaltoggled(bool myboolean)
{
	if(myboolean)
		EnterEqual();

	if(pbequal->isOn() && (!key_pressed))
		pbequal->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbpercenttoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbpercenttoggled(bool myboolean)
{
	if(myboolean)
		EnterPercent();

	if(pbpercent->isOn() && (!key_pressed))
		pbpercent->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbnegatetoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbnegatetoggled(bool myboolean)
{
	if(myboolean)
		EnterNotCmp();

	if(pbnegate->isOn() && (!key_pressed))
		pbnegate->setOn(false);
}

//-------------------------------------------------------------------------
// Name: pbmodtoggled(bool myboolean)
//-------------------------------------------------------------------------
void QtCalculator::pbmodtoggled(bool myboolean)
{
	if(myboolean)
		Mod();

	if(pbmod->isOn() && (!key_pressed))
		pbmod->setOn(false);
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
        mConfigureDialog->setActiveWindow();
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
		pbhyp->setText( "Hyp" );
		QToolTip::add(pbhyp, i18n("Hyperbolic mode"));
		pbSin->setText( "Sin" );
		QToolTip::add(pbSin, i18n("Sine"));
		pbCos->setText( "Cos" );
		QToolTip::add(pbCos, i18n("Cosine"));
		pbTan->setText( "Tan" );
		QToolTip::add(pbTan, i18n("Tangent"));
		pblog->setText( "Log" );
		QToolTip::remove(pblog);
		pbln ->setText( "Ln"  );
		QToolTip::add(pbln, i18n("Natural log"));
		break;

	case 1:
		pbhyp->setText( "N" );
		pbSin->setText( "Mea" );
		QToolTip::add(pbSin, i18n("Mean"));
		pbCos->setText( "Std" );
		QToolTip::add(pbCos, i18n("Standard deviation"));
		pbTan->setText( "Med" );
		QToolTip::add(pbTan, i18n("Median"));
		pblog->setText( "Dat" );
		QToolTip::add(pblog, i18n("Enter data"));
		pbln ->setText( "CSt"  );
		QToolTip::add(pbln, i18n("Clear data store"));
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
	QFont tmpFont("helvetica",14,QFont::Bold);
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
		mNumButtonList.first()->palette().active().button());
	config->writeEntry("FunctionButtonsColor",
		mFunctionButtonList.first()->palette().active().button());
	config->writeEntry("HexButtonsColor",
		mHexButtonList.first()->palette().active().button());
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
	for(p = mNumButtonList.first(); p; p=mNumButtonList.next())
	{
		p->setPalette(numPal);
	}

	QPalette funcPal(kcalcdefaults.functionButtonColor, bg);
	for(p = mFunctionButtonList.first(); p;
		p=mFunctionButtonList.next())
	{
		p->setPalette(funcPal);
	}

	QPalette hexPal(kcalcdefaults.hexButtonColor, bg);
	for(p = mHexButtonList.first(); p; p=mHexButtonList.next())
	{
		p->setPalette(hexPal);
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

	if((history_list.empty()) || (history_index >= (history_list.size() - 1)))
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
			if( mNumButtonList.findRef((QPushButton*)o) != -1)
			{
				list = &mNumButtonList;
			}
			else if( mFunctionButtonList.findRef((QPushButton*)o) != -1)
			{
				list = &mFunctionButtonList;
			}
			else if( mHexButtonList.findRef((QPushButton*)o) != -1)
			{
				list = &mHexButtonList;
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
		return KDialog::eventFilter(o, e);
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
