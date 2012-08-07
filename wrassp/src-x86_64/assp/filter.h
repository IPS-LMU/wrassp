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
* File:     filter.h                                                   *
* Contents: Constants, structures and prototypes for the 'affilter'    *
*           module.                                                    *
* Author:   Michel T.M. Scheffers                                      *
* References:                                                          *
*   Rabiner, L.R., Mc Gonegal, C.A. and Paul, D, (1979), "FIR Windowed *
*     Filter Design Program - WINDOW" in: "Programs for Digital Signal *
*     Processing." IEEE Press, Section 5.2                             *
*                                                                      *
*-- revision history --------------------------------------------------*
*  0.0   inplementation started                              MS 230903 *
*  0.1   up and running                                      MS 110604 *
*  0.2   switched from edge to cut-off (halfway the transition band;   *
*        ~ -6 dB) as in Rabiner et al.                       MS 140604 *
*  0.3   commented out b/e options (confusing)               MS 100804 *
*  0.4   renamed, minor changes                              MS 191004 *
*  0.5   new checkSound() and pathlims.h                     MS 280205 *
*  0.6   bug fix: output buffer not reset for pass 2         MS 190405 *
*  0.7   new winfuncs.c and asspdsp.h                        MS 190805 *
*  1.0   new version for linking with libassp and libmisc              *
*        changes from R 0.2 undone; IIR option added; gain in % full   *
*        scale; dithering in conversion to int; 16-, 24- and 32-bit    *
*        integer and 32- and 64-bit float supported          MS 120809 *
*  1.1   added channel option; tested for 16-bit             MS 190110 *
*  1.2   removed WAVE_X -> WAVE (now implemented); tested on 32-bit    *
*        float; included fixes for relevant bugs             MS 300310 *
*  1.3   bug fix: brackets forgotten in the computation of 'offset' in *
*        'storeBlock' (introduced in previous revision).     MS 060810 *
*                                                                      *
***********************************************************************/
/* $Id: filter.h,v 1.3 2010/09/20 09:23:32 mtms Exp $ */

#ifndef _FILTER_H
#define _FILTER_H

#include <dlldef.h>  /* ASSP_EXTERN */
#include <asspana.h> /* AOPTS */
#include <asspdsp.h> /* WF_... FILTER */
#include <dataobj.h> /* DOBJ DF_REAL64 GD_MAX_ID_LEN */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * version numbers (also to be updated for changes in affilter.c)
 */
#define FILT_MAJOR 1
#define FILT_MINOR 3

/*
 * default option values
 */
#define FILT_DEF_HP_CO 0.0   /* filters off/undefined */
#define FILT_DEF_LP_CO 0.0
#define FILT_DEF_WIDTH 250.0 /* FIR: width of transition band */
#define FILT_DEF_ATTEN 96.0  /* FIR: stop-band attenuation */
#define FILT_DEF_SECTS 4     /* IIR: number of 2nd-order sections */
#define FILT_DEF_CHANNEL 1   /* selected channel */

/*
 * option flags
 */
#define FILT_OPT_NONE     0x0000
#define FILT_OPT_AUTOGAIN 0x0001
#define FILT_OPT_USE_IIR  0x0002

#define FILT_NOPT_DITHER  0x0100

/*
 * constants
 */
#define FILT_FIR_WINFUNC "KAISER_B" /* at present fixed */
#define FILT_MIN_ATTEN 21.0 /* min. attenuation in Kaiser-window design */

/*
 * parameters determining audio format capabilities of 'affilter'
 */
#define FILT_I_CHANS (8)       /* maximum number of input channels */
#define FILT_O_CHANS (1)       /* maximum number of output channels */
#define FILT_PFORMAT DF_REAL64 /* processing format */

/*
 * generic data structure holding converted parameters
 * and filter coefficients 
 */
#define FILT_GD_IDENT "FILT_generics"
typedef struct FILT_parameters {
  char    ident[GD_MAX_ID_LEN+1]; /* identification string */
  long    options; /* not in FILTER structure */
  double  gain;    /* "" */
  int     channel; /* selected channel */
  FILTER *fPtr;
} FILT_GD;

/*
 * prototypes of public functions in filter.c
 */
ASSP_EXTERN int   setFILTdefaults(AOPTS *aoPtr);
ASSP_EXTERN int   getFILTtype(AOPTS *aoPtr, char *suffix);
ASSP_EXTERN DOBJ *createFilter(DOBJ *inpDOp, AOPTS *aoPtr);
ASSP_EXTERN DOBJ *filterSignal(DOBJ *inpDOp, DOBJ *filtDOp, DOBJ *outDOp);
ASSP_EXTERN DOBJ *destroyFilter(DOBJ *filtDOp);
ASSP_EXTERN void  freeFILT_GD(void *generic);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _FILTER_H */
