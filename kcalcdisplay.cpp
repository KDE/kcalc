/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 1996-2000 Bernd Johannes Wuebben <wuebben@kde.org>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalcdisplay.h"

#include <QApplication>
#include <QClipboard>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QTimer>

#include <KNotification>

#include "kcalc_core.h"
#include "kcalc_settings.h"

//------------------------------------------------------------------------------
// Name: KCalcDisplay
// Desc: constructor
//------------------------------------------------------------------------------
KCalcDisplay::KCalcDisplay(QWidget *parent)
    : QFrame(parent)
    , beep_(false)
    , groupdigits_(true)
    , twoscomplement_(true)
    , button_(0)
    , lit_(false)
    , num_base_(NB_DECIMAL)
    , precision_(9)
    , fixed_precision_(-1)
    , display_amount_(0)
    , history_index_(0)
    , selection_timer_(new QTimer(this))
{
    setAccessibleDescription(i18nc("@label accessibility description of the calculation result display", "Result Display"));
    setFocusPolicy(Qt::StrongFocus);

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

    setBackgroundRole(QPalette::Base);
    setForegroundRole(QPalette::Text);
    setFrameStyle(QFrame::NoFrame); // set in kalc.ui
    setContentsMargins(6, 0, 6, 6);

    KNumber::setDefaultFloatOutput(true);
    KNumber::setDefaultFractionalInput(true);

    connect(this, &KCalcDisplay::clicked, this, &KCalcDisplay::slotDisplaySelected);
    connect(selection_timer_, &QTimer::timeout, this, &KCalcDisplay::slotSelectionTimedOut);

    sendEvent(EventReset);
}

//------------------------------------------------------------------------------
// Name: ~KCalcDisplay
// Desc: destructor
//------------------------------------------------------------------------------
KCalcDisplay::~KCalcDisplay() = default;

//------------------------------------------------------------------------------
// Name: changeSettings
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::changeSettings()
{
    QPalette pal = palette();

    pal.setColor(QPalette::Text, KCalcSettings::foreColor());
    pal.setColor(QPalette::Base, KCalcSettings::backColor());

    setPalette(pal);

    setFont(KCalcSettings::displayFont());

    setPrecision(KCalcSettings::precision());

    if (!KCalcSettings::fixed()) {
        setFixedPrecision(-1);
    } else {
        setFixedPrecision(KCalcSettings::fixedPrecision());
    }

    setBeep(KCalcSettings::beep());
    setGroupDigits(KCalcSettings::groupDigits());
    setTwosComplement(KCalcSettings::twosComplement());
    setBinaryGrouping(KCalcSettings::binaryGrouping());
    setOctalGrouping(KCalcSettings::octalGrouping());
    setHexadecimalGrouping(KCalcSettings::hexadecimalGrouping());
    updateDisplay();
}

