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
* File:     fmt.h                                                      *
* Contents: Constants, structures and prototypes for the formant       *
*           estimator 'forest'.                                        *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
*-- revision history --------------------------------------------------*
*  0.0   compilation of 'klara' and 'ksort', omitting F0 data, label   *
*        option and pseudo formants                          MS 071102 *
*  0.1   raw analysis running                                MS 021202 *
*  0.2   fixed bug in sorting: array index overflow          MS 121202 *
*  0.3   improved limits table and sorting                   MS 181202 *
*  0.4   fixed screen output (may be binary)                 MS 160103 *
*  0.5   option syntax unified                               MS 110203 *
*  0.6   added output directory option                       MS 110803 *
*  0.7   corrected record alignment in SSFF format           MS 300903 *
*  0.8   attempt to improve performance on female voices     MS 190304 *
*  0.9   extended limits table; using dynamic programming    MS 070504 *
*  1.0   pre-emphasis sample rate dependent and reduced; classification*
*        in three passes and enforced solution for remaining clashes;  *
*        several small bugs fixed                            MS 250604 *
*  1.1   adapted to new checkSound() and pathlims.h          MS 010305 *
*  1.2   new winfuncs.c and asspdsp.h                        MS 190805 *
*  2.0   new version to include in libassp with forest as command-line *
*        wrapper; included channel and gender selection      MS 120509 *
*  2.1   bug fixes: windowing forgotten before LP analysis; replaced   *
*        getMeanCF() by getACF while former sometimes yields instable  *
*        LP filter; sharpened termination criteria           MS 140509 *
*  2.2   reduced eff. length for female voices               MS 180509 *
*  2.3   added quartiles/histogram in iteration statistics; allowed    *
*        negative silence threshold with bottom clip; added count of   *
*        silent frames; extended check on instable filter; restructured*
*        PQ initialization including use of PFs and retry    MS 290509 *
*  2.4   forest: removed output file upon analysis error     MS 230909 *
*  2.5   adapted to changes in dataobj.[ch] and asspana.[ch];          *
*        added setFMTgenderDefaults() function               MS 110110 *
*  2.6   used AOPT_USE_CTIME for event analysis              MS 120710 *
*  2.7   bug fix: retrying root solving with start values based on     *
*        PFs or nominal Fs deactivated by typo               MS 100810 *
*  2.8   avoided break off if seekSLAzx() fails              MS 110810 *
*                                                                      *
***********************************************************************/
/* $Id: fmt.h,v 1.9 2010/08/11 15:23:18 mtms Exp $ */

#ifndef _FMT_H
#define _FMT_H

#include <dlldef.h>  /* ASSP_EXTERN */
#include <misc.h>    /* STAT */
#include <asspana.h> /* AOPTS */
#include <asspdsp.h> /* wfunc_e */
#include <dataobj.h> /* DOBJ DF_INT16 DF_REAL64 GD_MAX_ID_LEN dtype_e */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * version numbers (also to be updated for changes in forest.c)
 */
#define FMT_MAJOR 2
#define FMT_MINOR 8

/*
 * default analysis parameters
 */
#define FMT_DEF_SIZE    -1.0       /* frame size undefined */
#define FMT_DEF_SHIFT   5.0        /* frame shift in ms */
#define FMT_DEF_EFFLENf 12.5       /* effective window length in ms (female) */
#define FMT_DEF_EFFLENm 20.0       /* effective window length in ms (male) */
#define FMT_DEF_FIXEMPH -0.8       /* pre-emphasis (if fixed) */
#define FMT_DEF_ORDER   0          /* analysis order automatic */
#define FMT_DEF_INCR    0          /* increment/decrement default order */
#define FMT_DEF_CHANNEL 1          /* selected channel */
#define FMT_DEF_WINDOW  "BLACKMAN" /* window function */
#define FMT_DEF_SILENCE 0.0        /* RMS threshold for silence in dB */
#define FMT_DEF_NOMF1f  560.0      /* nominal F1 frequency (female) */
#define FMT_DEF_NOMF1m  500.0      /* nominal F1 frequency (male) */
#define FMT_DEF_GENDER  'm'        /* default gender: male */
#define FMT_DEF_OUT     4          /* number of output formants */
#define FMT_DEF_SUFFIX  ".fms"     /* extension for output files */
#define FMT_DEF_FORMAT  "SSFF"     /* file format */
#define FMT_ASC_FORMAT  "RAW"      /* alternative ASCII format */
#define FMT_DEF_DIGITSA 14         /* digits accuracy (E-format) */

