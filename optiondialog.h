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
#include <klistbox.h>

#include "kcalc.h"

class ConfigureDialog : public KDialogBase
{
	Q_OBJECT
public:
	ConfigureDialog(QWidget *parent = 0, char *name = 0, bool modal = true);
	~ConfigureDialog();

	void setState(const DefStruct &state);
	DefStruct state();

protected slots:
	void slotOk();
	void slotApply(); 
	void slotCancel(); 
    void fixCheckToggled( bool );

private:
	void setupSettingPage();
	void setupFontPage();
	void setupColorPage();

private:
	QCheckBox    *mFixCheck;
	QSpinBox     *mPrecSpin;
	QSpinBox     *mFixSpin;
	QCheckBox    *mBeepCheck;
	QCheckBox    *mCaptionResult;
	QRadioButton *mTrigRadio;
	QRadioButton *mStatRadio;
    QLabel       *TextLabel2;
	KFontChooser *mFontChooser;
	KColorButton *mColorFGround; 
    KColorButton *mColorBGround;
    KColorButton *mColorFunctions;
    KColorButton *mColorHexa;
    KColorButton *mColorNumbers;
    KColorButton *mColorMemory;
    KColorButton *mColorOperations;

	DefStruct    mState;

signals:
	void valueChanged(const DefStruct &state);
};

#endif
