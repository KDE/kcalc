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
#include <qradiobutton.h>
#include <qtimer.h>
#include <qtooltip.h>

#include <kcolordrag.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <knotifyclient.h>
#include <kaboutdata.h>

#include "dlabel.h"
#include "optiondialog.h"
#include "version.h"
#include <kapplication.h>
#include <qpushbutton.h>


static const char *description =
	I18N_NOOP("KDE Calculator");

static const char *version = KCALCVERSION;

extern last_input_type last_input;
extern item_contents   display_data;
extern num_base        current_base;
extern int             display_error;
QPtrList<CALCAMNT>       temp_stack;

//
// * ported to QLayout (mosfet 10/28/99)
//
// * 1999-10-31 Espen Sand: More modifications.
//   All fixed sizes removed.
//   New config dialog.
//   To exit: Key_Q+ALT => Key_Q+CTRL,  Key_X+ALT => Key_X+CTRL
//   Look in updateGeometry() for size settings.
//


QtCalculator::QtCalculator( QWidget *parent, const char *name )
  : KDialog( parent, name ), mInternalSpacing(4),mConfigureDialog(0)
{
  key_pressed = false;
  selection_timer = new QTimer;
  status_timer = new QTimer;

  connect(status_timer,SIGNAL(timeout()),this,SLOT(clear_status_label()));
  connect(selection_timer,SIGNAL(timeout()),this,SLOT(selection_timed_out()));

  readSettings();

  //
  // Detect color change
  //
  connect(kapp,SIGNAL(kdisplayPaletteChanged()),this,SLOT(set_colors()));

  //
  // Accelerators to exit the program
  //
  QAccel *accel = new QAccel( this );
  accel->connectItem( accel->insertItem(Key_Q+CTRL),this,SLOT(quitCalc()) );
  accel->connectItem( accel->insertItem(Key_X+CTRL),this,SLOT(quitCalc()) );

  //
  // Create uppermost bar with buttons and numberdisplay
  //
  mConfigButton = new QPushButton( this, "configbutton" );
  mConfigButton->setText( i18n("Configure...") );
  mConfigButton->setAutoDefault(false);
  QToolTip::add( mConfigButton, i18n("Click to configure KCalc") );
  connect( mConfigButton, SIGNAL(clicked()), this, SLOT(configclicked()) );

  mHelpButton = new QPushButton( this, "helpbutton" );
  mHelpButton->setText("?");
  mHelpButton->setAutoDefault(false);
  QToolTip::add( mHelpButton, i18n("Help") );
  connect( mHelpButton, SIGNAL(clicked()), this, SLOT(helpclicked()) );

  calc_display = new DLabel( this, "display" );
  calc_display->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
  calc_display->setAlignment( AlignRight|AlignVCenter );
  calc_display->setFocus();
  calc_display->setFocusPolicy( QWidget::StrongFocus );

  connect(calc_display,SIGNAL(clicked()),this,SLOT(display_selected()));

  //
  // Status bar contents
  //
  statusINVLabel = new QLabel( this, "INV" );
  Q_CHECK_PTR( statusINVLabel );
  statusINVLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  statusINVLabel->setAlignment( AlignCenter );
  statusINVLabel->setText("NORM");

  statusHYPLabel = new QLabel( this, "HYP" );
  Q_CHECK_PTR( statusHYPLabel );
  statusHYPLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  statusHYPLabel->setAlignment( AlignCenter );

  statusERRORLabel = new QLabel( this, "ERROR" );
  Q_CHECK_PTR( statusERRORLabel );
  statusERRORLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  statusERRORLabel->setAlignment( AlignLeft|AlignVCenter );

  //
  // create angle button group
  //
  QButtonGroup *angle_group = new QButtonGroup( 3, Horizontal, this, "angle");
  angle_group->setTitle(i18n( "Angle") );
  connect( angle_group, SIGNAL(clicked(int)), SLOT(angle_selected(int)) );

  anglebutton[0] = new QRadioButton( angle_group );
  anglebutton[0]->setText( "&Deg" );
  anglebutton[0]->setChecked(TRUE);
  accel->connectItem(accel->insertItem(Key_D+ALT),this,SLOT(Deg_Selected()));

  anglebutton[1] = new QRadioButton( angle_group );
  anglebutton[1]->setText( "&Rad" );
  accel->connectItem(accel->insertItem(Key_R+ALT),this,SLOT(Rad_Selected()));

  anglebutton[2] = new QRadioButton( angle_group );
  anglebutton[2]->setText( "&Gra" );
  accel->connectItem(accel->insertItem(Key_G+ALT),this,SLOT(Gra_Selected()));

//////////////////////////////////////////////////////////////////////
//
// Create Number Base Button Group
//
  QButtonGroup *base_group = new QButtonGroup(4, Horizontal,  this, "base");
  base_group->setTitle( i18n("Base") );
  connect( base_group, SIGNAL(clicked(int)), SLOT(base_selected(int)) );

  basebutton[0] = new QRadioButton( base_group );
  basebutton[0]->setText( "&Hex" );
  accel->connectItem(accel->insertItem(Key_H+ALT),this,SLOT(Hex_Selected()));

  basebutton[1] = new QRadioButton( base_group );
  basebutton[1]->setText( "D&ec" );
  basebutton[1]->setChecked(TRUE);
  accel->connectItem(accel->insertItem(Key_E+ALT),this,SLOT(Dec_Selected()));

  basebutton[2] = new QRadioButton( base_group );
  basebutton[2]->setText( "&Oct" );
  accel->connectItem(accel->insertItem(Key_O+ALT),this,SLOT(Oct_Selected()));

  basebutton[3] = new QRadioButton( base_group );
  basebutton[3]->setText( "&Bin" );
  accel->connectItem(accel->insertItem(Key_B+ALT),this,SLOT(Bin_Selected()));

////////////////////////////////////////////////////////////////////////
//
//  Create Calculator Buttons
//

  //
  // First the widgets that are the parents of the buttons
  //
  mSmallPage = new QWidget( this );
  mLargePage = new QWidget( this );

  pbhyp = new QPushButton( mSmallPage, "hypbutton" );
  pbhyp->setText( "Hyp" );
  pbhyp->setAutoDefault(false);
  connect( pbhyp, SIGNAL(toggled(bool)), SLOT(pbhyptoggled(bool)));
  pbhyp->setToggleButton(TRUE);

  pbinv = new QPushButton( mSmallPage, "InverseButton" );
  pbinv->setText( "Inv" );
  pbinv->setAutoDefault(false);
  connect( pbinv, SIGNAL(toggled(bool)), SLOT(pbinvtoggled(bool)));
  pbinv->setToggleButton(TRUE);

  pbA = new QPushButton( mSmallPage, "Abutton" );
  pbA->setText( "A" );
  pbA->setAutoDefault(false);
  connect( pbA, SIGNAL(toggled(bool)), SLOT(pbAtoggled(bool)));
  pbA->setToggleButton(TRUE);

  pbSin = new QPushButton( mSmallPage, "Sinbutton" );
  pbSin->setAutoDefault(false);
  connect( pbSin, SIGNAL(toggled(bool)), SLOT(pbSintoggled(bool)));
  pbSin->setToggleButton(TRUE);

  pbplusminus = new QPushButton( mSmallPage, "plusminusbutton" );
  pbplusminus->setText( "+/-" );
  pbplusminus->setAutoDefault(false);
  connect(pbplusminus, SIGNAL(toggled(bool)),SLOT(pbplusminustoggled(bool)));
  pbplusminus->setToggleButton(TRUE);

  pbB = new QPushButton( mSmallPage, "Bbutton" );
  pbB->setText( "B" );
  pbB->setAutoDefault(false);
  connect( pbB, SIGNAL(toggled(bool)), SLOT(pbBtoggled(bool)));
  pbB->setToggleButton(TRUE);

  pbCos = new QPushButton( mSmallPage, "Cosbutton" );
  pbCos->setAutoDefault(false);
  connect( pbCos, SIGNAL(toggled(bool)), SLOT(pbCostoggled(bool)));
  pbCos->setToggleButton(TRUE);

  pbreci = new QPushButton( mSmallPage, "recibutton" );
  pbreci->setText( "1/x" );
  pbreci->setAutoDefault(false);
  connect( pbreci, SIGNAL(toggled(bool)), SLOT(pbrecitoggled(bool)));
  pbreci->setToggleButton(TRUE);

  pbC = new QPushButton( mSmallPage, "Cbutton" );
  pbC->setText( "C" );
  pbC->setAutoDefault(false);
  connect( pbC, SIGNAL(toggled(bool)), SLOT(pbCtoggled(bool)));
  pbC->setToggleButton(TRUE);

  pbTan = new QPushButton( mSmallPage, "Tanbutton" );
  pbTan->setAutoDefault(false);
  connect(pbTan, SIGNAL(toggled(bool)),SLOT(pbTantoggled(bool)));
  pbTan->setToggleButton(TRUE);
  setHypText(false);

  pbfactorial = new QPushButton( mSmallPage, "factorialbutton" );
  pbfactorial->setText( "x!" );
  pbfactorial->setAutoDefault(false);
  connect(pbfactorial, SIGNAL(toggled(bool)),SLOT(pbfactorialtoggled(bool)));
  pbfactorial->setToggleButton(TRUE);

  pbD = new QPushButton( mSmallPage, "Dbutton" );
  pbD->setText( "D" );
  pbD->setAutoDefault(false);
  connect( pbD, SIGNAL(toggled(bool)), SLOT(pbDtoggled(bool)));
  pbD->setToggleButton(TRUE);

  pblog = new QPushButton( mSmallPage, "logbutton" );
  pblog->setText( "Log" );
  pblog->setAutoDefault(false);
  connect( pblog, SIGNAL(toggled(bool)), SLOT(pblogtoggled(bool)));
  pblog->setToggleButton(TRUE);

  pbsquare = new QPushButton( mSmallPage, "squarebutton" );
  pbsquare->setText( "x^2" );
  pbsquare->setAutoDefault(false);
  connect( pbsquare, SIGNAL(toggled(bool)), SLOT(pbsquaretoggled(bool)));
  pbsquare->setToggleButton(TRUE);

  pbE = new QPushButton( mSmallPage, "Ebutton" );
  pbE->setText( "E" );
  pbE->setAutoDefault(false);
  connect( pbE, SIGNAL(toggled(bool)), SLOT(pbEtoggled(bool)));
  pbE->setToggleButton(TRUE);

  pbln = new QPushButton( mSmallPage, "lnbutton" );
  pbln->setText( "Ln" );
  pbln->setAutoDefault(false);
  connect( pbln, SIGNAL(toggled(bool)), SLOT(pblntoggled(bool)));
  pbln->setToggleButton(TRUE);

  pbpower = new QPushButton( mSmallPage, "powerbutton" );
  pbpower->setText( "x^y" );
  pbpower->setAutoDefault(false);
  connect( pbpower, SIGNAL(toggled(bool)), SLOT(pbpowertoggled(bool)));
  pbpower->setToggleButton(TRUE);

  pbF = new QPushButton( mSmallPage, "Fbutton" );
  pbF->setText( "F" );
  pbF->setAutoDefault(false);
  connect( pbF, SIGNAL(toggled(bool)), SLOT(pbFtoggled(bool)));
  pbF->setToggleButton(TRUE);

/////////////////////////////////////////////////////////////////////
//
//
  pbEE = new QPushButton( mLargePage, "EEbutton" );
  pbEE->setText( "EE" );
  pbEE->setToggleButton(TRUE);
  pbEE->setAutoDefault(false);
  connect( pbEE, SIGNAL(toggled(bool)), SLOT(EEtoggled(bool)));

  pbMR = new QPushButton( mLargePage, "MRbutton" );
  pbMR->setText( "MR" );
  pbMR->setAutoDefault(false);
  connect( pbMR, SIGNAL(toggled(bool)), SLOT(pbMRtoggled(bool)));
  pbMR->setToggleButton(TRUE);

  pbMplusminus = new QPushButton( mLargePage, "Mplusminusbutton" );
  pbMplusminus->setText( "M+-" );
  pbMplusminus->setAutoDefault(false);
  connect(pbMplusminus,SIGNAL(toggled(bool)),SLOT(pbMplusminustoggled(bool)));
  pbMplusminus->setToggleButton(TRUE);

  pbMC = new QPushButton( mLargePage, "MCbutton" );
  pbMC->setText( "MC" );
  pbMC->setAutoDefault(false);
  connect( pbMC, SIGNAL(toggled(bool)), SLOT(pbMCtoggled(bool)));
  pbMC->setToggleButton(TRUE);

  pbClear = new QPushButton( mLargePage, "Clearbutton" );
  pbClear->setText( "C" );
  pbClear->setAutoDefault(false);
  connect( pbClear, SIGNAL(toggled(bool)), SLOT(pbCleartoggled(bool)));
  pbClear->setToggleButton(TRUE);

  pbAC = new QPushButton( mLargePage, "ACbutton" );
  pbAC->setText( "AC" );
  pbAC->setAutoDefault(false);
  connect( pbAC, SIGNAL(toggled(bool)), SLOT(pbACtoggled(bool)));
  pbAC->setToggleButton(TRUE);


//////////////////////////////////////////////////////////////////////
//
//


  pb7 = new QPushButton( mLargePage, "7button" );
  pb7->setText( "7" );
  pb7->setAutoDefault(false);
  connect( pb7, SIGNAL(toggled(bool)), SLOT(pb7toggled(bool)));
  pb7->setToggleButton(TRUE);

  pb8 = new QPushButton( mLargePage, "8button" );
  pb8->setText( "8" );
  pb8->setAutoDefault(false);
  connect( pb8, SIGNAL(toggled(bool)), SLOT(pb8toggled(bool)));
  pb8->setToggleButton(TRUE);

  pb9 = new QPushButton( mLargePage, "9button" );
  pb9->setText( "9" );
  pb9->setAutoDefault(false);
  connect( pb9, SIGNAL(toggled(bool)), SLOT(pb9toggled(bool)));
  pb9->setToggleButton(TRUE);

  pbparenopen = new QPushButton( mLargePage, "parenopenbutton" );
  pbparenopen->setText( "(" );
  pbparenopen->setAutoDefault(false);
  connect( pbparenopen, SIGNAL(toggled(bool)),SLOT(pbparenopentoggled(bool)));
  pbparenopen->setToggleButton(TRUE);

  pbparenclose = new QPushButton( mLargePage, "parenclosebutton" );
  pbparenclose->setText( ")" );
  pbparenclose->setAutoDefault(false);
  connect(pbparenclose,SIGNAL(toggled(bool)),SLOT(pbparenclosetoggled(bool)));
  pbparenclose->setToggleButton(TRUE);

  pband = new QPushButton( mLargePage, "andbutton" );
  pband->setText( "And" );
  pband->setAutoDefault(false);
  connect( pband, SIGNAL(toggled(bool)), SLOT(pbandtoggled(bool)));
  pband->setToggleButton(TRUE);

//////////////////////////////////////////////////////////////////////
//
//

  pb4 = new QPushButton( mLargePage, "4button" );
  pb4->setText( "4" );
  pb4->setAutoDefault(false);
  connect( pb4, SIGNAL(toggled(bool)), SLOT(pb4toggled(bool)));
  pb4->setToggleButton(TRUE);

  pb5 = new QPushButton( mLargePage, "5button" );
  pb5->setText( "5" );
  pb5->setAutoDefault(false);
  connect( pb5, SIGNAL(toggled(bool)), SLOT(pb5toggled(bool)));
  pb5->setToggleButton(TRUE);

  pb6 = new QPushButton( mLargePage, "6button" );
  pb6->setText( "6" );
  pb6->setAutoDefault(false);
  connect( pb6, SIGNAL(toggled(bool)), SLOT(pb6toggled(bool)));
  pb6->setToggleButton(TRUE);

  pbX = new QPushButton( mLargePage, "Multiplybutton" );
  pbX->setText( "X" );
  pbX->setAutoDefault(false);
  connect( pbX, SIGNAL(toggled(bool)), SLOT(pbXtoggled(bool)));
  pbX->setToggleButton(TRUE);

  pbdivision = new QPushButton( mLargePage, "divisionbutton" );
  pbdivision->setText( "/" );
  pbdivision->setAutoDefault(false);
  connect( pbdivision, SIGNAL(toggled(bool)), SLOT(pbdivisiontoggled(bool)));
  pbdivision->setToggleButton(TRUE);

  pbor = new QPushButton( mLargePage, "orbutton" );
  pbor->setText( "Or" );
  pbor->setAutoDefault(false);
  connect( pbor, SIGNAL(toggled(bool)), SLOT(pbortoggled(bool)));
  pbor->setToggleButton(TRUE);

/////////////////////////////////////////////////////////////////////////////
//
//

  pb1 = new QPushButton( mLargePage, "1button" );
  pb1->setText( "1" );
  pb1->setAutoDefault(false);
  connect( pb1, SIGNAL(toggled(bool)), SLOT(pb1toggled(bool)));
  pb1->setToggleButton(TRUE);

  pb2 = new QPushButton( mLargePage, "2button" );
  pb2->setText( "2" );
  pb2->setAutoDefault(false);
  connect( pb2, SIGNAL(toggled(bool)), SLOT(pb2toggled(bool)));
  pb2->setToggleButton(TRUE);

  pb3 = new QPushButton( mLargePage, "3button" );
  pb3->setText( "3" );
  pb3->setAutoDefault(false);
  connect( pb3, SIGNAL(toggled(bool)), SLOT(pb3toggled(bool)));
  pb3->setToggleButton(TRUE);

  pbplus = new QPushButton( mLargePage, "plusbutton" );
  pbplus->setText( "+" );
  pbplus->setAutoDefault(false);
  connect( pbplus, SIGNAL(toggled(bool)), SLOT(pbplustoggled(bool)));
  pbplus->setToggleButton(TRUE);

  pbminus = new QPushButton( mLargePage, "minusbutton" );
  pbminus->setText( "-" );
  pbminus->setAutoDefault(false);
  connect( pbminus, SIGNAL(toggled(bool)), SLOT(pbminustoggled(bool)));
  pbminus->setToggleButton(TRUE);

  pbshift = new QPushButton( mLargePage, "shiftbutton" );
  pbshift->setText( "Lsh" );
  pbshift->setAutoDefault(false);
  connect( pbshift, SIGNAL(toggled(bool)), SLOT(pbshifttoggled(bool)));
  pbshift->setToggleButton(TRUE);

///////////////////////////////////////////////////////////////////////////
//
//
//
  pb0 = new QPushButton( mLargePage, "0button" );
  pb0->setText( "0" );
  pb0->setAutoDefault(false);
  connect( pb0, SIGNAL(toggled(bool)), SLOT(pb0toggled(bool)));
  pb0->setToggleButton(TRUE);

  pbperiod = new QPushButton( mLargePage, "periodbutton" );
  pbperiod->setText( "." );
  pbperiod->setAutoDefault(false);
  connect( pbperiod, SIGNAL(toggled(bool)), SLOT(pbperiodtoggled(bool)));
  pbperiod->setToggleButton(TRUE);

  pbequal = new QPushButton( mLargePage, "equalbutton" );
  pbequal->setText( "=" );
  pbequal->setAutoDefault(false);
  connect( pbequal, SIGNAL(toggled(bool)), SLOT(pbequaltoggled(bool)));
  pbequal->setToggleButton(TRUE);

  pbpercent = new QPushButton( mLargePage, "percentbutton" );
  pbpercent->setText( "%" );
  pbpercent->setAutoDefault(false);
  connect( pbpercent, SIGNAL(toggled(bool)), SLOT(pbpercenttoggled(bool)));
  pbpercent->setToggleButton(TRUE);

  pbnegate = new QPushButton( mLargePage, "OneComplementbutton" );
  pbnegate->setText( "Cmp" );
  pbnegate->setAutoDefault(false);
  connect( pbnegate, SIGNAL(toggled(bool)), SLOT(pbnegatetoggled(bool)));
  pbnegate->setToggleButton(TRUE);

  pbmod = new QPushButton( mLargePage, "modbutton" );
  pbmod->setText( "Mod" );
  pbmod->setAutoDefault(false);
  connect( pbmod, SIGNAL(toggled(bool)), SLOT(pbmodtoggled(bool)));
  pbmod->setToggleButton(TRUE);

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
						mInternalSpacing );
  QGridLayout *largeBtnLayout = new QGridLayout(mLargePage, 5, 6, 0,
						mInternalSpacing );
  QHBoxLayout *topLayout = new QHBoxLayout();
  QHBoxLayout *radioLayout = new QHBoxLayout();
  QHBoxLayout *btnLayout = new QHBoxLayout();
  QHBoxLayout *statusLayout = new QHBoxLayout();

  // bring them all together
  QVBoxLayout *mainLayout = new QVBoxLayout(this, mInternalSpacing,
					    mInternalSpacing );
  mainLayout->addLayout(topLayout );
  mainLayout->addLayout(radioLayout, 1 );
  mainLayout->addLayout(btnLayout  );
  mainLayout->addLayout(statusLayout );

  // button layout
  btnLayout->addWidget(mSmallPage,0,AlignTop);
  btnLayout->addSpacing( mInternalSpacing );
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

  smallBtnLayout->setRowStretch( 0, 0 );
  smallBtnLayout->setRowStretch( 1, 0 );
  smallBtnLayout->setRowStretch( 2, 0 );
  smallBtnLayout->setRowStretch( 3, 0 );
  smallBtnLayout->setRowStretch( 4, 0 );
  smallBtnLayout->setRowStretch( 5, 0 );

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

  largeBtnLayout->addWidget(pb0, 4, 0);
  largeBtnLayout->addWidget(pbperiod, 4, 1);
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
  statusLayout->addWidget(statusERRORLabel, 10 );

  mNumButtonList.append( pb0 );
  mNumButtonList.append( pb1 );
  mNumButtonList.append( pb2 );
  mNumButtonList.append( pb3 );
  mNumButtonList.append( pb4 );
  mNumButtonList.append( pb5 );
  mNumButtonList.append( pb6 );
  mNumButtonList.append( pb7 );
  mNumButtonList.append( pb8 );
  mNumButtonList.append( pb9 );

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
  InitializeCalculator();

  updateGeometry();
  setFixedHeight(minimumHeight());
}

