/*
    $Id$

    kCalculator, a simple scientific calculator for KDE
    
    Copyright (C) 1996 Bernd Johannes Wuebben wuebben@math.cornell.edu

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
#include <stdio.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

#include "kcalc.h"
#include "configdlg.h"
#include "fontdlg.h"
#include "version.h"
#include <kconfig.h>
#include <qlayout.h>

// Undefine HAVE_LONG_DOUBLE for Beta 4 since RedHat 5.0 comes with a borken
// glibc

//#ifdef HAVE_LONG_DOUBLE
//#undef HAVE_LONG_DOUBLE
//#endif

extern last_input_type last_input;
extern item_contents   display_data;
extern num_base        current_base;
KApplication           *mykapp;

QList<CALCAMNT>       temp_stack; 

// ported to QLayout (mosfet 10/28/99)
QtCalculator :: QtCalculator( QWidget *parent, const char *name )
  : QDialog( parent, name )
{
  int u = 0;

  bigbuttonwidth 	= 30;
  bigbuttonheight 	= 23;
  smallbuttonwidth 	= 30;
  smallbuttonheight 	= 20;
  helpbuttonwidth 	= 100;
  helpbuttonheight 	= 25;
  displaywidth 		= 233;
  displayheight 	= helpbuttonheight;

  key_pressed = false;
  selection_timer = new QTimer;
  status_timer = new QTimer;

  connect(status_timer,SIGNAL(timeout()),this,SLOT(clear_status_label()));
  connect(selection_timer,SIGNAL(timeout()),this,SLOT(selection_timed_out()));

  readSettings();

  //QFont buttonfont( "-misc-fixed-medium-*-semicondensed-*-13-*-*-*-*-*-*-*" );
  QFont buttonfont = font();
  buttonfont.setPointSize(12);
  buttonfont.setRawMode( true );

  // Set the window caption/title

  connect(mykapp,SIGNAL(kdisplayPaletteChanged()),this,SLOT(set_colors()));

  setCaption( mykapp->caption() );

  // create help button

  QPushButton *pb;

  pb = new QPushButton( this, "helpbutton" );
  pb->setText( "kCalc" );
  pb->setMinimumSize(helpbuttonwidth,helpbuttonheight );
  pb->setFont( QFont("times",12,QFont::Bold,FALSE) );   
  QToolTip::add( pb, i18n("KCalc Setup/Help") );

  connect( pb, SIGNAL(clicked()), SLOT(configclicked()) );

  // Create the display
    
  calc_display = new DLabel( this, "display" );
  calc_display->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
  calc_display->setAlignment( AlignRight|AlignVCenter );
  calc_display->setMinimumSize(displaywidth ,displayheight );
  calc_display->setFocus();
  calc_display->setFocusPolicy( QWidget::StrongFocus );

  connect(calc_display,SIGNAL(clicked()),this,SLOT(display_selected()));

  statusINVLabel = new QLabel( this, "INV" );
  CHECK_PTR( statusINVLabel );
  statusINVLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  statusINVLabel->setAlignment( AlignCenter );
  statusINVLabel->setMinimumSize(49, 20);
  statusINVLabel->setText("NORM");
  statusINVLabel->setFont(buttonfont);  
 
  statusHYPLabel = new QLabel( this, "HYP" );
  CHECK_PTR( statusHYPLabel );
  statusHYPLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  statusHYPLabel->setAlignment( AlignCenter );
  statusHYPLabel->setMinimumSize(49, 20);
  statusHYPLabel->setFont(buttonfont);  
    
  statusERRORLabel = new QLabel( this, "ERROR" );
  CHECK_PTR( statusERRORLabel );
  statusERRORLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  statusERRORLabel->setAlignment( AlignLeft|AlignVCenter );
  statusERRORLabel->setFont(QFont("Hevetica",12));  

  // create angle button group
  
  QAccel *accel = new QAccel( this );       
  accel->connectItem( accel->insertItem(Key_Q+ALT),this,SLOT(quitCalc()) );      
  accel->connectItem( accel->insertItem(Key_X+ALT),this,SLOT(quitCalc()) );      
  

  QButtonGroup *angle_group = new QButtonGroup( 3, Horizontal, this,
                                                "AngleButtons" );
  angle_group->setFont(buttonfont);
  
  angle_group->setTitle(i18n( "Angle") );
  
  anglebutton[0] = new QRadioButton( angle_group );
  anglebutton[0]->setText( "&Deg" )   ;
  anglebutton[0]->setChecked(	TRUE); 
  accel->connectItem( accel->insertItem(Key_D + ALT), this , 
			SLOT(Deg_Selected()) );       

  anglebutton[1] = new QRadioButton( angle_group );
  anglebutton[1]->setText( "&Rad" );
  accel->connectItem( accel->insertItem(Key_R + ALT), this , 
			SLOT(Rad_Selected()) );       

  anglebutton[2] = new QRadioButton( angle_group );
  anglebutton[2]->setText( "&Gra" );
  accel->connectItem( accel->insertItem(Key_G + ALT), this , 
			SLOT(Gra_Selected()) );       

  for(u = 0;u <3;u++) anglebutton[u]->setFont(buttonfont);
  
  connect( angle_group, SIGNAL(clicked(int)), SLOT(angle_selected(int)) );
  angle_group->setMinimumSize(angle_group->sizeHint());

//////////////////////////////////////////////////////////////////////
//
// Create Number Base Button Group
//



  QButtonGroup *base_group = new QButtonGroup(4, Horizontal,  this,
                                              "BaseButtons" );
    base_group->setFont(buttonfont);
    
    base_group->setTitle( i18n("Base") );
       
    basebutton[0] = new QRadioButton( base_group );
    basebutton[0]->setText( "&Hex" );
    accel->connectItem( accel->insertItem(Key_H + ALT), this , 
 		SLOT(Hex_Selected()) );       
    
    basebutton[1] = new QRadioButton( base_group );
    basebutton[1]->setText( "D&ec" );
    basebutton[1]->setChecked(TRUE);
    accel->connectItem( accel->insertItem(Key_E + ALT), this , 
			SLOT(Dec_Selected()) );       

    basebutton[2] = new QRadioButton( base_group );
    basebutton[2]->setText( "&Oct" );
    accel->connectItem( accel->insertItem(Key_O + ALT), this , 
                        SLOT(Oct_Selected()) );
   
    basebutton[3] = new QRadioButton( base_group);
    basebutton[3]->setText( "&Bin" );
    accel->connectItem( accel->insertItem(Key_B + ALT), this , 
			SLOT(Bin_Selected()) );       
    
    for(u = 0;u <4;u++) basebutton[u]->setFont(buttonfont);

    myxmargin = 9;
    connect( base_group, SIGNAL(clicked(int)), SLOT(base_selected(int)) );
    base_group->setMinimumSize(base_group->sizeHint());

////////////////////////////////////////////////////////////////////////
//
//  Create Calculator Buttons
//    
    
    pbhyp = new QPushButton( this, "hypbutton" );
    pbhyp->setText( "Hyp" );
    pbhyp->setMinimumSize(smallbuttonwidth, smallbuttonheight);
    connect( pbhyp, SIGNAL(toggled(bool)), SLOT(pbhyptoggled(bool)));
    pbhyp->setToggleButton(TRUE);
    pbhyp->setFont(buttonfont);

    pbinv = new QPushButton( this, "InverseButton" );
    pbinv->setText( "Inv" );
    pbinv->setMinimumSize(smallbuttonwidth, smallbuttonheight);
    connect( pbinv, SIGNAL(toggled(bool)), SLOT(pbinvtoggled(bool)));
    pbinv->setToggleButton(TRUE);
    pbinv->setFont(buttonfont);

    pbA = new QPushButton( this, "Abutton" );
    pbA->setText( "A" );
    pbA->setMinimumSize(smallbuttonwidth, smallbuttonheight);
    connect( pbA, SIGNAL(toggled(bool)), SLOT(pbAtoggled(bool)));
    pbA->setToggleButton(TRUE);    
    pbA->setFont(buttonfont);

    pbSin = new QPushButton( this, "Sinbutton" );
    pbSin->setText( "Sin" );
    pbSin->setMinimumSize(smallbuttonwidth, smallbuttonheight);
    connect( pbSin, SIGNAL(toggled(bool)), SLOT(pbSintoggled(bool)));
    pbSin->setToggleButton(TRUE);
    pbSin->setFont(buttonfont);

     
    pbplusminus = new QPushButton( this, "plusminusbutton" );
    pbplusminus->setText( "+/-" );
    pbplusminus->setMinimumSize(smallbuttonwidth, smallbuttonheight);
    connect( pbplusminus, SIGNAL(toggled(bool)), SLOT(pbplusminustoggled(bool)));
    pbplusminus->setToggleButton(TRUE);    
    pbplusminus->setFont(buttonfont);

    pbB = new QPushButton( this, "Bbutton" );
    pbB->setText( "B" );
    pbB->setMinimumSize(smallbuttonwidth, smallbuttonheight);
    connect( pbB, SIGNAL(toggled(bool)), SLOT(pbBtoggled(bool)));
    pbB->setToggleButton(TRUE);
    pbB->setFont(buttonfont);


    pbCos = new QPushButton( this, "Cosbutton" );
    pbCos->setText( "Cos" );
    pbCos->setMinimumSize(smallbuttonwidth, smallbuttonheight);
    connect( pbCos, SIGNAL(toggled(bool)), SLOT(pbCostoggled(bool)));
    pbCos->setToggleButton(TRUE);    
    pbCos->setFont(buttonfont);


    pbreci = new QPushButton( this, "recibutton" );
    pbreci->setText( "1/x" );
    pbreci->setMinimumSize(smallbuttonwidth, smallbuttonheight);
    connect( pbreci, SIGNAL(toggled(bool)), SLOT(pbrecitoggled(bool)));
    pbreci->setToggleButton(TRUE);    
    pbreci->setFont(buttonfont);

    pbC = new QPushButton( this, "Cbutton" );
    pbC->setText( "C" );
    pbC->setMinimumSize(smallbuttonwidth, smallbuttonheight);
    connect( pbC, SIGNAL(toggled(bool)), SLOT(pbCtoggled(bool)));
    pbC->setToggleButton(TRUE);
    pbC->setFont(buttonfont);


    pbTan = new QPushButton( this, "Tanbutton" );
    pbTan->setText( "Tan" );
    pbTan->setMinimumSize(smallbuttonwidth, smallbuttonheight);
    pbTan->setToggleButton(TRUE);
    pbTan->setFont(buttonfont);

    pbfactorial = new QPushButton( this, "factorialbutton" );
    pbfactorial->setText( "x!" );
    pbfactorial->setMinimumSize(smallbuttonwidth, smallbuttonheight);
    connect( pbfactorial, SIGNAL(toggled(bool)), SLOT(pbfactorialtoggled(bool)));
    pbfactorial->setToggleButton(TRUE);
    pbfactorial->setFont(buttonfont);


    pbD = new QPushButton( this, "Dbutton" );
    pbD->setText( "D" );
    pbD->setMinimumSize(smallbuttonwidth, smallbuttonheight);
    connect( pbD, SIGNAL(toggled(bool)), SLOT(pbDtoggled(bool)));
    pbD->setToggleButton(TRUE);
    pbD->setFont(buttonfont);

    pblog = new QPushButton( this, "logbutton" );
    pblog->setText( "Log" );
    pblog->setMinimumSize(smallbuttonwidth, smallbuttonheight);
    pblog->setToggleButton(TRUE);
    pblog->setFont(buttonfont);
    
    pbsquare = new QPushButton( this, "squarebutton" );
    pbsquare->setText( "x^2" );
    pbsquare->setMinimumSize(smallbuttonwidth, smallbuttonheight);
    connect( pbsquare, SIGNAL(toggled(bool)), SLOT(pbsquaretoggled(bool)));
    pbsquare->setToggleButton(TRUE);
    pbsquare->setFont(buttonfont);

    pbE = new QPushButton( this, "Ebutton" );
    pbE->setText( "E" );
    pbE->setMinimumSize(smallbuttonwidth, smallbuttonheight);
    connect( pbE, SIGNAL(toggled(bool)), SLOT(pbEtoggled(bool)));
    pbE->setToggleButton(TRUE);
    pbE->setFont(buttonfont);

    pbln = new QPushButton( this, "lnbutton" );
    pbln->setText( "Ln" );
    pbln->setMinimumSize(smallbuttonwidth, smallbuttonheight);
    connect( pbln, SIGNAL(toggled(bool)), SLOT(pblntoggled(bool)));
    pbln->setToggleButton(TRUE);
    pbln->setFont(buttonfont);

    pbpower = new QPushButton( this, "powerbutton" );
    pbpower->setText( "x^y" );
    pbpower->setMinimumSize(smallbuttonwidth, smallbuttonheight);
    connect( pbpower, SIGNAL(toggled(bool)), SLOT(pbpowertoggled(bool)));
    pbpower->setToggleButton(TRUE);
    pbpower->setFont(buttonfont);

    pbF = new QPushButton( this, "Fbutton" );
    pbF->setText( "F" );
    pbF->setMinimumSize(smallbuttonwidth, smallbuttonheight);
    connect( pbF, SIGNAL(toggled(bool)), SLOT(pbFtoggled(bool)));
    pbF->setToggleButton(TRUE);
    pbF->setFont(buttonfont);

/////////////////////////////////////////////////////////////////////
//   
//


    pbEE = new QPushButton( this, "EEbutton" );
    pbEE->setText( "EE" );
    pbEE->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    pbEE->setToggleButton(TRUE);
    connect( pbEE, SIGNAL(toggled(bool)), SLOT(EEtoggled(bool)));
    pbEE->setFont(buttonfont);

    pbMR = new QPushButton( this, "MRbutton" );
    pbMR->setText( "MR" );
    pbMR->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pbMR, SIGNAL(toggled(bool)), SLOT(pbMRtoggled(bool)));
    pbMR->setToggleButton(TRUE);
    pbMR->setFont(buttonfont);

    pbMplusminus = new QPushButton( this, "Mplusminusbutton" );
    pbMplusminus->setText( "M+-" );
    pbMplusminus->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pbMplusminus, SIGNAL(toggled(bool)), SLOT(pbMplusminustoggled(bool)));
    pbMplusminus->setToggleButton(TRUE);
    pbMplusminus->setFont(buttonfont);

    pbMC = new QPushButton( this, "MCbutton" );
    pbMC->setText( "MC" );
    pbMC->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pbMC, SIGNAL(toggled(bool)), SLOT(pbMCtoggled(bool)));
    pbMC->setToggleButton(TRUE);
    pbMC->setFont(buttonfont);

    pbClear = new QPushButton( this, "Clearbutton" );
    pbClear->setText( "C" );
    pbClear->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pbClear, SIGNAL(toggled(bool)), SLOT(pbCleartoggled(bool)));
    pbClear->setToggleButton(TRUE);
    pbClear->setFont(buttonfont);

    pbAC = new QPushButton( this, "ACbutton" );
    pbAC->setText( "AC" );
    pbAC->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pbAC, SIGNAL(toggled(bool)), SLOT(pbACtoggled(bool)));
    pbAC->setToggleButton(TRUE);
    pbAC->setFont(buttonfont);


//////////////////////////////////////////////////////////////////////
//
//
  

    pb7 = new QPushButton( this, "7button" );
    pb7->setText( "7" );
    pb7->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pb7, SIGNAL(toggled(bool)), SLOT(pb7toggled(bool)));
    pb7->setToggleButton(TRUE);
    pb7->setFont(buttonfont);

    pb8 = new QPushButton( this, "8button" );
    pb8->setText( "8" );
    pb8->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pb8, SIGNAL(toggled(bool)), SLOT(pb8toggled(bool)));
    pb8->setToggleButton(TRUE);
    pb8->setFont(buttonfont);

    pb9 = new QPushButton( this, "9button" );
    pb9->setText( "9" );
    pb9->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pb9, SIGNAL(toggled(bool)), SLOT(pb9toggled(bool)));
    pb9->setToggleButton(TRUE);
    pb9->setFont(buttonfont);

    pbparenopen = new QPushButton( this, "parenopenbutton" );
    pbparenopen->setText( "(" );
    pbparenopen->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pbparenopen, SIGNAL(toggled(bool)), SLOT(pbparenopentoggled(bool)));
    pbparenopen->setToggleButton(TRUE);
    pbparenopen->setFont(buttonfont);

    pbparenclose = new QPushButton( this, "parenclosebutton" );
    pbparenclose->setText( ")" );
    pbparenclose->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pbparenclose, SIGNAL(toggled(bool)), SLOT(pbparenclosetoggled(bool)));
    pbparenclose->setToggleButton(TRUE);
    pbparenclose->setFont(buttonfont);

    pband = new QPushButton( this, "andbutton" );
    pband->setText( "And" );
    pband->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pband, SIGNAL(toggled(bool)), SLOT(pbandtoggled(bool)));
    pband->setToggleButton(TRUE);
    pband->setFont(buttonfont);

//////////////////////////////////////////////////////////////////////
//
//


    pb4 = new QPushButton( this, "4button" );
    pb4->setText( "4" );
    pb4->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pb4, SIGNAL(toggled(bool)), SLOT(pb4toggled(bool)));
    pb4->setToggleButton(TRUE);
    pb4->setFont(buttonfont);

    pb5 = new QPushButton( this, "5button" );
    pb5->setText( "5" );
    pb5->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pb5, SIGNAL(toggled(bool)), SLOT(pb5toggled(bool)));
    pb5->setToggleButton(TRUE);
    pb5->setFont(buttonfont);

    pb6 = new QPushButton( this, "6button" );
    pb6->setText( "6" );
    pb6->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pb6, SIGNAL(toggled(bool)), SLOT(pb6toggled(bool)));
    pb6->setToggleButton(TRUE);
    pb6->setFont(buttonfont);

    pbX = new QPushButton( this, "Multiplybutton" );
    pbX->setText( "X" );
    pbX->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pbX, SIGNAL(toggled(bool)), SLOT(pbXtoggled(bool)));
    pbX->setToggleButton(TRUE);
    pbX->setFont(buttonfont);

    pbdivision = new QPushButton( this, "divisionbutton" );
    pbdivision->setText( "/" );
    pbdivision->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pbdivision, SIGNAL(toggled(bool)), SLOT(pbdivisiontoggled(bool)));
    pbdivision->setToggleButton(TRUE);
    pbdivision->setFont(buttonfont);

    pbor = new QPushButton( this, "orbutton" );
    pbor->setText( "Or" );
    pbor->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pbor, SIGNAL(toggled(bool)), SLOT(pbortoggled(bool)));
    pbor->setToggleButton(TRUE);
    pbor->setFont(buttonfont);

/////////////////////////////////////////////////////////////////////////////
//
//


    pb1 = new QPushButton( this, "1button" );
    pb1->setText( "1" );
    pb1->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    //connect( pb1, SIGNAL(clicked()), SLOT(button1()) );    
    connect( pb1, SIGNAL(toggled(bool)), SLOT(pb1toggled(bool)));
    pb1->setToggleButton(TRUE);	
    pb1->setFont(buttonfont);
 
    pb2 = new QPushButton( this, "2button" );
    pb2->setText( "2" );
    pb2->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    
    connect( pb2, SIGNAL(toggled(bool)), SLOT(pb2toggled(bool)));
    pb2->setToggleButton(TRUE);		
    pb2->setFont(buttonfont);

    pb3 = new QPushButton( this, "3button" );
    pb3->setText( "3" );
    pb3->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pb3, SIGNAL(toggled(bool)), SLOT(pb3toggled(bool)));
    pb3->setToggleButton(TRUE);
    pb3->setFont(buttonfont);

    pbplus = new QPushButton( this, "plusbutton" );
    pbplus->setText( "+" );
    pbplus->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pbplus, SIGNAL(toggled(bool)), SLOT(pbplustoggled(bool)));
    pbplus->setToggleButton(TRUE);
    pbplus->setFont(buttonfont);

    
    pbminus = new QPushButton( this, "minusbutton" );
    pbminus->setText( "-" );
    pbminus->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pbminus, SIGNAL(toggled(bool)), SLOT(pbminustoggled(bool)));
    pbminus->setToggleButton(TRUE);
    pbminus->setFont(buttonfont);

    pbshift = new QPushButton( this, "shiftbutton" );
    pbshift->setText( "Lsh" );
    pbshift->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pbshift, SIGNAL(toggled(bool)), SLOT(pbshifttoggled(bool)));
    pbshift->setToggleButton(TRUE);
    pbshift->setFont(buttonfont);

///////////////////////////////////////////////////////////////////////////
//
//

    pbperiod = new QPushButton( this, "periodbutton" );
    pbperiod->setText( "." );
    pbperiod->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pbperiod, SIGNAL(toggled(bool)), SLOT(pbperiodtoggled(bool)));
    pbperiod->setToggleButton(TRUE);
    pbperiod->setFont(buttonfont);

    pb0 = new QPushButton( this, "0button" );
    pb0->setText( "0" );
    pb0->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pb0, SIGNAL(toggled(bool)), SLOT(pb0toggled(bool)));
    pb0->setToggleButton(TRUE);
    pb0->setFont(buttonfont);

    pbequal = new QPushButton( this, "equalbutton" );
    pbequal->setText( "=" );
    pbequal->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pbequal, SIGNAL(toggled(bool)), SLOT(pbequaltoggled(bool)));
    pbequal->setToggleButton(TRUE);
    pbequal->setFont(buttonfont);

    pbpercent = new QPushButton( this, "percentbutton" );
    pbpercent->setText( "%" );
    pbpercent->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pbpercent, SIGNAL(toggled(bool)), SLOT(pbpercenttoggled(bool)));
    pbpercent->setToggleButton(TRUE);
    pbpercent->setFont(buttonfont);


    pbnegate = new QPushButton( this, "OneComplementbutton" );
    pbnegate->setText( "Cmp" );
    pbnegate->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pbnegate, SIGNAL(toggled(bool)), SLOT(pbnegatetoggled(bool))); 
    pbnegate->setToggleButton(TRUE);
    pbnegate->setFont(buttonfont);

    pbmod = new QPushButton( this, "modbutton" );
    pbmod->setText( "Mod" );
    pbmod->setMinimumSize(bigbuttonwidth,bigbuttonheight);
    connect( pbmod, SIGNAL(toggled(bool)), SLOT(pbmodtoggled(bool))); 
    pbmod->setToggleButton(TRUE);
    pbmod->setFont(buttonfont);


    set_colors();
    set_display_font();
    set_precision();
    set_style();

    /*    paper = new QListBox(0,"paper");
    paper->resize(200,400);
    paper->show();*/
    InitializeCalculator();

    // All these layouts are needed because all the groups have their
    // own size per row so we can't use one huge QGridLayout (mosfet)
    QGridLayout *smallBtnLayout = new QGridLayout(6, 3, 4);
    QGridLayout *largeBtnLayout = new QGridLayout(5, 6, 4);
    QHBoxLayout *topLayout = new QHBoxLayout(4);
    QHBoxLayout *radioLayout = new QHBoxLayout(4);
    QHBoxLayout *btnLayout = new QHBoxLayout(4);
    QHBoxLayout *statusLayout = new QHBoxLayout(4);

    // bring them all together
    QVBoxLayout *mainLayout = new QVBoxLayout(this, 4);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(radioLayout);
    mainLayout->addLayout(btnLayout);
    mainLayout->addLayout(statusLayout);

    // button layout
    btnLayout->addLayout(smallBtnLayout);
    btnLayout->addSpacing(4);
    btnLayout->addLayout(largeBtnLayout);
    btnLayout->addStretch(1);

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

    // top layout
    topLayout->addWidget(pb);
    topLayout->addStretch(1);
    topLayout->addWidget(calc_display);

    // radiobutton layout
    radioLayout->addWidget(base_group);
    radioLayout->addWidget(angle_group);
    radioLayout->addStretch(1);


    // status layout
    statusLayout->addWidget(statusINVLabel);
    statusLayout->addWidget(statusHYPLabel);
    statusLayout->addWidget(statusERRORLabel, 1);

    mainLayout->activate(); // whew!
}

