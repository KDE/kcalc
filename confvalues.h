/*

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
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */


#ifndef _CONF_VALUES_H_
#define _CONF_VALUES_H_

typedef struct _DefStruct
{
	QColor forecolor;
	QColor backcolor;
	QColor numberButtonColor;
	QColor functionButtonColor;
	QColor statButtonColor;
	QColor hexButtonColor;
	QColor memoryButtonColor;
	QColor operationButtonColor;

	int precision;
	int fixedprecision;
	bool fixed;
	bool beep;
	bool capres;
	QFont font;
} DefStruct;

#endif // _CONF_VALUES_H_
