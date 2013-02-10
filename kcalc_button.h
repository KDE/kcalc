/*
Copyright (C) 2001 - 2013 Evan Teran
                          evan.teran@gmail.com
						  
Copyright (C) 2003 - 2005 Klaus Niederkrueger
                          kniederk@math.uni-koeln.de

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KCALC_BUTTON_H_
#define KCALC_BUTTON_H_

#include <QMap>
#include <kpushbutton.h>

// The class KCalcButton is an overridden KPushButton. It offers extra
// functionality e.g. text can be richtext, and the button can be
// told to display its shortcuts in the label, but the most important
// thing is that the button may have several modes with corresponding
// labels and tooltips. When one switches modes, the corresponding
// label is displayed.


enum ButtonModeFlags {ModeNormal = 0, ModeShift = 1, ModeHyperbolic = 2};


// Each kcalc button can be in one of several modes.
// The following class describes label, tooltip etc. for each mode...
class ButtonMode
{
public:
    ButtonMode() {}
    ButtonMode(const QString &label,
               const QString &tooltip)
            : label(label), tooltip(tooltip) { }

    QString label;
    QString tooltip;
};


class KCalcButton : public KPushButton
{
    Q_OBJECT

public:
    explicit KCalcButton(QWidget *parent);
    KCalcButton(const QString &label, QWidget *parent,
                const QString &tooltip = QString());

    void addMode(ButtonModeFlags mode, const QString &label,
                 const QString &tooltip);

    virtual QSize sizeHint() const; // reimp

    void setFont(const QFont &fnt);
    void setText(const QString &text);   // reimp
    void setToolTip(const QString &tip);   // reimp

public slots:
    void slotSetMode(ButtonModeFlags mode, bool flag);
    void slotSetAccelDisplayMode(bool flag);

protected:
    virtual void paintEvent(QPaintEvent *e);

private:
    void calcSizeHint();

private:
    bool show_shortcut_mode_;
    ButtonModeFlags mode_flags_;
    QMap<ButtonModeFlags, ButtonMode> mode_;
    QSize size_;
};

#endif
