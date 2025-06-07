/*
    SPDX-FileCopyrightText: 2012-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2006 Michel Marti <mma@objectxp.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_bitset.h"
#include "kcalc_bitbutton.h"

#include <KLocalizedString>
#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>

#define BITSET_HEIGHT_RATIO (0.25) // 20% of available main window height
#define BITSET_MAX_WIDTH_RATIO (6.0) // Bits frame width cannot be wider than height multiped by it

// TODO: I think it would actually be appropriate to use a std::bitset<64>
//       for the internal representation of this class perhaps
//       the only real caveat is the conversion to/from quint64

using namespace Qt::Literals::StringLiterals;

//------------------------------------------------------------------------------
// Name: KCalcBitset
// Desc: constructor
//------------------------------------------------------------------------------
KCalcBitset::KCalcBitset(QWidget *parent)
    : QWidget(parent)
    , bit_button_group_(new QButtonGroup(this))
    , value_(0)
    , m_readOnly(false)
{
    bit_button_group_->setExclusive(false);

    connect(bit_button_group_, &QButtonGroup::buttonClicked, this, &KCalcBitset::slotToggleBit);

    // smaller label font
    QFont fnt = font();
    if (fnt.pointSize() > 6) {
        fnt.setPointSize(fnt.pointSize() - 1);
    }

    // main layout
    auto layout = new QGridLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(0);

    // create bits
    auto labelPalette = palette();
    auto textColor = palette().color(QPalette::Text);
    textColor.setAlpha(150);
    labelPalette.setColor(QPalette::WindowText, textColor);
    int bitCounter = 63;
    for (int rows = 0; rows < 2; rows++) {
        for (int cols = 0; cols < 4; cols++) {
            // two rows of four words
            auto const wordlayout = new QHBoxLayout();
            wordlayout->setContentsMargins(2, 2, 2, 2);
            wordlayout->setSpacing(2);
            layout->addLayout(wordlayout, rows, cols);

            for (int bit = 0; bit < 8; bit++) {
                auto const tmpBitButton = new BitButton(this);
                tmpBitButton->setToolTip(i18nc("@info:tooltip", "Bit %1 = %2", bitCounter, 1ULL << bitCounter));
                wordlayout->addWidget(tmpBitButton, 1);
                bit_button_group_->addButton(tmpBitButton, bitCounter);
                bitCounter--;
            }

            // label word
            auto label = new QLabel(this);
            label->setFont(fnt);
            // Make all labels have same size and space between words
            label->setMinimumSize(label->fontMetrics().size(Qt::TextSingleLine, u"56"_s));
            label->setText(QString::number(bitCounter + 1));
            label->setAlignment(Qt::AlignCenter);
            label->setPalette(labelPalette);
            wordlayout->addWidget(label, 1, Qt::AlignVCenter);
            wordlayout->addStretch(1);
        }
    }

    calculateMaxSize();
}

//------------------------------------------------------------------------------
// Name: setReadOnly
// Desc:
//------------------------------------------------------------------------------
void KCalcBitset::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
}

//------------------------------------------------------------------------------
// Name: setValue
// Desc: set the value of the bitset based on an unsigned 64-bit number
//------------------------------------------------------------------------------
void KCalcBitset::setValue(qint64 value)
{
    if (value_ == value) {
        // don't waste time if there was no change.
        return;
    }

    value_ = value;

    // set each bit button
    for (int i = 0; i < 64; i++) {
        if (auto bb = qobject_cast<BitButton *>(bit_button_group_->button(i))) {
            bb->setOn(value & 1);
        }
        value >>= 1;
    }
}

//------------------------------------------------------------------------------
// Name: clear
// Desc:
//------------------------------------------------------------------------------
void KCalcBitset::clear()
{
    // TODO implement a better looking cleared state
    setValue(0);
    m_readOnly = false;
}

//------------------------------------------------------------------------------
// Name: getValue
// Desc: returns the bitset value as an unsigned 64-bit number
//------------------------------------------------------------------------------
qint64 KCalcBitset::getValue() const
{
    return value_;
}

//------------------------------------------------------------------------------
// Name: calculateMaxSize
// Desc: finds maximum bits frame size according to parent
//------------------------------------------------------------------------------
void KCalcBitset::calculateMaxSize()
{
    QWidget *parent = parentWidget();
    if (parent) {
        qreal optHeight = parent->contentsRect().height() * BITSET_HEIGHT_RATIO;
        int optWidth = qCeil(optHeight * BITSET_MAX_WIDTH_RATIO);
        if (optWidth > parent->contentsRect().width()) {
            optHeight *= static_cast<qreal>(parent->contentsRect().width()) / optWidth;
        }
        setMaximumSize(qCeil(optHeight * BITSET_MAX_WIDTH_RATIO), qCeil(optHeight));
    }
}

//------------------------------------------------------------------------------
// Name: slotToggleBit
// Desc: inverts the value of a single bit
//------------------------------------------------------------------------------
void KCalcBitset::slotToggleBit(QAbstractButton *button)
{
    if (button && !m_readOnly) {
        const int bit = bit_button_group_->id(button);
        const qint64 nv = getValue() ^ (1LL << bit);
        setValue(nv);
        Q_EMIT valueChanged(value_);
    }
}

//------------------------------------------------------------------------------
// Name: resizeEvent
// Desc: make sure all bitButtons have the same size
//------------------------------------------------------------------------------
void KCalcBitset::resizeEvent(QResizeEvent *event)
{
    // Call the overridden resize event
    QWidget::resizeEvent(event);

    // Get the minimum size of all buttons
    int minWidth = INT_MAX;
    int minHeight = INT_MAX;
    for (QObject *obj : bit_button_group_->buttons()) {
        if (auto const button = qobject_cast<BitButton *>(obj)) {
            minWidth = qMin(minWidth, button->rect().width());
            minHeight = qMin(minHeight, button->rect().height());
        }
    }

    // If this worked, set the renderSize for all BitButtons
    if (minWidth != INT_MAX && minHeight != INT_MAX) {
        // Make sure the size is square
        if (minWidth > minHeight)
            minWidth = minHeight;
        else if (minHeight > minWidth)
            minHeight = minWidth;

        // Set it for all buttons
        for (QObject *obj : bit_button_group_->buttons()) {
            if (auto const button = qobject_cast<BitButton *>(obj)) {
                QSize size = QSize(button->renderSize());
                size.setWidth(minWidth);
                size.setHeight(minHeight);
                button->setRenderSize(size);
            }
        }
    }

    updateGeometry();
}

#include "moc_kcalc_bitset.cpp"