//------------------------------------------------------------------------------
// Name:
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::updateFromCore(const CalcEngine &core, bool store_result_in_history)
{
    bool tmp_error;
    const KNumber &output = core.lastOutput(tmp_error);

#if 0
	// TODO: do we really need explicit error tracking?
	// isn't the type of the KNumber good enough?
	// I think it is and that this error tracking is cruft
	// left over from a LONG time ago...
	if(output.type() == KNumber::TYPE_ERROR) {
#else
    if (tmp_error) {
#endif
    sendEvent(EventError);
}

if (setAmount(output) && store_result_in_history && (output != KNumber::Zero)) {
    // add this latest value to our history
    history_list_.insert(history_list_.begin(), output);
    history_index_ = 0;
}
}

//------------------------------------------------------------------------------
// Name: enterDigit
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::enterDigit(int data)
{
    switch (data) {
    case 0:
        newCharacter(QLatin1Char('0'));
        break;
    case 1:
        newCharacter(QLatin1Char('1'));
        break;
    case 2:
        newCharacter(QLatin1Char('2'));
        break;
    case 3:
        newCharacter(QLatin1Char('3'));
        break;
    case 4:
        newCharacter(QLatin1Char('4'));
        break;
    case 5:
        newCharacter(QLatin1Char('5'));
        break;
    case 6:
        newCharacter(QLatin1Char('6'));
        break;
    case 7:
        newCharacter(QLatin1Char('7'));
        break;
    case 8:
        newCharacter(QLatin1Char('8'));
        break;
    case 9:
        newCharacter(QLatin1Char('9'));
        break;
    case 0xa:
        newCharacter(QLatin1Char('A'));
        break;
    case 0xb:
        newCharacter(QLatin1Char('B'));
        break;
    case 0xc:
        newCharacter(QLatin1Char('C'));
        break;
    case 0xd:
        newCharacter(QLatin1Char('D'));
        break;
    case 0xe:
        newCharacter(QLatin1Char('E'));
        break;
    case 0xf:
        newCharacter(QLatin1Char('F'));
        break;
    default:
        Q_ASSERT(0);
        break;
    }
}

//------------------------------------------------------------------------------
// Name: slotHistoryForward
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::slotHistoryForward()
{
    if (history_list_.empty()) {
        return;
    }

    if (history_index_ <= 0) {
        return;
    }

    history_index_--;
    setAmount(history_list_[history_index_]);
}

//------------------------------------------------------------------------------
// Name: slotHistoryBack
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::slotHistoryBack()
{
    if (history_list_.empty()) {
        return;
    }

    if (history_index_ >= history_list_.size()) {
        return;
    }

    setAmount(history_list_[history_index_]);
    history_index_++;
}

//------------------------------------------------------------------------------
// Name: sendEvent
// Desc:
//------------------------------------------------------------------------------
bool KCalcDisplay::sendEvent(Event event)
{
    switch (event) {
    case EventClear:
    case EventReset:
        display_amount_ = KNumber::Zero;
        str_int_ = QStringLiteral("0");
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

//------------------------------------------------------------------------------
// Name: slotCut
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::slotCut()
{
    slotCopy();
    sendEvent(EventReset);
}

//------------------------------------------------------------------------------
// Name: slotCopy
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::slotCopy()
{
    QString txt = text_;

    switch (num_base_) {
    case NB_HEX:
        txt.prepend(QLatin1String("0x"));
        txt.remove(QLatin1Char(' '));
        break;
    case NB_BINARY:
        txt.prepend(QLatin1String("0b"));
        txt.remove(QLatin1Char(' '));
        break;
    case NB_OCTAL:
        txt.prepend(QLatin1String("0"));
        txt.remove(QLatin1Char(' '));
        break;
    case NB_DECIMAL:
        txt.remove(QLocale().groupSeparator());
        break;
    }

    QApplication::clipboard()->setText(txt, QClipboard::Clipboard);
    QApplication::clipboard()->setText(txt, QClipboard::Selection);
}

//------------------------------------------------------------------------------
// Name: slotDisplaySelected
// Desc:
//------------------------------------------------------------------------------
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
    }
}

//------------------------------------------------------------------------------
// Name: slotSelectionTimedOut
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::slotSelectionTimedOut()
{
    lit_ = false;
    invertColors();
    selection_timer_->stop();
}

//------------------------------------------------------------------------------
// Name: invertColors
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::invertColors()
{
    QPalette tmp_palette = palette();
    tmp_palette.setColor(QPalette::Base, palette().color(QPalette::Text));
    tmp_palette.setColor(QPalette::Text, palette().color(QPalette::Base));
    setPalette(tmp_palette);
}

//------------------------------------------------------------------------------
// Name: mousePressEvent
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        lit_ = !lit_;
        button_ = Qt::LeftButton;
    } else {
        button_ = Qt::MiddleButton;
    }

    Q_EMIT clicked();
}

//------------------------------------------------------------------------------
// Name: setPrecision
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::setPrecision(int precision)
{
    precision_ = precision;
}

//------------------------------------------------------------------------------
// Name: setFixedPrecision
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::setFixedPrecision(int precision)
{
    if (fixed_precision_ > precision_) {
        fixed_precision_ = -1;
    } else {
        fixed_precision_ = precision;
    }
}

//------------------------------------------------------------------------------
// Name: setBeep
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::setBeep(bool flag)
{
    beep_ = flag;
}

//------------------------------------------------------------------------------
// Name: setGroupDigits
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::setGroupDigits(bool flag)
{
    groupdigits_ = flag;
}

//------------------------------------------------------------------------------
// Name: setTwosComplement
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::setTwosComplement(bool flag)
{
    twoscomplement_ = flag;
}

//------------------------------------------------------------------------------
// Name: setBinaryGrouping
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::setBinaryGrouping(int digits)
{
    binaryGrouping_ = digits;
}

//------------------------------------------------------------------------------
// Name: setOctalGrouping
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::setOctalGrouping(int digits)
{
    octalGrouping_ = digits;
}

