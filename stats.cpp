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

KStats::KStats() {
	error_flag = false;
}

KStats::~KStats() {
}

void KStats::clearAll() {
	mData.clear();
}
 
void KStats::enterData(KNumber const & _data) {

	mData.push_back(_data);
#ifdef DEBUG_STATS
	printf("Added %Lg\n", _data);
	printf("count %d\n", mData.size());
#endif

}


void KStats::clearLast(void) {

	mData.pop_back();
#ifdef DEBUG_STATS
	printf("count %d\n",mData.size());
#endif   


}

KNumber KStats::sum(void) {

	KNumber result = 0;
	QVector<KNumber>::iterator p;

	for(p = mData.begin(); p != mData.end(); ++p) {
		result += *p;
	}
	
#ifdef DEBUG_STATS
	printf("Sum %Lg\n", result);
#endif

	return result;
}

KNumber KStats::median(void) {

	KNumber result = 0;
	unsigned int bound;
	size_t index;
	qSort(mData);

#ifdef DEBUG_STATS
	QVector<KNumber>::iterator p;
	for(p = mData.begin(); p != mData.end(); ++p) {
		printf("Sorted %Lg\n", *p)
	}
#endif

	bound = count();

	if (bound == 0){
		error_flag = true;
		return 0;
	}

	if (bound == 1)
		return mData.at(0);

	if( bound & 1) {  // odd
		index = (bound - 1 ) / 2 + 1;
		result =  mData.at(index - 1);
	} else { // even
	  index = bound / 2;
	  result = ((mData.at(index - 1))  + (mData.at(index))) / KNumber(2);
	}

	return result;
}


KNumber KStats::std_kernel(void)
{
	KNumber result = KNumber::Zero;
	KNumber _mean;
	QVector<KNumber>::iterator p;

	_mean = mean();

	for(p = mData.begin(); p != mData.end(); ++p) {
		result += (*p - _mean) * (*p - _mean);
	}

	return result;
}


KNumber KStats::sum_of_squares() {

	KNumber result = 0;
	QVector<KNumber>::iterator p;
  
	for(p = mData.begin(); p != mData.end(); ++p) {
		result += ((*p) * (*p));
	}

	return result;
}


KNumber KStats::mean(void)
{
	if(count() == 0){
		error_flag = true;
		return 0;
	}

	return (sum() / KNumber(count()));
}


KNumber KStats::std(void)
{
	if(count() == 0){
		error_flag = true;
		return KNumber::Zero;
	}

	return (std_kernel() / KNumber(count())).sqrt();
}


KNumber KStats::sample_std(void) {
	KNumber result = 0;

	if(count() < 2 ){
		error_flag = true;
		return KNumber::Zero;
	}

	result = (std_kernel() / KNumber(count() - 1)).sqrt();
	
	//  result = result/(count() - 1);
#ifdef DEBUG_STATS
	printf("sample std: %Lg\n",result);
#endif

	return result;
}


int KStats::count(void) const
{
  return static_cast<int>(mData.size());
}


bool KStats::error() {

  bool value = error_flag;
  error_flag = false;
  return value;
}



