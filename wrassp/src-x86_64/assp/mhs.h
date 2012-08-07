/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1979 - 1983  Michel Scheffers                          *
*                            Institute for Perception Research (IP0)   *
*                            Den Dolech 2                              *
*                            5600 MB Eindhoven, The Netherlands        *
* Copyright (C) 1986 - 2010  Michel Scheffers                          *
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
* File:     mhs.h                                                      *
* Contents: Constants, structures and prototypes for the 'MHS' pitch   *
*           analysis algorithm.                                        *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
*-- revision history --------------------------------------------------*
*  0.0   re-implementation of FORTRAN version of MDWS        MS 230999 *
*  0.1   playing with parameter values                       MS 270999 *
*  0.2   renamed to MHS; tuning to dynamic signals           MS 300999 *
*  0.3   added pitch tracking                                MS 121099 *
*  0.4   optional use of subharmonics as sieve bases         MS 151099 *
*  0.5   Q integer (0..100); let pending chains grow; meanQ calculated *
*        over MAXMEAN values                                 MS 191099 *
*  0.6   fixed bug in subharmonic generation; restricted convolution   *
*        range;                                              MS 231299 *
*  0.7   set a lower limit to the relative difference between F0's in  *
*        adjacent frames                                     MS 290200 *
*  1.0   completely reworked as part of the ASSP library; extended     *
*        voicing detection; omitted SPL estimates and fixated HF slope *
*        of excitation pattern; optionally unmasked spectrum; sieve    *
*        bases from subharmonics.                            MS 100108 *
*  1.1   bug fix: break missing in evalArgs()                MS 280208 *
*  1.2   fixed default values in usage(); incorporated changes         *
*        in mhs.c                                            MS 090408 *
*  1.3   moved revision history and version numbers to this file;      *
*        using new function 'getFrame' and extended audio capabilities *
*        correspondingly                                     MS 190109 *
*  1.4   removed verifyMHS() (too complex); renamed MHS in MHS_GD;     *
*        included channel and gender selection               MS 300309 *
*  1.5   bug fix: -od option didn't include base name        MS 200409 *
*  1.6   f0_mhs: removed output file upon analysis error     MS 230909 *
*  1.7   adapted to changes in dataobj.[ch] and asspana.[ch]; added    *
*        gender option and setMHSgenderDefaults() function   MS 080110 *
*  1.8   f0_mhs: int casts for isdigit()                     MS 300310 *
*  1.9   switch off test on ZCR by setting a low threshold   MS 140610 *
*  1.10  new default settings for timing parameters; error message     *
*        if single-frame analysis set                        MS 120710 *
*  1.11  ensured correct setting of timing parameters        MS 181010 *
*                                                                      *
***********************************************************************/
/* $Id: mhs.h,v 1.14 2010/10/18 13:07:34 mtms Exp $ */

#ifndef _MHS_H
#define _MHS_H

#include <stddef.h>  /* size_t */

#include <dlldef.h>  /* ASSP_EXTERN */
#include <asspana.h> /* AOPTS */
#include <asspdsp.h> /* wfunc_e */
#include <dataobj.h> /* DOBJ DF_REAL32 GD_MAX_ID_LEN */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * version numbers (also to be updated for changes in f0_mhs.c)
 */
#define MHS_MAJOR 1
#define MHS_MINOR 11

/*
 * gender-specific F0-ranges
 */
#define MHS_MINF0_f 80.0  /* female */
#define MHS_MAXF0_f 600.0
#define MHS_MINF0_m 50.0  /* male */
#define MHS_MAXF0_m 375.0
#define MHS_MINF0_u 50.0  /* unknown */
#define MHS_MAXF0_u 600.0

/*
 * default values for optional parameters
 */
#define MHS_DEF_SHIFT   5.0    /* window shift (ms) */
#define MHS_DEF_GENDER  'u'    /* gender unknown */
#define MHS_DEF_CHANNEL 1      /* selected channel */
/* voicing decision parameters */
#define MHS_DEF_VOIMAG  50.0   /* minimum signal magnitude */
#define MHS_DEF_VOIZCR  3000.0 /* maximum zero crossing rate (Hz) */
                               /* minimum would be minF0 */
#define MHS_DEF_VOIRMS  18.0   /* minimum RMS amplitude (dB) */
#define MHS_DEF_VOIAC1  0.25   /* minimum AC[1] (signal correlation) */
#define MHS_DEF_MINQVAL 0.52   /* minimum Q of fit (unscaled) */

