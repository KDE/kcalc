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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/


#include "stats.h"
#include <stdio.h>
#include <stdlib.h>

KStats::KStats(){

  error_flag = false;
  data.setAutoDelete(TRUE);

}

KStats::~KStats(){

  data.clear();

}

void KStats::clearAll(){

  data.clear();

}
 
void KStats::enterData(CALCAMNT _data){

  CALCAMNT *newdata;
  newdata = new CALCAMNT;
  *newdata = _data;
  data.append(newdata);

#ifdef DEBUG_STATS
  printf("Added %Lg\n",*newdata);
  printf("count %d\n",data.count());
#endif

}


void KStats::clearLast(){

printf("count %d\n",data.count());
 data.removeLast();
printf("count %d\n",data.count());

}
 
CALCAMNT KStats::sum(){

  CALCAMNT result = 0.0;
  CALCAMNT *dp;
  for ( dp=data.first(); dp != 0; dp=data.next() ){
    
    result += *dp;

  }

#ifdef DEBUG_STATS
  printf("Sum %Lg\n",result);
#endif

  return result;
}

CALCAMNT KStats::median(){

  int index;
  CALCAMNT result;


  CALCAMNT *array = (CALCAMNT*) malloc(data.count()*sizeof(CALCAMNT)+2);

  for (int l = 0; l < (int)data.count(); l++)
    array[l] = 0.0;

  CALCAMNT *dp;
  int j = 0;
  int bound = 0;

  for ( dp=data.first(); dp != 0; dp=data.next() ){

#ifdef DEBUG_STATS
    printf("%d %Lg\n",j,*dp);
#endif

    bool already_there= false;

    for(int i = 0; i < j;i++){
      if(*dp == array[i]){

#ifdef DEBUG_STATS
	printf("%Lg is already there\n",*dp);
#endif

	already_there = true;
      }
    }
    
    if(!already_there) {
      array[bound] = *dp;
      bound ++;
      already_there = false;
    }

    j ++;
  }

  // at this point array contains a list of all unique data items.
  // eg 1 2 2 5 6 7 7  will be turned in to 1 2 5 6 7
  // and we have 'bound' items ( 5 in the above example)

#ifdef DEBUG_STATS
  for(int k = 0;k < bound ; k++)
    printf("array: %Lg\n",array[k]);
#endif

  if (bound == 0){
    error_flag = true;
    return 0.0;
  }
  
  if ( bound == 1)
    return array[bound -1];


  if( bound % 2){  // odd
     
    index = (bound - 1 )/2 + 1;
    result =  getat(index,array,bound);
  }
  else { // even
    
    index = bound /2;
    result = (getat(index,array,bound) +  getat(index +1,array,bound))/2;
 }

  return result;

}


CALCAMNT KStats::getat(int position,CALCAMNT array[],int bound){ 

  // return the 'position' largest element in array

#ifdef DEBUG_STATS
  printf("k = %d\n",position);
#endif


  MyList list;

  for(int i=0; i<bound;i++){
    list.inSort(&array[i]);
  }  

#ifdef DEBUG_STATS
  for(int l = 0; l < (int)list.count();l++){

    printf("Sorted %Lg\n",*list.at(l));

  }
#endif

  return *list.at(position-1);

}


CALCAMNT KStats::sum_of_squares(){

  CALCAMNT result = 0.0;
  CALCAMNT *dp;
  for ( dp=data.first(); dp != 0; dp=data.next() ){
    
    result += (*dp) * (*dp);

  }

#ifdef DEBUG_STATS
  printf("Sum of Squares %Lg\n",result);
#endif

  return result;

}

CALCAMNT KStats::mean(){

  CALCAMNT result = 0.0;

  if(data.count() == 0){
    error_flag = true;
    return 0.0;
  }
  
  result = sum()/data.count();

#ifdef DEBUG_STATS
  printf("mean: %Lg\n",result);
#endif

  return result;

}

CALCAMNT KStats::std(){

  CALCAMNT result = 0.0;

  if(data.count() == 0){
    error_flag = true;

#ifdef DEBUG_STATS
    printf("set stats error\n");
#endif

    return 0.0;
  }
  
  result = SQRT(sum_of_squares());

#ifdef DEBUG_STATS
  printf ("data.count %d\n",data.count());
#endif

  result = result/data.count();

#ifdef DEBUG_STATS
  printf("std: %Lg\n",result);
#endif

  return result;

}

CALCAMNT KStats::sample_std(){

  CALCAMNT result = 0.0;

  if(data.count() < 2 ){
    error_flag = true;
    return 0.0;
  }
  
  result = SQRT(sum_of_squares());
  result = result/(data.count() - 1);
#ifdef DEBUG_STATS
  printf("sample std: %Lg\n",result);
#endif
  return result;

}

int  KStats::count(){

  return data.count();

}

bool KStats::error(){

  bool value;
  value = error_flag;
  error_flag = false;

  return value;

}

int MyList::compareItems(GCI item_1, GCI item_2){
  
  CALCAMNT *item1;
  CALCAMNT *item2;
  
  item1 = (CALCAMNT*) item_1;
  item2 = (CALCAMNT*) item_2;

  if(*item1 > *item2)
    return 1;

  if(*item2 > *item1)
    return -1;

  return 0;

}



