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
QList<ScienceConstant> scienceConstantList;

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

} // namespace

void KCalcConstMenu::initConsts(QDomDocument &doc)
{
    // print out the element names of all elements that are direct children
    // of the outermost element.
    QDomElement docElem = doc.documentElement();

    QDomNode n = docElem.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if (!e.isNull() && e.tagName() == QLatin1String("constant")) {
            ScienceConstant tmpConst;

            tmpConst.name = e.attributeNode(QStringLiteral("name")).value();
            tmpConst.label = e.attributeNode(QStringLiteral("symbol")).value();
            tmpConst.value = e.attributeNode(QStringLiteral("value")).value();

            QString tmpStrCategory = e.attributeNode(QStringLiteral("category")).value();

            tmpConst.category = stringToCategory(tmpStrCategory);
            tmpConst.whatsthis = e.firstChildElement(QStringLiteral("description")).text();

            scienceConstantList.append(tmpConst);
        }
        n = n.nextSibling();
    }
}

void KCalcConstMenu::initAll()
{
    QMenu *mathMenu = addMenu(i18n("Mathematics"));
    QMenu *emMenu = addMenu(i18n("Electromagnetism"));
    QMenu *nuclearMenu = addMenu(i18n("Atomic && Nuclear"));
    QMenu *thermoMenu = addMenu(i18n("Thermodynamics"));
    QMenu *gravitationMenu = addMenu(i18n("Gravitation"));

    connect(this, &KCalcConstMenu::triggered, this, &KCalcConstMenu::slotPassSignalThrough);

    for (int i = 0, total = scienceConstantList.size(); i < total; ++i) {
        const auto scienceConstantListItem = scienceConstantList.at(i);
        auto tmpAction = new QAction(i18n(scienceConstantListItem.name.toLatin1().data()), this);
        tmpAction->setData(QVariant(i));
        if (scienceConstantListItem.category & Mathematics) {
            mathMenu->addAction(tmpAction);
        }
        if (scienceConstantListItem.category & Electromagnetic) {
            emMenu->addAction(tmpAction);
        }
        if (scienceConstantListItem.category & Nuclear) {
            nuclearMenu->addAction(tmpAction);
        }
        if (scienceConstantListItem.category & Thermodynamics) {
            thermoMenu->addAction(tmpAction);
        }
        if (scienceConstantListItem.category & Gravitation) {
            gravitationMenu->addAction(tmpAction);
        }
    }
}

void KCalcConstMenu::slotPassSignalThrough(QAction *chosenConst)
{
    bool tmpBool = false;
    int chosenConstIdx = (chosenConst->data()).toInt(&tmpBool);
    Q_EMIT triggeredConstant(scienceConstantList.at(chosenConstIdx));
}

KCalcConstMenu::KCalcConstMenu(const QString &title, QWidget *parent)
    : QMenu(title, parent)
{
    initAll();
}

KCalcConstMenu::KCalcConstMenu(QWidget *parent)
    : QMenu(parent)
{
    initAll();
}

#include "moc_kcalc_const_menu.cpp"
