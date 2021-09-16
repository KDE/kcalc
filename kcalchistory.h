/*
 *  Copyright (c) 2021 Antonio Prcela <antonio.prcela@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KCALC_HISTORY_H_
#define KCALC_HISTORY_H_

#include <QTextEdit>

#include "kcalc_core.h"

/*
  This class provides a history display.
*/

class KCalcHistory : public QTextEdit
{
    Q_OBJECT

public:
    explicit KCalcHistory(QWidget *parent = nullptr);
    ~KCalcHistory() override;

    void addToHistory(const QString &, bool);
    void addResultToHistory(const QString &);
    void addFuncToHistory(const CalcEngine::Operation);
    void addFuncToHistory(const QString &);

    void changeSettings();

public Q_SLOTS:
    void clearHistory();

private:
    bool add_new_line = false;
};

#endif