//------------------------------------------------------------------------------
// Name: setHexadecimalGrouping
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::setHexadecimalGrouping(int digits)
{
    hexadecimalGrouping_ = digits;
}

//------------------------------------------------------------------------------
// Name: getAmount
// Desc:
//------------------------------------------------------------------------------
const KNumber &KCalcDisplay::getAmount() const
{
    return display_amount_;
}

//------------------------------------------------------------------------------
// Name: setAmount
// Desc:
//------------------------------------------------------------------------------
bool KCalcDisplay::setAmount(const KNumber &new_amount)
{
    QString display_str;

    str_int_ = QStringLiteral("0");
    str_int_exp_.clear();
    period_ = false;
    neg_sign_ = false;
    eestate_ = false;

    if ((num_base_ != NB_DECIMAL) && (new_amount.type() != KNumber::TYPE_ERROR)) {
        display_amount_ = new_amount.integerPart();

        if (twoscomplement_) {
            // treat number as 64-bit unsigned
            const quint64 tmp_workaround = display_amount_.toUint64();
            display_str = QString::number(tmp_workaround, num_base_).toUpper();
        } else {
            // QString::number treats non-decimal as unsigned
            qint64 tmp_workaround = display_amount_.toInt64();
            const bool neg = tmp_workaround < 0;
            if (neg) {
                tmp_workaround = qAbs(tmp_workaround);
            }

            display_str = QString::number(tmp_workaround, num_base_).toUpper();
            if (neg) {
                display_str.prepend(QLocale().negativeSign());
            }
        }
    } else {
        // num_base_ == NB_DECIMAL || new_amount.type() == KNumber::TYPE_ERROR
        display_amount_ = new_amount;
        display_str = display_amount_.toQString(KCalcSettings::precision(), fixed_precision_);
    }

    setText(display_str);
    Q_EMIT changedAmount(display_amount_);
    return true;
}

//------------------------------------------------------------------------------
// Name: setText
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::setText(const QString &string)
{
    // note that "C" locale is being used internally
    text_ = string;

    // don't mess with special numbers
    const bool special = (string.contains(QLatin1String("nan")) || string.contains(QLatin1String("inf")));

    const bool error = (string.contains(QLatin1String("error")) || string.contains(QLatin1String("malformed")));

    // The decimal mode needs special treatment for two reasons, because: a) it uses KGlobal::locale() to get a localized
    // format and b) it has possible numbers after the decimal place. Neither applies to Binary, Hexadecimal or Octal.

    if ((groupdigits_ || num_base_ == NB_DECIMAL) && !special && !error) {
        switch (num_base_) {
        case NB_DECIMAL:
            text_ = formatDecimalNumber(text_);
            break;

        case NB_BINARY:
            text_ = groupDigits(text_, binaryGrouping_);
            break;

        case NB_OCTAL:
            text_ = groupDigits(text_, octalGrouping_);
            break;

        case NB_HEX:
            text_ = groupDigits(text_, hexadecimalGrouping_);
            break;
        }
    } else if (special) {
#if 0
		// TODO: enable this code, it replaces the "inf" with an actual infinity
		//       symbol, but what should be put into the clip board when they copy?
		if(string.contains(QLatin1String("inf"))) {
			text_.replace("inf", QChar(0x221e));
		}
#endif
    }

    update();
    setAccessibleName(text_); // "Labels should be represented by only QAccessibleInterface and return their text as name"
    Q_EMIT changedText(text_);
}

//------------------------------------------------------------------------------
// Name: setFont
// Desc: Set the base font and recalculate the font size to better fit
//------------------------------------------------------------------------------
void KCalcDisplay::setFont(const QFont &font)
{
    // Overwrite current baseFont
    baseFont_ = font;
    updateFont();
}

//------------------------------------------------------------------------------
// Name: updateFont
// Desc: Update font using baseFont to better fit
//------------------------------------------------------------------------------
void KCalcDisplay::updateFont()
{
    // Make a working copy of the font
    QFont newFont(baseFont());

    // Calculate ideal font size
    // constants arbitrarily chosen, adjust/increase if scaling issues arise
    newFont.setPointSizeF(qMax(double(baseFont().pointSize()), qMin(contentsRect().height() / 4.5, contentsRect().width() / 24.6)));

    // Apply font
    QFrame::setFont(newFont);
}

