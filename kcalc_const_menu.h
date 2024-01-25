/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2003-2005 Klaus Niederkrueger <kniederk@math.uni-koeln.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QDomDocument>
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
    static void init_consts(QDomDocument &doc);

Q_SIGNALS:
    void triggeredConstant(const science_constant &);

private:
    void init_all();

public Q_SLOTS:
    void slotPassSignalThrough(QAction *chosen_const);
};

