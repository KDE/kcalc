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
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qspinbox.h>

#include <kcolorbutton.h>
#include <kfontdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include "optiondialog.h"

// little helper:
static inline QPixmap loadIcon( const char * name )
{
  return KGlobal::instance()->iconLoader()
      ->loadIcon( QString::fromLatin1(name), KIcon::NoGroup, KIcon::SizeMedium );
}

//-------------------------------------------------------------------------
// Name: ConfigureDialog(QWidget *parent, char *name, bool modal)
//-------------------------------------------------------------------------
ConfigureDialog::ConfigureDialog(QWidget *parent, char *name, bool modal)
	: KDialogBase( IconList, i18n("Configure KCalc"), Help|Apply|Ok|Cancel,
		Ok, parent, name, modal, true )
{
	setHelp("kcalc/index.html", QString::null);

	setupSettingPage();
	setupFontPage();
	setupColorPage();
}

//-------------------------------------------------------------------------
// Name: ~ConfigureDialog()
//-------------------------------------------------------------------------
ConfigureDialog::~ConfigureDialog()
{
}

//-------------------------------------------------------------------------
// Name: setState(const DefStruct &state)
//-------------------------------------------------------------------------
void ConfigureDialog::setState(const DefStruct &state)
{
	mState = state;

	mColorFGround->setColor(state.forecolor);
	mColorBGround->setColor(state.backcolor);
	mColorFunctions->setColor(state.functionButtonColor);
	mColorHexa->setColor(state.hexButtonColor);
	mColorNumbers->setColor(state.numberButtonColor);
	mColorMemory->setColor(state.memoryButtonColor);
	mColorOperations->setColor(state.operationButtonColor);

	mFixCheck->setChecked(mState.fixed);
	mPrecSpin->setValue(QMIN( mPrecSpin->maxValue(), mState.precision ));
	mFixSpin->setValue(mState.fixedprecision);
	mBeepCheck->setChecked(mState.beep);
	mCaptionResult->setChecked(mState.capres);
	mTrigRadio->setChecked(mState.style == 0);
	mStatRadio->setChecked(mState.style == 1);  
	mFontChooser->setFont(mState.font);

    kdDebug() << "mState.font.weight(): " << mState.font.weight() << endl;
    kdDebug() << "mFontChooser->font().weight(): " << mFontChooser->font().weight() << endl;

    fixCheckToggled( mState.fixed );
}

//-------------------------------------------------------------------------
// Name: state()
//-------------------------------------------------------------------------
DefStruct ConfigureDialog::state()
{
	DefStruct state;

	state.forecolor            = mColorFGround->color();
	state.backcolor            = mColorBGround->color();
	state.functionButtonColor  = mColorFunctions->color();
	state.hexButtonColor       = mColorHexa->color();
	state.numberButtonColor    = mColorNumbers->color();
	state.memoryButtonColor    = mColorMemory->color();
	state.operationButtonColor = mColorOperations->color();

	state.fixed          = mFixCheck->isChecked();
	state.precision      = mPrecSpin->value();
	state.fixedprecision = mFixSpin->value();
	state.beep           = mBeepCheck->isChecked();
	state.capres         = mCaptionResult->isChecked();
	state.style          = mTrigRadio->isChecked() ? 0 : 1;
	state.font           = mFontChooser->font();

	return state;
}

//-------------------------------------------------------------------------
// Name: slotOk()
//-------------------------------------------------------------------------
void ConfigureDialog::slotOk()
{
	slotApply();
	accept();
}

//-------------------------------------------------------------------------
// Name: slotApply()
//-------------------------------------------------------------------------
void ConfigureDialog::slotApply()
{
	mState = state();
	emit valueChanged(mState);
}

//-------------------------------------------------------------------------
// Name: slotCancel()
//-------------------------------------------------------------------------
void ConfigureDialog::slotCancel()
{
	//setState(mState);
	//emit valueChanged(mState);
	reject();
}