//------------------------------------------------------------------------------
// Name: baseFont
// Desc:
//------------------------------------------------------------------------------
const QFont& KCalcDisplay::baseFont() const
{
    return baseFont_;
}

//------------------------------------------------------------------------------
// Name: formatDecimalNumber
// Desc: Convert decimal number to locale-dependent format.
//      We cannot use QLocale::formatNumber(), because the
//      precision is limited to "double".
//------------------------------------------------------------------------------
QString KCalcDisplay::formatDecimalNumber(QString string)
{
    QLocale locale;

    string.replace(QLatin1Char('.'), locale.decimalPoint());

    if (groupdigits_ && !(locale.numberOptions() & QLocale::OmitGroupSeparator)) {
        // find position after last digit
        int pos = string.indexOf(locale.decimalPoint());
        if (pos < 0) {
            // do not group digits after the exponent part
            const int expPos = string.indexOf(QLatin1Char('e'));
            if (expPos > 0) {
                pos = expPos;
            } else {
                pos = string.length();
            }
        }

        // find first digit to not group leading spaces or signs
        int firstDigitPos = 0;
        for (int i = 0, total = string.length(); i < total; ++i) {
            if (string.at(i).isDigit()) {
                firstDigitPos = i;
                break;
            }
        }

        const auto groupSeparator = locale.groupSeparator();
        const int groupSize = 3;

        string.reserve(string.length() + (pos - 1) / groupSize);
        while ((pos -= groupSize) > firstDigitPos) {
            string.insert(pos, groupSeparator);
        }
    }

    string.replace(QLatin1Char('-'), locale.negativeSign());
    string.replace(QLatin1Char('+'), locale.positiveSign());

    // Digits in unicode is encoded in contiguous range and with the digit zero as the first.
    // To convert digits to other locales,
    // just add the digit value and the leading zero's code point.
    // ref: Unicode15 chapter 4.6 Numeric Value https://www.unicode.org/versions/Unicode15.0.0/ch04.pdf

    // QLocale switched return type of many functions from QChar to QString,
    // because UTF-16 may need surrogate pairs to represent these fields.
    // We only need digits, thus we only need the first QChar with Qt>=6.

    auto zero = locale.zeroDigit().at(0).unicode();

    for (auto &i : string) {
        if (i.isDigit()) {
            i = QChar(zero + i.digitValue());
        }
    }

    return string;
}

//------------------------------------------------------------------------------
// Name: groupDigits
// Desc:
//------------------------------------------------------------------------------
QString KCalcDisplay::groupDigits(const QString &displayString, int numDigits)
{
    QString tmpDisplayString;
    const int stringLength = displayString.length();

    for (int i = stringLength; i > 0; i--) {
        if (i % numDigits == 0 && i != stringLength) {
            tmpDisplayString = tmpDisplayString + QLatin1Char(' ');
        }

        tmpDisplayString = tmpDisplayString + displayString[stringLength - i];
    }

    return tmpDisplayString;
}

//------------------------------------------------------------------------------
// Name: text
// Desc:
//------------------------------------------------------------------------------
QString KCalcDisplay::text() const
{
    return text_;
}

//------------------------------------------------------------------------------
// Name: setBase
// Desc: change representation of display to new base (i.e. binary, decimal,
//       octal, hexadecimal). The amount being displayed is changed to this
//       base, but for now this amount can not be modified anymore (like
//       being set with "setAmount"). Return value is the new base.
//------------------------------------------------------------------------------
int KCalcDisplay::setBase(NumBase new_base)
{
    switch (new_base) {
    case NB_HEX:
        num_base_ = NB_HEX;
        period_ = false;
        break;
    case NB_DECIMAL:
        num_base_ = NB_DECIMAL;
        break;
    case NB_OCTAL:
        num_base_ = NB_OCTAL;
        period_ = false;
        break;
    case NB_BINARY:
        num_base_ = NB_BINARY;
        period_ = false;
        break;
    default:
        Q_ASSERT(0);
    }

    // reset amount
    setAmount(display_amount_);
    return num_base_;
}

//------------------------------------------------------------------------------
// Name: setStatusText
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::setStatusText(int i, const QString &text)
{
    if (i < NUM_STATUS_TEXT) {
        str_status_[i] = text;
    }

    update();
}

