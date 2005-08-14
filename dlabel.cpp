/*
    $Id$

    KCalc, a scientific calculator for the X window system using the
    Qt widget libraries, available at no cost at http://www.troll.no
    
    Copyright (C) 1996 Bernd Johannes Wuebben   
                       wuebben@math.cornell.edu

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <qglobal.h>
#include <kactioncollection.h>
#include <kstdaction.h>

#include "kcalc_settings.h"
#include "kcalc_core.h"
#include "dlabel.h"
#include "dlabel.moc"



DispLogic::DispLogic(QWidget *parent, const char *name,
		     KActionCollection *coll)
  :KCalcDisplay(parent,name), _history_index(0)
{
	KNumber::setDefaultFloatOutput(true);
	_back = KStdAction::undo(this, SLOT(history_back()), coll);
	_forward = KStdAction::redo(this, SLOT(history_forward()), coll);

	_forward->setEnabled(false);
	_back->setEnabled(false);
}

DispLogic::~DispLogic()
{
}

void DispLogic::changeSettings()
{
	QPalette pal = palette();

	pal.setColor(QColorGroup::Text, KCalcSettings::foreColor());
	pal.setColor(QColorGroup::Foreground, KCalcSettings::foreColor());
	pal.setColor(QColorGroup::Background, KCalcSettings::backColor());

	setPalette(pal);
	setBackgroundColor(KCalcSettings::backColor());

	setFont(KCalcSettings::font());

	setPrecision(KCalcSettings::precision());

	if(KCalcSettings::fixed() == false)
		setFixedPrecision(-1);
	else
		setFixedPrecision(KCalcSettings::fixedPrecision());

	setBeep(KCalcSettings::beep());
	setGroupDigits(KCalcSettings::groupDigits());
	updateDisplay();
}

void DispLogic::update_from_core(CalcEngine const &core,
				 bool store_result_in_history)
{
	bool tmp_error;
	KNumber const & output = core.lastOutput(tmp_error);
	if(tmp_error) sendEvent(EventError);
	if (setAmount(output)  &&  store_result_in_history  &&
	    output != KNumber::Zero)
	{
	  // add this latest value to our history
	  _history_list.insert(_history_list.begin(), output);
	  _history_index = 0;
	  _back->setEnabled(true);
	  _forward->setEnabled(false);
	}
}

void DispLogic::EnterDigit(int data)
{
	char tmp;
	switch(data)
	{
	case 0:
	  tmp = '0';
	  break;
	case 1:
	  tmp = '1';
	  break;
	case 2:
	  tmp = '2';
	  break;
	case 3:
	  tmp = '3';
	  break;
	case 4:
	  tmp = '4';
	  break;
	case 5:
	  tmp = '5';
	  break;
	case 6:
	  tmp = '6';
	  break;
	case 7:
	  tmp = '7';
	  break;
	case 8:
	  tmp = '8';
	  break;
	case 9:
	  tmp = '9';
	  break;
	case 0xA:
	  tmp = 'A';
	  break;
	case 0xB:
	  tmp = 'B';
	  break;
	case 0xC:
	  tmp = 'C';
	  break;
	case 0xD:
	  tmp = 'D';
	  break;
	case 0xE:
	  tmp = 'E';
	  break;
	case 0xF:
	  tmp = 'F';
	  break;
	default:
	  tmp = '?';
	  break;
	}

	newCharacter(tmp);
}

void DispLogic::history_forward()
{
	Q_ASSERT(! _history_list.empty());
	Q_ASSERT(_history_index > 0);

	_history_index --;

	setAmount(_history_list[_history_index]);

	if(_history_index == 0) _forward->setEnabled(false);

	_back->setEnabled(true);
}

void DispLogic::history_back()
{
	Q_ASSERT(! _history_list.empty());
	Q_ASSERT( _history_index < static_cast<int>(_history_list.size()) );

	setAmount(_history_list[_history_index]);

	_history_index ++;
	
	if( _history_index == static_cast<int>(_history_list.size()) )
		_back->setEnabled(false);
	_forward->setEnabled(true);
}

