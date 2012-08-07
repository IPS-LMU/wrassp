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
* File:     spectra.h                                                  *
* Contents: Constants, structures and prototypes for analysis of short-*
*           term (smoothed) spectra.                                   *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
*-- revision history --------------------------------------------------*
*  0.0   implementation as a separate program started        MS 081002 *
*  0.1   up and running                                      MS 211002 *
*  0.2   included non-trace output of cepstrum               MS 051102 *
*  0.3   bug fix: some audio files in SSFF format have start time      *
*        unequal zero                                        MS 211102 *
*  0.4   option syntax unified                               MS 110203 *
*  0.5   added output directory option, corrected isdigit()  MS 120803 *
*  0.6   corrected record alignment in SSFF format           MS 300903 *
*  0.7   bug fix: de-emphasis in getLPSpectrum() didn't clear the      *
*        additional buffer cell;                             MS 070604 *
*  0.8   adapted to new checkSound() and pathlims.h          MS 010305 *
*  0.9   bug fix: cepstral computations erroneous            MS 010805 *
*  0.10  new winfuncs.c and asspdsp.h                        MS 190805 *
*  0.11  work started on libassp version                     MS 090610 *
*  1.0   up and running again                                MS 220710 *
*                                                                      *
***********************************************************************/
/* $Id: spectra.h,v 1.2 2010/07/22 12:13:26 mtms Exp $ */

#ifndef _SPECTRA_H
#define _SPECTRA_H

#include <math.h>    /* floor() */

#include <dlldef.h>  /* ASSP_EXTERN */
#include <asspdsp.h> /* wfunc_e */
#include <asspana.h> /* AOPTS */
#include <dataobj.h> /* DOBJ DF_REAL32/64 GD_MAX_ID_LEN dtype_e */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * version numbers (also to be updated for changes in spectrum.c)
 */
#define SPECT_MAJOR 1
#define SPECT_MINOR 0

/*
 * default analysis parameters
 */
#define SPECT_DEF_SHIFT   5.0       /* frame shift in ms */
#define SPECT_DEF_RES     40.0      /* worse spectral resolution in Hz */
#define SPECT_DEF_NUMFFT  0         /* determined by resolution */
#define SPECT_DEF_CHANNEL 1         /* selected channel */
#define SPECT_DEF_WINDOW "BLACKMAN" /* window function */
#define SPECT_DEF_TYPE   "DFT"      /* spectrum type */
#define SPECT_DEF_FORMAT "SSFF"     /* file format */
#define SPECT_ASC_FORMAT "RAW"      /* alternative ASCII file format */
#define SPECT_DEF_DIGITSA 8         /* digits accuracy (E-format) */
#define SPECT_DEF_DIGITSP 2         /* digits precision (F-format) */

/*
 * spectral type specific defaults
 */
#define DFT_DEF_SIZE      0.0    /* window size defined by FFT length */
#define DFT_DEF_BANDWIDTH 0.0    /* effective bandwidth minimal */
#define DFT_DEF_PREEMPH   0.0    /* PRELIMINARY; no pre-emphasis */

#define LPS_DEF_SIZE      20.0   /* effective window size in ms */
#define LPS_DEF_ORDER     0      /* LP order automatic */
#define LPS_DEF_PREEMPH   -0.95  /* pre-emphasis */

#define CSS_DEF_SIZE      0.0    /* window size defined by resolution */
#define CSS_DEF_ORDER     0      /* number of coefficients automatic */
#define CSS_DEF_PREEMPH   0.0    /* no pre-emphasis */

#define CSS_DFLT_LAGS(sfr) ((int)floor(sfr/800.0))
#define CSS_DFLT_LAGS_STR  "sample rate div 800"

#define CEP_DEF_SIZE      0.0    /* window size defined by resolution */
#define CEP_DEF_PREEMPH   0.0    /* no pre-emphasis */

/*
 * option flags (some preliminary for sonagramm; not accessed in spectrum.c)
 */
