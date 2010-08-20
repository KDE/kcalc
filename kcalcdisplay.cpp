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
        : QFrame(parent), _beep(false), _groupdigits(true), _twoscomplement(true),
        _button(0), _lit(false), _num_base(NB_DECIMAL), _precision(9),
        _fixed_precision(-1), _display_amount(0), _history_index(0),
        _selection_timer(new QTimer)
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
    connect(_selection_timer, SIGNAL(timeout()),
            this, SLOT(slotSelectionTimedOut()));

    sendEvent(EventReset);
}

KCalcDisplay::~KCalcDisplay()
{
    delete _selection_timer;
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
        _history_list.insert(_history_list.begin(), output);
        _history_index = 0;
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
    if (_history_list.empty()) return;
    if (_history_index <= 0) return;

    _history_index --;
    setAmount(_history_list[_history_index]);
}

void KCalcDisplay::slotHistoryBack()
{
    if (_history_list.empty()) return;
    if (_history_index >= _history_list.size()) return;

    setAmount(_history_list[_history_index]);
    _history_index ++;
}

bool KCalcDisplay::sendEvent(Event const event)
{
    switch (event) {
    case EventClear:
    case EventReset:
        _display_amount = 0;
        _str_int = "0";
        _str_int_exp.clear();

        _eestate = false;
        _period = false;
        _neg_sign = false;

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

void KCalcDisplay::slotCut(void)
{
    slotCopy();
    sendEvent(EventReset);
}

void KCalcDisplay::slotCopy(void)
{
    QString txt = _text;
    if (_num_base == NB_HEX)
        txt.prepend("0x");
    (QApplication::clipboard())->setText(txt, QClipboard::Clipboard);
    (QApplication::clipboard())->setText(txt, QClipboard::Selection);
}

void KCalcDisplay::slotPaste(bool bClipboard)
{
    QString tmp_str = (QApplication::clipboard())->text(bClipboard ? QClipboard::Clipboard : QClipboard::Selection);

    if (tmp_str.isNull()) {
        if (_beep)  KNotification::beep();
        return;
    }

    NumBase tmp_num_base = _num_base;

    // fix up string
    tmp_str = tmp_str.trimmed();
    if (_groupdigits) {
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
            if (_beep) KNotification::beep();
            return ;
        }

        setAmount(KNumber(tmp_result));
    } else {
        setAmount(KNumber(tmp_str));
        if (_beep  &&  _display_amount == KNumber::NotDefined)
            KNotification::beep();

    }
}

void KCalcDisplay::slotDisplaySelected(void)
{
    if (_button == Qt::LeftButton) {
        if (_lit) {
            slotCopy();
            _selection_timer->start(100);
        } else {
            _selection_timer->stop();
        }

        invertColors();
    } else {
        slotPaste(false);   // Selection
    }
}

void KCalcDisplay::slotSelectionTimedOut(void)
{
    _lit = false;
    invertColors();
    _selection_timer->stop();
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
        _lit = !_lit;
        _button = Qt::LeftButton;
    } else {
        _button = Qt::MidButton;
    }

    emit clicked();
}

void KCalcDisplay::setPrecision(int precision)
{
    _precision = precision;
}

void KCalcDisplay::setFixedPrecision(int precision)
{
    if (_fixed_precision > _precision)
        _fixed_precision = -1;
    else
        _fixed_precision = precision;
}

void KCalcDisplay::setBeep(bool flag)
{
    _beep = flag;
}

void KCalcDisplay::setGroupDigits(bool flag)
{
    _groupdigits = flag;
}

void KCalcDisplay::setTwosComplement(bool flag)
{
    _twoscomplement = flag;
}

KNumber const & KCalcDisplay::getAmount(void) const
{
    return _display_amount;
}

