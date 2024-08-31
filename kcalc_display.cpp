/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 1996-2000 Bernd Johannes Wuebben <wuebben@kde.org>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_display.h"

#include <QApplication>
#include <QClipboard>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QTimer>

#include <KColorScheme>
#include <KNotification>

#include "kcalc_core.h"
#include "kcalc_settings.h"

#define ERROR_TEXT_FONTPOINT_REDUCTION 6

const QString KCalcDisplay::m_MathErrorText = QStringLiteral("Math error");
const QString KCalcDisplay::m_SyntaxErrorText = QStringLiteral("Syntax error");
const QString KCalcDisplay::m_MalformedExpressionText = QStringLiteral("Malformed expression");

KCalcDisplay::KCalcDisplay(QWidget *parent)
    : QFrame(parent)
    , m_usingTempSettings(false)
    , beep_(false)
    , groupdigits_(true)
    , twoscomplement_(true)
    , button_(0)
    , lit_(false)
    , num_base_(NB_DECIMAL)
    , precision_(9)
    , fixed_precision_(-1)
    , display_amount_(0)
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
    connect(this, &KCalcDisplay::changedText, this, &KCalcDisplay::restoreSettings);
    connect(this, &KCalcDisplay::changedAmount, this, &KCalcDisplay::restoreSettings);

    sendEvent(EventReset);
}

KCalcDisplay::~KCalcDisplay() = default;

void KCalcDisplay::changeSettings()
{
    basePalette_ = palette();
    baseFont_ = KCalcSettings::displayFont();

    basePalette_.setColor(QPalette::Text, KCalcSettings::foreColor());
    basePalette_.setColor(QPalette::Base, KCalcSettings::backColor());

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

    updateFont();
    if (!m_usingTempSettings) {
        setPalette(basePalette_);
        updateDisplay();
    }
}

void KCalcDisplay::updateFromCore(const CalcEngine &core)
{
    bool tmp_error;
    const KNumber &output = core.lastOutput(tmp_error);
    setAmount(output);
}

bool KCalcDisplay::sendEvent(Event event)
{
    switch (event) {
    case EventClear:
    case EventReset:
        display_amount_ = KNumber::Zero;
        text_.clear();

        updateDisplay();

        return true;
    case EventError:
        updateDisplay();
        return true;

    default:
        return false;
    }
}

void KCalcDisplay::setTempSettings()
{
    if (m_usingTempSettings) {
        return;
    }
    QFont newFont = font();
    newFont.setWeight(QFont::Weight::ExtraLight);
    newFont.setPointSize(newFont.pointSize() - ERROR_TEXT_FONTPOINT_REDUCTION);
    QFrame::setFont(newFont);

    QPalette tmpPalette = parentWidget()->palette();
    KColorScheme colorScheme(QPalette::Active, KColorScheme::View);
    colorScheme.adjustForeground(tmpPalette, KColorScheme::NegativeText);
    setPalette(tmpPalette);

    m_usingTempSettings = true;
}

void KCalcDisplay::showErrorMessage(ErrorMessage errorMessage)
{
    setTempSettings();

    switch (errorMessage) {
    case MathError:
        setUnformattedText(m_MathErrorText);
        break;
    case SyntaxError:
        setUnformattedText(m_SyntaxErrorText);
        break;
    case MalformedExpression:
        setUnformattedText(m_MalformedExpressionText);
        break;
    default:
        Q_UNREACHABLE();
    }

    if (beep_) {
        KNotification::beep();
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
        txt.prepend(QLatin1String("0o"));
        txt.remove(QLatin1Char(' '));
        break;
    case NB_DECIMAL:
        txt.remove(QLocale().groupSeparator());
        break;
    }

    QApplication::clipboard()->setText(txt, QClipboard::Clipboard);
    QApplication::clipboard()->setText(txt, QClipboard::Selection);
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
    tmp_palette.setColor(QPalette::Base, palette().color(QPalette::Text));
    tmp_palette.setColor(QPalette::Text, palette().color(QPalette::Base));
    setPalette(tmp_palette);
}

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

void KCalcDisplay::setPrecision(int precision)
{
    precision_ = precision;
}

