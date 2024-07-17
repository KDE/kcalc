/*
    SPDX-FileCopyrightText: 2012-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2006 Michel Marti <mma@objectxp.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_bitset.h"
#include "bitbutton.h"

#include <KLocalizedString>
#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>

// TODO: I think it would actually be appropriate to use a std::bitset<64>
//       for the internal representation of this class perhaps
//       the only real caveat is the conversion to/from quint64

//------------------------------------------------------------------------------
// Name: KCalcBitset
// Desc: constructor
//------------------------------------------------------------------------------
KCalcBitset::KCalcBitset(QWidget *parent)
    : QFrame(parent)
    , bit_button_group_(new QButtonGroup(this))
    , value_(0)
{
    setFrameStyle(QFrame::Panel | QFrame::Sunken);

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
                wordlayout->addWidget(tmpBitButton);
                wordlayout->setStretch(bit, 1);
                bit_button_group_->addButton(tmpBitButton, bitCounter);
                bitCounter--;
            }

            // label word
            auto label = new QLabel(this);
            label->setText(QString::number(bitCounter + 1));
            label->setFont(fnt);
            label->setMinimumSize(label->fontMetrics().size(Qt::TextSingleLine, QStringLiteral("56"))); // Make all labels have same size
            wordlayout->addWidget(label);
            wordlayout->setStretch(8, 1);
        }
        layout->setRowStretch(rows, 1);
    }

    // layout stretch for columns
    for (int cols = 0; cols < 4; cols++) {
        layout->setColumnStretch(cols, 1);
    }

    // store current aspect ratio (using width:height)
    QSize initialSize(size());
    if (initialSize.height() != 0.0 && float(initialSize.width()) / float(initialSize.height()) < 2.5) {
        ratio_ = float(initialSize.width()) / float(initialSize.height());
    } else {
        ratio_ = 1.355163727959698; // 538/397
    }
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
void KCalcBitset::setValue(quint64 value)
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
}

//------------------------------------------------------------------------------
// Name: getValue
// Desc: returns the bitset value as an unsigned 64-bit number
//------------------------------------------------------------------------------
quint64 KCalcBitset::getValue() const
{
    return value_;
}

//------------------------------------------------------------------------------
// Name: slotToggleBit
// Desc: inverts the value of a single bit
//------------------------------------------------------------------------------
void KCalcBitset::slotToggleBit(QAbstractButton *button)
{
    if (button && !m_readOnly) {
        const int bit = bit_button_group_->id(button);
        const quint64 nv = getValue() ^ (1LL << bit);
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
    QFrame::resizeEvent(event);

    // Set our maximum size based on the space available in the parent (to keep aspect ratio)
    QWidget *parent = parentWidget();
    if (parent) {
        QSize maxSize(parent->contentsRect().width(), parent->contentsRect().height());
        if (maxSize.width() != 0 && maxSize.height() != 0) {
            float actualRatio = float(maxSize.width()) / float(maxSize.height());

            if (actualRatio > ratio_) {
                // available space is too wide, limit width
                maxSize.setWidth(ratio_ * maxSize.height());
            } else if (actualRatio < ratio_) {
                // available space is too tall, limit height
                maxSize.setHeight(maxSize.width() / ratio_);
            }

            setMaximumSize(maxSize.width(), maxSize.height());
        }
    }

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
