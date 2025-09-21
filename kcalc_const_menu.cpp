/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 2003-2005 Klaus Niederkrueger <kniederk@math.uni-koeln.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_const_menu.h"
#include "kcalc_debug.h"

#include <QDebug>

#include <KLocalizedString>

namespace
{
QList<science_constant> scienceConstantList;

ConstantCategory stringToCategory(const QString &s)
{
    if (s == QLatin1String("mathematics")) {
        return Mathematics;
    }

    if (s == QLatin1String("electromagnetism")) {
        return Electromagnetic;
    }

    if (s == QLatin1String("nuclear")) {
        return Nuclear;
    }

    if (s == QLatin1String("thermodynamics")) {
        return Thermodynamics;
    }

    if (s == QLatin1String("gravitation")) {
        return Gravitation;
    }

    qCDebug(KCALC_LOG) << "Invalid Category For Constant: " << s;
    return Mathematics;
}

}

void KCalcConstMenu::init_consts(QDomDocument &doc)
{
    // print out the element names of all elements that are direct children
    // of the outermost element.
    QDomElement docElem = doc.documentElement();

    QDomNode n = docElem.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if (!e.isNull() && e.tagName() == QLatin1String("constant")) {
            science_constant tmp_const;

            tmp_const.name = e.attributeNode(QStringLiteral("name")).value();
            tmp_const.label = e.attributeNode(QStringLiteral("symbol")).value();
            tmp_const.value = e.attributeNode(QStringLiteral("value")).value();

            QString tmp_str_category = e.attributeNode(QStringLiteral("category")).value();

            tmp_const.category = stringToCategory(tmp_str_category);
            tmp_const.whatsthis = e.firstChildElement(QStringLiteral("description")).text();

            scienceConstantList.append(tmp_const);
        }
        n = n.nextSibling();
    }
}

void KCalcConstMenu::init_all()
{
    QMenu *math_menu = addMenu(i18n("Mathematics"));
    QMenu *em_menu = addMenu(i18n("Electromagnetism"));
    QMenu *nuclear_menu = addMenu(i18n("Atomic && Nuclear"));
    QMenu *thermo_menu = addMenu(i18n("Thermodynamics"));
    QMenu *gravitation_menu = addMenu(i18n("Gravitation"));

    connect(this, &KCalcConstMenu::triggered, this, &KCalcConstMenu::slotPassSignalThrough);

    for (int i = 0, total = scienceConstantList.size(); i < total; ++i) {
        const auto scienceConstantListItem = scienceConstantList.at(i);
        auto tmp_action = new QAction(i18n(scienceConstantListItem.name.toLatin1().data()), this);
        tmp_action->setData(QVariant(i));
        if (scienceConstantListItem.category & Mathematics)
            math_menu->addAction(tmp_action);
        if (scienceConstantListItem.category & Electromagnetic)
            em_menu->addAction(tmp_action);
        if (scienceConstantListItem.category & Nuclear)
            nuclear_menu->addAction(tmp_action);
        if (scienceConstantListItem.category & Thermodynamics)
            thermo_menu->addAction(tmp_action);
        if (scienceConstantListItem.category & Gravitation)
            gravitation_menu->addAction(tmp_action);
    }
}

void KCalcConstMenu::slotPassSignalThrough(QAction *chosen_const)
{
    bool tmp_bool;
    int chosen_const_idx = (chosen_const->data()).toInt(&tmp_bool);
    Q_EMIT triggeredConstant(scienceConstantList.at(chosen_const_idx));
}

KCalcConstMenu::KCalcConstMenu(const QString &title, QWidget *parent)
    : QMenu(title, parent)
{
    init_all();
}

KCalcConstMenu::KCalcConstMenu(QWidget *parent)
    : QMenu(parent)
{
    init_all();
}

#include "moc_kcalc_const_menu.cpp"
