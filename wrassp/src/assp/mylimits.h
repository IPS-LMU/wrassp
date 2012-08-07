/***********************************************************************
*                                                                      *
* This file is part of Michel Scheffers's miscellanous functions       *
* library.                                                             *
*                                                                      *
* Copyright (C) 2005 - 2007  Michel Scheffers                          *
*                            IPdS, CAU Kiel                            *
*                            Leibnizstr. 10                            *
*                            24118 Kiel                                *
*                            Germany                                   *
*                            ms@ipds.uni-kiel.de                       *
*                                                                      *
* This library is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 3 of the License, or    *
* (at your option) any later version.                                  *
*                                                                      *
* This library is distributed in the hope that it will be useful,      *
* but WITHOUT ANY WARRANTY; without even the implied warranty of       *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
* GNU General Public License for more details.                         *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with this library. If not, see <http://www.gnu.org/licenses/>. *
*                                                                      *
*----------------------------------------------------------------------*
*                                                                      *
* File:     mylimits.h                                                 *
* Contents: Header file to unify lengths of full path, file name and   *
*           suffix/extension. Includes define's of mininum and maximum *
*           values of integer types if not yet available.              *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: mylimits.h,v 1.2 2007/09/20 12:24:25 mtms Exp $ */

#ifndef _MYLIMITS_H
#define _MYLIMITS_H

#include <limits.h>      /* system limits */
#include <inttypes.h>    /* possibly INT8_MAX etc. */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PATH_MAX
#define PATH_MAX (2047)
#endif
#ifndef NAME_MAX
#define NAME_MAX (255)
#endif
#ifndef SUFF_MAX
#define SUFF_MAX (31)
#endif

#ifndef UINT8_MAX
#define UINT8_MAX (255)
#endif
#ifndef INT8_MAX
#define INT8_MAX (127)
#endif
#ifndef INT8_MIN
#define INT8_MIN (-128)
#endif
#ifndef UINT16_MAX
#define UINT16_MAX (65535)
#endif
#ifndef INT16_MAX
#define INT16_MAX (32767)
#endif
#ifndef INT16_MIN
#define INT16_MIN (-32768)
#endif
#ifndef UINT24_MAX
#define UINT24_MAX (16777215UL)
#endif
#ifndef INT24_MAX
#define INT24_MAX (8388607L)
#endif
#ifndef INT24_MIN
#define INT24_MIN (-8388608L)
#endif
#ifndef UINT32_MAX
#define UINT32_MAX (4294967295UL)
#endif
#ifndef INT32_MAX
#define INT32_MAX (2147483647L)
#endif
#ifndef INT32_MIN
#define INT32_MIN (-2147483648L)
#endif
#ifndef UINT64_MAX
# ifdef __USE_ISOC99
#define UINT64_MAX (18446744073709551615ULL)
# else
#define UINT64_MAX (18446744073709551615UL)
# endif
#endif
#ifndef INT64_MAX
# ifdef __USE_ISOC99
#define INT64_MAX (9223372036854775807LL)
# else
#define INT64_MAX (9223372036854775807L)
# endif
#endif
#ifndef INT64_MIN
# ifdef __USE_ISOC99
#define INT64_MIN (-9223372036854775808LL)
# else
#define INT64_MIN (-9223372036854775808L)
# endif
#endif

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _MYLIMITS_H */
