/*
    KCalc, a scientific calculator for the X window system using the
    Qt widget libraries, available at no cost at http://www.troll.no

    Copyright (C) 2004 Klaus Niederkruger
                       kniederk@ulb.ac.be

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KCALC_CONST_MENU_H_
#define KCALC_CONST_MENU_H_

#include <QList>
#include <QMenu>


enum ConstantCategory {Mathematics = 1, Electromagnetic = 2, Nuclear = 4, Thermodynamics = 8, Gravitation = 16};

struct science_constant {
    QString label;
    QString name;
    QString whatsthis;
    QString value;
    ConstantCategory category;
};

class KCalcConstMenu : public QMenu
{
    Q_OBJECT

public:
    KCalcConstMenu(QWidget * parent = 0);
    explicit KCalcConstMenu(const QString &title, QWidget * parent = 0);

    static QList<science_constant> Constants;
    static void init_consts();

signals:
    void triggeredConstant(const science_constant &);


private:
    void init_all();


public slots:
    void slotPassSignalThrough(QAction  *chosen_const);
};

#endif // KCALC_CONST_MENU_H_