QtCalculator::~QtCalculator( void )
{
  delete mConfigureDialog;
}


void QtCalculator::setHypText(bool hyp)
{
   if (hyp)
   {
      pbSin->setText( "Sinh" );
      pbCos->setText( "Cosh" );
      pbTan->setText( "Tanh" );
   }
   else
   {
      pbSin->setText( "Sin" );
      pbCos->setText( "Cos" );
      pbTan->setText( "Tan" );
   };
};

void QtCalculator::updateGeometry( void )
{
  QSize s;
  QObjectList *l;

  //
  // Uppermost bar
  //
  mHelpButton->setFixedWidth( mHelpButton->fontMetrics().width("MM") );
  calc_display->setMinimumWidth( calc_display->fontMetrics().maxWidth()*15 );

  //
  // Button groups (base and angle)
  //
  //QButtonGroup *g;
  //g = (QButtonGroup*)(anglebutton[0]->parentWidget());
  //g = (QButtonGroup*)(basebutton[0]->parentWidget());

  //
  // Calculator buttons
  //
  s.setWidth( mSmallPage->fontMetrics().width("MMM") );
  s.setHeight( mSmallPage->fontMetrics().lineSpacing() + 6 );

  l = (QObjectList*)mSmallPage->children(); // silence please
  for( uint i=0; i < l->count(); i++ )
  {
    QObject *o = l->at(i);
    if( o->isWidgetType() )
    {
      ((QWidget*)o)->setMinimumSize(s);
      ((QWidget*)o)->installEventFilter( this );
      ((QWidget*)o)->setAcceptDrops(true);
    }
  }

  l = (QObjectList*)mLargePage->children(); // silence please

  int h1 = pbF->minimumSize().height();
  int h2 = (int)((((float)h1+4.0)/5.0));
  s.setWidth( mLargePage->fontMetrics().width("MMM") );
  s.setHeight( h1+h2 );

  for( uint i=0; i < l->count(); i++ )
  {
    QObject *o = l->at(i);
    if( o->isWidgetType() )
    {
      ((QWidget*)o)->setFixedSize(s);
      ((QWidget*)o)->installEventFilter( this );
      ((QWidget*)o)->setAcceptDrops(true);
    }
  }

  //
  // The status bar
  //
  s.setWidth( statusINVLabel->fontMetrics().width("NORM") +
	      statusINVLabel->frameWidth()*2 + 10 );
  statusINVLabel->setMinimumWidth( s.width() );
  statusHYPLabel->setMinimumWidth( s.width() );

  //setFixedSize(minimumSize());
}

