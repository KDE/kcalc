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

enum {
    ErrorTextFontpointReduction = 6
};

const QString KCalcDisplay::m_mathErrorText = QStringLiteral("Math error");
const QString KCalcDisplay::m_syntaxErrorText = QStringLiteral("Syntax error");
const QString KCalcDisplay::m_malformedExpressionText = QStringLiteral("Malformed expression");

KCalcDisplay::KCalcDisplay(QWidget *parent)
    : QFrame(parent)
    , m_usingTempSettings(false)
    , m_beep(false)
    , m_groupDigits(true)
    , m_twosComplement(true)
    , m_button(0)
    , m_lit(false)
    , m_numBase(NbDecimal)
    , m_precision(9)
    , m_fixedPrecision(-1)
    , m_displayAmount(0)
    , m_selectionTimer(new QTimer(this))
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
    connect(m_selectionTimer, &QTimer::timeout, this, &KCalcDisplay::slotSelectionTimedOut);
    connect(this, &KCalcDisplay::changedText, this, &KCalcDisplay::restoreSettings);
    connect(this, &KCalcDisplay::changedAmount, this, &KCalcDisplay::restoreSettings);

    sendEvent(EventReset);
}

KCalcDisplay::~KCalcDisplay() = default;

void KCalcDisplay::changeSettings()
{
    KColorScheme schemeView(QPalette::Active, KColorScheme::View);
    m_basePalette = palette();
    m_baseFont = KCalcSettings::displayFont();

    m_basePalette.setColor(QPalette::Text, KCalcSettings::followSystemTheme() ? schemeView.foreground().color() : KCalcSettings::foreColor());
    m_basePalette.setColor(QPalette::Base, KCalcSettings::followSystemTheme() ? schemeView.background().color() : KCalcSettings::backColor());

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
        setPalette(m_basePalette);
        updateDisplay();
    }
}

void KCalcDisplay::updateFromCore(const CalcEngine &core)
{
    bool tmpError = false;
    const KNumber &output = core.lastOutput(tmpError);
    setAmount(output);
}

bool KCalcDisplay::sendEvent(Event event)
{
    switch (event) {
    case EventClear:
    case EventReset:
        m_displayAmount = KNumber::Zero;
        m_text.clear();

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
    newFont.setPointSize(newFont.pointSize() - ErrorTextFontpointReduction);
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
        setUnformattedText(m_mathErrorText);
        break;
    case SyntaxError:
        setUnformattedText(m_syntaxErrorText);
        break;
    case MalformedExpression:
        setUnformattedText(m_malformedExpressionText);
        break;
    default:
        Q_UNREACHABLE();
    }

    if (m_beep) {
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
    QString txt = m_text;

    switch (m_numBase) {
    case NbHex:
        txt.prepend(QLatin1String("0x"));
        txt.remove(QLatin1Char(' '));
        break;
    case NbBinary:
        txt.prepend(QLatin1String("0b"));
        txt.remove(QLatin1Char(' '));
        break;
    case NbOctal:
        txt.prepend(QLatin1String("0o"));
        txt.remove(QLatin1Char(' '));
        break;
    case NbDecimal:
        txt.remove(QLocale().groupSeparator());
        break;
    }

    QApplication::clipboard()->setText(txt, QClipboard::Clipboard);
    QApplication::clipboard()->setText(txt, QClipboard::Selection);
}

void KCalcDisplay::slotDisplaySelected()
{
    if (m_button == Qt::LeftButton) {
        if (m_lit) {
            slotCopy();
            m_selectionTimer->start(100);
        } else {
            m_selectionTimer->stop();
        }

        invertColors();
    }
}

void KCalcDisplay::slotSelectionTimedOut()
{
    m_lit = false;
    invertColors();
    m_selectionTimer->stop();
}

void KCalcDisplay::invertColors()
{
    QPalette tmpPalette = palette();
    tmpPalette.setColor(QPalette::Base, palette().color(QPalette::Text));
    tmpPalette.setColor(QPalette::Text, palette().color(QPalette::Base));
    setPalette(tmpPalette);
}

void KCalcDisplay::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_lit = !m_lit;
        m_button = Qt::LeftButton;
    } else {
        m_button = Qt::MiddleButton;
    }

    Q_EMIT clicked();
}

void KCalcDisplay::setPrecision(int precision)
{
    m_precision = precision;
}

void KCalcDisplay::setFixedPrecision(int precision)
{
    if (m_fixedPrecision > m_precision) {
        m_fixedPrecision = -1;
    } else {
        m_fixedPrecision = precision;
    }
}

void KCalcDisplay::setBeep(bool flag)
{
    m_beep = flag;
}

void KCalcDisplay::setGroupDigits(bool flag)
{
    m_groupDigits = flag;
}

void KCalcDisplay::setTwosComplement(bool flag)
{
    m_twosComplement = flag;
}

void KCalcDisplay::setBinaryGrouping(int digits)
{
    m_binaryGrouping = digits;
}

void KCalcDisplay::setOctalGrouping(int digits)
{
    m_octalGrouping = digits;
}

void KCalcDisplay::setHexadecimalGrouping(int digits)
{
    m_hexadecimalGrouping = digits;
}

const KNumber &KCalcDisplay::getAmount() const
{
    return m_displayAmount;
}

