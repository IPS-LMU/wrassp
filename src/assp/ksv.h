/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1989 - 2011  Michel Scheffers                          *
*                            IPdS, CAU Kiel                            *
* Copyright (C) 2011 - 2012  Michel Scheffers                          *
*                            ISFAS-ASW, CAU Kiel                       *
*                            Leibnizstr. 10                            *
*                            24118 Kiel, Germany                       *
*                            m.scheffers@isfas.uni-kiel.de             *
*                                                                      *
* This library is free software: you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation, either version 3 of the License, or    *
* (at your option) any later version.                                  *
*                                                                      *
* This library is distributed in the hope that it will be useful, but  *
* comes WITHOUT ANY WARRANTY; without even the implied warranty of     *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                 *
* See the GNU General Public License (GPL) for more details.           *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with this library. If not, see <http://www.gnu.org/licenses/>. *
*                                                                      *
*----------------------------------------------------------------------*
*                                                                      *
* File:     ksv.h                                                      *
* Contents: Constants, structures and prototypes implementing the      *
*           K. Schäfer-Vincent Periodicity Detection Algorithm.        *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
*-- revision history --------------------------------------------------*
*   -    first working ASSP implementation                   MS 140689 *
*  0.0   extracted from ASSP                                 MS 010395 *
*  0.1   tested on PAS and APOLLO                            MS 100395 *
*  1.0   tested on DOS; bugs in evalArgs(), ksvTwin() and handling     *
*        begSn/endSn in batch mode corrected                 MS 220395 *
*  1.1   minor modifications                                 MS 040495 *
*  1.2   buffer variable and settable                        MS 030595 *
*  1.3   frameDur bottom clipped to 1                        MS 180595 *
*  1.4   options unified                                     MS 130695 *
*  1.5   data handling unified; analysis interval option     MS 100795 *
*  1.6   system definitions => miscdefs.h                    MS 210895 *
*  1.7   optionally byte swapping of input signal            MS 241095 *
*  1.8   adapted to new version of miscdefs.h; use end-of-file to      *
*        determine end sample                                MS 190697 *
*  2.0   optionally output of period markers                 MS 011097 *
*  2.1   handles files with KTH/SWELL header                 MS 151097 *
*  2.2   file handling unified; restructured; voiced region included   *
*        in period markers                                   MS 101198 *
*  2.3   XASSP header in ASCII file output                   MS 231198 *
*  2.4   EOF marker in tag output (ksv_pda.c); new headers.[ch];       *
*        -r => -R options                                    MS 031298 *
*  2.5   items in FDESC structure changed                    MS 150199 *
*  2.6   data type definitions changed in headers.h          MS 231000 *
*  2.7   replaced atof() by strtod()                         MS 280301 *
*  2.8   installed message handler; used new headers.[ch]    MS 150801 *
*  2.9   bug fix: some audio files in SSFF format have start time      *
*        unequal zero                                        MS 211102 *
*  3.0   restructured to new standard; binary output in SSFF MS 140203 *
*  3.1   added output directory option                       MS 110803 *
*  3.2   corrected record alignment in SSFF format           MS 300903 *
*  3.3   new checkSound() and pathlims.h                     MS 280205 *
*  3.4   doubled size of twin buffer                         MS 051208 *
*  3.5   work started on new assp library version            MS 280110 *
*  4.0   up and running again                                MS 270510 *
*  4.1   bug fixes: initialization condition for extrema search        *
*        botched up, reload of workspace used incorrect time;          *
*        increased tolerance on period durations in twin to 12% and    *
*        added test on zero-crossing rate within period      MS 040610 *
*  4.2   new default settings for timing parameters; error message     *
*        if single-frame analysis set                        MS 120710 *
*  4.3   bug fix: last period marker of voiced stretch occasionally    *
*        missed, not cleared and output at wrong time        MS 200910 *
*  4.4   ensured correct setting of timing parameters        MS 181010 *
*  4.5   bug fix: only checked end of range, not end of data MS 040111 *
*  4.6   bug fix: after extending the twin buffer the previous last    *
*        element was used rather than the new one which then contained *
*        random values; removed sub-sub-octave chains        MS 060111 *
*  4.7   corrected range for 'unknown'                       MS 270612 *
*                                                                      *
***********************************************************************/
/* $Id: ksv.h,v 1.8 2012/06/27 13:18:04 mtms Exp $ */

