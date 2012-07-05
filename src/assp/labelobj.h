/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1989 - 2010  Michel Scheffers                          *
*                            IPdS, CAU Kiel                            *
*                            Leibnizstr. 10                            *
*                            24118 Kiel                                *
*                            Germany                                   *
*                            ms@ipds.uni-kiel.de                       *
*                                                                      *
* This library is free software: you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation, either version 3 of the License, or    *
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
* File:     labelobj.h                                                 *
* Contents: Structures and constants defining an ASSP label object.    *
*           Prototypes of handling functions.                          *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: labelobj.h,v 1.6 2010/05/28 07:29:30 mtms Exp $ */

#ifndef _LABELOBJ_H
#define _LABELOBJ_H

#include <dlldef.h>   /* ASSP_EXTERN */
#include <dataobj.h>  /* GD_MAX_ID_LEN DOBJ */
#include <misc.h>     /* LINK */
#include <ipds_lbl.h> /* IPdS MIX and SAMPA formats */
#include <esps_lbl.h> /* ESPS xlabel format */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Generic data structure for holding a copy of the fixed header part 
 * of a label file in IPdS MIX or SAMPA format.
 */
#define LBL_GD_IDENT "IPdS_label_header"

typedef struct {
  char  ident[GD_MAX_ID_LEN+1]; /* identification string */
  void *headCopy; /* ALLOCATED */
  long  copySize; /* size of copy in bytes */
} LBLHDR;

/*
 * Generic data structure for holding the header items 'signal', 
 * 'font' and 'color' of a label file in ESPS-xlabel format.
 */
#define XLBL_GD_IDENT "xlabel_header_data"

typedef struct {
  char  ident[GD_MAX_ID_LEN+1]; /* identification string */
  char *signal; /* ALLOCATED */
  char *font;   /* ALLOCATED */
  int   color;
} XLBL_GD;

#define XLBL_DEF_COLOR (121) /* as in example in xwaves manual */

/*
 * Structure for holding a label in IPdS MIX or SAMPA or ESPS-xlabel 
 * format as an element of a double-linked list.
 * Either 'smpNr' or 'time' may be undefined ( < 0 ) but not both.
 * 'smpNr' should follow ASSP conventions (count starts at 0).
 */
typedef struct {
  LINK  *prev; /* ALLOCATED */
  LINK  *next; /* ALLOCATED */
  char  *name; /* ALLOCATED */
  long   smpNr;
  double time;
} LABEL;

#define LBL_TIME_UNDEF (-1)

#define LBL_ADD_AS_FIRST 0x0001
#define LBL_ADD_AS_LAST  0x0002
#define LBL_ADD_AT_TIME  0x0010
#define LBL_ADD_BEFORE   0x0100
#define LBL_ADD_BEHIND   0x0200

/*
 * Prototypes of functions in labelobj.c
 */
ASSP_EXTERN void freeLabel(void *ptr);
ASSP_EXTERN void freeLabelList(void *ptr);
ASSP_EXTERN int  getLabelHead(DOBJ *dop);
ASSP_EXTERN void freeHeadCopy(void *generic);
ASSP_EXTERN void freeXLBL_GD(void *generic);

ASSP_EXTERN long loadLabels(DOBJ *dop);
ASSP_EXTERN long saveLabels(DOBJ *dop);

ASSP_EXTERN LABEL *makeLabel(char *name, long smpNr, double time);
ASSP_EXTERN LABEL *firstLabel(DOBJ *dop);
ASSP_EXTERN LABEL *lastLabel(DOBJ *dop);
ASSP_EXTERN LABEL *prevLabel(LABEL *lPtr);
ASSP_EXTERN LABEL *nextLabel(LABEL *lPtr);
ASSP_EXTERN LABEL *addLabel(DOBJ *dop, LABEL *new, int pos, LABEL *ref);
ASSP_EXTERN LABEL *delLabel(DOBJ *dop, LABEL *lPtr);

ASSP_EXTERN double estRefRate(long smpNr, double time, int round);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _LABELOBJ_H */
