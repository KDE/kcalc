/*
 *   kcalc - KDE calculator
 *   This file only: Copyright (C) 1999  Espen Sand, espensa@online.no
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qspinbox.h>

#include <kcolorbtn.h>
#include <kfontdialog.h>
#include <kiconloader.h>
#include <klocale.h>

#include "optiondialog.h"
#include "version.h"

//
// Undefine HAVE_LONG_DOUBLE for Beta 4 since RedHat 5.0 comes with a broken
// glibc

//#ifdef HAVE_LONG_DOUBLE
//#undef HAVE_LONG_DOUBLE
//#endif


ConfigureDialog::ConfigureDialog( QWidget *parent, char *name, bool modal )
  :KDialogBase( Tabbed, i18n("Configuration"), Help|Apply|Ok|Cancel,
		Ok, parent, name, modal )
{
  setHelp( "kcalc/index.html", QString::null );
 
  setupSettingPage();
  setupFontPage();
  setupAboutPage();
}

ConfigureDialog::~ConfigureDialog( void )
{
}


void ConfigureDialog::setState( const DefStruct &state )
{
  mState = state;

  mFgColor->setColor( mState.forecolor );
  mBgColor->setColor( mState.backcolor );
  mFixCheck->setChecked( mState.fixed );
  mPrecSpin->setValue( QMIN( mPrecSpin->maxValue(), mState.precision ) );
  mFixSpin->setValue( mState.fixedprecision );
  mBeepCheck->setChecked( mState.beep );
  mTrigRadio->setChecked( mState.style == 0 );
  mStatRadio->setChecked( mState.style == 1 );  
  mFontChooser->setFont( mState.font );
}


DefStruct ConfigureDialog::state( void )
{
  DefStruct state;

  state.forecolor      = mFgColor->color();
  state.backcolor      = mBgColor->color();
  state.fixed          = mFixCheck->isChecked();
  state.precision      = mPrecSpin->value();
  state.fixedprecision = mFixSpin->value();
  state.beep           = mBeepCheck->isChecked();
  state.style          = mTrigRadio->isChecked() ? 0 : 1;
  state.font           = mFontChooser->font();

  return( state );
}


void ConfigureDialog::slotOk( void )  
{
  slotApply();
  accept();
}


void ConfigureDialog::slotApply( void )  
{
  mState = state();
  emit valueChanged( mState );
}


void ConfigureDialog::slotCancel( void )  
{
  setState( mState );
  emit valueChanged( mState );
  reject();
}


void ConfigureDialog::setupSettingPage( void )
{
  QFrame *page = addPage( i18n("&Settings") );
  if( page == 0 ) { return; }
  
  QGridLayout *topLayout = new QGridLayout( page, 9, 2, 0, spacingHint() );
  
  QLabel *label = new QLabel( i18n("Foregrond color"), page );
  topLayout->addWidget( label, 0, 0 );
  label = new QLabel( i18n("Backgrond color"), page );
  topLayout->addWidget( label, 1, 0 );

  mFgColor = new KColorButton( page );
  mFgColor->setFixedWidth( fontMetrics().maxWidth() * 8 );
  topLayout->addWidget( mFgColor, 0, 1, AlignLeft );
  mBgColor = new KColorButton( page );
  mBgColor->setFixedWidth( fontMetrics().maxWidth() * 8 );
  topLayout->addWidget( mBgColor, 1, 1, AlignLeft );

  topLayout->addRowSpacing( 2, spacingHint() );

  label = new QLabel( i18n("Precision"), page );
  topLayout->addWidget( label, 3, 0 );
  mFixCheck = new QCheckBox( page );
  mFixCheck->setText(i18n("Set fixed precision") );
  topLayout->addWidget( mFixCheck, 4, 0 );

  int maxprec;
#ifdef HAVE_LONG_DOUBLE 
  maxprec = 16 ;
#else 
  maxprec = 12 ;
#endif 

  mPrecSpin = new QSpinBox(0, maxprec, 1, page );
  mPrecSpin->setFixedWidth( fontMetrics().maxWidth() * 4 );
  topLayout->addWidget( mPrecSpin, 3, 1, AlignLeft );
  mFixSpin = new QSpinBox(0, 10, 1, page );
  mFixSpin->setFixedWidth( fontMetrics().maxWidth() * 4 );
  topLayout->addWidget( mFixSpin, 4, 1, AlignLeft );

  mBeepCheck = new QCheckBox( page );
  mBeepCheck->setText(i18n("Beep on error") );
  topLayout->addWidget( mBeepCheck, 5, 0 );

  topLayout->addRowSpacing( 6, spacingHint() );

  QButtonGroup *group = new QButtonGroup( page, "stylegroup" );
  group->setFrameStyle(QFrame::NoFrame);
  topLayout->addWidget( group, 7, 0 );

  QVBoxLayout *vbox = new QVBoxLayout( group, 0, spacingHint() );

  mTrigRadio = new QRadioButton( group,"trigstyle" );
  mTrigRadio->setText(i18n("Trigonometry Mode"));
  vbox->addWidget(mTrigRadio);

  mStatRadio = new QRadioButton( group,"statstyle" );
  mStatRadio->setText(i18n("Statistical Mode"));
  vbox->addWidget(mStatRadio);
  
  topLayout->setRowStretch(8, 10);
  topLayout->activate();
}


void ConfigureDialog::setupFontPage( void )
{
  QFrame *page = addPage( i18n("&Display Font") );
  if( page == 0 ) { return; }

  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );
  if( topLayout == 0 ) { return; }

  mFontChooser = new KFontChooser(page,"fonts",false,QStringList(),false,6);
  topLayout->addWidget( mFontChooser );
  topLayout->activate();
}



void ConfigureDialog::setupAboutPage( void )
{
  QFrame *page = addPage( i18n("About &KCalc") );
  if( page == 0 ) { return; }

  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );
  if( topLayout == 0 ) { return; }

  QString authorText = i18n(""
    "KCalc %1\n"
    "Bernd Johannes Wuebben\n"
    "wuebben@math.cornell.edu\n"
    "wuebben@kde.org\n"
    "Copyright (C) 1996-98").arg(KCALCVERSION);
  
#ifdef HAVE_LONG_DOUBLE
  QString baseText = i18n("Base type: long double");
#else 
  QString baseText = i18n(""
    "Due to broken glibc's everywhere, I had to\n"
    "reduce KCalc's precision from 'long double'\n"
    "to 'double'. Owners of systems with a\n"
    "working libc should recompile KCalc with\n"
    "'long double' precision enabled. See the\n"
    "README for details.");

#endif

  topLayout->addSpacing( spacingHint() );

  QHBoxLayout *hbox = new QHBoxLayout();
  topLayout->addLayout(hbox);

  hbox->addSpacing( spacingHint() );

  QLabel *logo = new QLabel(page);
  logo->setPixmap( BarIcon("kcalclogo") );
  hbox->addWidget( logo );

  QLabel *label = new QLabel( authorText, page );
  hbox->addWidget( label, 10, AlignHCenter );
  label->setAlignment( AlignVCenter );

  label = new QLabel( baseText, page );
  topLayout->addWidget( label, 10, AlignHCenter );
  label->setAlignment( AlignVCenter );

  topLayout->activate();
}


#include "optiondialog.moc"
