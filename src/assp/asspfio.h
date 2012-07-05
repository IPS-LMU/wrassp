/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1989 - 2008  Michel Scheffers                          *
*                            IPdS, CAU Kiel                            *
*                            Leibnizstr. 10                            *
*                            24118 Kiel, Germany                       *
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
* File:     asspfio.h                                                  *
* Contents: Header file for programs and functions using the ASSP      *
*           file and data handler.                                     *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: asspfio.h,v 1.4 2008/11/04 15:47:58 mtms Exp $ */

#ifndef _ASSPFIO_H
#define _ASSPFIO_H

#include <dlldef.h>    /* ASSP_EXTERN */
#include <dataobj.h>   /* DOBJ */
#include <headers.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * constants for 'mode' in asspFOpen()
 */
#define AFO_NONE   0x0000
#define AFO_READ   0x0001
#define AFO_WRITE  0x0002
#define AFO_UPDATE (AFO_READ + AFO_WRITE)
/* #define AFO_CREATE (0x0004 + AFO_WRITE) */
#define AFO_TEXT   0x0100  /* we normally set the 'b' flag in fopen() */

/*
 * constants for 'action' in asspFClose()
 */
#define AFC_NONE   0x0000
#define AFC_KEEP   0x0000
#define AFC_CLEAR  0x0001
#define AFC_FREE   0x0002

/*
 * constants for 'opts' in asspFFlush() and 'extra' in asspFPrint()
 */
#define AFW_NONE     0x0000
#define AFW_CLEAR    0x0000
#define AFW_KEEP     0x0001
#define AFW_ADD_TIME 0x0010

/*
 * prototypes of functions in asspfio.c
 */
ASSP_EXTERN DOBJ *asspFOpen(char *filePath, int mode, DOBJ *doPtr);
ASSP_EXTERN int   asspFClose(DOBJ *dop, int action);
ASSP_EXTERN long  asspFSeek(DOBJ *dop, long recordNr);
ASSP_EXTERN long  asspFTell(DOBJ *dop);
ASSP_EXTERN long  asspFRead(void *buffer, long numRecords, DOBJ *dop);
ASSP_EXTERN long  asspFWrite(void *buffer, long numRecords, DOBJ *dop);
ASSP_EXTERN long  asspFPrint(void *buffer, long startRecord, long numRecords,\
			     DOBJ *dop, int extra);
ASSP_EXTERN long  asspFFill(DOBJ *dop);
ASSP_EXTERN long  asspFFlush(DOBJ *dop, int opts);
ASSP_EXTERN long  recordIndex(DOBJ *dop, long nr, long head, long tail);
ASSP_EXTERN long  frameIndex(DOBJ *smpDOp, long nr, long size, long shift,\
			     long head, long tail);
ASSP_EXTERN long  putRecord(DOBJ *dst, DOBJ *src, long recordNr);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _ASSPFIO_H */