#define SPECT_OPT_NONE    0x000000
#define SPECT_OPT_LIN_AMP 0x000001 /* linear amplitude (default dB) */
#define SPECT_OPT_LIN_POW 0x000002 /* linear power */
#define SPECT_OPT_DOUBLE  0x000004 /* keep output in double precision */
#define SPECT_OPT_QUANT   0x000010 /* quantize spectral levels */
#define SPECT_OPT_COLOUR  0x000020 /* colour- rather than grey-scale */
#define LPS_OPT_DEEMPH    0x001000 /* de-emphasize LP smoothed spectrum */

/*
 * fixed parameters
 */
#define SPECT_MIN_RES 0.01      /* lowest spectral resolution */
#define CSS_MIN_LAGS  1         /* minimum number of lags (excl. 0) */
#define SPECT_DFORMAT DF_REAL32 /* file data format */

/*
 * parameters determining audio format capabilities of spectral analysis
 */
#define SPECT_I_CHANS (8)       /* maximum number of input channels */
#define SPECT_O_CHANS (1)       /* maximum number of output channels */
#define SPECT_PFORMAT DF_REAL64 /* processing format */

/*
 * generic data structure for holding converted analysis parameters
 * Note: - Parameters like 'sampFreq' and 'frameShift' are in the 
 *         data object itself.
 *       - Structure contains processing buffers so that on-the-fly
 *         single-frame analysis can be performed with as little 
 *         overhead as possible.
 */
#define SPECT_GD_IDENT "SPECT_generics"
typedef struct SPECT_analysis_parameters {
  char    ident[GD_MAX_ID_LEN+1]; /* identification string */
  long    options;
  long    frameSize;
  long    begFrameNr; /* analysis interval in frames */
  long    endFrameNr;
  long    numFFT;
  dtype_e spType;     /* data type number of spectrum */
  wfunc_e winFunc;    /* type number of window function */
  double  binWidth;   /* (worse) spectral resolution in Hz */
  double  preEmph;
  double *frame;      /* frame buffer (allocated) */
  double *fftBuf;     /* FFT buffer (allocated) */
  double *wfc;        /* window function coefficients (allocated) */
  double *acf;        /* autocorrelation coefficients (allocated) */
  double  corrFac;    /* correction factor for spectral levels */
  double  gain;       /* PRELIMINARY (for sonagram/section) */
  double  range;      /* PRELIMINARY (for sonagram/section) */
  double  maxF;       /* PRELIMINARY (for sonagram/section) */
  double  minF;       /* PRELIMINARY (for sonagram/section) */
  int     numLevels;  /* PRELIMINARY (for sonagram) */
  int     order;      /* LP order / number of cepstral coefficients */
  int     channel;    /* selected channel */
  int     writeOpts;  /* options for writing data to file */
  int     accuracy;   /* digits accuracy in ASCII output */
  int     precision;  /* digits precision in ASCII output */
} SPECT_GD;

/*
 * structure to relate spectrum types as string with data type code
 * and file name extension
 */
typedef struct spectrum_types {
  char   *ident;
  dtype_e type;
  char   *ext;
} SPECT_TYPE;
/* external reference to table in spectra.c */
ASSP_EXTERN SPECT_TYPE spectType[];

/*
 * prototypes of public functions in spectra.c
 */
ASSP_EXTERN dtype_e getSPECTtype(char *str, char *suffix);
ASSP_EXTERN int   setSPECTdefaults(AOPTS *aoPtr);
ASSP_EXTERN int   setDFTdefaults(AOPTS *aoPtr);
ASSP_EXTERN int   setLPSdefaults(AOPTS *aoPtr);
ASSP_EXTERN int   setCSSdefaults(AOPTS *aoPtr);
ASSP_EXTERN int   setCEPdefaults(AOPTS *aoPtr);
ASSP_EXTERN DOBJ *createSPECT(DOBJ *smpDOp, AOPTS *aoPtr);
ASSP_EXTERN DOBJ *computeSPECT(DOBJ *smpDOp, AOPTS *aoPtr, DOBJ *spectDOp);
ASSP_EXTERN void  freeSPECT_GD(void *ptr);
ASSP_EXTERN int   getFTSpectrum(DOBJ *dop);
ASSP_EXTERN int   getLPSpectrum(DOBJ *dop);
ASSP_EXTERN int   getCSSpectrum(DOBJ *dop);
ASSP_EXTERN int   getCepstrum(DOBJ *dop);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _SPECTRA_H */
