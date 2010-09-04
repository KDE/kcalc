// -*- indent-tabs-mode: nil -*-
/*
    $Id$

    KCalc, a scientific calculator for the X window system using the
    Qt widget libraries, available at no cost at http://www.troll.no

    Copyright (C) 1996 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "kcalcdisplay.h"

#include <errno.h>
#include <stdlib.h>
#include <ctype.h>

#include <QClipboard>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QTimer>

#include <kglobal.h>
#include <klocale.h>
#include <knotification.h>
#include "kcalc_core.h"
#include "kcalc_settings.h"
#include "kcalcdisplay.moc"

KCalcDisplay::KCalcDisplay(QWidget *parent)
        : QFrame(parent), beep_(false), groupdigits_(true), twoscomplement_(true),
        button_(0), lit_(false), num_base_(NB_DECIMAL), precision_(9),
        fixed_precision_(-1), display_amount_(0), history_index_(0),
        selection_timer_(new QTimer)
{
    setFocusPolicy(Qt::StrongFocus);

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

    setBackgroundRole(QPalette::Base);
    setForegroundRole(QPalette::Text);
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);   // set in kalc.ui

    KNumber::setDefaultFloatOutput(true);
    KNumber::setDefaultFractionalInput(true);

    connect(this, SIGNAL(clicked()),
            this, SLOT(slotDisplaySelected()));
    connect(selection_timer_, SIGNAL(timeout()),
            this, SLOT(slotSelectionTimedOut()));

    sendEvent(EventReset);
}

KCalcDisplay::~KCalcDisplay()
{
    delete selection_timer_;
}

void KCalcDisplay::changeSettings()
{
    QPalette pal = palette();

    pal.setColor(QPalette::Text, KCalcSettings::foreColor());
    pal.setColor(QPalette::Base, KCalcSettings::backColor());

    setPalette(pal);

    setFont(KCalcSettings::displayFont());

    setPrecision(KCalcSettings::precision());

    if (KCalcSettings::fixed() == false)
        setFixedPrecision(-1);
    else
        setFixedPrecision(KCalcSettings::fixedPrecision());

    setBeep(KCalcSettings::beep());
    setGroupDigits(KCalcSettings::groupDigits());
    setTwosComplement(KCalcSettings::twosComplement());
    updateDisplay();
}

void KCalcDisplay::updateFromCore(CalcEngine const &core,
                                  bool store_result_in_history)
{
    bool tmp_error;
    KNumber const & output = core.lastOutput(tmp_error);
    if (tmp_error) sendEvent(EventError);
    if (setAmount(output)
            && store_result_in_history
            && (output != KNumber::Zero)) {
        // add this latest value to our history
        history_list_.insert(history_list_.begin(), output);
        history_index_ = 0;
    }
}

void KCalcDisplay::enterDigit(int data)
{
    char tmp;
    switch (data) {
    case 0:
        tmp = '0';
        break;
    case 1:
        tmp = '1';
        break;
    case 2:
        tmp = '2';
        break;
    case 3:
        tmp = '3';
        break;
    case 4:
        tmp = '4';
        break;
    case 5:
        tmp = '5';
        break;
    case 6:
        tmp = '6';
        break;
    case 7:
        tmp = '7';
        break;
    case 8:
        tmp = '8';
        break;
    case 9:
        tmp = '9';
        break;
    case 0xA:
        tmp = 'A';
        break;
    case 0xB:
        tmp = 'B';
        break;
    case 0xC:
        tmp = 'C';
        break;
    case 0xD:
        tmp = 'D';
        break;
    case 0xE:
        tmp = 'E';
        break;
    case 0xF:
        tmp = 'F';
        break;
    default:
        tmp = '?';
        break;
    }

    newCharacter(tmp);
}

void KCalcDisplay::slotHistoryForward()
{
    if (history_list_.empty()) return;
    if (history_index_ <= 0) return;

    history_index_ --;
    setAmount(history_list_[history_index_]);
}

void KCalcDisplay::slotHistoryBack()
{
    if (history_list_.empty()) return;
    if (history_index_ >= history_list_.size()) return;

    setAmount(history_list_[history_index_]);
    history_index_ ++;
}

bool KCalcDisplay::sendEvent(Event event)
{
    switch (event) {
    case EventClear:
    case EventReset:
        display_amount_ = 0;
        str_int_ = "0";
        str_int_exp_.clear();

        eestate_ = false;
        period_ = false;
        neg_sign_ = false;

        updateDisplay();

        return true;
    case EventChangeSign:
        return changeSign();
    case EventError:
        updateDisplay();

        return true;
    default:
        return false;
    }
}

void KCalcDisplay::slotCut()
{
    slotCopy();
    sendEvent(EventReset);
}

void KCalcDisplay::slotCopy()
{
    QString txt = text_;
    if (num_base_ == NB_HEX)
        txt.prepend("0x");
    (QApplication::clipboard())->setText(txt, QClipboard::Clipboard);
    (QApplication::clipboard())->setText(txt, QClipboard::Selection);
}

void KCalcDisplay::slotPaste(bool bClipboard)
{
    QString tmp_str = (QApplication::clipboard())->text(bClipboard ? QClipboard::Clipboard : QClipboard::Selection);

    if (tmp_str.isNull()) {
        if (beep_)  KNotification::beep();
        return;
    }

    NumBase tmp_num_base = num_base_;

    // fix up string
    tmp_str = tmp_str.trimmed();
    if (groupdigits_) {
        tmp_str.remove(KGlobal::locale()->thousandsSeparator());
    }
    tmp_str = tmp_str.toLower();

    // determine base
    if (tmp_str.startsWith(QLatin1String("0x"))) {
        tmp_num_base = NB_HEX;
    } else if (tmp_str.startsWith(QLatin1String("0b"))) {
        tmp_num_base = NB_BINARY;
        tmp_str.remove(0, 2);
    }

    if (tmp_num_base != NB_DECIMAL) {
        bool was_ok;
        qint64 tmp_result = tmp_str.toULongLong(&was_ok, tmp_num_base);

        if (!was_ok) {
            setAmount(KNumber::NotDefined);
            if (beep_) KNotification::beep();
            return ;
        }

        setAmount(KNumber(tmp_result));
    } else {
        setAmount(KNumber(tmp_str));
        if (beep_  &&  display_amount_ == KNumber::NotDefined)
            KNotification::beep();

    }
}

void KCalcDisplay::slotDisplaySelected()
{
    if (button_ == Qt::LeftButton) {
        if (lit_) {
            slotCopy();
            selection_timer_->start(100);
        } else {
            selection_timer_->stop();
        }

        invertColors();
    } else {
        slotPaste(false);   // Selection
    }
}

void KCalcDisplay::slotSelectionTimedOut()
{
    lit_ = false;
    invertColors();
    selection_timer_->stop();
}

void KCalcDisplay::invertColors()
{
    QPalette tmp_palette = palette();
    tmp_palette.setColor(QPalette::Base,
                         palette().color(QPalette::Text));
    tmp_palette.setColor(QPalette::Text,
                         palette().color(QPalette::Base));
    setPalette(tmp_palette);
}

void KCalcDisplay::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        lit_ = !lit_;
        button_ = Qt::LeftButton;
    } else {
        button_ = Qt::MidButton;
    }

    emit clicked();
}

void KCalcDisplay::setPrecision(int precision)
{
    precision_ = precision;
}

void KCalcDisplay::setFixedPrecision(int precision)
{
    if (fixed_precision_ > precision_)
        fixed_precision_ = -1;
    else
        fixed_precision_ = precision;
}

void KCalcDisplay::setBeep(bool flag)
{
    beep_ = flag;
}

void KCalcDisplay::setGroupDigits(bool flag)
{
    groupdigits_ = flag;
}

void KCalcDisplay::setTwosComplement(bool flag)
{
    twoscomplement_ = flag;
}

KNumber const & KCalcDisplay::getAmount() const
{
    return display_amount_;
}

bool KCalcDisplay::setAmount(KNumber const & new_amount)
{
    QString display_str;

    str_int_ = "0";
    str_int_exp_.clear();
    period_ = false;
    neg_sign_ = false;
    eestate_ = false;

    if ((num_base_ != NB_DECIMAL) &&
            (new_amount.type() != KNumber::SpecialType)) {
        display_amount_ = new_amount.integerPart();
        if (twoscomplement_) {
            // treat number as 64-bit unsigned
            quint64 tmp_workaround = static_cast<quint64>(display_amount_);
            display_str = QString::number(tmp_workaround, num_base_).toUpper();
        } else {
            // QString::number treats non-decimal as unsigned
            qint64 tmp_workaround = static_cast<qint64>(display_amount_);
            bool neg = tmp_workaround < 0;
            if (neg) tmp_workaround = qAbs(tmp_workaround);
            display_str = QString::number(tmp_workaround, num_base_).toUpper();
            if (neg) display_str.prepend(KGlobal::locale()->negativeSign());
        }
    } else {
        // num_base_ == NB_DECIMAL || new_amount.type() == KNumber::SpecialType
        display_amount_ = new_amount;
        display_str = display_amount_.toQString(KCalcSettings::precision(),
                                                fixed_precision_);
    }

    setText(display_str);
    emit changedAmount(display_amount_);
    return true;
}

void KCalcDisplay::setText(QString const &string)
{
    // note that "C" locale is being used internally
    text_ = string;
    // don't mess with special numbers
    bool special = (string.contains("nan") || string.contains("inf"));
    // if we aren't in decimal mode, we don't need to modify the string
    if ((num_base_ == NB_DECIMAL) && groupdigits_ && !special) {
        // when input ends with "." (because incomplete), the
        // formatNumber method does not work; fix by hand by
        // truncating, formatting and appending again
        if (string.endsWith('.')) {
            text_.chop(1);
            // Note: rounding happened already above!
            text_ = KGlobal::locale()->formatNumber(text_, false, 0);
            text_.append(KGlobal::locale()->decimalSymbol());
        } else {
            // Note: rounding happened already above!
            text_ = KGlobal::locale()->formatNumber(text_, false, 0);
        }
    }

    update();
    emit changedText(text_);
}

QString KCalcDisplay::text() const
{
    return text_;
}

/* change representation of display to new base (i.e. binary, decimal,
   octal, hexadecimal). The amount being displayed is changed to this
   base, but for now this amount can not be modified anymore (like
   being set with "setAmount"). Return value is the new base. */
