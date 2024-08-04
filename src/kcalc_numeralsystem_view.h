/*
 *    SPDX-FileCopyrightText: 2024 Eugene Popov <popov895@ukr.net>
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KCALCNUMERALSYSTEMVIEW_H
#define KCALCNUMERALSYSTEMVIEW_H

#include <QWidget>

class QLabel;

class KSqueezedTextLabel;

// KCalcNumeralSystemLabel
class KCalcNumeralSystemLabel : public QWidget
{
    Q_OBJECT

public:
    explicit KCalcNumeralSystemLabel(QWidget *parent = nullptr);

    void setNumber(quint64 number, int base);

protected:
    void changeEvent(QEvent *event) override;

private:
    void updateAlignment();

    KSqueezedTextLabel *m_textLabel;
    QLabel *m_numeralSystemLabel;
};

// KCalcNumeralSystemView
class KCalcNumeralSystemView : public QWidget
{
    Q_OBJECT

public:
    explicit KCalcNumeralSystemView(QWidget *parent = nullptr);

    void setNumber(quint64 number, int base);
    void clearNumber();

private:
    void updateLabels();

    KCalcNumeralSystemLabel *m_label1 = nullptr;
    KCalcNumeralSystemLabel *m_label2 = nullptr;
    KCalcNumeralSystemLabel *m_label3 = nullptr;

    std::optional<quint64> m_number;
    int m_base = 10;
};

#endif // KCALCNUMERALSYSTEMVIEW_H
