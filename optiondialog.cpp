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
#include <qpainter.h>

#include <kcolordlg.h>
#include <kcolordrag.h>
#include <kfontdialog.h>
#include <kiconloader.h>
#include <klocale.h>

#include "kcalclogo.xpm"

#include "optiondialog.h"
#include "version.h"

//-------------------------------------------------------------------------
// Name: ConfigureDialog(QWidget *parent, char *name, bool modal)
//-------------------------------------------------------------------------
ConfigureDialog::ConfigureDialog(QWidget *parent, char *name, bool modal)
	: KDialogBase( Tabbed, i18n("Configuration"), Help|Apply|Ok|Cancel,
		Ok, parent, name, modal )
{
	setHelp("kcalc/index.html", QString::null);

	setupSettingPage();
	setupFontPage();
	setupColorPage();
	setupAboutPage();
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

	// Careful with the sequence (search for SEQ to find other locations)
	mColorList->setColor(0, state.forecolor);
	mColorList->setColor(1, state.backcolor);
	mColorList->setColor(2, state.functionButtonColor);
	mColorList->setColor(3, state.hexButtonColor);
	mColorList->setColor(4, state.numberButtonColor);
	mColorList->setColor(5, state.memoryButtonColor);
	mColorList->setColor(6, state.operationButtonColor);

	mFixCheck->setChecked(mState.fixed);
	mPrecSpin->setValue(QMIN( mPrecSpin->maxValue(), mState.precision ));
	mFixSpin->setValue(mState.fixedprecision);
	mBeepCheck->setChecked(mState.beep);
	mTrigRadio->setChecked(mState.style == 0);
	mStatRadio->setChecked(mState.style == 1);  
	mFontChooser->setFont(mState.font);
}

//-------------------------------------------------------------------------
// Name: state()
//-------------------------------------------------------------------------
DefStruct ConfigureDialog::state()
{
	DefStruct state;

	// Careful with the sequence (search for SEQ to find other locations)
	state.forecolor            = mColorList->color(0);
	state.backcolor            = mColorList->color(1);
	state.functionButtonColor  = mColorList->color(2);
	state.hexButtonColor       = mColorList->color(3);
	state.numberButtonColor    = mColorList->color(4);
	state.memoryButtonColor    = mColorList->color(5);
	state.operationButtonColor = mColorList->color(6);

	state.fixed          = mFixCheck->isChecked();
	state.precision      = mPrecSpin->value();
	state.fixedprecision = mFixSpin->value();
	state.beep           = mBeepCheck->isChecked();
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
	setState(mState);
	emit valueChanged(mState);
	reject();
}

//-------------------------------------------------------------------------
// Name: setupSettingPage()
//-------------------------------------------------------------------------
void ConfigureDialog::setupSettingPage()
{
	QFrame *page = addPage(i18n("Settings"));

	if(page == NULL)
		return;
		
	// create our layouts	
	QVBoxLayout *topLayout = new QVBoxLayout(page);
	QHBoxLayout *hLayout1 = new QHBoxLayout(topLayout);
	QHBoxLayout *hLayout2 = new QHBoxLayout(topLayout);
	
	QLabel *label = new QLabel(i18n("Precision:"), page);
	hLayout1->addWidget(label);
	
	mFixCheck = new QCheckBox(i18n("Set fixed precision"), page);
	hLayout2->addWidget(mFixCheck);

#ifdef HAVE_LONG_DOUBLE 
	int maxprec = 16;	
#else 
	int maxprec = 12 ;
#endif 

	mPrecSpin = new QSpinBox(0, maxprec, 1, page);
	hLayout1->addWidget(mPrecSpin);
	
	mFixSpin = new QSpinBox(0, 10, 1, page );
	hLayout2->addWidget(mFixSpin);
	
	topLayout->addSpacing(10);

	mBeepCheck = new QCheckBox(i18n("Beep on error"), page);
	topLayout->addWidget(mBeepCheck);

	QButtonGroup *group = new QButtonGroup(page, "stylegroup");
	group->setFrameStyle(QFrame::NoFrame);
	topLayout->addWidget(group);

	QVBoxLayout *vbox = new QVBoxLayout(group);

	mTrigRadio = new QRadioButton(i18n("Trigonometrical Mode"), 
		group,"trigstyle");
	vbox->addWidget(mTrigRadio);

	mStatRadio = new QRadioButton(i18n("Statistical Mode"), 
		group,"statstyle");
	vbox->addWidget(mStatRadio);
	
	topLayout->addStretch();

	topLayout->activate();
}