QString KCalcDisplay::getAmountQString(bool addPreffix /*= true*/) const
{
    QString amountQString = m_text;

    switch (m_numBase) {
    case NbHex:
        if (addPreffix) {
            amountQString.prepend(QLatin1String("0x"));
        }
        amountQString.remove(QLatin1Char(' '));
        break;
    case NbBinary:
        if (addPreffix) {
            amountQString.prepend(QLatin1String("0b"));
        }
        amountQString.remove(QLatin1Char(' '));
        break;
    case NbOctal:
        if (addPreffix) {
            amountQString.prepend(QLatin1String("0o"));
        }
        amountQString.remove(QLatin1Char(' '));
        break;
    case NbDecimal:
        amountQString.remove(QLocale().groupSeparator());
        break;
    }
    return amountQString;
}

bool KCalcDisplay::setAmount(const KNumber &newAmount)
{
    QString displayStr;

    if ((m_numBase != NbDecimal) && (newAmount.type() != KNumber::TypeError)) {
        m_displayAmount = newAmount.integerPart();

        if (m_twosComplement) {
            // treat number as 64-bit unsigned
            const quint64 tmpWorkaround = m_displayAmount.toUint64();
            displayStr = QString::number(tmpWorkaround, m_numBase).toUpper();
        } else {
            // QString::number treats non-decimal as unsigned
            qint64 tmpWorkaround = m_displayAmount.toInt64();
            const bool neg = tmpWorkaround < 0;
            if (neg) {
                tmpWorkaround = qAbs(tmpWorkaround);
            }

            displayStr = QString::number(tmpWorkaround, m_numBase).toUpper();
            if (neg) {
                displayStr.prepend(QLocale().negativeSign());
            }
        }
    } else {
        // numBase == NbDecimal || newAmount.type() == KNumber::TypeError
        m_displayAmount = newAmount;
        displayStr = m_displayAmount.toQString(KCalcSettings::precision(), m_fixedPrecision);
    }

    // TODO: to avoid code duplication, don't format complex number for now,
    //       we need to mode this functionality to the KNumber library
    if (m_displayAmount.type() != KNumber::TypeComplex) {
        displayStr = formatNumber(displayStr);
    }

    setText(displayStr);

    Q_EMIT changedAmount(m_displayAmount);
    return true;
}

void KCalcDisplay::setUnformattedText(const QString &string)
{
    m_text = string;
    update();
    setAccessibleName(m_text);
}

void KCalcDisplay::setText(const QString &string)
{
    // note that "C" locale is being used internally
    m_text = string;
    // m_text = formatNumber(m_text);

    update();
    setAccessibleName(m_text); // "Labels should be represented by only QAccessibleInterface and return their text as name"
    Q_EMIT changedText(m_text);
}

void KCalcDisplay::setFont(const QFont &font)
{
    // Overwrite current baseFont
    m_baseFont = font;
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
        setPalette(m_basePalette);
    }
}

const QFont &KCalcDisplay::baseFont() const
{
    return m_baseFont;
}

QString KCalcDisplay::formatNumber(const QString &string)
{
    QString formattedNumber = string;
    const bool special = (string.contains(QLatin1String("nan")) || string.contains(QLatin1String("inf")));

    // The decimal mode needs special treatment for two reasons, because: a) it uses KGlobal::locale() to get a localized
    // format and b) it has possible numbers after the decimal place. Neither applies to Binary, Hexadecimal or Octal.

    if ((m_groupDigits || m_numBase == NbDecimal) && !special) {
        switch (m_numBase) {
        case NbDecimal:
            formattedNumber = formatDecimalNumber(string);
            break;
        case NbBinary:
            formattedNumber = groupDigits(string, m_binaryGrouping);
            break;
        case NbOctal:
            formattedNumber = groupDigits(string, m_octalGrouping);
            break;
        case NbHex:
            formattedNumber = groupDigits(string, m_hexadecimalGrouping);
            break;
        }
    }

    return formattedNumber;
}

QString KCalcDisplay::formatDecimalNumber(QString string)
{
    QLocale locale;

    string.replace(QLatin1Char('.'), locale.decimalPoint());

    if (m_groupDigits && !(locale.numberOptions() & QLocale::OmitGroupSeparator)) {
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
    return m_text;
}

int KCalcDisplay::setBase(NumBase base)
{
    switch (base) {
    case NbHex:
        m_numBase = NbHex;
        break;
    case NbDecimal:
        m_numBase = NbDecimal;
        break;
    case NbOctal:
        m_numBase = NbOctal;
        break;
    case NbBinary:
        m_numBase = NbBinary;
        break;
    default:
        Q_ASSERT(0);
    }

    setAmount(m_displayAmount);

    return m_numBase;
}

void KCalcDisplay::setStatusText(int i, const QString &text)
{
    if (i < NumStatusText) {
        m_strStatus[i] = text;
    }

    update();
}

void KCalcDisplay::updateDisplay()
{
    QString txt = m_text;
    txt.remove(QLatin1Char(' '));
    if (m_numBase == NbDecimal) {
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
    painter.drawText(cr, align | Qt::TextSingleLine, m_text);

    // draw the status texts using half of the normal
    // font size but not smaller than 7pt
    QFont fnt(font());
    fnt.setPointSizeF(qMax((fnt.pointSize() / 2.0), 7.0));
    painter.setFont(fnt);

    QFontMetrics fm(fnt);
    const uint w = fm.boundingRect(QStringLiteral("________")).width();
    const uint h = fm.height();

    for (int n = 0; n < NumStatusText; ++n) {
        painter.drawText(5 + n * w, h, m_strStatus[n]);
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
