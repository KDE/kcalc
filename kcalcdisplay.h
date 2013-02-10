/*
Copyright (C) 2001 - 2013 Evan Teran
                          evan.teran@gmail.com

Copyright (C) 1996 - 2000 Bernd Johannes Wuebben
                          wuebben@kde.org

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KCALCDISPLAY_H_
#define KCALCDISPLAY_H_

#include <QFrame>
#include <QVector>
#include "knumber.h"

class CalcEngine;
class KAction;
class QTimer;
class QStyleOptionFrame;

#define NUM_STATUS_TEXT 4

/*
  This class provides a pocket calculator display.  The display has
  implicitely two major modes: One is for editing and one is purely
  for displaying.

  When one uses "setAmount", the given amount is displayed, and the
  amount which was possibly typed in before is lost. At the same time
  this new value can not be modified.

  On the other hand, "addNewChar" adds a new digit to the amount that
  is being typed in. If "setAmount" was used before, the display is
  cleared and a new input starts.

  TODO: Check overflows, number of digits and such...
*/

enum NumBase {
    NB_BINARY = 2,
    NB_OCTAL = 8,
    NB_DECIMAL = 10,
    NB_HEX = 16
};


class KCalcDisplay : public QFrame
{
    Q_OBJECT

public:
    explicit KCalcDisplay(QWidget *parent = 0);
    ~KCalcDisplay();

    enum Event {
        EventReset, // resets display
        EventClear, // if no error reset display
        EventError,
        EventChangeSign
    };

    bool sendEvent(Event event);
    void deleteLastDigit();
    const KNumber &getAmount() const;
    void newCharacter(const QChar new_char);
    bool setAmount(const KNumber &new_amount);
    int setBase(NumBase new_base);
    void setBeep(bool flag);
    void setGroupDigits(bool flag);
    void setTwosComplement(bool flag);
    void setBinaryGrouping(int digits);
    void setOctalGrouping(int digits);
    void setHexadecimalGrouping(int digits);
    void setFixedPrecision(int precision);
    void setPrecision(int precision);
    void setText(const QString &string);
    QString groupDigits(const QString &displayString, int numDigits);
    QString text() const;
    void updateDisplay();
    void setStatusText(int i, const QString &text);
    virtual QSize sizeHint() const;

    void changeSettings();
    void enterDigit(int data);
    void updateFromCore(const CalcEngine &core,
                        bool store_result_in_history = false);

public slots:
    void slotCut();
    void slotCopy();
    void slotPaste(bool bClipboard = true);

signals:
    void clicked();
    void changedText(const QString &);
    void changedAmount(const KNumber &);

protected:
    void  mousePressEvent(QMouseEvent *);
    virtual void paintEvent(QPaintEvent *p);

private:
    bool changeSign();
    void invertColors();
    void initStyleOption(QStyleOptionFrame *option) const;

private slots:
    void slotSelectionTimedOut();
    void slotDisplaySelected();
    void slotHistoryBack();
    void slotHistoryForward();

private:
    QString text_;
    bool beep_;
    bool groupdigits_;
    bool twoscomplement_;
    int  binaryGrouping_;
    int  octalGrouping_;
    int  hexadecimalGrouping_;
    int  button_;
    bool lit_;
    NumBase num_base_;

    int precision_;
    int fixed_precision_; // "-1" = no fixed_precision

    KNumber display_amount_;

    QVector<KNumber> history_list_;
    int history_index_;

    // only used for input of new numbers
    bool eestate_;
    bool period_;
    bool neg_sign_;
    QString str_int_;
    QString str_int_exp_;
    QString str_status_[NUM_STATUS_TEXT];

    QTimer* selection_timer_;
};

#endif
