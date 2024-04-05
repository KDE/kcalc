/*
    SPDX-FileCopyrightText: 2021 Antonio Prcela <antonio.prcela@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCALC_HISTORY_H_
#define KCALC_HISTORY_H_

#include <QTextEdit>

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
    void addFuncToHistory(const QString &);

    void changeSettings();
    void setFont(const QFont &font);
    const QFont &baseFont() const;

public Q_SLOTS:
    void clearHistory();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    bool add_new_line_ = false;
    QFont baseFont_;
    double idealPointSizeF_;
    void updateFont(double zoomFactor = 1.0);
};

#endif