void KCalcDisplay::setFixedPrecision(int precision)
{
    if (fixed_precision_ > precision_) {
        fixed_precision_ = -1;
    } else {
        fixed_precision_ = precision;
    }
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

void KCalcDisplay::setBinaryGrouping(int digits)
{
    binaryGrouping_ = digits;
}

void KCalcDisplay::setOctalGrouping(int digits)
{
    octalGrouping_ = digits;
}

void KCalcDisplay::setHexadecimalGrouping(int digits)
{
    hexadecimalGrouping_ = digits;
}

const KNumber &KCalcDisplay::getAmount() const
{
    return display_amount_;
}

QString KCalcDisplay::getAmountQString(bool addPreffix /*= true*/) const
{
    QString amountQString = text_;

    switch (num_base_) {
    case NB_HEX:
        if (addPreffix) {
            amountQString.prepend(QLatin1String("0x"));
        }
        amountQString.remove(QLatin1Char(' '));
        break;
    case NB_BINARY:
        if (addPreffix) {
            amountQString.prepend(QLatin1String("0b"));
        }
        amountQString.remove(QLatin1Char(' '));
        break;
    case NB_OCTAL:
        if (addPreffix) {
            amountQString.prepend(QLatin1String("0o"));
        }
        amountQString.remove(QLatin1Char(' '));
        break;
    case NB_DECIMAL:
        amountQString.remove(QLocale().groupSeparator());
        break;
    }
    return amountQString;
}

bool KCalcDisplay::setAmount(const KNumber &new_amount)
{
    QString display_str;

    if ((num_base_ != NB_DECIMAL) && (new_amount.type() != KNumber::TypeError)) {
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
        // num_base_ == NB_DECIMAL || new_amount.type() == KNumber::TypeError
        display_amount_ = new_amount;
        display_str = display_amount_.toQString(KCalcSettings::precision(), fixed_precision_);
    }

    // TODO: to avoid code duplication, don't format complex number for now,
    //       we need to mode this functionality to the KNumber library
    if (display_amount_.type() != KNumber::TypeComplex) {
        display_str = formatNumber(display_str);
    }

    setText(display_str);

    Q_EMIT changedAmount(display_amount_);
    return true;
}

void KCalcDisplay::setUnformattedText(const QString &string)
{
    text_ = string;
    update();
    setAccessibleName(text_);
}

void KCalcDisplay::setText(const QString &string)
{
    // note that "C" locale is being used internally
    text_ = string;
    // text_ = formatNumber(text_);

    update();
    setAccessibleName(text_); // "Labels should be represented by only QAccessibleInterface and return their text as name"
    Q_EMIT changedText(text_);
}

void KCalcDisplay::setFont(const QFont &font)
{
    // Overwrite current baseFont
    baseFont_ = font;
    updateFont();
}

void KCalcDisplay::updateFont()
{
    // Make a working copy of the font
    QFont newFont(baseFont());
    // Calculate ideal font size
    // constants arbitrarily chosen, adjust/increase if scaling issues arise
    newFont.setPointSizeF(qMax(double(baseFont().pointSize()), qMin(contentsRect().height() / 4.5, contentsRect().width() / 24.6)));

    QFrame::setFont(newFont);

    if (m_usingTempSettings) {
        setTempSettings();
    }
}

void KCalcDisplay::restoreSettings()
{
    if (m_usingTempSettings) {
        m_usingTempSettings = false;
        updateFont();
        setPalette(basePalette_);
    }
}

const QFont &KCalcDisplay::baseFont() const
{
    return baseFont_;
}

QString KCalcDisplay::formatNumber(QString string)
{
    QString formattedNumber = string;
    const bool special = (string.contains(QLatin1String("nan")) || string.contains(QLatin1String("inf")));

    // The decimal mode needs special treatment for two reasons, because: a) it uses KGlobal::locale() to get a localized
    // format and b) it has possible numbers after the decimal place. Neither applies to Binary, Hexadecimal or Octal.

    if ((groupdigits_ || num_base_ == NB_DECIMAL) && !special) {
        switch (num_base_) {
        case NB_DECIMAL:
            formattedNumber = formatDecimalNumber(string);
            break;
        case NB_BINARY:
            formattedNumber = groupDigits(string, binaryGrouping_);
            break;
        case NB_OCTAL:
            formattedNumber = groupDigits(string, octalGrouping_);
            break;
        case NB_HEX:
            formattedNumber = groupDigits(string, hexadecimalGrouping_);
            break;
        }
    }

    return formattedNumber;
}

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

QString KCalcDisplay::text() const
{
    return text_;
}

int KCalcDisplay::setBase(NumBase base)
{
    switch (base) {
    case NB_HEX:
        num_base_ = NB_HEX;
        break;
    case NB_DECIMAL:
        num_base_ = NB_DECIMAL;
        break;
    case NB_OCTAL:
        num_base_ = NB_OCTAL;
        break;
    case NB_BINARY:
        num_base_ = NB_BINARY;
        break;
    default:
        Q_ASSERT(0);
    }

    setAmount(display_amount_);

    return num_base_;
}

void KCalcDisplay::setStatusText(int i, const QString &text)
{
    if (i < NUM_STATUS_TEXT) {
        str_status_[i] = text;
    }

    update();
}

void KCalcDisplay::updateDisplay()
{
    QString txt = text_;
    txt.remove(QLatin1Char(' '));
    if (num_base_ == NB_DECIMAL) {
        txt.remove(QLocale().groupSeparator());
    }

    setText(formatNumber(txt));
}

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

void KCalcDisplay::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);

    // Update font size
    updateFont();
}

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

#include "moc_kcalc_display.cpp"
