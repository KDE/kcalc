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

#include <config-kcalc.h>

#ifndef KCALC_TYPE_H
#define KCALC_TYPE_H

#include <limits.h>
#include <float.h>
#include <math.h>

#undef HAVE_LONG_LONG
#if defined(LLONG_MAX) && defined(HAVE_LONG_DOUBLE)
#define KCALC_LONG_MIN LLONG_MIN
#define KCALC_LONG_MAX LLONG_MAX
#define KCALC_ULONG_MAX ULLONG_MAX
#define KCALC_LONG long long
#define HAVE_LONG_LONG
#define STRTOUL         strtoull
#else
#if defined(LONG_LONG_MAX) && defined(HAVE_LONG_DOUBLE)
#define KCALC_LONG_MIN LONG_LONG_MIN
#define KCALC_LONG_MAX LONG_LONG_MAX
#define KCALC_ULONG_MAX ULONG_LONG_MAX
#define KCALC_LONG long long
#define HAVE_LONG_LONG
#ifdef _HPUX_SOURCE
#define STRTOUL         strtoul
#else
#define STRTOUL         strtoull
#endif
#else
#define KCALC_LONG_MIN LONG_MIN
#define KCALC_LONG_MAX LONG_MAX
#define KCALC_ULONG_MAX ULONG_MAX
#define KCALC_LONG long
#define STRTOUL         strtoul
#endif
#endif
#endif
