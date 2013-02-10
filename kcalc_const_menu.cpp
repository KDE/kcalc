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

#include "kcalc_const_menu.h"

#include <QDomDocument>
#include <QFile>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>

namespace {
	QList<science_constant> Constants;
	
	ConstantCategory stringToCategory(const QString &s) {
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

		kDebug() << "Invalid Category For Constant: " << s;
		return Mathematics;
	}
	
}


void KCalcConstMenu::init_consts() {
    QDomDocument doc(QLatin1String("list_of_constants"));
    QFile file(KGlobal::dirs()->findResource("appdata", QLatin1String("scienceconstants.xml")));

    if (!file.open(QIODevice::ReadOnly)) {
        kDebug() << "Did not find file \"scienceconstants.xml\". No constants will be available.";
        return;
    }
    if (!doc.setContent(&file)) {
        file.close();
        kDebug() << "The file \"scienceconstants.xml\" does not seem to be a valid description file. No constants will be available.";
        return;
    }
    file.close();

    // print out the element names of all elements that are direct children
    // of the outermost element.
    QDomElement docElem = doc.documentElement();

    int i = 0;
    QDomNode n = docElem.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if (!e.isNull() && e.tagName() == QLatin1String("constant")) {
            science_constant tmp_const;

            tmp_const.name = I18N_NOOP(e.attributeNode(QLatin1String("name")).value());
            tmp_const.label = e.attributeNode(QLatin1String("symbol")).value();
            tmp_const.value = e.attributeNode(QLatin1String("value")).value();

            QString tmp_str_category = e.attributeNode(QLatin1String("category")).value();

			tmp_const.category  = stringToCategory(tmp_str_category);
            tmp_const.whatsthis = e.firstChildElement(QLatin1String("description")).text();

            Constants.append(tmp_const);
        }
        n = n.nextSibling();
        i++;
    }
}

void KCalcConstMenu::init_all()
{
    QMenu *math_menu        = addMenu(i18n("Mathematics"));
    QMenu *em_menu          = addMenu(i18n("Electromagnetism"));
    QMenu *nuclear_menu     = addMenu(i18n("Atomic && Nuclear"));
    QMenu *thermo_menu      = addMenu(i18n("Thermodynamics"));
    QMenu *gravitation_menu = addMenu(i18n("Gravitation"));

    connect(this, SIGNAL(triggered(QAction*)), SLOT(slotPassSignalThrough(QAction*)));


    for (int i = 0; i < Constants.size(); i++) {
        QAction *tmp_action = new QAction(i18n(Constants.at(i).name.toAscii().data()), this);
        tmp_action->setData(QVariant(i));
        if (Constants.at(i).category  &  Mathematics)
            math_menu->addAction(tmp_action);
        if (Constants.at(i).category  &  Electromagnetic)
            em_menu->addAction(tmp_action);
        if (Constants.at(i).category  &  Nuclear)
            nuclear_menu->addAction(tmp_action);
        if (Constants.at(i).category  &  Thermodynamics)
            thermo_menu->addAction(tmp_action);
        if (Constants.at(i).category  &  Gravitation)
            gravitation_menu->addAction(tmp_action);
    }
}

void KCalcConstMenu::slotPassSignalThrough(QAction  *chosen_const)
{
    bool tmp_bool;
    int chosen_const_idx = (chosen_const->data()).toInt(& tmp_bool);
    emit triggeredConstant(Constants.at(chosen_const_idx));
}

KCalcConstMenu::KCalcConstMenu(const QString &title, QWidget * parent)
        : QMenu(title, parent)
{
    init_all();
}

KCalcConstMenu::KCalcConstMenu(QWidget * parent)
        : QMenu(parent)
{
    init_all();
}


#include "kcalc_const_menu.moc"