int KCalcDisplay::setBase(NumBase new_base)
{
    switch (new_base) {
    case NB_HEX:
        num_base_ = NB_HEX;
        period_  = false;
        break;
    case NB_DECIMAL:
        num_base_ = NB_DECIMAL;
        break;
    case NB_OCTAL:
        num_base_ = NB_OCTAL;
        period_  = false;
        break;
    case NB_BINARY:
        num_base_ = NB_BINARY;
        period_  = false;
        break;
    default: // we shouldn't ever end up here
        num_base_ = NB_DECIMAL;
    }

    // reset amount
    setAmount(display_amount_);

    return num_base_;
}

void KCalcDisplay::setStatusText(int i, const QString& text)
{
    if (i < NUM_STATUS_TEXT)
        str_status_[i] = text;
    update();
}

bool KCalcDisplay::updateDisplay()
{
    // Put sign in front.
    QString tmp_string;
    if (neg_sign_ == true) {
        tmp_string = '-' + str_int_;
    } else {
        tmp_string = str_int_;
    }
	
	bool ok;

    switch (num_base_) {
        // TODO: use QString:::toULongLong() instead of STRTOUL
    case NB_BINARY:
        Q_ASSERT(period_ == false  && eestate_ == false);
        setText(tmp_string);
		display_amount_ = str_int_.toULongLong(&ok, 2);
        if (neg_sign_) display_amount_ = -display_amount_;
        break;

    case NB_OCTAL:
        Q_ASSERT(period_ == false  && eestate_ == false);
        setText(tmp_string);
        display_amount_ = str_int_.toULongLong(&ok, 8);
        if (neg_sign_) display_amount_ = -display_amount_;
        break;

    case NB_HEX:
        Q_ASSERT(period_ == false  && eestate_ == false);
        setText(tmp_string);
        display_amount_ = str_int_.toULongLong(&ok, 16);
        if (neg_sign_) display_amount_ = -display_amount_;
        break;

    case NB_DECIMAL:
        if (eestate_ == false) {
            setText(tmp_string);
            display_amount_ = tmp_string;
        } else {
            if (str_int_exp_.isNull()) {
                // add 'e0' to display but not to conversion
                display_amount_ = tmp_string;
                setText(tmp_string + "e0");
            } else {
                tmp_string +=  'e' + str_int_exp_;
                setText(tmp_string);
                display_amount_ = tmp_string;
            }
        }
        break;

    default:
        return false;
    }
    emit changedAmount(display_amount_);
    return true;
}