void QtCalculator::Hex_Selected()
{
  basebutton[0]->setChecked(TRUE);
  basebutton[1]->setChecked(FALSE);
  basebutton[2]->setChecked(FALSE);
  basebutton[3]->setChecked(FALSE);
  SetHex();
}

void QtCalculator::Dec_Selected()
{
  basebutton[0]->setChecked(FALSE);
  basebutton[1]->setChecked(TRUE);
  basebutton[2]->setChecked(FALSE);
  basebutton[3]->setChecked(FALSE);
  SetDec();
}

void QtCalculator::Oct_Selected()
{
  basebutton[0]->setChecked(FALSE);
  basebutton[1]->setChecked(FALSE);
  basebutton[2]->setChecked(TRUE);
  basebutton[3]->setChecked(FALSE);
  SetOct();
}

void QtCalculator::Bin_Selected()
{
  basebutton[0]->setChecked(FALSE);
  basebutton[1]->setChecked(FALSE);
  basebutton[2]->setChecked(FALSE);
  basebutton[3]->setChecked(TRUE);
  SetBin();
}

void QtCalculator::Deg_Selected()
{
  anglebutton[0]->setChecked(TRUE);
  anglebutton[1]->setChecked(FALSE);
  anglebutton[2]->setChecked(FALSE);
  SetDeg();
}