bool KCalcDisplay::setAmount(KNumber const & new_amount)
{
    QString display_str;

    _str_int = "0";
    _str_int_exp.clear();
    _period = false;
    _neg_sign = false;
    _eestate = false;

    if ((_num_base != NB_DECIMAL) &&
            (new_amount.type() != KNumber::SpecialType)) {
        _display_amount = new_amount.integerPart();
        if (_twoscomplement) {
            // treat number as 64-bit unsigned
            quint64 tmp_workaround = static_cast<quint64>(_display_amount);
            display_str = QString::number(tmp_workaround, _num_base).toUpper();
        } else {
            // QString::number treats non-decimal as unsigned
            qint64 tmp_workaround = static_cast<qint64>(_display_amount);
            bool neg = tmp_workaround < 0;
            if (neg) tmp_workaround = qAbs(tmp_workaround);
            display_str = QString::number(tmp_workaround, _num_base).toUpper();
            if (neg) display_str.prepend(KGlobal::locale()->negativeSign());
        }
    } else {
        // _num_base == NB_DECIMAL || new_amount.type() == KNumber::SpecialType
        _display_amount = new_amount;
        display_str = _display_amount.toQString(KCalcSettings::precision(),
                                                _fixed_precision);
    }

    setText(display_str);
    emit changedAmount(_display_amount);
    return true;
}

void KCalcDisplay::setText(QString const &string)
{
    // note that "C" locale is being used internally
    _text = string;
    // don't mess with special numbers
    bool special = (string.contains("nan") || string.contains("inf"));
    // if we aren't in decimal mode, we don't need to modify the string
    if ((_num_base == NB_DECIMAL) && _groupdigits && !special) {
        // when input ends with "." (because incomplete), the
        // formatNumber method does not work; fix by hand by
        // truncating, formatting and appending again
        if (string.endsWith('.')) {
            _text.chop(1);
            // Note: rounding happened already above!
            _text = KGlobal::locale()->formatNumber(_text, false, 0);
            _text.append(KGlobal::locale()->decimalSymbol());
        } else {
            // Note: rounding happened already above!
            _text = KGlobal::locale()->formatNumber(_text, false, 0);
        }
    }

    update();
    emit changedText(_text);
}

QString KCalcDisplay::text() const
{
    return _text;
}

/* change representation of display to new base (i.e. binary, decimal,
   octal, hexadecimal). The amount being displayed is changed to this
   base, but for now this amount can not be modified anymore (like
   being set with "setAmount"). Return value is the new base. */
int KCalcDisplay::setBase(NumBase new_base)
{
    switch (new_base) {
    case NB_HEX:
        _num_base = NB_HEX;
        _period  = false;
        break;
    case NB_DECIMAL:
        _num_base = NB_DECIMAL;
        break;
    case NB_OCTAL:
        _num_base = NB_OCTAL;
        _period  = false;
        break;
    case NB_BINARY:
        _num_base = NB_BINARY;
        _period  = false;
        break;
    default: // we shouldn't ever end up here
        _num_base = NB_DECIMAL;
    }

    // reset amount
    setAmount(_display_amount);

    return _num_base;
}

void KCalcDisplay::setStatusText(int i, const QString& text)
{
    if (i < NUM_STATUS_TEXT)
        _str_status[i] = text;
    update();
}

bool KCalcDisplay::updateDisplay(void)
{
    // Put sign in front.
    QString tmp_string;
    if (_neg_sign == true) {
        tmp_string = '-' + _str_int;
    } else {
        tmp_string = _str_int;
    }

    switch (_num_base) {
        // TODO: use QString:::toULongLong() instead of STRTOUL
    case NB_BINARY:
        Q_ASSERT(_period == false  && _eestate == false);
        setText(tmp_string);
        _display_amount = static_cast<qint64>(STRTOUL(_str_int.toLatin1(), 0, 2));
        if (_neg_sign) _display_amount = -_display_amount;
        break;

    case NB_OCTAL:
        Q_ASSERT(_period == false  && _eestate == false);
        setText(tmp_string);
        _display_amount = static_cast<qint64>(STRTOUL(_str_int.toLatin1(), 0, 8));
        if (_neg_sign) _display_amount = -_display_amount;
        break;

    case NB_HEX:
        Q_ASSERT(_period == false  && _eestate == false);
        setText(tmp_string);
        _display_amount = static_cast<qint64>(STRTOUL(_str_int.toLatin1(), 0, 16));
        if (_neg_sign) _display_amount = -_display_amount;
        break;

    case NB_DECIMAL:
        if (_eestate == false) {
            setText(tmp_string);
            _display_amount = tmp_string;
        } else {
            if (_str_int_exp.isNull()) {
                // add 'e0' to display but not to conversion
                _display_amount = tmp_string;
                setText(tmp_string + "e0");
            } else {
                tmp_string +=  'e' + _str_int_exp;
                setText(tmp_string);
                _display_amount = tmp_string;
            }
        }
        break;

    default:
        return false;
    }
    emit changedAmount(_display_amount);
    return true;
}

