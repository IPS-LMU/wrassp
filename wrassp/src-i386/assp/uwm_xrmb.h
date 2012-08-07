/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1989 - 2007  Michel Scheffers                          *
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
* File:     uwm_xrmb.h                                                 *
* Contents: Constants and structures for handling X-ray Microbeam      *
*           pellet data from the University of Wisconsin at Madison.   *
* Author:   Michel T.M. Scheffers                                      *
* See:      Westerby, J.R. (1994), "X-Ray Microbeam Speech Production  *
*           Database User's Hanbook Version 1.0". Madison: University  *
*           of Wisconsin.                                              *
*                                                                      *
***********************************************************************/
/* $Id: uwm_xrmb.h,v 1.2 2007/10/04 08:27:51 mtms Exp $ */

#ifndef _UWM_XRMB_H
#define _UWM_XRMB_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * constants
 */
#define XRM_PELLETS 8              /* total number of pellets */
#define XRM_L_PELLETS 2            /* number of pellets on lips */
#define XRM_T_PELLETS 4            /* number of pellets on tongue */
#define XRM_M_PELLETS 2            /* number of pellets on teeth */
#define XYD_FIELDS (2*XRM_PELLETS) /* number of fields in XYD format */
#define TXY_FIELDS (1+XYD_FIELDS)  /* number of fields in TXY format */
#define XRM_FIELD_SEP "\t"         /* field separator */
#define XRM_INFINITE 1000000       /* initial/invalid pellet data */
#define XRM_SAMPFREQ 1000000.0     /* pseudo sampling frequency */
#define XRM_FRAMEDUR 6866          /* default frame duration in samples */
#define XRM_MS_SHIFT 6.866         /* default frame shift in ms */
/*
 * structures
 */
typedef struct pellet_coordinates {
  long x;       /* coordinates in um */
  long y;
} PELLET;

typedef struct X_Ray_Microbeam_record {
  PELLET UL;    /* Upper Lip */
  PELLET LL;    /* Lower Lip */
  PELLET TT;    /* Tongue Tip */
  PELLET T1;    /* Tongue Blade 1 */
  PELLET T2;    /* Tongue Blade 2 */
  PELLET TD;    /* Tongue Dorsum */
  PELLET MI;    /* Mandible Incisor */
  PELLET MM;    /* Mandible Molar */
} XRMREC;

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _UWM_XRMB_H */