void QtCalculator::Rad_Selected()
{
  anglebutton[0]->setChecked(FALSE);
  anglebutton[1]->setChecked(TRUE);
  anglebutton[2]->setChecked(FALSE);
  SetRad();

}

void QtCalculator::Gra_Selected()
{
  anglebutton[0]->setChecked(FALSE);
  anglebutton[1]->setChecked(FALSE);
  anglebutton[2]->setChecked(TRUE);
  SetGra();
}

void QtCalculator::helpclicked()
{
  kapp->invokeHelp( );
}

void QtCalculator::configurationChanged( const DefStruct &state )
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

void QtCalculator::keyPressEvent( QKeyEvent *e ){

  switch (e->key() ){

  case Key_F1:
     helpclicked();
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
     temp_stack_prev();
     break;
  case Key_Down:
     temp_stack_next();
     break;

  case Key_Next:
     key_pressed = TRUE;
     pbAC->setOn(TRUE);
     break;
  case Key_Prior:
     key_pressed = TRUE;
     pbClear->setOn(TRUE);
     break;

  case Key_H:
     key_pressed = TRUE;
     pbhyp->setOn(TRUE);
     break;
  case Key_I:
     key_pressed = TRUE;
     pbinv->setOn(TRUE);
     break;
  case Key_A:
     key_pressed = TRUE;
     pbA->setOn(TRUE);

     break;
  case Key_E:
     key_pressed = TRUE;
    if (current_base == NB_HEX)
     pbE->setOn(TRUE);
    else
     pbEE->setOn(TRUE);
     break;
  case Key_Escape:
     key_pressed = TRUE;
     pbClear->setOn(TRUE);
     break;
  case Key_Delete:
     key_pressed = TRUE;
     pbAC->setOn(TRUE);
     break;
  case Key_S:
     key_pressed = TRUE;
     pbSin->setOn(TRUE);
     break;
  case Key_Backslash:
     key_pressed = TRUE;
     pbplusminus->setOn(TRUE);
     break;
  case Key_B:
     key_pressed = TRUE;
     pbB->setOn(TRUE);
     break;
  case Key_7:
     key_pressed = TRUE;
     pb7->setOn(TRUE);
     break;
  case Key_8:
     key_pressed = TRUE;
     pb8->setOn(TRUE);
     break;
  case Key_9:
     key_pressed = TRUE;
     pb9->setOn(TRUE);
     break;
  case Key_ParenLeft:
     key_pressed = TRUE;
     pbparenopen->setOn(TRUE);
     break;
  case Key_ParenRight:
     key_pressed = TRUE;
     pbparenclose->setOn(TRUE);
     break;
  case Key_Ampersand:
     key_pressed = TRUE;
     pband->setOn(TRUE);
     break;
  case Key_C:
     key_pressed = TRUE;
    if (current_base == NB_HEX)
     pbC->setOn(TRUE);
    else
     pbCos->setOn(TRUE);
     break;
  case Key_4:
     key_pressed = TRUE;
     pb4->setOn(TRUE);
     break;
  case Key_5:
     key_pressed = TRUE;
     pb5->setOn(TRUE);
     break;
  case Key_6:
     key_pressed = TRUE;
     pb6->setOn(TRUE);
     break;
  case Key_Asterisk:
     key_pressed = TRUE;
     pbX->setOn(TRUE);
     break;
  case Key_Slash:
     key_pressed = TRUE;
     pbdivision->setOn(TRUE);
     break;
  case Key_O:
     key_pressed = TRUE;
     pbor->setOn(TRUE);
     break;
  case Key_T:
     key_pressed = TRUE;
     pbTan->setOn(TRUE);
     break;
  case Key_Exclam:
     key_pressed = TRUE;
     pbfactorial->setOn(TRUE);
     break;
  case Key_D:
     key_pressed = TRUE;
     if(kcalcdefaults.style == 0)
       pbD->setOn(TRUE); // trig mode
     else
       pblog->setOn(TRUE); // stat mode
    break;
  case Key_1:
     key_pressed = TRUE;
     pb1->setOn(TRUE);
     break;
  case Key_2:
     key_pressed = TRUE;
     pb2->setOn(TRUE);
     break;
  case Key_3:
     key_pressed = TRUE;
     pb3->setOn(TRUE);
     break;
  case Key_Plus:
     key_pressed = TRUE;
     pbplus->setOn(TRUE);
     break;
  case Key_Minus:
     key_pressed = TRUE;
     pbminus->setOn(TRUE);
     break;
  case Key_Less:
     key_pressed = TRUE;
     pbshift->setOn(TRUE);
     break;
  case Key_N:
     key_pressed = TRUE;
     pbln->setOn(TRUE);
     break;
  case Key_L:
     key_pressed = TRUE;
     pblog->setOn(TRUE);
     break;
  case Key_AsciiCircum:
     key_pressed = TRUE;
     pbpower->setOn(TRUE);
     break;
  case Key_F:
     key_pressed = TRUE;
     pbF->setOn(TRUE);
     break;
  case Key_Period:
     key_pressed = TRUE;
     pbperiod->setOn(TRUE);
     break;
  case Key_Comma:
     key_pressed = TRUE;
     pbperiod->setOn(TRUE);
     break;
  case Key_0:
     key_pressed = TRUE;
     pb0->setOn(TRUE);
     break;
     case Key_Equal:
     key_pressed = TRUE;
     pbequal->setOn(TRUE);
     break;
  case Key_Return:
     key_pressed = TRUE;
     pbequal->setOn(TRUE);
     break;
  case Key_Enter:
     key_pressed = TRUE;
     pbequal->setOn(TRUE);
     break;
  case Key_Percent:
     key_pressed = TRUE;
     pbpercent->setOn(TRUE);
     break;
  case Key_AsciiTilde:
     key_pressed = TRUE;
     pbnegate->setOn(TRUE);
     break;
  case Key_Colon:
     key_pressed = TRUE;
     pbmod->setOn(TRUE);
     break;
  case Key_BracketLeft:
     key_pressed = TRUE;
     pbsquare->setOn(TRUE);
     break;
 case Key_Backspace:
     /*** HEYA ***/
     SubtractDigit();
//     key_pressed = TRUE;
//     pbAC->setOn(TRUE);
     break;
  case Key_R:
     key_pressed = TRUE;
     pbreci->setOn(TRUE);
     break;
  case Key_F5:
     Hex_Selected();
     break;
  case Key_F6:
     Dec_Selected();
     break;
  case Key_F7:
     Oct_Selected();
     break;
  case Key_F8:
     Bin_Selected();
     break;
  case Key_F9:
     Deg_Selected();
     break;
  case Key_F10:
     Rad_Selected();
     break;
  case Key_F11:
     Gra_Selected();
     break;
  }
}

