/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 2003 - 2010  Michel Scheffers                          *
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
* File:     diff.h                                                     *
* Contents: Constants, structures and prototypes for audio signal      *
*           differentiation.                                           *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
*-- revision history --------------------------------------------------*
*  0.0   inplementation started                              MS 230903 *
*  0.1   up and running (finally)                            MS 270404 *
*  0.2   fixed some minor bugs                               MS 140604 *
*  0.3   commented out b/e options (confusing)               MS 100804 *
*  0.4   adapted to new checkSound() and pathlims.h          MS 280205 *
*  0.5   bug fix: output buffer not reset for pass 2         MS 190405 *
*  1.0   new version for libassp; 16-, 24- and 32-bit integer          *
*        supported; channel selection added; always rescaling          *
*        upon overflow                                       MS 170310 *
*  1.1   removed WAVE_X -> WAVE (now implemented)            MS 300310 *
*                                                                      *
***********************************************************************/
/* $Id: diff.h,v 1.2 2010/03/30 14:36:27 mtms Exp $ */

#ifndef _DIFF_H
#define _DIFF_H

#include <dlldef.h>  /* ASSP_EXTERN */
#include <asspana.h> /* AOPTS */
#include <dataobj.h> /* DOBJ DF_INT32 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * version numbers (also to be updated for changes in afdiff.c)
 */
#define DIFF_MAJOR 1
#define DIFF_MINOR 1

/*
 * constants
 */
/* #define DIFF_SSFF_ID "dsamples" */ /* would probably be too confusing */

/*
 * default option values
 */
#define DIFF_DEF_CHANNEL (1) /* selected channel */

/*
 * option flags
 */
#define DIFF_OPT_NONE     0x0000
#define DIFF_OPT_BACKWARD 0x0001 /* backward difference */
#define DIFF_OPT_CENTRAL  0x0002 /* central difference */

/*
 * parameters determining audio format capabilities of 'afdiff'
 */
#define DIFF_I_CHANS (8)      /* maximum number of input channels */
#define DIFF_O_CHANS (1)      /* maximum number of output channels */
#define DIFF_PFORMAT DF_INT32 /* processing format */

/*
 * prototypes of public functions in diff.c
 */
ASSP_EXTERN int   setDiffDefaults(AOPTS *aoPtr);
ASSP_EXTERN DOBJ *createDiff(DOBJ *inpDOp, AOPTS *aoPtr);
ASSP_EXTERN DOBJ *diffSignal(DOBJ *inpDOp, AOPTS *aoPtr, DOBJ *outDOp);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _DIFF_H */
