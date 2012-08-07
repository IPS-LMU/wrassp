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
* File:     acf.h                                                      *
* Contents: Constants, structures and prototypes for analysis of short-*
*           term autocorrelation function.                             *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
*-- revision history --------------------------------------------------*
*  0.0   first implementation                                MS 310502 *
*  0.1   fixed SSFF header (forgotten end/IBM_PC iso IBM-PC) MS 180602 *
*  0.2   adapted to new FDESC/DDESC                          MS 300902 *
*  0.3   pre-emphasis option commented out                   MS 081002 *
*  0.4   bug fix: some audio files in SSFF format have start time      *
*        unequal zero                                        MS 211102 *
*  0.5   option syntax unified                               MS 110203 *
*  0.6   added output directory option                       MS 150503 *
*  0.7   output directory option unified                     MS 110803 *
*  0.8   corrected record alignment in SSFF format           MS 290903 *
*  0.9   new checkSound() and pathlims.h                     MS 280205 *
*  0.10  new winfuncs.c and asspdsp.h                        MS 190805 *
*  1.0   new version for linking with libassp and libmisc    MS 021107 *
*  1.1   fixed default values in usage(); new winfuncs.c and           *
*        asspdsp.h                                           MS 210408 *
*  1.2   fixed problems with verifyACF() (now public); renamed ACFANA  *
*        in ACF_GD; included channel selection               MS 250309 *
*  1.3   fixed toggle msEffLen/msSize                        MS 010409 *
*  1.4   bug fix: -od option didn't include base name        MS 200409 *
*  1.5   acfana: removed output file upon analysis error     MS 230909 *
*  1.6   adapted to changes in dataobj.[ch] and asspana.[ch] MS 060110 *
*  1.7   used/set data/frame rate in verifyACF()             MS 220410 *
*  1.8   used AOPT_USE_CTIME for event analysis              MS 050710 *
*                                                                      *
***********************************************************************/
/* $Id: acf.h,v 1.13 2010/07/14 13:49:46 mtms Exp $ */

#ifndef _ACF_H
#define _ACF_H

#include <dlldef.h>  /* ASSP_EXTERN */
#include <asspdsp.h> /* wfunc_e */
#include <asspana.h> /* AOPTS */
#include <dataobj.h> /* DOBJ DF_REAL64 GD_MAX_ID_LEN */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * version numbers (also to be updated for changes in acfana.c)
 */
#define ACF_MAJOR 1
#define ACF_MINOR 8

/*
 * default analysis parameters
 */
#define ACF_DEF_SHIFT   5.0       /* frame shift in ms */
#define ACF_DEF_SIZE    20.0      /* (effective) frame size in ms */
#define ACF_DEF_ORDER   0         /* analysis order automatic */
#define ACF_DEF_CHANNEL 1         /* selected channel */
#define ACF_DEF_WINDOW "BLACKMAN" /* window function */
#define ACF_DEF_SUFFIX ".acf"     /* file name extension */
#define ACF_DEF_FORMAT "SSFF"     /* file format */
#define ACF_ASC_FORMAT "RAW"      /* alternative ASCII file format */
#define ACF_DEF_DIGITS  14        /* digits accuracy (E-format) */

/*
 * option flags
 */
#define ACF_OPT_NONE   0x0000
#define ACF_OPT_MEAN   0x0001  /* length-normalized coefficients */
#define ACF_OPT_NORM   0x0002  /* energy-normalized coefficients */

/*
 * fixed parameters
 */
#define ACF_DFORMAT DF_REAL64  /* data format */

/*
 * parameters determining audio format capabilities of analysis
 */
#define ACF_I_CHANS (8)       /* maximum number of input channels */
#define ACF_O_CHANS (1)       /* maximum number of output channels */
#define ACF_PFORMAT DF_REAL64 /* processing format */

/*
 * generic data structure for holding converted analysis parameters
 * Note: - Parameters like 'sampFreq' and 'frameShift' are in the 
 *         data object itself.
 *       - Structure contains processing buffers so that on-the-fly
 *         single-frame analysis can be performed with as little 
 *         overhead as possible.
 */
#define ACF_GD_IDENT "ACF_generics"
typedef struct ACF_analysis_parameters {
  char    ident[GD_MAX_ID_LEN+1]; /* identification string */
  long    options;
  long    frameSize;
  long    begFrameNr; /* analysis interval in frames */
  long    endFrameNr;
  int     order;      /* analysis order */
  wfunc_e winFunc;    /* type number of window function */
  double *frame;      /* frame buffer (allocated) */
  double *wfc;        /* window function coefficients (allocated) */
  double *acf;        /* autocorrelation coefficients (allocated) */
  double  gainCorr;   /* correction for gain of window function */
  int     channel;    /* selected channel */
  int     writeOpts;  /* options for writing data to file */
  int     accuracy;   /* digits accuracy in ASCII output */
} ACF_GD;

/*
 * prototypes of public functions in acf.c
 */
ASSP_EXTERN int   setACFdefaults(AOPTS *aoPtr);
ASSP_EXTERN DOBJ *createACF(DOBJ *smpDOp, AOPTS *aoPtr);
ASSP_EXTERN DOBJ *computeACF(DOBJ *smpDOp, AOPTS *aoPtr, DOBJ *acfDOp);
ASSP_EXTERN int   verifyACF(DOBJ *acfDOp, DOBJ *smpDOp, AOPTS *aoPtr);
ASSP_EXTERN void  freeACF_GD(void *ptr);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _ACF_H */