void QtCalculator::keyReleaseEvent( QKeyEvent *e ){
  switch (e->key() ){

  case Key_Next:
     key_pressed = FALSE;
     pbAC->setOn(FALSE);
     break;
  case Key_Prior:
     key_pressed = FALSE;
     pbClear->setOn(FALSE);
     break;

  case Key_H:
    key_pressed = FALSE;
     pbhyp->setOn(FALSE);
     break;
  case Key_I:
    key_pressed = FALSE;
     pbinv->setOn(FALSE);
     break;
  case Key_A:
    key_pressed = FALSE;
     pbA->setOn(FALSE);
     break;
  case Key_E:
    key_pressed = FALSE;
    if (current_base == NB_HEX)
     pbE->setOn(FALSE);
    else
     pbEE->setOn(FALSE);
     break;
  case Key_Escape:
    key_pressed = FALSE;
     pbClear->setOn(FALSE);
     break;
  case Key_Delete:
    key_pressed = FALSE;
     pbAC->setOn(FALSE);
     break;
  case Key_S:
    key_pressed = FALSE;
     pbSin->setOn(FALSE);
     break;
  case Key_Backslash:
    key_pressed = FALSE;
     pbplusminus->setOn(FALSE);
     break;
  case Key_B:
    key_pressed = FALSE;
     pbB->setOn(FALSE);
     break;
  case Key_7:
    key_pressed = FALSE;
     pb7->setOn(FALSE);
     break;
  case Key_8:
    key_pressed = FALSE;
     pb8->setOn(FALSE);
     break;
  case Key_9:
    key_pressed = FALSE;
     pb9->setOn(FALSE);
     break;
  case Key_ParenLeft:
    key_pressed = FALSE;
     pbparenopen->setOn(FALSE);
     break;
  case Key_ParenRight:
    key_pressed = FALSE;
     pbparenclose->setOn(FALSE);
     break;
  case Key_Ampersand:
    key_pressed = FALSE;
     pband->setOn(FALSE);
     break;
  case Key_C:
    key_pressed = FALSE;
    if (current_base == NB_HEX)
     pbC->setOn(FALSE);
    else
     pbCos->setOn(FALSE);
     break;
  case Key_4:
    key_pressed = FALSE;
     pb4->setOn(FALSE);
     break;
  case Key_5:
    key_pressed = FALSE;
     pb5->setOn(FALSE);
     break;
  case Key_6:
    key_pressed = FALSE;
     pb6->setOn(FALSE);
     break;
  case Key_Asterisk:
    key_pressed = FALSE;
     pbX->setOn(FALSE);
     break;
  case Key_Slash:
    key_pressed = FALSE;
     pbdivision->setOn(FALSE);
     break;
  case Key_O:
    key_pressed = FALSE;
     pbor->setOn(FALSE);
     break;
  case Key_T:
    key_pressed = FALSE;
     pbTan->setOn(FALSE);
     break;
  case Key_Exclam:
    key_pressed = FALSE;
     pbfactorial->setOn(FALSE);
     break;
  case Key_D:
    key_pressed = FALSE;
    if(kcalcdefaults.style == 0)
      pbD->setOn(FALSE); // trig mode
    else
      pblog->setOn(FALSE);// stat mode
     break;
  case Key_1:
    key_pressed = FALSE;
     pb1->setOn(FALSE);
     break;
  case Key_2:
    key_pressed = FALSE;
     pb2->setOn(FALSE);
     break;
  case Key_3:
    key_pressed = FALSE;
     pb3->setOn(FALSE);
     break;
  case Key_Plus:
    key_pressed = FALSE;
     pbplus->setOn(FALSE);
     break;
  case Key_Minus:
    key_pressed = FALSE;
     pbminus->setOn(FALSE);
     break;
  case Key_Less:
    key_pressed = FALSE;
     pbshift->setOn(FALSE);
     break;
  case Key_N:
    key_pressed = FALSE;
     pbln->setOn(FALSE);
     break;
  case Key_L:
    key_pressed = FALSE;
     pblog->setOn(FALSE);
     break;
  case Key_AsciiCircum:
    key_pressed = FALSE;
     pbpower->setOn(FALSE);
     break;
  case Key_F:
    key_pressed = FALSE;
     pbF->setOn(FALSE);
     break;
  case Key_Period:
    key_pressed = FALSE;
     pbperiod->setOn(FALSE);
     break;
  case Key_Comma:
    key_pressed = FALSE;
     pbperiod->setOn(FALSE);
     break;
  case Key_0:
    key_pressed = FALSE;
     pb0->setOn(FALSE);
     break;
  case Key_Equal:
    key_pressed = FALSE;
     pbequal->setOn(FALSE);
     break;
  case Key_Return:
    key_pressed = FALSE;
     pbequal->setOn(FALSE);
     break;
  case Key_Enter:
     key_pressed = FALSE;
     pbequal->setOn(FALSE);
     break;
  case Key_Percent:
    key_pressed = FALSE;
     pbpercent->setOn(FALSE);
     break;
  case Key_AsciiTilde:
    key_pressed = FALSE;
     pbnegate->setOn(FALSE);
     break;
  case Key_Colon:
    key_pressed = FALSE;
     pbmod->setOn(FALSE);
     break;
  case Key_BracketLeft:
     key_pressed = FALSE;
     pbsquare->setOn(FALSE);
     break;
  case Key_Backspace:
     key_pressed = FALSE;
     pbAC->setOn(FALSE);
     break;
  case Key_R:
     key_pressed = FALSE;
     pbreci->setOn(FALSE);
     break;
  }

  clear_buttons();
}

