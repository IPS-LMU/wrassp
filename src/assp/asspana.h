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
* File:     asspana.h                                                  *
* Contents: Header file for speech analysis functions/programs.        *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: asspana.h,v 1.17 2010/06/14 12:38:18 mtms Exp $ */

#ifndef _ASSPANA_H
#define _ASSPANA_H

#include <math.h>    /* floor() */

#include <dlldef.h>  /* ASSP_EXTERN */
#include <dataobj.h> /* DOBJ */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * some handy definitions
 */
#define ANA_BUF_BYTES (65536)  /* if you need some size */
#define ANA_BUF_SECS  (2.0)    /* if you prefer time */

#define DFLT_ORDER(sfr) ((int)floor((double)(sfr)/1000.0 + 3.5))
#define DFLT_ORDER_STR  "sample rate in kHz + 3"

/*DOC

This structure is intended to hold all possible optional parameter 
values for the analysis/filtering programs within the package. 

DOC*/

#define AOPT_STRLEN 31

#define AOPT_RESERVED  0xFF000000 /* mask for upper byte */
#define AOPT_VERBOSE   0x01000000
#define AOPT_BATCH     0x02000000 /* batch processing mode */
#define AOPT_IN_DIR    0x04000000 /* store output in input directory */
#define AOPT_INIT_ONLY 0x08000000 /* only allocate memory etc. */
#define AOPT_EFFECTIVE 0x10000000 /* effective rather than true value */
#define AOPT_USE_ENBW  0x20000000 /* use bandwidth to get window size */
#define AOPT_USE_CTIME 0x40000000 /* use centre time / event analysis */

typedef struct analysis_options {
  long   options;        /* for bit flags (upper byte reserved) */
  double beginTime;      /* times in seconds */
  double endTime;
  double centreTime;     /* for single-frame/event analysis */
  double msSize;         /* (effective) window size in ms */
  double msShift;        /* window shift in ms */
  double msSmooth;       /* size of smoothing window in ms */
  double bandwidth;      /* (effective) bandwidth in Hz */
  double resolution;     /* spectral resolution in Hz */
  double gain;           /* for sonagram/section/filter */
  double range;          /* for sonagram/section */
  double preEmph;        /* preemphasis coefficient */
  double alpha;          /* some paramater value */
  double threshold;      /* some threshold value */
  double maxF;           /* for F0 analysis but also e.g. spectrum */
  double minF;
  double nomF1;          /* e.g. for formant analysis */
  double voiAC1;         /* voicing thresholds */
  double voiMag;
  double voiProb;
  double voiRMS;
  double voiZCR;
  double hpCutOff;       /* filter parameters */
  double lpCutOff;
  double stopDB;
  double tbWidth;
  long   FFTLen;
  int    channel;        /* selected channel (> 0) */
  int    gender;         /* 'u', 'f', 'm', maybe 'b', 'g' or 'c' */
  int    order;          /* analysis/prediction order */
  int    increment;      /* increment/decrement some integral value */
  int    numLevels;      /* for sonagram */
  int    numFormants;
  int    precision;      /* e.g. digits precision of ASCII output */
  int    accuracy;       /* e.g. digits accuracy of ASCII output */
  char   type[AOPT_STRLEN+1];   /* hold-all */
  char   format[AOPT_STRLEN+1];
  char   winFunc[AOPT_STRLEN+1];
} AOPTS;

/*DOC

This structure contains the analysis timing parameters converted to 
sample/frame numbers.

DOC*/

typedef struct analysis_timing {
  double sampFreq;
  long   frameSize;
  long   frameShift;
  long   smoothSize;
  long   begFrameNr;
  long   endFrameNr;
} ATIME;

/*
 * Prototypes of functions in asspana.c.
 */
ASSP_EXTERN int anaTiming(DOBJ *smpDOp, AOPTS *aoPtr, ATIME *tPtr);
ASSP_EXTERN int checkDataBufs(DOBJ *smpDOp, DOBJ *anaDOp, long frameSamples,\
			      long begFrameNr, long endFrameNr);

/*
 * Include the header files with constants, structures and prototypes 
 * for each analysis.
 */
#include <acf.h>
#include <fmt.h>
/* #include <ksv.h> */
#include <mhs.h>
#include <rfc.h>
#include <rms.h>
#include <zcr.h>

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _ASSPANA_H */
