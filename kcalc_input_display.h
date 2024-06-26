/*
    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QLineEdit>
#include <QString>

class KCalcInputDisplay : public QLineEdit
{
    Q_OBJECT

public:
    explicit KCalcInputDisplay(QWidget *parent = nullptr);
    ~KCalcInputDisplay() override;

    void insertToken(const QString &token);
    void insertTokenNumeric(const QString &token);
    void insertTokenFunction(const QString &token);

public Q_SLOTS:
    void reset();
    void setHasResult();
    void slotSetOverwrite();
    void slotSetHardOverwrite();
    void slotClearOverwrite();
    void clearHasResult();

Q_SIGNALS:

protected:
    void focusInEvent(QFocusEvent *e) override;

private Q_SLOTS:

private:
    bool overwrite_;
    bool hard_overwrite_;
    bool has_result_;
};