void QtCalculator::clear_buttons(){

}

void QtCalculator::EEtoggled(bool myboolean){
  if(!display_error) {
    if(myboolean)
      EE();
    if(pbEE->isOn() && (!key_pressed))
      pbEE->setOn(FALSE);
  } else
    KNotifyClient::beep();
}

void QtCalculator::pbinvtoggled(bool myboolean){
  if(myboolean)
    SetInverse();
  if(pbinv->isOn() && (!key_pressed))
    pbinv->setOn(FALSE);
}

void QtCalculator::pbhyptoggled(bool myboolean){
  if(myboolean)
    EnterHyp();
  if(pbhyp->isOn() && (!key_pressed))
    pbhyp->setOn(FALSE);
}
void QtCalculator::pbMRtoggled(bool myboolean){
  if(myboolean)
    MR();
  if(pbMR->isOn() && (!key_pressed))
    pbMR->setOn(FALSE);
}

void QtCalculator::pbAtoggled(bool myboolean){
  if(myboolean)
    buttonA();
    if(pbA->isOn() && (!key_pressed))
   pbA->setOn(FALSE);
}

void QtCalculator::pbSintoggled(bool myboolean){
  if(myboolean)
    ExecSin();
  if(pbSin->isOn() && (!key_pressed))
    pbSin->setOn(FALSE);
}
void QtCalculator::pbplusminustoggled(bool myboolean){
  if(myboolean)
    EnterNegate();
  if(pbplusminus->isOn() && (!key_pressed))
    pbplusminus->setOn(FALSE);
}
void QtCalculator::pbMplusminustoggled(bool myboolean){
  if(myboolean)
    Mplusminus();
  if(pbMplusminus->isOn() && (!key_pressed))
    pbMplusminus->setOn(FALSE);
}
void QtCalculator::pbBtoggled(bool myboolean){
  if(myboolean)
    buttonB();
  if(pbB->isOn() && (!key_pressed))
    pbB->setOn(FALSE);
}
void QtCalculator::pbCostoggled(bool myboolean){
  if(myboolean)
    ExecCos();
  if(pbCos->isOn() && (!key_pressed))
    pbCos->setOn(FALSE);
}
void QtCalculator::pbrecitoggled(bool myboolean){
  if(myboolean)
    EnterRecip();
  if(pbreci->isOn() && (!key_pressed))
    pbreci->setOn(FALSE);
}
void QtCalculator::pbCtoggled(bool myboolean){
  if(myboolean)
    buttonC();
  if(pbC->isOn() && (!key_pressed))
    pbC->setOn(FALSE);
}
void QtCalculator::pbTantoggled(bool myboolean){
  if(myboolean)
    ExecTan();
  if(pbTan->isOn() && (!key_pressed))
    pbTan->setOn(FALSE);
}
void QtCalculator::pbfactorialtoggled(bool myboolean){
  if(myboolean)
    EnterFactorial();
  if(pbfactorial->isOn() && (!key_pressed))
    pbfactorial->setOn(FALSE);
}
void QtCalculator::pbDtoggled(bool myboolean){
  if(myboolean)
    buttonD();
  if(pbD->isOn() && (!key_pressed))
    pbD->setOn(FALSE);
}
void QtCalculator::pblogtoggled(bool myboolean){
  if(myboolean)
   EnterLogr();
  if(pblog->isOn() && (!key_pressed))
    pblog->setOn(FALSE);
}
void QtCalculator::pbsquaretoggled(bool myboolean){
  if(myboolean)
    EnterSquare();
  if(pbsquare->isOn() && (!key_pressed))
    pbsquare->setOn(FALSE);
}
void QtCalculator::pbEtoggled(bool myboolean){
  if(!display_error) {
    if(myboolean)
      buttonE();
    if(pbE->isOn() && (!key_pressed))
      pbE->setOn(FALSE);
  } else
    KNotifyClient::beep();
}
void QtCalculator::pblntoggled(bool myboolean){
  if(myboolean)
    EnterLogn();
  if(pbln->isOn() && (!key_pressed))
    pbln->setOn(FALSE);
}
void QtCalculator::pbpowertoggled(bool myboolean){
  if(myboolean)
    Power();
  if(pbpower->isOn() && (!key_pressed))
    pbpower->setOn(FALSE);
}
void QtCalculator::pbFtoggled(bool myboolean){
  if(myboolean)
    buttonF();
  if(pbF->isOn() && (!key_pressed))
    pbF->setOn(FALSE);
}
void QtCalculator::pbMCtoggled(bool myboolean){
  if(myboolean)
    MC();
  if(pbMC->isOn() && (!key_pressed))
    pbMC->setOn(FALSE);
}
void QtCalculator::pbCleartoggled(bool myboolean){
  if(myboolean)
    Clear();
  if(pbClear->isOn() && (!key_pressed))
    pbClear->setOn(FALSE);
}
void QtCalculator::pbACtoggled(bool myboolean){
  if(myboolean)
    ClearAll();
  if(pbAC->isOn() && (!key_pressed))
    pbAC->setOn(FALSE);
}
void QtCalculator::pb7toggled(bool myboolean){
  if(myboolean)
    button7();
  if(pb7->isOn() && (!key_pressed))
    pb7->setOn(FALSE);
}
void QtCalculator::pb8toggled(bool myboolean){
  if(myboolean)
    button8();
  if(pb8->isOn() && (!key_pressed))
    pb8->setOn(FALSE);
}
void QtCalculator::pb9toggled(bool myboolean){
  if(myboolean)
    button9();
  if(pb9->isOn() && (!key_pressed))
    pb9->setOn(FALSE);
}
void QtCalculator::pbparenopentoggled(bool myboolean){
  if(myboolean)
    EnterOpenParen();
  if(pbparenopen->isOn() && (!key_pressed))
    pbparenopen->setOn(FALSE);
}
void QtCalculator::pbparenclosetoggled(bool myboolean){
  if(myboolean)
    EnterCloseParen();
  if(pbparenclose->isOn() && (!key_pressed))
    pbparenclose->setOn(FALSE);
}
void QtCalculator::pbandtoggled(bool myboolean){
  if(myboolean)
    And();
  if(pband->isOn() && (!key_pressed))
    pband->setOn(FALSE);
}
void QtCalculator::pb4toggled(bool myboolean){
  if(myboolean)
    button4();
  if(pb4->isOn() && (!key_pressed))
    pb4->setOn(FALSE);
}
void QtCalculator::pb5toggled(bool myboolean){
  if(myboolean)
    button5();
  if(pb5->isOn() && (!key_pressed))
    pb5->setOn(FALSE);
}
void QtCalculator::pb6toggled(bool myboolean){
  if(myboolean)
    button6();
  if(pb6->isOn() && (!key_pressed))
    pb6->setOn(FALSE);
}
void QtCalculator::pbXtoggled(bool myboolean){
  if(myboolean)
    Multiply();
  if(pbX->isOn() && (!key_pressed))
    pbX->setOn(FALSE);
}
void QtCalculator::pbdivisiontoggled(bool myboolean){
  if(myboolean)
    Divide();
  if(pbdivision->isOn() && (!key_pressed))
    pbdivision->setOn(FALSE);
}
void QtCalculator::pbortoggled(bool myboolean){
  if(myboolean)
    Or();
  if(pbor->isOn() && (!key_pressed))
    pbor->setOn(FALSE);
}
void QtCalculator::pb1toggled(bool myboolean){
  if(myboolean)
    button1();
  if(pb1->isOn() && (!key_pressed))
    pb1->setOn(FALSE);
}
void QtCalculator::pb2toggled(bool myboolean){
  if(myboolean)
    button2();
  if(pb2->isOn() && (!key_pressed))
    pb2->setOn(FALSE);
}
void QtCalculator::pb3toggled(bool myboolean){
  if(myboolean)
    button3();
  if(pb3->isOn() && (!key_pressed))
    pb3->setOn(FALSE);
}
void QtCalculator::pbplustoggled(bool myboolean){
  if(myboolean)
    Plus();
  if(pbplus->isOn() && (!key_pressed))
    pbplus->setOn(FALSE);
}
void QtCalculator::pbminustoggled(bool myboolean){
  if(myboolean)
    Minus();
  if(pbminus->isOn() && (!key_pressed))
    pbminus->setOn(FALSE);
}
void QtCalculator::pbshifttoggled(bool myboolean){
  if(myboolean)
    Shift();
  if(pbshift->isOn() && (!key_pressed))
    pbshift->setOn(FALSE);
}
void QtCalculator::pbperiodtoggled(bool myboolean){
  if(myboolean)
    EnterDecimal();
  if(pbperiod->isOn() && (!key_pressed))
    pbperiod->setOn(FALSE);
}
void QtCalculator::pb0toggled(bool myboolean){
  if(myboolean)
    button0();
  if(pb0->isOn() && (!key_pressed))
    pb0->setOn(FALSE);
}
void QtCalculator::pbequaltoggled(bool myboolean){
  if(myboolean)
    EnterEqual();
  if(pbequal->isOn() && (!key_pressed))
    pbequal->setOn(FALSE);
}
void QtCalculator::pbpercenttoggled(bool myboolean){
  if(myboolean)
    EnterPercent();
  if(pbpercent->isOn() && (!key_pressed))
    pbpercent->setOn(FALSE);
}
void QtCalculator::pbnegatetoggled(bool myboolean){
  if(myboolean)
    EnterNotCmp();
  if(pbnegate->isOn() && (!key_pressed))
    pbnegate->setOn(FALSE);
}
void QtCalculator::pbmodtoggled(bool myboolean)  {
  if(myboolean)
    Mod();
  if(pbmod->isOn() && (!key_pressed))
    pbmod->setOn(FALSE);
}

