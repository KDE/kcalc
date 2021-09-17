/*
    SPDX-FileCopyrightText: 2021 Antonio Prcela <antonio.prcela@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
