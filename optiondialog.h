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

#ifndef _OPTION_DIALOG_H_
#define _OPTION_DIALOG_H_ 

class QCheckBox;
class QLabel;
class QRadioButton;
class QSpinBox;
class KColorButton;
class KFontChooser;

#include <kdialogbase.h>

#include "kcalc.h"

class ConfigureDialog : public KDialogBase
{
  Q_OBJECT
  
  public:
    ConfigureDialog( QWidget *parent=0, char *name=0, bool modal=true );
    ~ConfigureDialog( void );

    void setState( const DefStruct &state );
    DefStruct state( void );
    
  protected slots:
    virtual void slotOk( void );
    virtual void slotApply( void ); 
    virtual void slotCancel( void ); 

  private:
    void setupSettingPage( void );
    void setupFontPage( void );
    void setupAboutPage( void );

  private:
    KColorButton *mFgColor;
    KColorButton *mBgColor;
    QCheckBox    *mFixCheck;
    QSpinBox     *mPrecSpin;
    QSpinBox     *mFixSpin;
    QCheckBox    *mBeepCheck;
    QRadioButton *mTrigRadio;
    QRadioButton *mStatRadio;
    KFontChooser *mFontChooser;

    DefStruct    mState;

  signals:
    void valueChanged( const DefStruct &state );
};

#endif
