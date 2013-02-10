/*
Copyright (C) 2001 - 2013 Evan Teran
                          evan.teran@gmail.com

Copyright (C) 1996 - 2000 Bernd Johannes Wuebben
                          wuebben@kde.org

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KSTATS_H_
#define KSTATS_H_

#include <QVector>
#include "knumber.h"

class KStats {
public:
    KStats();
    ~KStats();

public:
    void clearAll();
    void enterData(const KNumber &data);
    void clearLast();
    KNumber sum() const;
    KNumber sum_of_squares() const;
    KNumber mean();
    KNumber median();
    KNumber std_kernel();
    KNumber std();
    KNumber sample_std();
    int count() const;
    bool error();

private:
    QVector<KNumber> data_;
    bool             error_flag_;
};

#endif

