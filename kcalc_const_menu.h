/*
Copyright (C) 2001 - 2013 Evan Teran
                          evan.teran@gmail.com

Copyright (C) 2003 - 2005 Klaus Niederkrueger
                          kniederk@math.uni-koeln.de

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QMenu>
#include <QString>

enum ConstantCategory { Mathematics = 1, Electromagnetic = 2, Nuclear = 4, Thermodynamics = 8, Gravitation = 16 };

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
    explicit KCalcConstMenu(QWidget *parent = nullptr);
    explicit KCalcConstMenu(const QString &title, QWidget *parent = nullptr);

public:
    static void init_consts();

Q_SIGNALS:
    void triggeredConstant(const science_constant &);

private:
    void init_all();

public Q_SLOTS:
    void slotPassSignalThrough(QAction *chosen_const);
};