//-------------------------------------------------------------------------
// Name: setupSettingPage()
//-------------------------------------------------------------------------
void ConfigureDialog::setupSettingPage()
{
    QFrame *page = addPage(i18n("General"), i18n("General Settings"), loadIcon("kcalc"));

    if(!page)
        return;

#ifdef HAVE_LONG_DOUBLE 
    int maxprec = 16;	
#else 
    int maxprec = 12 ;
#endif 

    QVBoxLayout* Form1Layout = new QVBoxLayout( page, 0, spacingHint() ); 

    QGroupBox* GroupBox2 = new QGroupBox( page, "GroupBox2" );
    GroupBox2->setTitle( i18n( "Precision" ) );
    GroupBox2->setColumnLayout(0, Qt::Vertical );
    QGridLayout* GroupBox2Layout = new QGridLayout( GroupBox2->layout(),
            3, 4, spacingHint() );

    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Fixed, 
            QSizePolicy::Minimum );
    GroupBox2Layout->addItem( spacer, 2, 0 );

    mFixSpin = new QSpinBox(0, 10, 1, GroupBox2, "mFixSpin" );
    mFixSpin->setValue( 2 );

    GroupBox2Layout->addMultiCellWidget( mFixSpin, 2, 2, 2, 3 );

    mPrecSpin = new QSpinBox(0, maxprec, 1, GroupBox2, "mPrecSpin" );

    GroupBox2Layout->addWidget( mPrecSpin, 0, 3 );

    TextLabel2 = new QLabel( GroupBox2, "TextLabel2" );
    TextLabel2->setText( i18n( "Decimal &places:" ) );

    GroupBox2Layout->addWidget( TextLabel2, 2, 1 );

    mFixCheck = new QCheckBox( GroupBox2, "mFixCheck" );
    mFixCheck->setText( i18n( "Set &decimal precision" ) );

    GroupBox2Layout->addMultiCellWidget( mFixCheck, 1, 1, 0, 2 );

    QLabel* TextLabel1 = new QLabel( GroupBox2, "TextLabel1" );
    TextLabel1->setText( i18n( "&Maximum number of digits:" ) );

    GroupBox2Layout->addMultiCellWidget( TextLabel1, 0, 0, 0, 2 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, 
            QSizePolicy::Minimum );
    GroupBox2Layout->addItem( spacer_2, 1, 4 );
    Form1Layout->addWidget( GroupBox2 );

    QButtonGroup* GroupBox1 = new QButtonGroup( page, "GroupBox1" );
    GroupBox1->setTitle( i18n( "Mode" ) );
    GroupBox1->setColumnLayout(0, Qt::Vertical );
    GroupBox1->layout()->setSpacing( KDialog::spacingHint() );
    GroupBox1->layout()->setMargin( KDialog::marginHint() );
    QVBoxLayout* GroupBox1Layout = new QVBoxLayout( GroupBox1->layout() );
    GroupBox1Layout->setAlignment( Qt::AlignTop );

    mTrigRadio = new QRadioButton( GroupBox1, "mTrigRadio" );
    mTrigRadio->setText( i18n( "&Trigonometric" ) );
    mTrigRadio->setChecked( TRUE );
    GroupBox1Layout->addWidget( mTrigRadio );

    mStatRadio = new QRadioButton( GroupBox1, "mStatRadio" );
    mStatRadio->setText( i18n( "&Statistical" ) );
    GroupBox1Layout->addWidget( mStatRadio );
    Form1Layout->addWidget( GroupBox1 );

    QGroupBox* GroupBox3 = new QGroupBox( page, "GroupBox3" );
    GroupBox3->setTitle( i18n( "Misc" ) );
    GroupBox3->setColumnLayout(0, Qt::Vertical );
    GroupBox3->layout()->setSpacing( KDialog::spacingHint() );
    GroupBox3->layout()->setMargin( KDialog::marginHint() );
    QVBoxLayout* GroupBox3Layout = new QVBoxLayout( GroupBox3->layout() );
    GroupBox3Layout->setAlignment( Qt::AlignTop );

    mBeepCheck = new QCheckBox( GroupBox3, "mBeepCheck" );
    mBeepCheck->setText( i18n( "&Beep on error" ) );
    GroupBox3Layout->addWidget( mBeepCheck );
    mCaptionResult = new QCheckBox( GroupBox3, "mCaptionResult" );
    mCaptionResult->setText( i18n( "Show &result in window title" ) );
    GroupBox3Layout->addWidget( mCaptionResult );
    Form1Layout->addWidget( GroupBox3 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, 
            QSizePolicy::Expanding );
    Form1Layout->addItem( spacer_3 );

    // signals and slots connections
    connect( mFixCheck, SIGNAL( toggled( bool ) ), 
            SLOT( fixCheckToggled( bool ) ) );

    // buddies
    TextLabel2->setBuddy( mFixSpin );
    TextLabel1->setBuddy( mPrecSpin );
}

