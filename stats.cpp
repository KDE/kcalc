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
#include <kdebug.h>

KStats::KStats() : error_flag_(false) {
}

KStats::~KStats() {
}

void KStats::clearAll() {
    data_.clear();
}

void KStats::enterData(const KNumber &data) {

    data_.push_back(data);
#ifdef DEBUG_STATS
    kDebug() << "Added " << data.toQString();
    kDebug() << "count" <<  data_.size();
#endif

}


void KStats::clearLast() {

	if(!data_.isEmpty()) {
	    data_.pop_back();
	#ifdef DEBUG_STATS
	    kDebug() << "count " << data_.size();
	#endif
	}
}

KNumber KStats::sum() {

    KNumber result = KNumber::Zero;
	
    for (QVector<KNumber>::const_iterator p = data_.constBegin(); p != data_.constEnd(); ++p) {
        result += *p;
    }

#ifdef DEBUG_STATS
    kDebug() << "Sum " << result.toQString();
#endif

    return result;
}

KNumber KStats::median() {

	KNumber result = KNumber::Zero;
	size_t index;

	unsigned int bound = count();

	if (bound == 0) {
    	error_flag_ = true;
    	return KNumber::Zero;
	}

	if (bound == 1)
    	return data_.at(0);

	// need to copy data_-list, because sorting afterwards
	QVector<KNumber> tmp_data(data_);
	qSort(tmp_data);

	if (bound & 1) {    // odd
    	index = (bound - 1) / 2 + 1;
    	result =  tmp_data.at(index - 1);
	} else { // even
    	index = bound / 2;
    	result = ((tmp_data.at(index - 1))  + (tmp_data.at(index))) / KNumber(2);
	}

	return result;
}


KNumber KStats::std_kernel() {
    KNumber result           = KNumber::Zero;
    const KNumber mean_value = mean();

    for (QVector<KNumber>::const_iterator p = data_.constBegin(); p != data_.constEnd(); ++p) {
        result += (*p - mean_value) * (*p - mean_value);
    }

    return result;
}


KNumber KStats::sum_of_squares() {

    KNumber result = KNumber::Zero;

    for (QVector<KNumber>::const_iterator p = data_.constBegin(); p != data_.constEnd(); ++p) {
        result += ((*p) * (*p));
    }

    return result;
}


KNumber KStats::mean() {

	if (count() == 0) {
    	error_flag_ = true;
    	return KNumber::Zero;
	}

	return (sum() / KNumber(count()));
}


KNumber KStats::std() {

	if (count() == 0) {
    	error_flag_ = true;
    	return KNumber::Zero;
	}

	return (std_kernel() / KNumber(count())).sqrt();
}


KNumber KStats::sample_std() {
	KNumber result = KNumber::Zero;

	if (count() < 2) {
    	error_flag_ = true;
    	return KNumber::Zero;
	}

	result = (std_kernel() / KNumber(count() - 1)).sqrt();

#ifdef DEBUG_STATS
	kDebug() << "sample std: " << result.toQString();
#endif

	return result;
}


int KStats::count() const {
    return data_.size();
}


bool KStats::error() {

    bool value = error_flag_;
    error_flag_ = false;
    return value;
}