/*
 * option flags
 */
#define FMT_OPT_NONE      0x00000000
#define FMT_OPT_PE_FIXED  0x00000001 /* pre-emphasis fixed */
#define FMT_OPT_LPO_FIXED 0x00000004 /* LP order fixed */
#define FMT_OPT_INS_ESTS  0x00000008 /* insert estimates */

/* NOT-options for evaluation purposes only */
#define FMT_NOT_SORT_PQ   0x00010000
#define FMT_NOT_TRACK_PQ  0x00020000
#define FMT_NOT_USE_PF    0x00040000
#define FMT_NOT_RETRY_PQ  0x00080000
/* preliminary via TRACE until evaluated */
#define FMT_OPT_PE_ADAPT  0x00000002 /* signal-adaptive pre-emphasis */

/*
 * analysis constants
 */
#define FMT_MIN_ORDER  4   /* minimum LP order (for sorting) */
#define FMT_MAX_OUT    8   /* maximum number of output formants */
#define FMT_MAX_BUF (FMT_MAX_OUT+2) /* extra space for shifting/sorting */
#define FMT_RGFORMAT   DF_REAL32 /* RMS signal and gain amplitudes */
#define FMT_PEFORMAT   DF_REAL64 /* adaptive pre-emphasis coefficient */
#define FMT_FBFORMAT   DF_INT16  /* frequencies and bandwidths */
#define FMT_NUM_FSTATS 5   /* number of statistical variables for rf */
#define FMT_NUM_PSTATS FMT_MAX_BUF /* same for pf */

/*
 * parameters determining audio format capabilities of analysis
 */
#define FMT_I_CHANS (8)       /* maximum number of input channels */
#define FMT_O_CHANS (1)       /* maximum number of output channels */
#define FMT_PFORMAT DF_REAL64 /* processing format */

/*
 * generic data structure for holding converted analysis parameters
 * Note: - Parameters like 'sampFreq' and 'frameShift' are in the 
 *         data object itself.
 */
#define FMT_GD_IDENT "forest_generics"

typedef struct forest_analysis_parameters {
  char    ident[GD_MAX_ID_LEN+1]; /* identification string */
  long    options;
  long    frameSize;
  long    begFrameNr;  /* analysis interval in frames */
  long    endFrameNr;
  wfunc_e winFunc;     /* type number of window function */
  double  nomF1;
  double  rmsSil;      /* silence threshold (dB) */
  double  preEmph;     /* pre-emphasis coefficient */
  int     lpOrder;     /* LP analysis order */
  int     numFormants; /* number of output formants */
  int     channel;     /* selected channel */
  int     writeOpts;   /* options for writing data to file */
  int     accuracy;    /* digits accuracy adaptive premphasis in ASCII */
} FMT_GD;

/*
 * prototypes of public functions in fmt.c
 */
ASSP_EXTERN void  printFMTrefs(void);
ASSP_EXTERN int   setFMTgenderDefaults(AOPTS *aoPtr, char gender);
ASSP_EXTERN int   setFMTdefaults(AOPTS *aoPtr);
ASSP_EXTERN DOBJ *createFMT(DOBJ *smpDOp, AOPTS *aoPtr);
ASSP_EXTERN DOBJ *computeFMT(DOBJ *smpDOp, AOPTS *aoPtr, DOBJ *fmtDOp);
ASSP_EXTERN void  freeFMT_GD(void *ptr);
ASSP_EXTERN void  initFMTstats(void);
ASSP_EXTERN void  freeFMTstats(void);

/*
 * public variables from fmt.c
 */
ASSP_EXTERN STAT statPF, statPQ, statP[], statF[];
ASSP_EXTERN unsigned long totFMTfiles, totFMTframes, totFMTsilent, totFMTfail;

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _FMT_H */
