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
class KFontChooser;

#include <kdialogbase.h>
#include <klistbox.h>

#include "kcalc.h"



class ColorListBox : public KListBox
{
  Q_OBJECT

  public:
    ColorListBox( QWidget *parent=0, const char * name=0, WFlags f=0 );
    void setColor( uint index, const QColor &color );
    const QColor color( uint index );

  public slots:
    virtual void setEnabled( bool state );

  protected:
    void dragEnterEvent( QDragEnterEvent *e );
    void dragLeaveEvent( QDragLeaveEvent *e );
    void dragMoveEvent( QDragMoveEvent *e );
    void dropEvent( QDropEvent *e );

  private slots:
    void newColor( int index );

  private:
    int mCurrentOnDragEnter;

};


class ColorListItem : public QListBoxItem
{
  public:
    ColorListItem( const QString &text, const QColor &color=Qt::black );
    const QColor &color( void );
    void  setColor( const QColor &color );
  
  protected:
    virtual void paint( QPainter * );
    virtual int height( const QListBox * ) const;
    virtual int width( const QListBox * ) const;

  private:
    QColor mColor;
    int mBoxWidth;
};




class ConfigureDialog : public KDialogBase
{
	Q_OBJECT
public:
	ConfigureDialog(QWidget *parent = 0, char *name = 0, bool modal = true);
	~ConfigureDialog();

	void setState(const DefStruct &state);
	DefStruct state();

protected slots:
	virtual void slotOk();
	virtual void slotApply(); 
	virtual void slotCancel(); 

private:
	void setupSettingPage();
	void setupFontPage();
	void setupColorPage();
	void setupAboutPage();

private:
	QCheckBox    *mFixCheck;
	QSpinBox     *mPrecSpin;
	QSpinBox     *mFixSpin;
	QCheckBox    *mBeepCheck;
	QRadioButton *mTrigRadio;
	QRadioButton *mStatRadio;
	KFontChooser *mFontChooser;
	ColorListBox *mColorList; 

	DefStruct    mState;

signals:
	void valueChanged(const DefStruct &state);
};

#endif
