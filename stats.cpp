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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/


#include "stats.h"
#ifdef DEBUG_STATS
	#include <stdio.h>
#endif

//-------------------------------------------------------------------------
// Name: KStats()
//-------------------------------------------------------------------------
KStats::KStats() {
	error_flag = false;
}

//-------------------------------------------------------------------------
// Name: ~KStats()
//-------------------------------------------------------------------------
KStats::~KStats() {
}

//-------------------------------------------------------------------------
// Name: clearAll()
//-------------------------------------------------------------------------
void KStats::clearAll() {
	mData.clear();
}
 
//-------------------------------------------------------------------------
// Name: enterData(CALCAMNT _data
//-------------------------------------------------------------------------
void KStats::enterData(CALCAMNT _data) {

	mData.push_back(_data);
#ifdef DEBUG_STATS
	printf("Added %Lg\n", _data);
	printf("count %d\n", mData.size());
#endif

}

//-------------------------------------------------------------------------
// Name: clearLast()
//-------------------------------------------------------------------------
void KStats::clearLast() {

	mData.pop_back();
#ifdef DEBUG_STATS
	printf("count %d\n",mData.size());
#endif   


}

//-------------------------------------------------------------------------
// Name: sum()
//-------------------------------------------------------------------------
CALCAMNT KStats::sum() {

	CALCAMNT result = 0.0;
	QValueVector<CALCAMNT>::iterator p;

	for(p = mData.begin(); p != mData.end(); p++) {
		result += *p;
	}
	
#ifdef DEBUG_STATS
	printf("Sum %Lg\n", result);
#endif

	return result;
}

//-------------------------------------------------------------------------
// Name: median()
//-------------------------------------------------------------------------
CALCAMNT KStats::median() {

	CALCAMNT result = 0.0;
	size_t bound;
	size_t index;
	qHeapSort(mData);

#ifdef DEBUG_STATS
	QValueVector<CALCAMNT>::iterator p;
	for(p = mData.begin(); p != mData.end(); p++) {
		printf("Sorted %Lg\n", *p)
	}
#endif

	bound = count();

	if (bound == 0){
		error_flag = true;
		return 0.0;
	}

	if (bound == 1)
		return mData.at(0);

	if( bound & 1) {  // odd
		index = (bound - 1 ) / 2 + 1;
		result =  mData.at(index - 1);
	} else { // even
		index = bound / 2;
		result = ((mData.at(index - 1))  + (mData.at(index))) / 2;
	}

	return result;
}

//-------------------------------------------------------------------------
// Name: std_kernel()
//-------------------------------------------------------------------------
CALCAMNT KStats::std_kernel() {

	CALCAMNT result = 0.0;
	CALCAMNT _mean;
	QValueVector<CALCAMNT>::iterator p;

	_mean = mean();

	for(p = mData.begin(); p != mData.end(); p++) {
		result += (*p - _mean) * (*p - _mean);
	}


#ifdef DEBUG_STATS
	printf("std_kernel %Lg\n",result);
#endif

	return result;
}

//-------------------------------------------------------------------------
// Name: sum_of_squares()
//-------------------------------------------------------------------------
CALCAMNT KStats::sum_of_squares() {

	CALCAMNT result = 0.0;
	QValueVector<CALCAMNT>::iterator p;
  
	for(p = mData.begin(); p != mData.end(); p++) {
		result += ((*p) * (*p));
	}

#ifdef DEBUG_STATS
	printf("Sum of Squares %Lg\n",result);
#endif

	return result;
}

//-------------------------------------------------------------------------
// Name: mean()
//-------------------------------------------------------------------------
CALCAMNT KStats::mean() {

	CALCAMNT result = 0.0;

	if(count() == 0){
		error_flag = true;
		return 0.0;
	}

	result = sum() / count();

#ifdef DEBUG_STATS
	printf("mean: %Lg\n", result);
#endif

	return result;
}

//-------------------------------------------------------------------------
// Name: std()
//-------------------------------------------------------------------------
CALCAMNT KStats::std() {

	CALCAMNT result = 0.0;

	if(count() == 0){
		error_flag = true;
		return 0.0;
	}

	result = SQRT(std_kernel() / count());

#ifdef DEBUG_STATS
	printf("data.count %d\n", count());
	printf("std: %Lg\n", result);
#endif

  return result;

}

//-------------------------------------------------------------------------
// Name: sample_std()
//-------------------------------------------------------------------------
CALCAMNT KStats::sample_std() {
	CALCAMNT result = 0.0;

	if(count() < 2 ){
		error_flag = true;
		return 0.0;
	}

	result = SQRT(std_kernel() / (count() - 1));
	
	//  result = result/(count() - 1);
#ifdef DEBUG_STATS
	printf("sample std: %Lg\n",result);
#endif

	return result;
}

//-------------------------------------------------------------------------
// Name: count()
//-------------------------------------------------------------------------
int KStats::count() {
	return mData.size();
}

//-------------------------------------------------------------------------
// Name: error()
//-------------------------------------------------------------------------
bool KStats::error() {

  bool value = error_flag;
  error_flag = false;
  return value;
}