#define MHS_DEF_SUFFIX ".pit"  /* file name extension */
#define MHS_DEF_FORMAT "SSFF"  /* file format */
#define MHS_ASC_FORMAT "XASSP" /* alternative ASCII file format */
#define MHS_DEF_DIGITS  2      /* digits precision (ASCII) */

/*
 * option flags
 */
#define MHS_OPT_NONE  0x0000
#define MHS_OPT_POWER 0x0001   /* use unmasked power spectrum */

/*
 * fixed parameters values
 */
#define MHS_SSFF_ID "pitch"   /* SSFF track name */
#define MHS_DFORMAT DF_REAL32 /* data format */
#define MHS_WINFUNC "HAMMING" /* window function */
#define MHS_MINBINHZ  20.0    /* worst frequency resolution of FFT */
#define MHS_ABSMIN_F0 25.0    /* lowest minF0 allowed */
#define MHS_MAXPKFREQ 3200.0  /* highest frequency of potential harmonic */
#define MHS_MINPEAKS  3       /* min. number of harmonics for analysis */
#define MHS_MAXPEAKS  12      /* max. number of potential harmonics */
#define MHS_MAXSUBS   6       /* max. number of subharmonics */
#define MHS_MESHWIDTH 0.08    /* width of meshes in sieve */
#define MHS_Q_SCALE   1000.0  /* upscaling factor for quality-of-fit values */
#define MHS_REL_TOPQ  0.80    /* factor for determining top canditates */
/* pitch tracking */
#define MHS_MAXCANDS  5       /* max. number of concurrent F0 candidates */
#define MHS_MINF0DIFF 0.5     /* min. difference between F0 candidates (ST) */
#define MHS_MAXDELTA  240.0   /* max. frame-to-frame change in F0 (ST/s) */
#define MHS_MINDURVS  25.0    /* min. duration of voiced segment (ms) */
#define MHS_MINPRDVS  2.5     /* min. number of periods in voiced segment */
#define MHS_MINDELAY  100.0   /* min. delay (ms) before data output */
#define MHS_MAXDURTQ  30.0    /* max. duration for computing track-Q (ms) */
#define MHS_MINNUMTQ  3       /* min. number of values for computing track-Q */

/* dependent constants */
#define MHS_MAXSIEVES (MHS_MAXPEAKS*MHS_MAXSUBS*3) /* max. number of sieve F0s */
#define MHS_MAXTRACKS (2*MHS_MAXCANDS) /* ample because tracks die when not continued */

/*
 * parameters determining audio format capabilities of analysis
 */
#define MHS_I_CHANS (8)       /* maximum number of input channels */
#define MHS_O_CHANS (1)       /* maximum number of output channels */
#define MHS_PFORMAT DF_REAL64 /* processing format */

/*
 * generic data structure for holding converted analysis parameters
 * Note: - parameters like 'sampFreq' and 'frameShift' are in the 
 *         data object itself
 */
#define MHS_GD_IDENT "MHS_generics"

typedef struct MHS_analysis_parameters {
  char    ident[GD_MAX_ID_LEN+1]; /* identification string */
  long    options;
  long    frameSize;
  long    begFrameNr; /* analysis interval in frames */
  long    endFrameNr;
  wfunc_e winFunc;    /* type number of window function */
  double  minF0;
  double  maxF0;
  double  voiMag;     /* general voicing thresholds */
  double  voiZCR;
  double  voiRMS;
  double  voiAC1;
  int     minQval;    /* lowest (upscaled) quality value for voiced */
  int     channel;    /* selected channel */
  int     writeOpts;  /* options for writing data to file */
  int     precision;  /* digits precision in ASCII output */
} MHS_GD;

/*
 * prototypes of public functions in mhs.c
 */
ASSP_EXTERN void  printMHSrefs(void);
ASSP_EXTERN int   setMHSgenderDefaults(AOPTS *aoPtr, char gender);
ASSP_EXTERN int   setMHSdefaults(AOPTS *aoPtr);
ASSP_EXTERN DOBJ *createMHS(DOBJ *smpDOp, AOPTS *aoPtr);
ASSP_EXTERN DOBJ *computeMHS(DOBJ *smpDOp, AOPTS *aoPtr, DOBJ *pitDOp);
ASSP_EXTERN void  freeMHS_GD(void *ptr);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _MHS_H */
