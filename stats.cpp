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

#include "stats.h"
#include <kdebug.h>

//------------------------------------------------------------------------------
// Name: KStats
// Desc: constructor
//------------------------------------------------------------------------------
KStats::KStats() : error_flag_(false) {
}

//------------------------------------------------------------------------------
// Name: ~KStats
// Desc: destructor
//------------------------------------------------------------------------------
KStats::~KStats() {
}

//------------------------------------------------------------------------------
// Name: clearAll
// Desc: empties the data set
//------------------------------------------------------------------------------
void KStats::clearAll() {
    data_.clear();
}

//------------------------------------------------------------------------------
// Name: enterData
// Desc: adds an item to the data set
//------------------------------------------------------------------------------
void KStats::enterData(const KNumber &data) {

    data_.push_back(data);
#ifdef DEBUG_STATS
    kDebug() << "Added " << data.toQString();
    kDebug() << "count" <<  data_.size();
#endif

}

//------------------------------------------------------------------------------
// Name: clearLast
// Desc: remoaves the last item from the data set
//------------------------------------------------------------------------------
void KStats::clearLast() {

	if(!data_.isEmpty()) {
	    data_.pop_back();
	#ifdef DEBUG_STATS
	    kDebug() << "count " << data_.size();
	#endif
	}
}

//------------------------------------------------------------------------------
// Name: sum
// Desc: calculates the SUM of all values in the data set
//------------------------------------------------------------------------------
KNumber KStats::sum() const {

    KNumber result = KNumber::Zero;
	
    for (QVector<KNumber>::const_iterator p = data_.constBegin(); p != data_.constEnd(); ++p) {
        result += *p;
    }

#ifdef DEBUG_STATS
    kDebug() << "Sum " << result.toQString();
#endif

    return result;
}

//------------------------------------------------------------------------------
// Name: median
// Desc: calculates the MEDIAN of all values in the data set
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// Name: std_kernel() 
// Desc: calculates the STD Kernel of all values in the data set
//------------------------------------------------------------------------------
KNumber KStats::std_kernel() {
    KNumber result           = KNumber::Zero;
    const KNumber mean_value = mean();

    for (QVector<KNumber>::const_iterator p = data_.constBegin(); p != data_.constEnd(); ++p) {
        result += (*p - mean_value) * (*p - mean_value);
    }

    return result;
}

//------------------------------------------------------------------------------
// Name: std_kernel() const 
// Desc: calculates the SUM of all values in the data set (each squared)
//------------------------------------------------------------------------------
KNumber KStats::sum_of_squares() const {

    KNumber result = KNumber::Zero;

    for (QVector<KNumber>::const_iterator p = data_.constBegin(); p != data_.constEnd(); ++p) {
        result += ((*p) * (*p));
    }

    return result;
}

//------------------------------------------------------------------------------
// Name: median
// Desc: calculates the MEAN of all values in the data set
//------------------------------------------------------------------------------
KNumber KStats::mean() {

	if (count() == 0) {
    	error_flag_ = true;
    	return KNumber::Zero;
	}

	return (sum() / KNumber(count()));
}

//------------------------------------------------------------------------------
// Name: median
// Desc: calculates the STANDARD DEVIATION of all values in the data set
//------------------------------------------------------------------------------
KNumber KStats::std() {

	if (count() == 0) {
    	error_flag_ = true;
    	return KNumber::Zero;
	}

	return (std_kernel() / KNumber(count())).sqrt();
}

//------------------------------------------------------------------------------
// Name: median
// Desc: calculates the SAMPLE STANDARD DEVIATION of all values in the data set
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// Name: count
// Desc: returns the amount of values in the data set
//------------------------------------------------------------------------------
int KStats::count() const {

    return data_.size();
}

//------------------------------------------------------------------------------
// Name: error
// Desc: returns the error state AND clears it
//------------------------------------------------------------------------------
bool KStats::error() {

    bool value = error_flag_;
    error_flag_ = false;
    return value;
}



