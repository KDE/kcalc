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


#include "stats.h"
#ifdef DEBUG_STATS
#include <stdio.h>
#endif

KStats::KStats()
{
    error_flag_ = false;
}

KStats::~KStats()
{
}

void KStats::clearAll()
{
    data_.clear();
}

void KStats::enterData(KNumber const & data)
{

    data_.push_back(data);
#ifdef DEBUG_STATS
    printf("Added %Lg\n", data);
    printf("count %d\n", data.size());
#endif

}


void KStats::clearLast()
{

    data_.pop_back();
#ifdef DEBUG_STATS
    printf("count %d\n", data_.size());
#endif


}

KNumber KStats::sum()
{

    KNumber result = 0;
    QVector<KNumber>::iterator p;

    for (p = data_.begin(); p != data_.end(); ++p) {
        result += *p;
    }

#ifdef DEBUG_STATS
    printf("Sum %Lg\n", result);
#endif

    return result;
}

KNumber KStats::median()
{

    KNumber result = 0;
    unsigned int bound;
    size_t index;

    bound = count();

    if (bound == 0) {
        error_flag_ = true;
        return 0;
    }

    if (bound == 1)
        return data_.at(0);

    // need to copy data_-list, because sorting afterwards
    QVector<KNumber> tmp_mData(data_);
    qSort(tmp_mData);

    if (bound & 1) {    // odd
        index = (bound - 1) / 2 + 1;
        result =  tmp_mData.at(index - 1);
    } else { // even
        index = bound / 2;
        result = ((tmp_mData.at(index - 1))  + (tmp_mData.at(index))) / KNumber(2);
    }

    return result;
}


KNumber KStats::std_kernel()
{
    KNumber result = KNumber::Zero;
    KNumber mean_value;
    QVector<KNumber>::iterator p;

    mean_value = mean();

    for (p = data_.begin(); p != data_.end(); ++p) {
        result += (*p - mean_value) * (*p - mean_value);
    }

    return result;
}


KNumber KStats::sum_of_squares()
{

    KNumber result = 0;
    QVector<KNumber>::iterator p;

    for (p = data_.begin(); p != data_.end(); ++p) {
        result += ((*p) * (*p));
    }

    return result;
}


KNumber KStats::mean()
{
    if (count() == 0) {
        error_flag_ = true;
        return 0;
    }

    return (sum() / KNumber(count()));
}


KNumber KStats::std()
{
    if (count() == 0) {
        error_flag_ = true;
        return KNumber::Zero;
    }

    return (std_kernel() / KNumber(count())).sqrt();
}


KNumber KStats::sample_std()
{
    KNumber result = 0;

    if (count() < 2) {
        error_flag_ = true;
        return KNumber::Zero;
    }

    result = (std_kernel() / KNumber(count() - 1)).sqrt();

    //  result = result/(count() - 1);
#ifdef DEBUG_STATS
    printf("sample std: %Lg\n", result);
#endif

    return result;
}


int KStats::count() const
{
    return static_cast<int>(data_.size());
}


bool KStats::error()
{

    bool value = error_flag_;
    error_flag_ = false;
    return value;
}