void QtCalculator::exit()
{
  QApplication::exit();
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


void QtCalculator::helpclicked(){

  mykapp->invokeHTMLHelp("","");

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
     key_pressed = TRUE;
     pbAC->setOn(TRUE);
     break;
  case Key_R:
     key_pressed = TRUE;
     pbreci->setOn(TRUE);
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

  if(myboolean)
    EE();
  if(pbEE->isOn() && (!key_pressed))
    pbEE->setOn(FALSE);
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
  if(myboolean)
    buttonE();
  if(pbE->isOn() && (!key_pressed))
    pbE->setOn(FALSE);
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

void QtCalculator::configclicked(){


  QTabDialog * tabdialog;
  tabdialog = new QTabDialog(0,"tabdialog",TRUE);

  tabdialog->setCaption( i18n("KCalc Configuraton") );
  tabdialog->resize( 350, 350 );
  tabdialog->setCancelButton( i18n("Cancel") );

  QWidget *about = new QWidget(tabdialog,"about");

  QGroupBox *box = new QGroupBox(about,"box");
  QLabel  *label = new QLabel(box,"label");
  QLabel  *label2 = new QLabel(box,"label2");
  box->setGeometry(10,10,320,260);

  box->setTitle(i18n("About"));


  label->setGeometry(140,30,160,170);
  label2->setGeometry(20,150,280,100);

  QString labelstring;
  labelstring = i18n("KCalc %1\n"
			   "Bernd Johannes Wuebben\n"
			   "wuebben@math.cornell.edu\n"
			   "wuebben@kde.org\n"
			   "Copyright (C) 1996-98\n"
			   "\n\n").arg(KCALCVERSION);

  QString labelstring2 =
#ifdef HAVE_LONG_DOUBLE
		i18n( "Base type: long double\n");
#else 
		i18n( "Due to broken glibc's everywhere, "\
		      "I had to reduce KCalc's precision from 'long double' "\
		      "to 'double'. "\
		      "Owners of systems with a working libc "\
		      "should recompile KCalc with 'long double' precision "\
		      "enabled. See the README for details.");
#endif 

  label->setAlignment(AlignLeft|WordBreak|ExpandTabs);
  label->setText(labelstring);

  label2->setAlignment(AlignLeft|WordBreak|ExpandTabs);
  label2->setText(labelstring2);
  
  QPixmap pm( BarIcon("kcalclogo"));
  QLabel *logo = new QLabel(box);
  logo->setPixmap(pm);
  logo->setGeometry(30, 20, pm.width(), pm.height());


  DefStruct newdefstruct;
  newdefstruct.forecolor  = kcalcdefaults.forecolor;
  newdefstruct.backcolor  = kcalcdefaults.backcolor;
  newdefstruct.font       = kcalcdefaults.font;
  newdefstruct.precision  = kcalcdefaults.precision;
  newdefstruct.fixedprecision  = kcalcdefaults.fixedprecision;
  newdefstruct.fixed  = kcalcdefaults.fixed;
  newdefstruct.style  = kcalcdefaults.style;
  newdefstruct.beep  = kcalcdefaults.beep;
  
  ConfigDlg *configdlg;
  configdlg = new ConfigDlg(tabdialog,"configdlg",mykapp,&newdefstruct);

  FontDlg* fontdlg;
  fontdlg = new FontDlg(tabdialog,"fontdlg",mykapp,&newdefstruct);

  tabdialog->addTab(configdlg,i18n("Defaults"));
  tabdialog->addTab(fontdlg,i18n("Display Font"));
  tabdialog->addTab(about,i18n("About"));


  if(tabdialog->exec() == QDialog::Accepted){


    kcalcdefaults.forecolor  = newdefstruct.forecolor;
    kcalcdefaults.backcolor  = newdefstruct.backcolor;
    kcalcdefaults.font       = newdefstruct.font;
    kcalcdefaults.precision  = newdefstruct.precision;
    kcalcdefaults.fixedprecision  = newdefstruct.fixedprecision;
    kcalcdefaults.fixed  = newdefstruct.fixed;
    kcalcdefaults.style  = newdefstruct.style;
    kcalcdefaults.beep  = newdefstruct.beep;

    set_colors();
    set_precision();
    set_display_font();
    set_style();
  }

}


void QtCalculator::set_style(){

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

  KConfig *config = mykapp->config();
  config->setGroup( "Font" );
    
  kcalcdefaults.font = config->readFontEntry("Font",
	new QFont("helvetica",16,QFont::Bold));

  config->setGroup("Colors");
  QColor tmpC(189, 255, 222);
  QColor blackC(0,0,0);

  kcalcdefaults.forecolor = config->readColorEntry("ForeColor",&blackC);
  kcalcdefaults.backcolor = config->readColorEntry("BackColor",&tmpC);

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

  KConfig *config = mykapp->config();		
  
  config->setGroup( "Font" );
  config->writeEntry("Font",kcalcdefaults.font);
  
  config->setGroup("Colors");
  config->writeEntry("ForeColor",kcalcdefaults.forecolor);
  config->writeEntry("BackColor",kcalcdefaults.backcolor);
  
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
    const char *text = cb->text().ascii();
    result = (CALCAMNT) strtod( text?text:"0",0);
    //    printf("%Lg\n",result);
    last_input = PASTE;
    DISPLAY_AMOUNT = result;
    UpdateDisplay();
  }

}

void QtCalculator::selection_timed_out(){

  //  printf("selection timed out\n");
  selection_timer->stop();
  calc_display->setLit(FALSE);
  invertColors();


}


void QtCalculator::clear_status_label(){

  statusERRORLabel->setText("");
  status_timer->stop();
}

void QtCalculator::setStatusLabel(const QString& string){

  statusERRORLabel->setText(string);
  status_timer->start(3000,TRUE);

}


void QtCalculator::invertColors(){

  QColor tmpcolor;

  if(calc_display->isLit()){
    tmpcolor = kcalcdefaults.backcolor;
    kcalcdefaults.backcolor = kcalcdefaults.forecolor;
    kcalcdefaults.forecolor = tmpcolor;
    set_colors();
    tmpcolor = kcalcdefaults.backcolor;
    kcalcdefaults.backcolor = kcalcdefaults.forecolor;
    kcalcdefaults.forecolor = tmpcolor;
  }
  else{
    set_colors();
    //printf("normal Colors\n");
  }
}

void QtCalculator::closeEvent( QCloseEvent * ){
  
  quitCalc();

}

void QtCalculator::quitCalc(){

  writeSettings();
  qApp->quit();

}

void QtCalculator::set_colors(){


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
        QApplication::beep();
	return;
  }

  number = temp_stack.next();

  if(number == NULL){
       QApplication::beep();
    return;
  }
  else{
    //    printf("Number: %Lg\n",*number);
    last_input = RECALL;
    DISPLAY_AMOUNT = *number;
    UpdateDisplay();

  }


}