void KCalcDisplay::newCharacter(char const new_char)
{
    // test if character is valid
    switch (new_char) {
    case 'e':
        // EE can be set only once and in decimal mode
        if (num_base_ != NB_DECIMAL  ||
                eestate_ == true) {
            if (beep_) KNotification::beep();
            return;
        }
        eestate_ = true;
        break;

    case '.':
        // Period can be set only once and only in decimal
        // mode, also not in EE-mode
        if (num_base_ != NB_DECIMAL  ||
                period_ == true  ||
                eestate_ == true) {
            if (beep_) KNotification::beep();
            return;
        }
        period_ = true;
        break;

    case 'F':
    case 'E':
    case 'D':
    case 'C':
    case 'B':
    case 'A':
        if (num_base_ == NB_DECIMAL) {
            if (beep_) KNotification::beep();
            return;
        }
        // no break
    case '9':
    case '8':
        if (num_base_ == NB_OCTAL) {
            if (beep_) KNotification::beep();
            return;
        }
        // no break
    case '7':
    case '6':
    case '5':
    case '4':
    case '3':
    case '2':
        if (num_base_ == NB_BINARY) {
            if (beep_) KNotification::beep();
            return;
        }
        // no break
    case '1':
    case '0':
        break;

    default:
        if (beep_) KNotification::beep();
        return;
    }

    // change exponent or mantissa
    if (eestate_) {
        // ignore ',' before 'e'. turn e.g. '123.e' into '123e'
        if (new_char == 'e'  &&  str_int_.endsWith('.')) {
            str_int_.chop(1);
            period_ = false;
        }

        // 'e' only starts ee_mode, leaves strings unchanged
        if (new_char != 'e'  &&
                // do not add '0' if at start of exp
                !(str_int_exp_.isNull() && new_char == '0'))
            str_int_exp_.append(new_char);
    } else {
        // handle first character
        if (str_int_ == "0") {
            switch (new_char) {
            case '.':
                // display "0." not just "."
                str_int_.append(new_char);
                break;
            case 'e':
                // display "0e" not just "e"
                // "0e" does not make sense either, but...
                str_int_.append(new_char);
                break;
            default:
                // no leading '0's
                str_int_[0] = new_char;
            }
        } else
            str_int_.append(new_char);
    }

    updateDisplay();
}

