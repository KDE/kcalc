/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 1996-2000 Bernd Johannes Wuebben <wuebben@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "knumber.h"
#include <QFrame>
#include <QList>

class CalcEngine;
class QTimer;
class QStyleOptionFrame;

#define NUM_STATUS_TEXT 4

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

    enum NumBase {
        NB_BINARY = 2,
        NB_OCTAL = 8,
        NB_DECIMAL = 10,
        NB_HEX = 16,
    };

    bool sendEvent(Event event);
    const KNumber &getAmount() const;
    QString getAmountQString(bool addPreffix = true) const;
    bool setAmount(const KNumber &new_amount);
    int setBase(NumBase base);
    void setBeep(bool flag);
    void setGroupDigits(bool flag);
    void setTwosComplement(bool flag);
    void setBinaryGrouping(int digits);
    void setOctalGrouping(int digits);
    void setHexadecimalGrouping(int digits);
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
    QString formatNumber(QString string);
    QString formatDecimalNumber(QString string);
    QString groupDigits(const QString &displayString, int numDigits);

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
    QString text_;
    bool m_usingTempSettings;
    bool beep_;
    bool groupdigits_;
    bool twoscomplement_;
    int binaryGrouping_;
    int octalGrouping_;
    int hexadecimalGrouping_;
    int button_;
    bool lit_;
    NumBase num_base_;

    int precision_;
    int fixed_precision_; // "-1" = no fixed_precision

    KNumber display_amount_;

    QFont baseFont_;
    QPalette basePalette_;

    // only used for input of new numbers
    QString str_status_[NUM_STATUS_TEXT];

    QTimer *const selection_timer_;
    static const QString m_MathErrorText;
    static const QString m_SyntaxErrorText;
    static const QString m_MalformedExpressionText;
};
