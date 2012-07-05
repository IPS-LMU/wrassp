/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1989 - 2011  Michel Scheffers                          *
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
* File:     asspdsp.h                                                  *
* Contents: Macros, constants and prototypes of signal processing and  *
*           mathematical functions.                                    *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: asspdsp.h,v 1.29 2012/03/19 16:16:38 lasselasse Exp $ */

#ifndef _ASSPDSP_H
#define _ASSPDSP_H

#include <stdio.h>    /* FILE */
#include <stddef.h>   /* size_t */
#include <math.h>     /* log() log10() pow() */
#include <inttypes.h> /* int16_t uint32_t */

#include <dlldef.h>   /* ASSP_EXTERN */
#include <miscdefs.h> /* PI TWO_PI */

#ifdef __cplusplus
extern "C" {
#endif

/* the other logarithms */

#define LOG2(x) (log((double)(x))/log(2.0))
#define LOGb(b,x) (log((double)(x))/log((double)(b)))

/* clip values for log conversion of small values in double precision */

#define TINYLIN (1.0E-150)
#define TINYLOG (-150.0)
#define TINYLN  (-345.4)
#define TINYSQR (1.0E-300)
#define TINYPLG (-300.0)
#define TINYPLN (-690.8)
#define TINYPdB (-3000.0)

/* clip values for conversion of RMS amplitude values to dB */

#define RMS_MIN_AMP 0.1  /* generally well below quantization noise */
#define RMS_MIN_SQR 0.01 /* squared value, i.e. without Root */
#define RMS_MIN_dB -20.0 /* corresponding dB value */
/* same for LP filter gain */
#define GAIN_MIN_LIN 0.01
#define GAIN_MIN_SQR 0.0001 /* squared value, i.e. without Root */
#define GAIN_MIN_dB -40.0   /* corresponding dB value */

/* macros to convert to and from decibels */

#define LINtodB(x) (20.0*log10((double)(x)))
#define SQRtodB(x) (10.0*log10((double)(x)))
#define dBtoLIN(x) (pow(10.0, (double)(x)/20.0))
#define dBtoSQR(x) (pow(10.0, (double)(x)/10.0))

/* autogain values for 'afconvert', 'affilter', .. (percent full scale) */

#define AF_MAX_GAIN (99.0)
#define AF_DEF_GAIN (95.0)
#define AF_MIN_GAIN (1.0)

/* maximum number of temporary files for conversion/filtering */

#define AF_MAX_TEMP (9)

/* dither magnitude according to HPT (usually 1 LSB) */

#define DITHER_MAG (0.88)

/* a 'beautiful' prime to start a fixed random sequence */

#define START_SEED (1276543UL)

/*
 * mathematical functions
 */
typedef struct bairstow_termination {
  int    maxIter;  /* maximum number of iterations e.g. 100 */
  double absPeps;  /* maximum abolute error in dp value e.g. 1.0e-12 */
  double relPeps;  /* maximum relative error in dp value e.g. 1.0e-6 */
  double absQeps;  /* maximum abolute error in dq value e.g. 1.0e-12 */
  double relQeps;  /* maximum relative error in dq value e.g. 1.0e-6 */
} BAIRSTOW;

/*
 * prototypes of functions in math.c
 */
ASSP_EXTERN uint32_t GCD(uint32_t N, uint32_t M);
ASSP_EXTERN double   LCM(uint32_t N, uint32_t M);
ASSP_EXTERN int linterpol(double X1, double Y1, double X2, double Y2,\
			  double x, double *yPtr);
ASSP_EXTERN int parabola(double y1, double y2, double y3, double dx,\
			 double *X0ptr, double *Y0ptr, double *Aptr);
ASSP_EXTERN int bairstow(double *c, double *p, double *q, double *r,\
			 int M, double *t, BAIRSTOW *term);
ASSP_EXTERN int hasCCR(double p, double q);
ASSP_EXTERN double besselI0(double x, double e);
ASSP_EXTERN double bessi0(double x);

/*
 * window functions
 */
#define WF_FULL_SIZE  0x00 /* full length and symmetric */
#define WF_PERIODIC   0x01 /* periodic (true length) and symmetric */
#define WF_ASYMMETRIC 0x03 /* periodic and asymmetric */
#define WF_MIN_SIZE   (3)  /* minimum length */

typedef enum window_function_type_number {
  WF_ERROR = -1,
  WF_NONE      , /* not the same as a rectangular window ! */
  WF_RECTANGLE ,
  WF_TRIANGLE  ,
  WF_BARTLETT = WF_TRIANGLE,
  WF_FEJER = WF_TRIANGLE,
  WF_PARABOLA  ,
  WF_RIESZ = WF_PARABOLA,
  WF_WELCH = WF_PARABOLA,
  WF_COSINE    ,
  WF_COS = WF_COSINE,
  WF_COS_2     , /* cos^2 */
  WF_HANN = WF_COS_2, /* Hann/hanning */
  WF_COS_3     , /* cos^3 */
  WF_COS_4     , /* cos^4 */
  WF_HAMMING   , /* standard hamming */
  WF_BLACKMAN  , /* Blackman */
  WF_BLACK_X   , /* exact Blackman */
  WF_BLACK_3   , /* 3-term Blackman-Harris */
  WF_BLACK_M3  , /* min. 3-term Blackman-Nuttal */
  WF_BLACK_4   , /* 4-term Blackman-Harris */
  WF_BLACK_M4  , /* min. 4-term Blackman-Nuttal */
  WF_NUTTAL_3  , /* 3-term Nuttal */
  WF_NUTTAL_4  , /* 4-term Nuttal */
  WF_GAUSS2_5  , /* Gaussian */
  WF_GAUSS3_0  ,
  WF_GAUSS3_5  ,
  WF_KAISER2_0 , /* Kaiser-Bessel */
  WF_KAISER2_5 ,
  WF_KAISER3_0 ,
  WF_KAISER3_5 ,
  WF_KAISER4_0 ,
  WF_NUM_FIX   , /* adjusts itself to give the number of fixed windows */
  /* here follow the parametric window functions */
  WF_COS_A = WF_NUM_FIX, /* cos^alpha */
  WF_GEN_HAMM  , /* generalised Hamming */
  WF_GAUSS_A   , /* Gaussian(alpha) */
  WF_KAISER_A  , /* Kaiser-Bessel(alpha) */
  WF_KAISER_B  , /* Kaiser-Bessel(beta) */
  WF_KBD_A     , /* Kaiser-Bessel-derived (alpha) */
  WF_NUM_ALL
} wfunc_e;

typedef struct window_function_list {
  char   *code;  /* code name of function */
  char   *desc;  /* brief description */
  wfunc_e type;  /* type number (see above) */
} WFLIST;
/* refence to tables in winfuncs.c */
ASSP_EXTERN WFLIST wfShortList[], wfLongList[], wfAlphaList[];

typedef struct window_function_properties {
  WFLIST *entry; /* pointer to entry in window function list */
  double  hsll;  /* highest side lobe level in dB */
  double  roff;  /* side lobe roll-off in dB/oct */
  double  gain;  /* coherent gain (mean of coeff's; linear) */
  double  msqr;  /* incoherent gain (mean of squares; linear) */
  double  dB_3;  /* -3 dB bandwidth (bins) */
  double  enbw;  /* equivalent noise/rectangular bandwidth (bins) */
  double  dB_6;  /* -6 dB bandwidth (bins) */
  double  mlbw;  /* main lobe bandwidth (bins) */
} WFDATA;
/*
 * prototypes of functions in winfuncs.c
 */
ASSP_EXTERN wfunc_e wfType(char *str);
ASSP_EXTERN WFLIST *wfListEntry(WFLIST *list, char *code, char *desc,\
				wfunc_e type);
ASSP_EXTERN WFDATA *wfSpecs(wfunc_e type);
ASSP_EXTERN double *makeWF(wfunc_e type, long N, int flags);
ASSP_EXTERN double *makeWF_A(wfunc_e type, double alpha, long N,\
                             int flags);
ASSP_EXTERN void    mulSigWF(double *s, double *w, long N);
ASSP_EXTERN void    freeWF(double *w);
ASSP_EXTERN void    listWFs(WFLIST *list, FILE *fp);
ASSP_EXTERN double  wfCohGain(double *w, long N);
ASSP_EXTERN double  wfIncGain(double *w, long N);
ASSP_EXTERN double  wfENBW(double *w, long N);
ASSP_EXTERN double  kaiserBeta(double att);
ASSP_EXTERN long    kaiserLength(double sfr, double trb, double att);
ASSP_EXTERN long    bandwidth2frameSize(double bandwidth, wfunc_e type,\
					double sampFreq, long nFFT);
ASSP_EXTERN double  frameSize2bandwidth(long frameSize, wfunc_e type,\
					double sampFreq, long nFFT);

/*
 * digital filters
 */
#define FILTER_ERROR (-1)
#define FILTER_NONE 0x0000  /* filter type codes */
#define FILTER_HP   0x0001  /* high pass */
#define FILTER_LP   0x0002  /* low pass */
#define FILTER_BP   0x0003  /* band pass */
#define FILTER_BS   0x0007  /* band stop */
#define FILTER_RSN  0x0010  /* resonance (PRELIMINARY) */
#define FILTER_ARN  0x0020  /* anti-resonance (PRELIMINARY) */
#define FILT_MASK_C 0x00FF  /* mask to get the filter characteristics */
#define FILTER_FIR  0x0100  /* symmetrical (linear-phase) FIR filter */
#define FILTER_IIR  0x0200  /* generalised IIR filter */
#define FILTER_IIR2 0x0400  /* IIR filter (cascaded 2nd order sections) */
#define FILT_MASK_S 0x0F00  /* mask to get the filter structure */

typedef struct FIR_filter {
  size_t  numCoeffs;   /* total length of filter */
  double *c;           /* filter coefficients (ALLOCATED) */
} FIR;

typedef struct IIR_filter { /* Type II/canonical form */
  size_t  numZeros;    /* number of zeros (a-coefficients) */
  size_t  numPoles;    /* number of poles (b-coefficients) */
  double *a;           /* numerator coefficients (ALLOCATED) */
  double *b;           /* denominator coefficients (ALLOCATED) */
  double *z;           /* filter taps (ALLOCATED) */
} IIR;

typedef struct IIR2_filter { /* Type II/canonical form */
  size_t  numSections; /* number of 2nd-order sections */
  double *a0;          /* numerator coefficients (ALLOCATED) */
  double *a1;
  double *a2;
  /* b0 is per definition 1 */
  double *b1;          /* denominator coefficients (ALLOCATED) */
  double *b2;
  double *z1;          /* filter taps (ALLOCATED) */
  double *z2;
} IIR2;

typedef struct filter_parameters {
  double  sampFreq;    /* sampling frequency */
  double  hpCutOff;    /* high-pass cut-off frequency */
  double  lpCutOff;    /* low-pass cut-off frequency */
  double  centreFreq;  /* (anti-)resonance frequency */
  double  quality;     /* IIR: Q-value of (anti-)resonance */
                       /* use negative value for anti-resonance (also FIR) */
  double  tbWidth;     /* FIR: width of transition band */
  double  stopDB;      /* FIR: stop-band attenuation in dB ( > 0) */
  wfunc_e winFunc;     /* FIR: window function for design (PRELIMINARY) */
  int     type;        /* type of filter */
  union filter_data {
    FIR  fir;
    IIR  iir;
    IIR2 iir2;
  } data;
} FILTER;
/*
 * prototypes of functions in filters.c
 */
ASSP_EXTERN int    setFilterType(FILTER *fip);
ASSP_EXTERN int    checkFilter(FILTER *fip);
ASSP_EXTERN int    designFIR(FILTER *fip);
ASSP_EXTERN int    designIIR2(FILTER *fip);
ASSP_EXTERN void   clearTaps(FILTER *fip);
ASSP_EXTERN void   freeFilter(FILTER *fip);
ASSP_EXTERN double FIRfilter(FILTER *fip, double *firstSample);
ASSP_EXTERN double IIRfilter(FILTER *fip, double sample);
ASSP_EXTERN double IIR2filter(FILTER *fip, double sample);
/* ASSP_EXTERN int     invertIIR(FILTER *inv, FILTER *fip); */

/*
 * FFT
 */
#define FFT_FORWARD 1
#define FFT_INVERSE (-1)
#define MIN_NFFT 4
/*
 * prototypes of functions in fft.c
 */
ASSP_EXTERN int    fft(double *x, long N, int DIRECT);
ASSP_EXTERN int    rfft(double *x, long N, int DIRECT);
ASSP_EXTERN void   rfftRe(double *c, double *r, long N);
ASSP_EXTERN void   rfftIm(double *c, double *r, long N);
ASSP_EXTERN void   rfftLinAmp(double *c, double *a, long N);
ASSP_EXTERN void   rfftLinPow(double *c, double *p, long N);
ASSP_EXTERN void   rfftLogPow(double *c, double *p, long N);
ASSP_EXTERN void   rfftPower(double *c, double *p, long N);
ASSP_EXTERN void   rfftPhase(double *c, double *p, long N);
ASSP_EXTERN long   freq2bin(double freq, double sampFreq, long nDFT);
ASSP_EXTERN double bin2freq(long bin, double sampFreq, long nDFT);

/*
 * frequency conversions
 * prototypes of functions in freqconv.c
 */
ASSP_EXTERN double hz2st(double f, double R);
ASSP_EXTERN double st2hz(double s, double R);
ASSP_EXTERN double rel2st(double r);
ASSP_EXTERN double st2rel(double s);
ASSP_EXTERN double hz2mel(double f);
ASSP_EXTERN double mel2hz(double m);
ASSP_EXTERN double hz2bark(double f);
ASSP_EXTERN double bark2hz(double z);
ASSP_EXTERN double cb_hz_at_z(double z);
ASSP_EXTERN double hz2erb(double f);
ASSP_EXTERN double erb2hz(double e);
ASSP_EXTERN double erb_hz_at_f(double f);

/*
 * Linear Prediction
 *
 * dimensions (M is prediction order):
 * acf[M+1]
 * arf[M+1]
 * cep[M+1]
 * lar[M]
 * lpc[M+1]
 * rfc[M]
 * ffb[M]
 * pqp[M]
 * rfb[M]
 * pf[M+1/2]
 *
 */
#define MAXLPORDER  50 /* some functions use fixed-size arrays */
#define MAXFORMANTS 25 /* corresponding number of formants */
/*
 * prototypes of functions in lpc.c
 */
ASSP_EXTERN int asspDurbin(double *acf, double *lpc, double *rfc,\
		       double *errPtr, int M);
ASSP_EXTERN int arf2rfc(double *arf, double *rfc, int M);
ASSP_EXTERN int lar2rfc(double *lar, double *rfc, int M);
ASSP_EXTERN int lpc2cep(double *lpc, double sqerr, double *cep, int M);
ASSP_EXTERN int lpc2rfc(double *lpc, double *rfc, int M);
ASSP_EXTERN int rfc2arf(double *rfc, double *arf, int M);
ASSP_EXTERN int rfc2lar(double *rfc, double *lar, int M);
ASSP_EXTERN int rfc2lpc(double *rfc, double *lpc, int M);
ASSP_EXTERN int lpc2pqp(double *lpc, double *pqp, int M, BAIRSTOW *tPtr);
ASSP_EXTERN int ffb2pqp(double *ffb, double *pqp, int N, double sampFreq);
ASSP_EXTERN int pqp2rfb(double *pqp, double *rfb, int N, double sampFreq);
ASSP_EXTERN int lpSLA(double *acf, double *lpc, double *normPtr, int M,\
		      double *pf, double sampFreq);

/*
 * utilities
 *
 * dimensions:
 * double s[N]
 * double o[N]
 * double r[M+1]
 * double a[N]
 * double b[N+M+1]
 * double c[M+1]
 */
#define freq2emph(f, sfr) (exp(-1.0 * TWO_PI * (f) / (sfr)))
/*
 * prototypes of functions in dsputils.c
 */
ASSP_EXTERN double removeDC(double *s, double *o, long N);
ASSP_EXTERN int    preEmphasis(double *s, double u, double tap, long N);
ASSP_EXTERN int    emphWinI16(int16_t *s, double u, double tap,\
			      double *wf, double *o, long N);
ASSP_EXTERN int    mulWinI16(int16_t *s, double *wf, double *o, long N);
ASSP_EXTERN int    getACF(double *s, double *r, long N, int M);
ASSP_EXTERN int    getMeanACF(double *s, double *r, long N, int M);
ASSP_EXTERN double getNormACF(double *s, double *r, long N, int M);
ASSP_EXTERN int    getCCF(double *a, double *b, double *c, long N, int M);
ASSP_EXTERN int    getAMDF(double *s, double *c, long N, int minLag, int maxLag);
ASSP_EXTERN double getZCR(double *s, long N, double sfr);
ASSP_EXTERN double getRMS(double *s, long N);
ASSP_EXTERN double getMaxMag(double *s, long N);
ASSP_EXTERN long   getMaxMagI16(int16_t *s, long N);
ASSP_EXTERN double randRPDF(uint32_t *seedPtr);
ASSP_EXTERN double randTPDF(uint32_t *seedPtr);
/* ASSP_EXTERN double randGPDF(uint32_t *seedPtr); */ /* TO BE IMPLEMENTED */

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _ASSPDSP_H */