void ConfigureDialog::fixCheckToggled( bool b )
{
    TextLabel2->setEnabled( b );
    mFixSpin->setEnabled( b );
}

//-------------------------------------------------------------------------
// Name: setupColorPage()
//-------------------------------------------------------------------------
void ConfigureDialog::setupColorPage()
{
    QFrame *page = addPage(i18n("Colors"), i18n("Button & Display Colors"), loadIcon("colors"));
    QVBoxLayout *topLayout = new QVBoxLayout(page, 0, spacingHint());

    QGroupBox* displayGroup = new QGroupBox(i18n("Display Colors"), page);

    displayGroup->setColumnLayout(0, Qt::Horizontal);
    QGridLayout *displayGrid = new QGridLayout(displayGroup->layout(),
            2, 2, spacingHint());

    QLabel* colorLable = new QLabel(i18n("&Foreground:"), displayGroup);
    mColorFGround = new KColorButton(displayGroup);
    colorLable->setBuddy(mColorFGround);
    displayGrid->addWidget(colorLable, 0, 0);
    displayGrid->addWidget(mColorFGround, 0, 1);

    colorLable = new QLabel(i18n("&Background:"), displayGroup);
    mColorBGround = new KColorButton(displayGroup);
    colorLable->setBuddy(mColorBGround);
    displayGrid->addWidget(colorLable, 1, 0);
    displayGrid->addWidget(mColorBGround, 1, 1);

    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, 
            QSizePolicy::Minimum );
    displayGrid->addItem(spacer, 1, 2);


    QGroupBox* buttonGroup = new QGroupBox(i18n("Button Colors"), page);

    buttonGroup->setColumnLayout(0, Qt::Horizontal);
    QGridLayout *buttonGrid = new QGridLayout(buttonGroup->layout(), 
            5, 2, spacingHint());

    colorLable = new QLabel(i18n("&Functions:"), buttonGroup);
    mColorFunctions = new KColorButton(buttonGroup);
    colorLable->setBuddy(mColorFunctions);
    buttonGrid->addWidget(colorLable, 0, 0);
    buttonGrid->addWidget(mColorFunctions, 0, 1);

    colorLable = new QLabel(i18n("He&xadecimals:"), buttonGroup);
    mColorHexa = new KColorButton(buttonGroup);
    colorLable->setBuddy(mColorHexa);
    buttonGrid->addWidget(colorLable, 1, 0);
    buttonGrid->addWidget(mColorHexa, 1, 1);

    colorLable = new QLabel(i18n("&Numbers:"), buttonGroup);
    mColorNumbers = new KColorButton(buttonGroup);
    colorLable->setBuddy(mColorNumbers);
    buttonGrid->addWidget(colorLable, 2, 0);
    buttonGrid->addWidget(mColorNumbers, 2, 1);

    colorLable = new QLabel(i18n("&Memory:"), buttonGroup);
    mColorMemory = new KColorButton(buttonGroup);
    colorLable->setBuddy(mColorMemory);
    buttonGrid->addWidget(colorLable, 3, 0);
    buttonGrid->addWidget(mColorMemory, 3, 1);

    colorLable = new QLabel(i18n("O&perations:"), buttonGroup);
    mColorOperations = new KColorButton(buttonGroup);
    colorLable->setBuddy(mColorOperations);
    buttonGrid->addWidget(colorLable, 4, 0);
    buttonGrid->addWidget(mColorOperations, 4, 1);

    spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, 
            QSizePolicy::Minimum );
    buttonGrid->addItem(spacer, 1, 2);

    topLayout->addWidget(displayGroup);
    topLayout->addWidget(buttonGroup);
    topLayout->addStretch( 1 );
}

//-------------------------------------------------------------------------
// Name: setupFontPage()
//-------------------------------------------------------------------------
void ConfigureDialog::setupFontPage()
{
	QFrame *page = addPage(i18n("Font"), i18n("Select Display Font"), loadIcon("fonts"));
	if( page == 0 )
		return;

	QVBoxLayout *topLayout = new QVBoxLayout(page, 0, spacingHint());
	if( topLayout == 0 )
		return;

	mFontChooser = new KFontChooser(page, "fonts", 
		false, QStringList(), false, 6);
		
	topLayout->addWidget(mFontChooser);
	topLayout->activate();
}

#include "optiondialog.moc"