void QtCalculator::configclicked()
{
  if( mConfigureDialog == 0 )
  {
    mConfigureDialog = new ConfigureDialog( this, 0, false );
    mConfigureDialog->setState( kcalcdefaults );
    connect( mConfigureDialog, SIGNAL( valueChanged(const DefStruct &)),
	     this, SLOT(configurationChanged(const DefStruct &)));
  }
  mConfigureDialog->show();
}


void QtCalculator::set_style()
{

  switch(kcalcdefaults.style){
  case  0:{
    pbhyp->setText( "Hyp" );
    pbSin->setText( "Sin" );
    pbCos->setText( "Cos" );
    pbTan->setText( "Tan" );
    pblog->setText( "Log" );
    pbln ->setText( "Ln"  );
    break;
  }
  case 1:{
    pbhyp->setText( "N" );
    pbSin->setText( "Mea" );
    pbCos->setText( "Std" );
    pbTan->setText( "Med" );
    pblog->setText( "Dat" );
    pbln ->setText( "CSt"  );
    break;
  }

  default:
    break;
  }
}

void QtCalculator::readSettings()
{

  QString str;

  KConfig *config = KGlobal::config();
  config->setGroup( "Font" );

  kcalcdefaults.font = config->readFontEntry("Font",
	new QFont("helvetica",14,QFont::Bold));

  config->setGroup("Colors");
  QColor tmpC(189, 255, 180);
  QColor blackC(0,0,0);
  QColor defaultButtonColor = palette().active().background();

  kcalcdefaults.forecolor = config->readColorEntry("ForeColor",&blackC);
  kcalcdefaults.backcolor = config->readColorEntry("BackColor",&tmpC);
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
  kcalcdefaults.precision =  config->readNumEntry("precision",(int)14);
#else
  kcalcdefaults.precision =  config->readNumEntry("precision",(int)10);
#endif

  kcalcdefaults.fixedprecision =  config->readNumEntry("fixedprecision",(int)2);
  kcalcdefaults.fixed = (bool) config->readNumEntry("fixed",(int)0);

  config->setGroup("General");
  kcalcdefaults.style          = config->readNumEntry("style",(int)0);
  kcalcdefaults.beep          = config->readNumEntry("beep",(int)1);

}