void KCalcDisplay::newCharacter(char const new_char)
{
    // test if character is valid
    switch (new_char) {
    case 'e':
        // EE can be set only once and in decimal mode
        if (_num_base != NB_DECIMAL  ||
                _eestate == true) {
            if (_beep) KNotification::beep();
            return;
        }
        _eestate = true;
        break;

    case '.':
        // Period can be set only once and only in decimal
        // mode, also not in EE-mode
        if (_num_base != NB_DECIMAL  ||
                _period == true  ||
                _eestate == true) {
            if (_beep) KNotification::beep();
            return;
        }
        _period = true;
        break;

    case 'F':
    case 'E':
    case 'D':
    case 'C':
    case 'B':
    case 'A':
        if (_num_base == NB_DECIMAL) {
            if (_beep) KNotification::beep();
            return;
        }
        // no break
    case '9':
    case '8':
        if (_num_base == NB_OCTAL) {
            if (_beep) KNotification::beep();
            return;
        }
        // no break
    case '7':
    case '6':
    case '5':
    case '4':
    case '3':
    case '2':
        if (_num_base == NB_BINARY) {
            if (_beep) KNotification::beep();
            return;
        }
        // no break
    case '1':
    case '0':
        break;

    default:
        if (_beep) KNotification::beep();
        return;
    }

    // change exponent or mantissa
    if (_eestate) {
        // ignore ',' before 'e'. turn e.g. '123.e' into '123e'
        if (new_char == 'e'  &&  _str_int.endsWith('.')) {
            _str_int.chop(1);
            _period = false;
        }

        // 'e' only starts ee_mode, leaves strings unchanged
        if (new_char != 'e'  &&
                // do not add '0' if at start of exp
                !(_str_int_exp.isNull() && new_char == '0'))
            _str_int_exp.append(new_char);
    } else {
        // handle first character
        if (_str_int == "0") {
            switch (new_char) {
            case '.':
                // display "0." not just "."
                _str_int.append(new_char);
                break;
            case 'e':
                // display "0e" not just "e"
                // "0e" does not make sense either, but...
                _str_int.append(new_char);
                break;
            default:
                // no leading '0's
                _str_int[0] = new_char;
            }
        } else
            _str_int.append(new_char);
    }

    updateDisplay();
}

void KCalcDisplay::deleteLastDigit(void)
{
    // Only partially implemented !!
    if (_eestate) {
        if (_str_int_exp.isNull()) {
            _eestate = false;
        } else {
            int length = _str_int_exp.length();
            if (length > 1) {
                _str_int_exp.chop(1);
            } else {
                _str_int_exp = (char *)0;
            }
        }
    } else {
        int length = _str_int.length();
        if (length > 1) {
            if (_str_int[length-1] == '.')
                _period = false;
            _str_int.chop(1);
        } else {
            Q_ASSERT(_period == false);
            _str_int[0] = '0';
        }
    }

    updateDisplay();
}

// change Sign of display. Problem: Only possible here, when in input
// mode. Otherwise return 'false' so that the kcalc_core can handle
// things.
bool KCalcDisplay::changeSign(void)
{
    //stupid way, to see if in input_mode or display_mode
    if (_str_int == "0") return false;

    if (_eestate) {
        if (!_str_int_exp.isNull()) {
            if (_str_int_exp[0] != '-')
                _str_int_exp.prepend('-');
            else
                _str_int_exp.remove('-');
        }
    } else {
        _neg_sign = ! _neg_sign;
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
    painter.drawText(cr, align | Qt::TextSingleLine, _text);

    // draw the status texts using half of the normal
    // font size but not smaller than 7pt
    QFont fnt(font());
    fnt.setPointSize(qMax((fnt.pointSize() / 2), 7));
    painter.setFont(fnt);
    QFontMetrics fm(fnt);
    uint w = fm.width("________");
    uint h = fm.height();

    for (int n = 0; n < NUM_STATUS_TEXT; n++) {
        painter.drawText(5 + n * w, h, _str_status[n]);
    }
}

QSize KCalcDisplay::sizeHint() const
{
    // basic size
    QSize sz = fontMetrics().size(Qt::TextSingleLine, _text);

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
