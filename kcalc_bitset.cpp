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
#include <QPainter>

// TODO: I think it would actually be appropriate to use a std::bitset<64>
//       for the internal representation of this class perhaps
//       the only real caveat is the conversion to/from quint64

//------------------------------------------------------------------------------
// Name: paintEvent
// Desc: draws the button
//------------------------------------------------------------------------------
void BitButton::paintEvent(QPaintEvent *)
{
    uint8_t alpha = 0x60;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(palette().text(), 1);
    pen.setJoinStyle(Qt::MiterJoin);
    painter.setPen(pen);

    if (on_) {
        painter.setBrush(palette().text());
        alpha = 0xB0;
    } else {
        painter.setBrush(palette().base());
    }

    if (over_) {
        painter.setBrush(QColor(palette().text().color().red(), palette().text().color().green(), palette().text().color().blue(), alpha));
    }
    
    // Calculate button size (make it square)
    QRect square = rect();
    if (square.width() > square.height())
        square.setWidth(square.height());
    else if (square.height() > square.width())
        square.setHeight(square.width());

    // Draw button
    painter.translate(QPoint(0, (rect().height() - square.height()) / 2)); // center button
    painter.drawRect(square.adjusted(1, 1, -1, -1));
}

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

    connect(bit_button_group_, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked), this, &KCalcBitset::slotToggleBit);

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
                tmpBitButton->setToolTip(i18n("Bit %1 = %2", bitCounter, 1ULL << bitCounter));
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
}

//------------------------------------------------------------------------------
// Name: setValue
// Desc: set the value of the bitset based on an unsigned 64-bit number
//------------------------------------------------------------------------------
void KCalcBitset::setValue(quint64 value)
{
    if (value_ == value) {
        // don't waste time if there was no change..
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
    if (button) {
        const int bit = bit_button_group_->id(button);
        const quint64 nv = getValue() ^ (1LL << bit);
        setValue(nv);
        Q_EMIT valueChanged(value_);
    }
}