//------------------------------------------------------------------------------
// Name: updateDisplay
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::updateDisplay()
{
    // Put sign in front.
    QString tmp_string;
    if (neg_sign_) {
        tmp_string = QLatin1Char('-') + str_int_;
    } else {
        tmp_string = str_int_;
    }

    bool ok;

    switch (num_base_) {
    case NB_BINARY:
        Q_ASSERT(!period_ && !eestate_);
        setText(tmp_string);
        display_amount_ = KNumber(str_int_.toULongLong(&ok, 2));
        if (neg_sign_) {
            display_amount_ = -display_amount_;
        }
        break;

    case NB_OCTAL:
        Q_ASSERT(!period_ && !eestate_);
        setText(tmp_string);
        display_amount_ = KNumber(str_int_.toULongLong(&ok, 8));
        if (neg_sign_) {
            display_amount_ = -display_amount_;
        }
        break;

    case NB_HEX:
        Q_ASSERT(!period_ && !eestate_);
        setText(tmp_string);
        display_amount_ = KNumber(str_int_.toULongLong(&ok, 16));
        if (neg_sign_) {
            display_amount_ = -display_amount_;
        }
        break;

    case NB_DECIMAL:
        if (!eestate_) {
            setText(tmp_string);
            display_amount_ = KNumber(tmp_string);
        } else {
            if (str_int_exp_.isNull()) {
                // add 'e0' to display but not to conversion
                display_amount_ = KNumber(tmp_string);
                setText(tmp_string + QLatin1String("e0"));
            } else {
                tmp_string += QLatin1Char('e') + str_int_exp_;
                setText(tmp_string);
                display_amount_ = KNumber(tmp_string);
            }
        }
        break;

    default:
        Q_ASSERT(0);
    }

    Q_EMIT changedAmount(display_amount_);
}

//------------------------------------------------------------------------------
// Name: newCharacter
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::newCharacter(const QChar new_char)
{
    // test if character is valid
    switch (new_char.toLatin1()) {
    case 'e':
        // EE can be set only once and in decimal mode
        if (num_base_ != NB_DECIMAL || eestate_) {
            if (beep_) {
                KNotification::beep();
            }
            return;
        }
        eestate_ = true;
        break;

    case 'F':
    case 'E':
    case 'D':
    case 'C':
    case 'B':
    case 'A':
        if (num_base_ == NB_DECIMAL) {
            if (beep_) {
                KNotification::beep();
            }
            return;
        }
        Q_FALLTHROUGH();
    case '9':
    case '8':
        if (num_base_ == NB_OCTAL) {
            if (beep_) {
                KNotification::beep();
            }
            return;
        }
        Q_FALLTHROUGH();
    case '7':
    case '6':
    case '5':
    case '4':
    case '3':
    case '2':
        if (num_base_ == NB_BINARY) {
            if (beep_) {
                KNotification::beep();
            }
            return;
        }
        Q_FALLTHROUGH();
    case '1':
    case '0':
        break;

    default:
        if (new_char == QLocale().decimalPoint()) {
            // Period can be set only once and only in decimal
            // mode, also not in EE-mode
            if (num_base_ != NB_DECIMAL || period_ || eestate_) {
                if (beep_) {
                    KNotification::beep();
                }
                return;
            }
            period_ = true;
        } else {
            if (beep_) {
                KNotification::beep();
            }
            return;
        }
    }

    // change exponent or mantissa
    if (eestate_) {
        // ignore '.' before 'e'. turn e.g. '123.e' into '123e'
        if (new_char == QLatin1Char('e') && str_int_.endsWith(QLocale().decimalPoint())) {
            str_int_.chop(1);
            period_ = false;
        }

        // 'e' only starts ee_mode, leaves strings unchanged
        // do not add '0' if at start of exp
        if (new_char != QLatin1Char('e') && !(str_int_exp_.isNull() && new_char == QLatin1Char('0'))) {
            str_int_exp_.append(new_char);
        }
    } else {
        // handle first character
        if (str_int_ == QLatin1Char('0')) {
            switch (new_char.toLatin1()) {
            case 'e':
                // display "0e" not just "e"
                // "0e" does not make sense either, but...
                str_int_.append(new_char);
                break;
            default:
                if (new_char == QLocale().decimalPoint()) {
                    // display "0." not just "."
                    str_int_.append(new_char);
                } else {
                    // no leading '0's
                    str_int_[0] = new_char;
                }
            }
        } else {
            str_int_.append(new_char);
        }
    }

    updateDisplay();
}

