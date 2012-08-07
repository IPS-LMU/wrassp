/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1989 - 2010  Michel Scheffers                          *
*                            IPdS, CAU Kiel                            *
*                            Leibnizstr. 10                            *
*                            24118 Kiel, Germany                       *
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
* File:     rms.h                                                      *
* Contents: Constants, structures and prototypes for RMS amplitude     *
*           analysis.                                                  *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
*-- revision history --------------------------------------------------*
*  0.0   extracted from ASSP                                 MS 130695 *
*  0.1   tested on PAS and APOLLO                            MS 230695 *
*  1.0   further unified with other programs                 MS 100795 *
*  1.1   adapted to new version of miscdefs.h                MS 180697 *
*  2.0   unified file header handling                        MS 230798 *
*  2.1   updated with new headers.c; correction for gain of window     *
*        function included                                   MS 191198 *
*  2.2   XASSP header in ASCII file output                   MS 231198 *
*  2.3   adapted to new headers.[ch]; -r => -R options       MS 031298 *
*  2.4   items in FDESC structure changed                    MS 150199 *
*  3.0   restructured to new standard                        MS 300902 *
*  3.1   bug fix: some audio files in SSFF format have start time      *
*        unequal zero                                        MS 211102 *
*  3.2   option syntax unified                               MS 110203 *
*  3.3   added output directory option, corrected isdigit()  MS 120803 *
*  3.4   corrected record alignment in SSFF format           MS 300903 *
*  3.5   adapted to new checkSound() and pathlims.h          MS 010305 *
*  3.6   new winfuncs.c and asspdsp.h                        MS 190805 *
*  4.0   new version for linking with libassp and libmisc    MS 021107 *
*  4.1   fixed default values in usage(); new winfuncs.c and           *
*        asspdsp.h                                           MS 210408 *
*  4.2   moved revision history and version numbers to this file;      *
*        using new function 'getFrame' and extended audio capabilities *
*        correspondingly                                     MS 180109 *
*  4.3   fixed problems with verifyRMS() (now public); moved 'frame'   *
*        from generic data to local global variable; renamed RMSANA    *
*        in RMS_GD; included channel selection               MS 200309 *
*  4.4   fixed toggle msEffLen/msSize                        MS 010409 *
*  4.5   bug fix: -od option didn't include base name        MS 200409 *
*  4.6   rmsana: removed output file upon analysis error     MS 230909 *
*  4.7   adapted to changes in dataobj.[ch] and asspana.[ch] MS 060110 *
*  4.8   used/set data/frame rate in verifyRMS()             MS 220410 *
*  4.9   used AOPT_USE_CTIME for event analysis              MS 060710 *
*                                                                      *
***********************************************************************/
/* $Id: rms.h,v 1.12 2010/07/14 13:49:46 mtms Exp $ */

#ifndef _RMS_H
#define _RMS_H

#include <dlldef.h>  /* ASSP_EXTERN */
#include <asspdsp.h> /* wfunc_e RMS_MIN_AMP/_dB */
#include <asspana.h> /* AOPTS */
#include <dataobj.h> /* DOBJ DF_REAL32 GD_MAX_ID_LEN */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * version numbers (also to be updated for changes in rmsana.c)
 */
#define RMS_MAJOR 4
#define RMS_MINOR 9

/*
 * default analysis parameters
 */
#define RMS_DEF_SIZE    20.0     /* (effective) frame size in ms */
#define RMS_DEF_SHIFT   5.0      /* frame shift in ms */
#define RMS_DEF_CHANNEL 0        /* multi-channel output */
#define RMS_DEF_WINDOW "HAMMING" /* window function */
#define RMS_DEF_SUFFIX ".rms"    /* file name extension */
#define RMS_DEF_FORMAT "SSFF"    /* file format */
#define RMS_ASC_FORMAT "XASSP"   /* alternative ASCII file format */
#define RMS_DEF_DIGITS  2        /* digits precision (ASCII) */

/*
 * option flags
 */
#define RMS_OPT_NONE   0x0000
#define RMS_OPT_LINEAR 0x0001 /* linear amplitude instead of dB */

/*
 * fixed parameters
 */
#define RMS_DFORMAT DF_REAL32 /* data format */

/*
 * parameters determining audio format capabilities of analysis
 */
#define RMS_I_CHANS (8)       /* maximum number of input channels */
#define RMS_O_CHANS RMS_I_CHANS /* maximum number of output channels */
#define RMS_PFORMAT DF_REAL64 /* processing format */

/*
 * generic data structure holding converted analysis parameters
 * Note: - Parameters like 'sampFreq' and 'frameShift' are in the 
 *         data object itself.
 */
#define RMS_GD_IDENT "RMS_generics"
typedef struct RMS_analysis_parameters {
  char    ident[GD_MAX_ID_LEN+1]; /* identification string */
  long    options;
  long    frameSize;
  long    begFrameNr; /* analysis interval in frames */
  long    endFrameNr;
  wfunc_e winFunc;    /* type number of window function */
  int     channel;    /* either 0 or selected channel */
  int     writeOpts;  /* options for writing data to file */
  int     precision;  /* digits precision in ASCII output */
} RMS_GD;

/*
 * prototypes of public functions in rms.c
 */
ASSP_EXTERN int   setRMSdefaults(AOPTS *aoPtr);
ASSP_EXTERN DOBJ *createRMS(DOBJ *smpDOp, AOPTS *aoPtr);
ASSP_EXTERN DOBJ *computeRMS(DOBJ *smpDOp, AOPTS *aoPtr, DOBJ *rmsDOp);
ASSP_EXTERN int   verifyRMS(DOBJ *rmsDOp, DOBJ *smpDOp, AOPTS *aoPtr);
ASSP_EXTERN void  freeRMS_GD(void *ptr);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _RMS_H */
