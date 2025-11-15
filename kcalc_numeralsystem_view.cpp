/*
 *    SPDX-FileCopyrightText: 2024 Eugene Popov <popov895@ukr.net>
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kcalc_numeralsystem_view.h"

#include <QApplication>
#include <QBoxLayout>
#include <QClipboard>
#include <QEvent>
#include <QLabel>

#include <KLocalizedString>
#include <KSqueezedTextLabel>

#include "kcalc_settings.h"

// KCalcNumeralSystemLabel

KCalcNumeralSystemLabel::KCalcNumeralSystemLabel(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    m_textLabel = new KSqueezedTextLabel;
    m_textLabel->setContextMenuPolicy(Qt::NoContextMenu);

    m_numeralSystemLabel = new QLabel;

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);
    layout->addWidget(m_textLabel);
    layout->addWidget(m_numeralSystemLabel, 0, Qt::AlignBottom);
    setLayout(layout);

    setContextMenuPolicy(Qt::ActionsContextMenu);

    addAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18nc("@action:inmenu", "&Copy"), this, [this]() {
        QApplication::clipboard()->setText(m_textLabel->fullText());
    });

    updateAlignment();
}

void KCalcNumeralSystemLabel::setNumber(quint64 number, KCalcNumberBase base)
{
    number_ = number;
    number_base_ = base;

    updateLabel();
}

void KCalcNumeralSystemLabel::updateLabel()
{
    QString num_str = QString::number(number_, number_base_).toUpper();
    m_textLabel->setText(KCalcNumberFormatter::formatNumber(num_str, number_base_));
    m_textLabel->setAccessibleName(m_textLabel->fullText());

    m_numeralSystemLabel->setText(QStringLiteral("<sub><b>%1</b></sub>").arg(number_base_));
}

void KCalcNumeralSystemLabel::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    if (event->type() == QEvent::LayoutDirectionChange) {
        updateAlignment();
    }
}

void KCalcNumeralSystemLabel::updateAlignment()
{
    m_textLabel->setAlignment(layoutDirection() == Qt::RightToLeft ? Qt::AlignLeft : Qt::AlignRight);
}

// KCalcNumeralSystemView

KCalcNumeralSystemView::KCalcNumeralSystemView(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    m_label1 = new KCalcNumeralSystemLabel;
    m_label2 = new KCalcNumeralSystemLabel;
    m_label3 = new KCalcNumeralSystemLabel;

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(12);
    layout->addWidget(m_label1, 1);
    layout->addWidget(m_label2, 1);
    layout->addWidget(m_label3, 1);
    setLayout(layout);

    updateLabels();
}

void KCalcNumeralSystemView::setNumber(quint64 number, KCalcNumberBase base)
{
    if (m_number == number && m_base == base) {
        return;
    }

    m_number = number;
    m_base = base;

    updateLabels();
}

void KCalcNumeralSystemView::clearNumber()
{
    if (!m_number) {
        return;
    }

    m_number.reset();

    updateLabels();
}

void KCalcNumeralSystemView::changeSettings()
{
    updateLabels();
}

void KCalcNumeralSystemView::updateLabels()
{
    const bool isValidNumber = m_number.has_value();
    if (isValidNumber) {
        const quint64 number = m_number.value();
        switch (m_base) {
        case NbHex:
            m_label1->setNumber(number, NbDecimal);
            m_label2->setNumber(number, NbOctal);
            m_label3->setNumber(number, NbBinary);
            break;
        case NbOctal:
            m_label1->setNumber(number, NbHex);
            m_label2->setNumber(number, NbDecimal);
            m_label3->setNumber(number, NbBinary);
            break;
        case NbBinary:
            m_label1->setNumber(number, NbHex);
            m_label2->setNumber(number, NbDecimal);
            m_label3->setNumber(number, NbOctal);
            break;
        default: // NbDecimal
            m_label1->setNumber(number, NbHex);
            m_label2->setNumber(number, NbOctal);
            m_label3->setNumber(number, NbBinary);
            break;
        }
    }

    for (KCalcNumeralSystemLabel *label : {m_label1, m_label2, m_label3}) {
        label->setVisible(isValidNumber);
    }
}

#include "moc_kcalc_numeralsystem_view.cpp"
