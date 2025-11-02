/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2003-2005 Klaus Niederkrueger <kniederk@math.uni-koeln.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QMap>
#include <QPushButton>

// The class KCalcButton is an overridden QPushButton. It offers extra
// functionality e.g. text can be richtext, and the button can be
// told to display its shortcuts in the label, but the most important
// thing is that the button may have several modes with corresponding
// labels and tooltips. When one switches modes, the corresponding
// label is displayed.

enum ButtonModeFlags {
    ModeNormal = 0,
    ModeShift = 1,
    ModeHyperbolic = 2
};

// Each kcalc button can be in one of several modes.
// The following class describes label, tooltip etc. for each mode...
class ButtonMode
{
public:
    ButtonMode() = default;

    ButtonMode(const QString &label, const QString &tooltip)
        : label(label)
        , tooltip(tooltip)
    {
    }

    QString label;
    QString tooltip;
};

class KCalcButton : public QPushButton
{
    Q_OBJECT

public:
    explicit KCalcButton(QWidget *parent);
    KCalcButton(const QString &label, QWidget *parent, const QString &tooltip = QString());

    void addMode(ButtonModeFlags mode, const QString &label, const QString &tooltip);

    QSize sizeHint() const override;

    void setFont(const QFont &fnt);
    void setTextColor(const QColor &color);
    void setText(const QString &text); // reimp
    void setToolTip(const QString &tip); // reimp

public Q_SLOTS:
    void slotSetMode(ButtonModeFlags mode, bool flag);
    void slotSetAccelDisplayMode(bool flag);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    void calcSizeHint();

private:
    bool m_showShortcutMode = false;
    ButtonModeFlags m_modeFlags;
    QMap<ButtonModeFlags, ButtonMode> m_mode;
    QSize m_size;
    QColor m_textColor;
};