void KCalcDisplay::deleteLastDigit()
{
    // Only partially implemented !!
    if (eestate_) {
        if (str_int_exp_.isNull()) {
            eestate_ = false;
        } else {
            int length = str_int_exp_.length();
            if (length > 1) {
                str_int_exp_.chop(1);
            } else {
                str_int_exp_ = (char *)0;
            }
        }
    } else {
        int length = str_int_.length();
        if (length > 1) {
            if (str_int_[length-1] == '.')
                period_ = false;
            str_int_.chop(1);
        } else {
            Q_ASSERT(period_ == false);
            str_int_[0] = '0';
        }
    }

    updateDisplay();
}

// change Sign of display. Problem: Only possible here, when in input
// mode. Otherwise return 'false' so that the kcalc_core can handle
// things.
bool KCalcDisplay::changeSign()
{
    //stupid way, to see if in input_mode or display_mode
    if (str_int_ == "0") return false;

    if (eestate_) {
        if (!str_int_exp_.isNull()) {
            if (str_int_exp_[0] != '-')
                str_int_exp_.prepend('-');
            else
                str_int_exp_.remove('-');
        }
    } else {
        neg_sign_ = ! neg_sign_;
    }

    updateDisplay();

    return true;
}

void KCalcDisplay::initStyleOption(QStyleOptionFrame *option) const
{
    if (!option) return;

    option->initFrom(this);
    option->state &= ~QStyle::State_HasFocus; // don't draw focus highlight
    if (frameShadow() == QFrame::Sunken) {
        option->state |= QStyle::State_Sunken;
    } else if (frameShadow() == QFrame::Raised) {
        option->state |= QStyle::State_Raised;
    }
    option->lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth,
                        option, this);
    option->midLineWidth = 0;
}

void KCalcDisplay::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QStyleOptionFrame option;
    initStyleOption(&option);

    style()->drawPrimitive(QStyle::PE_PanelLineEdit, &option, &painter, this);

    // draw display text
    int margin = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, 0, 0);
    QRect cr = contentsRect();
    cr.adjust(margin*2, 0, -margin*2, 0);   // provide a margin
    int align = QStyle::visualAlignment(layoutDirection(),
                                        Qt::AlignRight | Qt::AlignVCenter);
    painter.drawText(cr, align | Qt::TextSingleLine, text_);

    // draw the status texts using half of the normal
    // font size but not smaller than 7pt
    QFont fnt(font());
    fnt.setPointSize(qMax((fnt.pointSize() / 2), 7));
    painter.setFont(fnt);
    QFontMetrics fm(fnt);
    uint w = fm.width("________");
    uint h = fm.height();

    for (int n = 0; n < NUM_STATUS_TEXT; n++) {
        painter.drawText(5 + n * w, h, str_status_[n]);
    }
}

QSize KCalcDisplay::sizeHint() const
{
    // basic size
    QSize sz = fontMetrics().size(Qt::TextSingleLine, text_);

    // expanded by half font height to make room for  the status texts
    QFont fnt(font());
    fnt.setPointSize(qMax((fnt.pointSize() / 2), 7));
    QFontMetrics fm(fnt);
    sz.setHeight(sz.height() + fm.height());

    QStyleOptionFrame option;
    initStyleOption(&option);
    return (style()->sizeFromContents(QStyle::CT_LineEdit,
                                      &option,
                                      sz.expandedTo(QApplication::globalStrut()),
                                      this));
}
