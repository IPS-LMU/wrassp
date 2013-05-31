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
* File:     zcr.h                                                      *
* Contents: Constants, structures and prototypes for zero-crossing     *
*           analysis.                                                  *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
*-- revision history --------------------------------------------------*
*  0.0   first implementation                                MS 100798 *
*  0.1   centre clipping included                            MS 150798 *
*  0.2   used standard function checkSound()                 MS 101198 *
*  0.3   channel => track; -r => -R options                  MS 031298 *
*  0.4   items in FDESC structure changed                    MS 150199 *
*  1.0   restructured as new standard                        MS 160502 *
*  1.1   adapted to new FDESC/DDESC                          MS 300902 *
*  1.2   bug fix: some audio files in SSFF format have start time      *
*        unequal zero                                        MS 211102 *
*  1.3   option syntax unified                               MS 110203 *
*  1.4   added output directory option, corrected isdigit()  MS 120803 *
*  1.5   corrected record alignment in SSFF format           MS 300903 *
*  1.6   adapted to new checkSound() and pathlims.h          MS 010305 *
*  2.0   new version for linking with libassp and libmisc    MS 151007 *
*  2.1   zcrTrack back to pointer; improved getZCR function  MS 021107 *
*  2.2   fixed default values in usage(); new winfuncs.c and           *
*        asspdsp.h                                           MS 210408 *
*  2.3   moved revision history and version numbers to this file;      *
*        using new function 'getFrame' and extended audio capabilities *
*        correspondingly                                     MS 170109 *
*  2.4   fixed problems with verifyZCR() (now public); moved 'frame'   *
*        from generic data to local global variable; renamed ZCRANA    *
*        in ZCR_GD; included channel selection               MS 190309 *
*  2.5   extended verifyZCR(); corrected channel loop; disabled        *
*        effective length                                    MS 230309 *
*  2.6   bug fix: -od option didn't include base name        MS 200409 *
*  2.7   zcrana: removed output file upon analysis error     MS 230909 *
*  2.8   adapted to changes in dataobj.[ch]]                 MS 030110 *
*  2.9   used/set data/frame rate in verifyZCR()             MS 220410 *
*  2.10   used AOPT_USE_CTIME for event analysis              MS 060710 *
*                                                                      *
***********************************************************************/
/* $Id: zcr.h,v 1.10 2010/07/14 13:49:46 mtms Exp $ */

#ifndef _ZCR_H
#define _ZCR_H

#include <dlldef.h>  /* ASSP_EXTERN */
#include <asspana.h> /* AOPTS */
#include <dataobj.h> /* DOBJ DF_REAL32 GD_MAX_ID_LEN */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * version numbers (also to be updated for changes in zcrana.c)
 */
#define ZCR_MAJOR 2
#define ZCR_MINOR 10

/*
 * default analysis parameters
 */
#define ZCR_DEF_SIZE    25.0   /* frame size in ms */
#define ZCR_DEF_SHIFT   5.0    /* frame shift in ms */
#define ZCR_DEF_CHANNEL 0      /* multi-channel output */
#define ZCR_DEF_DIGITS  1      /* digits precision (ASCII) */
#define ZCR_DEF_SUFFIX  ".zcr" /* file name extension */
#define ZCR_DEF_FORMAT  "SSFF" /* file format */
#define ZCR_ASC_FORMAT  "RAW"  /* alternative ASCII file format */

/*
 * option flags
 */
#define ZCR_OPT_NONE  0x0000
/* #define ZCR_OPT_RM_DC 0x0001  problematic */

/*
 * fixed parameters
 */
#define ZCR_DFORMAT DF_REAL32 /* data format */
#define ZCR_HEAD   (1L)       /* leading samples in frame buffer */
#define ZCR_TAIL   (0L)       /* trailing samples in frame buffer */
/*
 * parameters determining audio format capabilities of analysis
 */
#define ZCR_I_CHANS (8)       /* maximum number of input channels */
#define ZCR_O_CHANS ZCR_I_CHANS /* maximum number of output channels */
#define ZCR_PFORMAT DF_REAL64 /* processing format */

/*
 * generic data structure for holding converted analysis parameters
 * Note: - Parameters like 'sampFreq' and 'frameShift' are in the 
 *         data object itself.
 */
#define ZCR_GD_IDENT "ZCR_generics"
typedef struct ZCR_analysis_parameters {
  char ident[GD_MAX_ID_LEN+1]; /* identification string */
  long options;
  long frameSize;
  long begFrameNr;
  long endFrameNr;
  int  channel;    /* either 0 or selected channel */
  int  writeOpts;  /* options for writing data to file */
  int  precision;  /* digits precision in ASCII output */
} ZCR_GD;

/*
 * prototypes of public functions in zcr.c
 */
ASSP_EXTERN int   setZCRdefaults(AOPTS *aoPtr);
ASSP_EXTERN DOBJ *createZCR(DOBJ *smpDOp, AOPTS *aoPtr);
ASSP_EXTERN DOBJ *computeZCR(DOBJ *smpDOp, AOPTS *aoPtr, DOBJ *zcrDOp);
ASSP_EXTERN int   verifyZCR(DOBJ *zcrDOp, DOBJ *smpDOp, AOPTS *aoPtr);
ASSP_EXTERN void  freeZCR_GD(void *ptr);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _ZCR_H */