//-------------------------------------------------------------------------
// Name: setupColorPage()
//-------------------------------------------------------------------------
void ConfigureDialog::setupColorPage()
{
	QFrame *page = addPage(i18n("Colors"));
	QVBoxLayout *topLayout = new QVBoxLayout(page, 0, spacingHint());

	// Careful with the sequence (search for SEQ to find other locations)
	QStringList modeList;
	modeList.append(i18n("Display Foreground"));
	modeList.append(i18n("Display Background"));
	modeList.append(i18n("Function Buttons"));
	modeList.append(i18n("Hexadecimal Buttons"));
	modeList.append(i18n("Number buttons"));
	modeList.append(i18n("Memory Buttons"));
	modeList.append(i18n("Operation Buttons"));

	mColorList = new ColorListBox(page);
	topLayout->addWidget(mColorList, 10);
	
	for(uint i = 0; i < 7; i++)
	{
		ColorListItem *listItem = new ColorListItem(modeList[i]);
		mColorList->insertItem(listItem);
	}
	
	mColorList->setCurrentItem(0);
}



//-------------------------------------------------------------------------
// Name: setupFontPage()
//-------------------------------------------------------------------------
void ConfigureDialog::setupFontPage()
{
	QFrame *page = addPage(i18n("Display Font"));
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

//-------------------------------------------------------------------------
// Name: setupAboutPage()
//-------------------------------------------------------------------------
void ConfigureDialog::setupAboutPage()
{
	QFrame *page = addPage(i18n("About"));

	if(page == 0)
		return;

	QVBoxLayout *topLayout = new QVBoxLayout(page, 0, spacingHint());
	
	if(topLayout == 0)
		return;

	QString authorText = i18n(""
		"KCalc %1\n"
		"Bernd Johannes Wuebben\n"
		"wuebben@kde.org\n"
		"Copyright (C) 1996-98\n\n"
		"Additions by Espen Sand\n"
		"espen@kde.org, 2000\n\n"
		"Additions by Evan Teran\n"
		"emt3734@rit.edu, 2001\n").arg(KCALCVERSION);

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

	topLayout->addSpacing(spacingHint());

	QHBoxLayout *hbox = new QHBoxLayout();
	topLayout->addLayout(hbox);

	hbox->addSpacing(spacingHint());

	QLabel *logo = new QLabel(page);
	logo->setPixmap(QPixmap(kcalclogo));
	hbox->addWidget(logo);

	QLabel *label = new QLabel(authorText, page);
	hbox->addWidget(label, 10, AlignHCenter);
	label->setAlignment(AlignVCenter);

	label = new QLabel(baseText, page);
	topLayout->addWidget(label, 10, AlignHCenter);
	label->setAlignment(AlignVCenter);

	topLayout->activate();
}


//
// 2000-07-15 (espen): The ColorListBox and the ColorListItem will
// be moved to kdeui after KDE 2.0. The same code is used in khexedit and
// kmail. If you read this after KDE 2.0, feel free to send me a reminder :)
//


//-------------------------------------------------------------------------
// Name: ColorListBox(QWidget *parent, const char *name, WFlags f)
//-------------------------------------------------------------------------
ColorListBox::ColorListBox(QWidget *parent, const char *name, WFlags f)
  :KListBox(parent, name, f), mCurrentOnDragEnter(-1)
{
	connect(this, SIGNAL(selected(int)), this, SLOT(newColor(int)));
	setAcceptDrops(true);
}

//-------------------------------------------------------------------------
// Name: setEnabled(bool state)
//-------------------------------------------------------------------------
void ColorListBox::setEnabled(bool state)
{
	if(state == isEnabled())
		return;

	QListBox::setEnabled(state);
	for(uint i = 0; i < count(); i++)
	{
		updateItem(i);
	}
}

//-------------------------------------------------------------------------
// Name: setColor(uint index, const QColor &color)
//-------------------------------------------------------------------------
void ColorListBox::setColor(uint index, const QColor &color)
{
	if(index < count())
	{
		ColorListItem *colorItem = (ColorListItem*)item(index);
		colorItem->setColor(color);
		updateItem(colorItem);
	}
}

//-------------------------------------------------------------------------
// Name: color(uint index)
//-------------------------------------------------------------------------
const QColor ColorListBox::color(uint index)
{
	if(index < count())
	{
		ColorListItem *colorItem = (ColorListItem*)item(index);
		return(colorItem->color());
	}
	else
	{
		return(black);
	}
}

//-------------------------------------------------------------------------
// Name: newColor(int index)
//-------------------------------------------------------------------------
void ColorListBox::newColor(int index)
{
	if(isEnabled() == false)
		return;

	if((uint)index < count())
	{
		QColor c = color(index);
		if(KColorDialog::getColor(c, this) != QDialog::Rejected)
		{
			setColor(index, c);
		}
	}
}

//-------------------------------------------------------------------------
// Name: dragEnterEvent(QDragEnterEvent *e)
//-------------------------------------------------------------------------
void ColorListBox::dragEnterEvent(QDragEnterEvent *e)
{
	if(KColorDrag::canDecode(e) && isEnabled())
	{
		mCurrentOnDragEnter = currentItem();
		e->accept(true);
	}
	else
	{
		mCurrentOnDragEnter = -1;
		e->accept(false);
	}
}

//-------------------------------------------------------------------------
// Name: dragLeaveEvent(QDragLeaveEvent *)
//-------------------------------------------------------------------------
void ColorListBox::dragLeaveEvent(QDragLeaveEvent *)
{
	if(mCurrentOnDragEnter != -1)
	{
		setCurrentItem(mCurrentOnDragEnter);
		mCurrentOnDragEnter = -1;
	}
}

//-------------------------------------------------------------------------
// Name: dragMoveEvent(QDragMoveEvent *e)
//-------------------------------------------------------------------------
void ColorListBox::dragMoveEvent(QDragMoveEvent *e)
{
	if(KColorDrag::canDecode(e) && isEnabled())
	{
		ColorListItem *item = (ColorListItem*)itemAt(e->pos());
		if(item != 0)
		{
			setCurrentItem(item);
		}
	}
}

//-------------------------------------------------------------------------
// Name: dropEvent(QDropEvent *e)
//-------------------------------------------------------------------------
void ColorListBox::dropEvent(QDropEvent *e)
{
	QColor color;
	if( KColorDrag::decode(e, color))
	{
		int index = currentItem();
		if(index != -1)
		{
		ColorListItem *colorItem = (ColorListItem*)item(index);
		colorItem->setColor(color);
		triggerUpdate(false); // Redraw item
		}
		mCurrentOnDragEnter = -1;
	}
}


//-------------------------------------------------------------------------
// Name: ColorListItem(const QString &text, const QColor &color)
//-------------------------------------------------------------------------
ColorListItem::ColorListItem(const QString &text, const QColor &color)
	: QListBoxItem(), mColor(color), mBoxWidth(30)
{
	setText(text);
}


//-------------------------------------------------------------------------
// Name: color()
//-------------------------------------------------------------------------
const QColor &ColorListItem::color()
{
	return mColor;
}

//-------------------------------------------------------------------------
// Name: setColor(const QColor &color)
//-------------------------------------------------------------------------
void ColorListItem::setColor(const QColor &color)
{
	mColor = color;
}

//-------------------------------------------------------------------------
// Name: paint(QPainter *p)
//-------------------------------------------------------------------------
void ColorListItem::paint(QPainter *p)
{
	QFontMetrics fm = p->fontMetrics();
	int h = fm.height();

	p->drawText(mBoxWidth + 3 * 2, fm.ascent() + fm.leading() / 2, text());

	p->setPen(Qt::black);
	p->drawRect(3, 1, mBoxWidth, h - 1);
	p->fillRect(4, 2, mBoxWidth - 2, h - 3, mColor);
}

//-------------------------------------------------------------------------
// Name: height(const QListBox *lb) const
//-------------------------------------------------------------------------
int ColorListItem::height(const QListBox *lb) const
{
	return(lb->fontMetrics().lineSpacing() + 1);
}


//-------------------------------------------------------------------------
// Name: width(const QListBox *lb) const
//-------------------------------------------------------------------------
int ColorListItem::width(const QListBox *lb) const
{
	return(mBoxWidth + lb->fontMetrics().width(text()) + 6);
}




#include "optiondialog.moc"
