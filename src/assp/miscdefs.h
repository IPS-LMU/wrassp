/***********************************************************************
*                                                                      *
* This file is part of Michel Scheffers's miscellanous functions       *
* library.                                                             *
*                                                                      *
* Copyright (C) 1989 - 2009  Michel Scheffers                          *
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
* File:     miscdefs.h                                                 *
* Contents: Definitions of operating system and machine, useful        *
*           constants and some handy macros.                           *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: miscdefs.h,v 1.3 2009/03/18 10:35:13 mtms Exp $ */

#ifndef _MISCDEFS_H
#define _MISCDEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/* #define OLD_KandR_C */
/* #define IPDS_VERSION */

#if defined(_WIN32) || defined(__WIN32__) ||\
    defined(_WIN64) || defined(__WIN64__) ||\
    defined(__MINGW32__) || defined(__MINGW64__) ||\
    defined(__CGYWIN__) 
#ifndef OS_DOS
#define OS_DOS
#endif
#define DIR_SEP_CHR '\\'
#define DIR_SEP_STR "\\"
#define NATIVE_EOL  "\x0D\x0A"

#else
#if defined(macintosh) || defined(darwin)
#ifndef OS_MAC
#define OS_MAC
#endif
/* #define DIR_SEP_CHR ':' */
/* #define DIR_SEP_STR ":" */
/* #define NATIVE_EOL  "\x0A\x0D" */
#define DIR_SEP_CHR '/' /* Mac OS-X is BSD Unix */
#define DIR_SEP_STR "/"
#define NATIVE_EOL  "\x0A"

#else /* fall through */
#ifndef OS_UNIX
#define OS_UNIX
#endif
#define DIR_SEP_CHR '/'
#define DIR_SEP_STR "/"
#define NATIVE_EOL  "\x0A"
#endif
#endif
 
#define NIX
#define TRUE 1
#define FALSE 0
#define EOS '\0'
#define LOCAL static
#define GLOBAL extern
#define ONEkBYTE 1024

#define SGN(x) ( ((x) < 0) ? -1 : 1 )
#define ROUND(x) ( ((x) < 0) ? (x) - 0.5 : (x) + 0.5 )
#define MIN(x,y) ( ((x) < (y)) ? (x) : (y) )
#define MAX(x,y) ( ((x) > (y)) ? (x) : (y) )
#define ODD(ix) ( (ix) & 0x01 )
#define EVEN(ix) ( !ODD(ix) )
#define STRIP(s,n) (s)[(n)]=EOS

# ifndef PI
#  define PI (3.14159265358979323846)          /* pi */
# endif

# ifndef TWO_PI
#  define TWO_PI (6.28318530717958647692)      /* 2*pi */
# endif

# ifndef HLF_PI
#  define HLF_PI (1.57079632679489661923)      /* pi/2 */
# endif

# ifndef QRT_PI
#  define QRT_PI (0.78539816339744830962)      /* pi/4 */
# endif

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _MISCDEFS_H */
