/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 1996-2000 Bernd Johannes Wuebben <wuebben@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "kcalc_number_format.h"
#include "knumber.h"
#include <QFrame>
#include <QList>

class CalcEngine;
class QTimer;
class QStyleOptionFrame;

/*
  This class provides a pocket calculator display.

  When one uses "setAmount", the given amount is displayed, and the
  amount which was possibly typed in before is lost. At the same time
  this new value can not be modified.

  TODO: Check overflows, number of digits and such...
*/

class KCalcDisplay : public QFrame
{
    Q_OBJECT

public:
    explicit KCalcDisplay(QWidget *parent = nullptr);
    ~KCalcDisplay() override;

    enum Event {
        EventReset, // resets display
        EventClear, // if no error reset display
        EventError,
    };

    enum ErrorMessage {
        MathError,
        SyntaxError,
        MalformedExpression,
    };

    bool sendEvent(Event event);
    const KNumber &getAmount() const;
    QString getAmountQString(bool addPreffix = true) const;
    bool setAmount(const KNumber &newAmount);
    int setBase(KCalcNumberBase base);
    void setBeep(bool flag);
    void setTwosComplement(bool flag);
    void setFixedPrecision(int precision);
    void setPrecision(int precision);
    void setText(const QString &string);
    void setFont(const QFont &font);
    const QFont &baseFont() const;
    QString text() const;
    void updateDisplay();
    void setStatusText(int i, const QString &text);
    QSize sizeHint() const override;

    void changeSettings();
    void updateFromCore(const CalcEngine &core);

public Q_SLOTS:
    void showErrorMessage(ErrorMessage errorMessage);
    void slotCut();
    void slotCopy();

Q_SIGNALS:
    void clicked();
    void changedText(const QString &);
    void changedAmount(const KNumber &);

protected:
    void mousePressEvent(QMouseEvent *) override;
    void paintEvent(QPaintEvent *p) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void invertColors();
    void initStyleOption(QStyleOptionFrame *option) const override;
    void setUnformattedText(const QString &string);
    void updateFont();

private Q_SLOTS:
    void restoreSettings();
    void setTempSettings();
    void slotSelectionTimedOut();
    void slotDisplaySelected();

private:
    enum {
        NumStatusText = 4,
    };

    QString m_text;
    bool m_usingTempSettings;
    bool m_beep;
    bool m_twosComplement;
    int m_button;
    bool m_lit;
    KCalcNumberBase m_numBase;

    int m_precision;
    int m_fixedPrecision; // "-1" = no fixed_precision

    KNumber m_displayAmount;

    QFont m_baseFont;
    QPalette m_basePalette;

    // only used for input of new numbers
    QString m_strStatus[NumStatusText];

    QTimer *const m_selectionTimer;
    static const QString m_mathErrorText;
    static const QString m_syntaxErrorText;
    static const QString m_malformedExpressionText;
};
