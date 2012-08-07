/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1989 - 20010  Michel Scheffers                          *
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
* File:     rfc.h                                                      *
* Contents: Constants, structures and prototypes for the analysis of   *
*           reflection coefficients or other linear prediction data.   *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
*-- revision history --------------------------------------------------*
*  0.0   first implementation as portable version            MS 180595 *
*  0.1   RMS value stored in dB; bug in fwrite() corrected   MS 260595 *
*  1.0   tested on Apollo                                    MS 310595 *
*  1.1   tested on Linux; -help option introduced            MS 070695 *
*  1.2   options unified                                     MS 200695 *
*  1.3   data handling unified; buffer check corrected       MS 100795 *
*  2.0   more flexible version; binary output in SSFF format MS 210502 *
*  2.1   fixed SSFF header                                   MS 180602 *
*  2.2   adapted to new FDESC/DDESC                          MS 011002 *
*  2.3   bug fix: some audio files in SSFF format have start time      *
*        unequal zero                                        MS 211102 *
*  2.4   option syntax unified                               MS 110203 *
*  2.5   added output directory option                       MS 110803 *
*  2.6   corrected record alignment in SSFF format           MS 300903 *
*  2.7   minor modifications                                 MS 021104 *
*  2.8   adapted to new checkSound() and pathlims.h          MS 010305 *
*  2.9   new winfuncs.c and asspdsp.h                        MS 190805 *
*  3.0   new version to include in libassp with rfcana as command-line *
*        wrapper                                             MS 070209 *
*  3.1   removed verifyLP() (too complex); renamed LP_ANA in LP_GD;    *
*        included channel selection                          MS 270309 *
*  3.2   fixed toggle msEffLen/msSize                        MS 010409 *
*  3.3   bug fix: -od option didn't include base name        MS 200409 *
*  3.4   bug fix: average ACF removed while getMeanACF() sometimes     *
*        yields instable LP filter                           MS 140509 *
*  3.5   rfcana: removed output file upon analysis error     MS 230909 *
*  3.6   adapted to changes in dataobj.[ch] and asspana.[ch] MS 060110 *
*  3.7   used AOPT_USE_CTIME for event analysis              MS 050710 *
*                                                                      *
***********************************************************************/
/* $Id: rfc.h,v 1.9 2010/07/14 13:49:46 mtms Exp $ */

#ifndef _RFC_H
#define _RFC_H

#include <dlldef.h>  /* ASSP_EXTERN */
#include <asspdsp.h> /* wfunc_e RMS_MIN_AMP/_dB */
#include <asspana.h> /* AOPTS */
#include <dataobj.h> /* DOBJ DF_REAL32 DF_REAL64 GD_MAX_ID_LEN dtype_e */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * version numbers (also to be updated for changes in rfcana.c)
 */
#define RFC_MAJOR 3
#define RFC_MINOR 7

/*
 * default analysis parameters
 */
#define LP_DEF_SHIFT   5.0        /* frame shift in ms */
#define LP_DEF_SIZE    20.0       /* (effective) frame size in ms */
#define LP_DEF_PREEMPH -0.95      /* preemphasis */
#define LP_DEF_ORDER   0          /* analysis order automatic */
#define LP_DEF_CHANNEL 1          /* selected channel */
#define LP_DEF_WINDOW  "BLACKMAN" /* window function */
#define LP_DEF_TYPE    "RFC"      /* parameter coding type */
#define LP_DEF_FORMAT  "SSFF"     /* file format */
#define LP_ASC_FORMAT  "RAW"      /* alternative ASCII format */
#define LP_DEF_DIGITSA 14         /* digits accuracy (E-format) */
#define LP_DEF_DIGITSP 2          /* digits precision (F-format) */

/*
 * option flags
 */
#define LP_OPT_NONE 0x0000
/* #define LP_OPT_MEAN 0x0001 */ /* use length-normalized autocorrelation */

/*
 * fixed parameters
 */
#define LP_RFORMAT DF_REAL32 /* RMS & gain format (dB) */
#define LP_CFORMAT DF_REAL64 /* coefficients format */

/*
 * parameters determining audio format capabilities of analysis
 */
#define LP_I_CHANS (8)       /* maximum number of input channels */
#define LP_O_CHANS (1)       /* maximum number of output channels */
#define LP_PFORMAT DF_REAL64 /* processing format */

/*
 * generic data structure for holding converted analysis parameters
 * Note: - Parameters like 'sampFreq' and 'frameShift' are in the 
 *         data object itself.
 */
#define LP_GD_IDENT "LP_generics"

typedef struct LP_analysis_parameters {
  char    ident[GD_MAX_ID_LEN+1]; /* identification string */
  long    options;
  long    frameSize;
  long    begFrameNr; /* analysis interval in frames */
  long    endFrameNr;
  double  preEmph;    /* preemphasis coefficient */
  int     order;      /* analysis order */
  dtype_e dataType;   /* parameter coding type */
  wfunc_e winFunc;    /* type number of window function */
  int     channel;    /* selected channel */
  int     writeOpts;  /* options for writing data to file */
  int     precision;  /* digits precision RMS and gain in ASCII */
  int     accuracy;   /* digits accuracy coefficients in ASCII */
} LP_GD;

/*
 * structure to relate parameter coding as string with data type code
 * and file name extension
 */
typedef struct LP_coding_types {
  char   *ident;
  dtype_e type;
  char   *ext;
} LP_TYPE;
/* external reference to table in rfc.c */
ASSP_EXTERN LP_TYPE lpType[];

/*
 * prototypes of functions in rfc.c
 */
ASSP_EXTERN int   setLPdefaults(AOPTS *aoPtr);
ASSP_EXTERN DOBJ *createLP(DOBJ *smpDOp, AOPTS *aoPtr);
ASSP_EXTERN DOBJ *computeLP(DOBJ *smpDOp, AOPTS *aoPtr, DOBJ *lpDOp);
ASSP_EXTERN void  freeLP_GD(void *ptr);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _RFC_H */