#ifndef _KSV_H
#define _KSV_H

#include <stddef.h>  /* size_t */

#include <dlldef.h>  /* ASSP_EXTERN */
#include <asspana.h> /* AOPTS */
#include <asspdsp.h> /* wfunc_e */
#include <dataobj.h> /* DOBJ DF_REAL32 GD_MAX_ID_LEN */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * version numbers (also to be updated for changes in f0_ksv.c)
 */
#define KSV_MAJOR 4
#define KSV_MINOR 6

/*
 * gender-specific F0-ranges
 */
#define KSV_MINF0_f 80.0  /* female */
#define KSV_MAXF0_f 640.0
#define KSV_MINF0_m 50.0  /* male */
#define KSV_MAXF0_m 400.0
#define KSV_MINF0_u 50.0  /* unknown */
#define KSV_MAXF0_u 640.0

/*
 * default values for optional parameters
 */
#define KSV_DEF_SHIFT   5.0    /* window shift (ms) */
#define KSV_DEF_GENDER  'u'    /* gender unknown */
#define KSV_DEF_CHANNEL 1      /* selected channel */
/* voicing decision parameters */
#define KSV_DEF_VOIMAG  50.0   /* minimum extremum magnitude */
#define KSV_DEF_VOIZCR  3000.0 /* maximum zero crossing rate (Hz) */

#define KSV_DEF_SUFFIX ".f0"   /* file name extension */
#define KSV_DEF_FORMAT "SSFF"  /* file format */
#define KSV_ASC_FORMAT "XASSP" /* alternative ASCII file format */
#define KSV_DEF_DIGITS  2      /* digits precision (ASCII) */
#define KSV_DEF_PRDEXT ".prd"  /* extension for period markers */

/*
 * option flags
 */
#define KSV_OPT_NONE    0x0000
#define KSV_OPT_PRD_OUT 0x0001
#define KSV_OPT_PRD_MIX 0x0002

/*
 * constants
 */
#define KSV_SSFF_ID "F0"      /* SSFF track name */
#define KSV_DFORMAT DF_REAL32 /* data format */
#define KSV_ABSMIN_F0 10.0    /* lowest minF0 allowed */

/*
 * parameters determining audio format capabilities of analysis
 */
#define KSV_I_CHANS (8)       /* maximum number of input channels */
#define KSV_O_CHANS (1)       /* maximum number of output channels */
#define KSV_PFORMAT DF_REAL32 /* processing format */

/*
 * generic data structure for holding converted analysis parameters
 * Note: - parameters like 'sampFreq' and 'frameShift' are in the 
 *         data object itself
 */
#define KSV_GD_IDENT "KSV_generics"

typedef struct KSV_analysis_parameters {
  char    ident[GD_MAX_ID_LEN+1]; /* identification string */
  long    options;
  long    begFrameNr; /* analysis interval in frames */
  long    endFrameNr;
  double  minF0;
  double  maxF0;
  double  voiMag;     /* voiced magnitude threshold */
  double  voiZCR;     /* zero crossing threshold */
  int     channel;    /* selected channel */
  int     precision;  /* digits precision in ASCII output */
  int     writeOpts;  /* options for writing data to file */
} KSV_GD;

/*
 * prototypes of public functions in ksv.c
 */
ASSP_EXTERN void  printKSVrefs(void);
ASSP_EXTERN int   setKSVgenderDefaults(AOPTS *aoPtr, char gender);
ASSP_EXTERN int   setKSVdefaults(AOPTS *aoPtr);
ASSP_EXTERN DOBJ *createKSV(DOBJ *smpDOp, AOPTS *aoPtr);
ASSP_EXTERN DOBJ *createPRD(DOBJ *smpDOp, AOPTS *aoPtr);
ASSP_EXTERN DOBJ *computeKSV(DOBJ *smpDOp, AOPTS *aoPtr,\
			     DOBJ *f0DOp, DOBJ *prdDOp);
ASSP_EXTERN int   verifyKSV(DOBJ *f0DOp, DOBJ *smpDOp, AOPTS *aoPtr);
ASSP_EXTERN void  freeKSV_GD(void *ptr);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _KSV_H */