//------------------------------------------------------------------------------
// Name: deleteLastDigit
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::deleteLastDigit()
{
    // Only partially implemented !!
    if (eestate_) {
        if (str_int_exp_.isNull()) {
            eestate_ = false;
        } else {
            const int length = str_int_exp_.length();
            if (length > 1) {
                str_int_exp_.chop(1);
            } else {
                str_int_exp_ = QLatin1String((const char *)nullptr);
            }
        }
    } else {
        const int length = str_int_.length();
        if (length > 1) {
            if (str_int_[length - 1] == QLocale().decimalPoint()) {
                period_ = false;
            }
            str_int_.chop(1);
        } else {
            Q_ASSERT(!period_);
            str_int_[0] = QLatin1Char('0');
        }
    }

    updateDisplay();
}

//------------------------------------------------------------------------------
// Name: changeSign
// Desc: change Sign of display. Problem: Only possible here, when in input
//       mode. Otherwise return 'false' so that the kcalc_core can handle
//       things.
//------------------------------------------------------------------------------
bool KCalcDisplay::changeSign()
{
    // stupid way, to see if in input_mode or display_mode
    if (str_int_ == QLatin1Char('0')) {
        return false;
    }

    if (eestate_) {
        if (!str_int_exp_.isNull()) {
            if (str_int_exp_[0] != QLatin1Char('-')) {
                str_int_exp_.prepend(QLatin1Char('-'));
            } else {
                str_int_exp_.remove(QLatin1Char('-'));
            }
        }
    } else {
        neg_sign_ = !neg_sign_;
    }

    updateDisplay();
    return true;
}

//------------------------------------------------------------------------------
// Name: initStyleOption
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::initStyleOption(QStyleOptionFrame *option) const
{
    if (!option) {
        return;
    }

    option->initFrom(this);
    option->state &= ~QStyle::State_HasFocus; // don't draw focus highlight

    if (frameShadow() == QFrame::Sunken) {
        option->state |= QStyle::State_Sunken;
    } else if (frameShadow() == QFrame::Raised) {
        option->state |= QStyle::State_Raised;
    }

    option->lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, option, this);
    option->midLineWidth = 0;
}

//------------------------------------------------------------------------------
// Name: paintEvent
// Desc:
//------------------------------------------------------------------------------
void KCalcDisplay::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QStyleOptionFrame option;
    initStyleOption(&option);

    style()->drawPrimitive(QStyle::PE_PanelLineEdit, &option, &painter, this);

    // draw display text
    const int margin = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, nullptr);
    QRect cr = contentsRect();
    cr.adjust(margin * 2, 0, -margin * 2, 0); // provide a margin

    const int align = QStyle::visualAlignment(layoutDirection(), Qt::AlignRight | Qt::AlignVCenter);
    painter.drawText(cr, align | Qt::TextSingleLine, text_);

    // draw the status texts using half of the normal
    // font size but not smaller than 7pt
    QFont fnt(font());
    fnt.setPointSizeF(qMax((fnt.pointSize() / 2.0), 7.0));
    painter.setFont(fnt);

    QFontMetrics fm(fnt);
    const uint w = fm.boundingRect(QStringLiteral("________")).width();
    const uint h = fm.height();

    for (int n = 0; n < NUM_STATUS_TEXT; ++n) {
        painter.drawText(5 + n * w, h, str_status_[n]);
    }
}

//------------------------------------------------------------------------------
// Name: resizeEvent
// Desc: resize display and adjust font size
//------------------------------------------------------------------------------
void KCalcDisplay::resizeEvent(QResizeEvent* event)
{
    QFrame::resizeEvent(event);

    // Update font size
    updateFont();
}

//------------------------------------------------------------------------------
// Name: sizeHint
// Desc:
//------------------------------------------------------------------------------
QSize KCalcDisplay::sizeHint() const
{
    // font metrics of base font
    const QFontMetrics fmBase(baseFont());
    
    // basic size
    QSize sz = fmBase.size(0, QStringLiteral("M"));

    // expanded by 3/4 font height to make room for the status texts
    QFont fnt(baseFont());
    fnt.setPointSize(qMax(((fnt.pointSize() * 3) / 4), 7));

    const QFontMetrics fm(fnt);
    sz.setHeight(sz.height() + fm.height());

    QStyleOptionFrame option;
    initStyleOption(&option);

    return (style()->sizeFromContents(QStyle::CT_LineEdit, &option, sz, this));
}

#include "moc_kcalcdisplay.cpp"
