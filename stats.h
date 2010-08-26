/*
    $Id$

    KCalc, a scientific calculator for the X window system using the
    Qt widget libraries, available at no cost at http://www.troll.no

    Copyright (C) 1996 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KSTATS_H_
#define KSTATS_H_

#include <QVector>
#include "knumber.h"

class KStats
{

public:
    KStats();
    ~KStats();

public:
    void clearAll();
    void enterData(const KNumber & data);
    void clearLast();
    KNumber sum();
    KNumber sum_of_squares();
    KNumber mean();
    KNumber median();
    KNumber std_kernel();
    KNumber std();
    KNumber sample_std();
    int count() const;
    bool     error();

private:
    QVector<KNumber> data_;
    bool error_flag_;

};


#endif // KSTATS_H_