void QtCalculator::temp_stack_prev(){

  CALCAMNT *number;

  if( temp_stack.current() == temp_stack.getFirst()){
        QApplication::beep();
	return;
  }

  number = temp_stack.prev();

  if(number == NULL){
    QApplication::beep();
    return;
  }
  else{
    //    printf("Number: %Lg\n",*number);
    last_input = RECALL;
    DISPLAY_AMOUNT = *number;
    UpdateDisplay();

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

    mykapp = new KApplication (argc, argv, "kcalc");

#if 0    
    mykapp->enableSessionManagement(TRUE);
    mykapp->setWmCommand(argv[0]);
#endif    

    if( argc >1 ){
      argv++;
      for(int i = 1; i <= argc ;i++){
	if(strcmp(*argv,"-v")==0 || strcmp(*argv, "-h")== 0){
	  printf(i18n("KCalc %s\n"
		      "Copyright 1997 Bernd Johannes Wuebben"
		      " <wuebben@kde.org>\n").ascii(), KCALCVERSION);
	  exit(1);
	}
	argv++;
      }

    }

    QtCalculator  *calc = new QtCalculator;

    mykapp->setTopWidget( calc );

    // calc->setGeometry(300, 100, 9 + 100 + 9 + 233 + 9, 220);   

    //calc->setFixedSize( 9 + 100 + 9 + 233 + 9, 239);
    calc->setFixedSize(calc->minimumSize());
    calc->show();
    return mykapp->exec();
}










