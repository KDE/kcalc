
/*

 $Id$

 KCalc

 Copyright (C) Bernd Johannes Wuebben
               wuebben@math.cornell.edu
	       wuebben@kde.org

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 */


#include <kmsgbox.h>

#include "configdlg.h"
#include "configdlg.moc"

ConfigDlg::ConfigDlg(QWidget *parent=0, const char *name=0,
		     KApplication *_mykapp=NULL,DefStruct *defstruct=NULL)
  : QDialog(parent, name)
{

    // Torben
  mykapp = _mykapp;
  defst = defstruct;



  box = new QGroupBox(this, "box");
  box->setGeometry(10,10,320,260);
  box->setTitle("Defaults");


  label1 = new QLabel(this);
  label1->setGeometry(30,40,135,25);
  label1->setText("Foreground Color:");


  qframe1 = new QFrame(this);
  qframe1->setGeometry(155,40,30,25);	
  qframe1->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
  qframe1->setBackgroundColor(defst->forecolor);


  button1 = new QPushButton(this);
  button1->setGeometry(205,40,100,25);
  button1->setText("Change");

  connect(button1,SIGNAL(clicked()),this,SLOT(set_fore_color()));

  label2 = new QLabel(this);
  label2->setGeometry(30,75,135,25);
  label2->setText("Background Color:");


  qframe2 = new QFrame(this);
  qframe2->setGeometry(155,75,30,25);	
  qframe2->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
  qframe2->setBackgroundColor(defst->backcolor);

  button2 = new QPushButton(this);
  button2->setGeometry(205,75,100,25);
  button2->setText("Change");

  connect(button2,SIGNAL(clicked()),this,SLOT(set_background_color()));


  label5 = new QLabel(this);
  label5->setGeometry(30,115,100,25);
  label5->setText("Precision:");

  int maxprec;
#ifdef HAVE_FABSL 
  maxprec = 16 ;
#else 
  maxprec = 12 ;
#endif 

  precspin = new KNumericSpinBox(this);
  precspin->setRange(0,maxprec);
  precspin->setGeometry(205,115,40,23);

  if( defst->precision <= maxprec)
    precspin->setValue(defst->precision);
  else
    precspin->setValue(maxprec);


  cb = new QCheckBox(this);
  cb->setGeometry(30,145,130,25);
  cb->setText("Set Fixed Precision at:");
  if(defst->fixed)
    cb->setChecked(true);

  int fixprec;
#ifdef HAVE_FABSL 
  fixprec = 14 ;
#else 
  fixprec = 10 ;
#endif 

  precspin2 = new KNumericSpinBox(this);
  precspin2->setRange(0,fixprec);
  precspin2->setGeometry(205,145,40,23);

  if( defst->fixedprecision <= fixprec)
    precspin2->setValue(defst->fixedprecision);
  else
    precspin2->setValue(fixprec);

  
  button3 = new QPushButton(this);
  button3->setGeometry(205,225,100,25);
  button3->setText("Help");

  connect(button3,SIGNAL(clicked()),this,SLOT(help()));

  cb2 = new QCheckBox(this);
  cb2->setGeometry(30,170,130,25);
  cb2->setText("Beep on Error");
  if(defst->beep)
    cb2->setChecked(true);

  stylegroup = new QButtonGroup(this,"stylegroup");
  stylegroup->setGeometry(20,190,170,60);
  stylegroup->setFrameStyle(QFrame::NoFrame);

  trigstyle = new QRadioButton("Trigonometry Mode",stylegroup,"trigstyle");
  trigstyle->move(10,15);
  trigstyle->adjustSize();
  trigstyle->setChecked(defst->style == 0);

  statstyle = new QRadioButton("Statistical Mode",stylegroup,"Stats");
  statstyle->move(10,40);
  statstyle->adjustSize();
  statstyle->setChecked(defst->style ==1);
  
  connect(parent,SIGNAL(applyButtonPressed()),SLOT(okButton()));

}



void ConfigDlg::help(){

  mykapp->invokeHTMLHelp("","");

}

void ConfigDlg::okButton(){

  defst->precision = precspin->getValue();
  defst->fixedprecision = precspin2->getValue();
  defst->fixed = cb->isChecked();
  defst->beep = cb2->isChecked();
  

  if( trigstyle->isChecked())
    defst->style = 0;
  else
    defst->style = 1;

}
void ConfigDlg::cancelbutton() {
  reject();
}




void ConfigDlg::set_fore_color(){


  KColorDialog::getColor(defst->forecolor);
  qframe1->setBackgroundColor(defst->forecolor);


}

void ConfigDlg::set_background_color(){


  KColorDialog::getColor(defst->backcolor);
  qframe2->setBackgroundColor(defst->backcolor);


}






