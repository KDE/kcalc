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

#ifdef HAVE_CONFIG_H
	#include "../config.h"
#endif
#include <config-kcalc.h>

#ifndef KCALC_TYPE_H
#define KCALC_TYPE_H

#include <limits.h>
#include <float.h>
#include <math.h>

#if defined(HAVE_LONG_DOUBLE) && defined(HAVE_L_FUNCS)
/* should be detected by autoconf and defined in config.h
   Be carefull when modifying these lines. HAVE_LONG_DOUBLE
   is used all over kcalc's sources to determine whether 
   long double of double is the fundamental data type for kcalc*/



	typedef long double CALCAMNT;
#else
	typedef double CALCAMNT;
#endif



#if defined(HAVE_LONG_DOUBLE) && defined(HAVE_L_FUNCS)
	#define MODF(X,Y)	modfl(X,Y)
	#define SIN(X)		sinl(X)
	#define ASIN(X)		asinl(X)
	#define SINH(X)		sinhl(X)
	#define ASINH(X)	asinhl(X)
	#define COS(X)		cosl(X)
	#define COSH(X)		coshl(X)
	#define ACOS(X)		acosl(X)
	#define ACOSH(X)	acoshl(X)
	#define TAN(X)		tanl(X)
	#define TANH(X)		tanhl(X)
	#define ATAN(X)		atanl(X)
	#define ATANH(X)	atanhl(X)
	#define EXP(X)		expl(X)
	#define POW(X,Y)	powl(X,Y)
	#define LN(X)		logl(X)
	#define LOG_TEN(X)	log10l(X)
	#define SQRT(X)		sqrtl(X)
	#define CBRT(X)		cbrtl(X)
	#define ISINF(X)	isinfl(X)
	#define STRTOD(X,Y)	strtold(X,Y)
	#define ROUND(X)	roundl(X)
	#define CALCAMNT_EPSILON	LDBL_EPSILON
#else
	#define MODF(X,Y)	modf(X,Y)
	#define SIN(X)		sin(X)
	#define ASIN(X)		asin(X)
	#define SINH(X)		sinh(X)
	#define ASINH(X)	asinh(X)
	#define COS(X)		cos(X)
	#define COSH(X)		cosh(X)
	#define ACOS(X)		acos(X)
	#define ACOSH(X)	acosh(X)
	#define TAN(X)		tan(X)
	#define TANH(X)		tanh(X)
	#define ATAN(X)		atan(X)
	#define ATANH(X)	atanh(X)
	#define EXP(X)		exp(X)
	#define POW(X,Y)	pow(X,Y)
	#define LN(X)		log(X)
	#define LOG_TEN(X)	log10(X)
	#define SQRT(X)		sqrt(X)
	#define CBRT(X)		cbrt(X)
	#define ISINF(X)	isinf(X)
	#define STRTOD(X,Y)	strtod(X,Y)
	#define ROUND(X)	round(X)
	#define CALCAMNT_EPSILON	DBL_EPSILON
#endif

#if !defined(HAVE_FUNC_ROUND) && !defined(HAVE_FUNC_ROUNDL)
	#undef	ROUND
	#define ROUND(X)	rint(X)
#endif

#undef HAVE_LONG_LONG
#if defined(LLONG_MAX) && defined(HAVE_LONG_DOUBLE)
#define KCALC_LONG_MIN	LLONG_MIN
#define KCALC_LONG_MAX	LLONG_MAX
#define KCALC_ULONG_MAX	ULLONG_MAX
#define KCALC_LONG	long long
#define HAVE_LONG_LONG
#define STRTOUL         strtoull
#else
#if defined(LONG_LONG_MAX) && defined(HAVE_LONG_DOUBLE)
#define KCALC_LONG_MIN	LONG_LONG_MIN
#define KCALC_LONG_MAX	LONG_LONG_MAX
#define KCALC_ULONG_MAX	ULONG_LONG_MAX
#define KCALC_LONG	long long
#define HAVE_LONG_LONG
#define STRTOUL         strtoull
#else
#define KCALC_LONG_MIN	LONG_MIN
#define KCALC_LONG_MAX	LONG_MAX
#define KCALC_ULONG_MAX	ULONG_MAX
#define KCALC_LONG	long
#define STRTOUL         strtoul
#endif
#endif
#endif 
