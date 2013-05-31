/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1997 - 2007  Michel Scheffers                          *
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
* File:     asspendian.h                                               *
* Contents: Structure and macros for determining/handling byte order.  *
*           Macros are extended for use with labels and articulatory   *
*           data.                                                      *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: asspendian.h,v 1.2 2007/08/17 13:08:05 mtms Exp $ */

#ifndef _ASSPENDIAN_H /* somebody else is apparently using _ENDIAN_H */
#define _ASSPENDIAN_H

#include <stddef.h>   /* size_t */
#include <inttypes.h> /* uint16_t uint8_t */

#include <dlldef.h>   /* ASSP_EXTERN */

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
  uint16_t word;      /* short first for initialization */
  uint8_t  byte[2];
} ENDIAN;

#define MSB 0x0100
#define SETENDIAN(e)    ((e).word=MSB)
#define CLRENDIAN(e)    ((e).word=0x0000)
#define CPYENDIAN(d,s)  ((d).word=(s).word)
#define SETMSBFIRST(e)  ((e).byte[0]=((e).byte[1]=0x00)+1)
#define SETMSBLAST(e)   ((e).byte[1]=((e).byte[0]=0x00)+1)
#define SETLSBFIRST(e)  SETMSBLAST(e)
#define SETLSBLAST(e)   SETMSBFIRST(e)
#define MSBFIRST(e)     ((e).byte[0]>(e).byte[1])
#define MSBLAST(e)      ((e).byte[0]<(e).byte[1])
#define MSBUNDEF(e)     ((e).byte[0]==(e).byte[1])
#define LSBFIRST(e)     MSBLAST(e)
#define LSBLAST(e)      MSBFIRST(e)
#define LSBUNDEF(e)     MSBUNDEF(e)
#define DIFFENDIAN(a,b) ((MSBFIRST(a)&&MSBLAST(b)) || (MSBLAST(a)&&MSBFIRST(b)))

/***********************************************************************
* can use the same concept for other 2-dimensional stuff               *
***********************************************************************/

/*
 * labels:
 * endian in this case whether begin and/or end are marked
 */
#define SETBEGIN(l)    SETMSBFIRST(l)
#define SETEND(l)      SETMSBLAST(l)
#define SETRANGE(l)    ((l).word=0x0101)
#define SETEVENT(l)    ((l).word=0xFFFF)
#define MARKSBEGIN(l)  MSBFIRST(l)
#define MARKSEND(l)    MSBLAST(l)
#define MARKSRANGE(l)  ((l).word==0x0101)
#define MARKSEVENT(l)  ((l).word==0xFFFF)
#define MARKSUNDEF(l)  ((l).word==0x0000)
#define DIFFMARKS(a,b) ((a).word!=(b).word)

/*
 * palatogram:
 * code for top view: MSB-last, bottom view: MSB-first
 */
#define CLRVIEW(v)    CLRENDIAN(v)
#define CPYVIEW(d,s)  CPYENDIAN(d,s)
#define SETTOPVIEW(v) SETMSBLAST(v)
#define SETBOTVIEW(v) SETMSBFIRST(v)
#define VIEWUNDEF(v)  MSBUNDEF(v)
#define TOPVIEW(v)    MSBLAST(v)
#define BOTVIEW(v)    MSBFIRST(v)
#define DIFFVIEW(a,b) DIFFENDIAN(a,b)

/*
 * articulogram:
 * code for facing left: MSB-last, facing right: MSB-first
 */
#define CLRFACE(f)      CLRENDIAN(f)
#define CPYFACE(d,s)    CPYENDIAN(d,s)
#define SETFACELEFT(f)  SETMSBLAST(f)
#define SETFACERIGHT(f) SETMSBFIRST(f)
#define FACEUNDEF(f)    MSBUNDEF(f)
#define FACELEFT(f)     MSBLAST(f)
#define FACERIGHT(f)    MSBFIRST(f)
#define DIFFFACE(a,b)   DIFFENDIAN(a,b)

/*
 * prototypes
 */
ASSP_EXTERN void memswab(void *dst, void *src, size_t varSize, size_t numVars);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _ASSPENDIAN_H */