void QtCalculator::writeSettings()
{

  KConfig *config = KGlobal::config();

  config->setGroup( "Font" );
  config->writeEntry("Font",kcalcdefaults.font);

  config->setGroup("Colors");
  config->writeEntry("ForeColor",kcalcdefaults.forecolor);
  config->writeEntry("BackColor",kcalcdefaults.backcolor);
  config->writeEntry( "NumberButtonsColor",
    mNumButtonList.first()->palette().active().button() );
  config->writeEntry( "FunctionButtonsColor",
    mFunctionButtonList.first()->palette().active().button() );
  config->writeEntry( "HexButtonsColor",
    mHexButtonList.first()->palette().active().button() );
  config->writeEntry( "MemoryButtonsColor",
    mMemButtonList.first()->palette().active().button() );
  config->writeEntry( "OperationButtonsColor",
    mOperationButtonList.first()->palette().active().button() );

  config->setGroup("Precision");
  config->writeEntry("precision",  kcalcdefaults.precision);
  config->writeEntry("fixedprecision",  kcalcdefaults.fixedprecision);
  config->writeEntry("fixed",  (int)kcalcdefaults.fixed);

  config->setGroup("General");
  config->writeEntry("style",(int)kcalcdefaults.style);
  config->writeEntry("beep",(int)kcalcdefaults.beep);
  config->sync();

}

void QtCalculator::display_selected(){

  if(calc_display->Button() == LeftButton){

    if(calc_display->isLit()){

      QClipboard *cb = QApplication::clipboard();
      cb->setText(calc_display->text());
      selection_timer->start(100);

    }
    else{

      selection_timer->stop();

    }

    invertColors();
  }
  else{

    QClipboard *cb = QApplication::clipboard();

    CALCAMNT result;
    bool was_ok;
    result = (CALCAMNT) cb->text().toDouble(&was_ok);
    if (!was_ok) result = (CALCAMNT) (0);

    last_input = PASTE;
    DISPLAY_AMOUNT = result;
    UpdateDisplay();
  }

}

void QtCalculator::selection_timed_out(){

  selection_timer->stop();
  calc_display->setLit(FALSE);
  invertColors();
}


void QtCalculator::clear_status_label(){

  statusERRORLabel->clear();
  status_timer->stop();
}

void QtCalculator::setStatusLabel(const QString& string){

  statusERRORLabel->setText(string);
  status_timer->start(3000,TRUE);

}


void QtCalculator::invertColors(){

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
  {
    set_colors();
  }
}

void QtCalculator::closeEvent( QCloseEvent * ){

  quitCalc();

}

void QtCalculator::quitCalc(){

  writeSettings();
  qApp->quit();

}

void QtCalculator::set_colors()
{
  QPalette pal = calc_display->palette();
  pal.setColor( QColorGroup::Text, kcalcdefaults.forecolor );
  pal.setColor( QColorGroup::Foreground, kcalcdefaults.forecolor );
  pal.setColor( QColorGroup::Background, kcalcdefaults.backcolor );
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

  QPalette numPal( kcalcdefaults.numberButtonColor, bg );
  for( QPushButton *p = mNumButtonList.first(); p; p=mNumButtonList.next() )
  {
    p->setPalette(numPal);
  }

  QPalette funcPal( kcalcdefaults.functionButtonColor, bg );
  for( QPushButton *p = mFunctionButtonList.first(); p;
       p=mFunctionButtonList.next() )
  {
    p->setPalette(funcPal);
  }

  QPalette hexPal( kcalcdefaults.hexButtonColor, bg );
  for( QPushButton *p = mHexButtonList.first(); p; p=mHexButtonList.next() )
  {
    p->setPalette(hexPal);
  }

  QPalette memPal( kcalcdefaults.memoryButtonColor, bg );
  for( QPushButton *p = mMemButtonList.first(); p; p=mMemButtonList.next() )
  {
    p->setPalette(memPal);
  }

  QPalette opPal( kcalcdefaults.operationButtonColor, bg );
  for( QPushButton *p = mOperationButtonList.first(); p;
       p=mOperationButtonList.next() )
  {
    p->setPalette(opPal);
  }


}

void QtCalculator::set_precision(){

  UpdateDisplay();
}

void QtCalculator::set_display_font(){

  calc_display->setFont(kcalcdefaults.font);

}

void QtCalculator::temp_stack_next(){

  CALCAMNT *number;

  if( temp_stack.current() == temp_stack.getLast()){
        KNotifyClient::beep();
	return;
  }

  number = temp_stack.next();

  if(number == NULL){
       KNotifyClient::beep();
    return;
  }
  else{
    last_input = RECALL;
    DISPLAY_AMOUNT = *number;
    UpdateDisplay();

  }
}

void QtCalculator::temp_stack_prev(){

  CALCAMNT *number;

  if( temp_stack.current() == temp_stack.getFirst()){
        KNotifyClient::beep();
	return;
  }

  number = temp_stack.prev();

  if(number == NULL){
    KNotifyClient::beep();
    return;
  }
  else{
    //    printf("Number: %Lg\n",*number);
    last_input = RECALL;
    DISPLAY_AMOUNT = *number;
    UpdateDisplay();

  }
}


bool QtCalculator::eventFilter( QObject *o, QEvent *e )
{
  if( e->type() == QEvent::DragEnter )
  {
    QDragEnterEvent *ev = (QDragEnterEvent *)e;
    ev->accept( KColorDrag::canDecode(ev) );
    return true;
  }
  else if( e->type() == QEvent::DragLeave )
  {
    return true;
  }
  else if( e->type() == QEvent::Drop )
  {
    if( !o->isA("QPushButton") )
    {
      return false;
    }

    QColor c;
    QDropEvent *ev = (QDropEvent *)e;
    if( KColorDrag::decode( ev, c))
    {
      QPtrList<QPushButton> *list;
      if( mNumButtonList.findRef( (QPushButton*)o ) != -1 )
      {
	list = &mNumButtonList;
      }
      else if( mFunctionButtonList.findRef( (QPushButton*)o ) != -1 )
      {
	list = &mFunctionButtonList;
      }
      else if( mHexButtonList.findRef( (QPushButton*)o ) != -1 )
      {
	list = &mHexButtonList;
      }
      else if( mMemButtonList.findRef( (QPushButton*)o ) != -1 )
      {
	list = &mMemButtonList;
      }
      else if( mOperationButtonList.findRef( (QPushButton*)o ) != -1 )
      {
	list = &mOperationButtonList;
      }
      else
      {
	return false;
      }

      QPalette pal( c, palette().active().background() );
      for( QPushButton *p = list->first(); p; p=list->next() )
      {
	p->setPalette(pal);
      }
    }
    return true;
  }
  else
  {
    return KDialog::eventFilter( o, e );
  }
}


////////////////////////////////////////////////////////////////
// Include the meta-object code for classes in this file
//

#include "kcalc.moc"


/////////////////////////////////////////////////////////////////
// Create and display our WidgetView.
//

int main( int argc, char **argv )
{
  KAboutData aboutData( "kcalc", I18N_NOOP("KCalc"),
    version, description, KAboutData::License_GPL,
    "(c) 1996-2000, Bernd Johannes Wuebben");
  aboutData.addAuthor("Bernd Johannes Wuebben", 0, "wuebben@kde.org");
  aboutData.addAuthor("Charles Samuels", "Support for the new Backspace Action", "charles@altair.dhs.org");
  KCmdLineArgs::init( argc, argv, &aboutData );

  KApplication app;
#if 0
  app->enableSessionManagement(TRUE);
  app->setWmCommand(argv[0]);
#endif

  QtCalculator  *calc = new QtCalculator;
  app.setTopWidget( calc );
  calc->setCaption( QString::null );
  calc->show();

  int exitCode = app.exec();

  return( exitCode );
}
