/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1989 - 2010  Michel Scheffers                          *
*                            IPdS, CAU Kiel                            *
*                            Leibnizstr. 10                            *
*                            24118 Kiel                                *
*                            Germany                                   *
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
* File:     filters.c                                                  *
* Contents: Functions for digital filters.                             *
*           Prototypes etc. are in asspdsp.h                           *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: filters.c,v 1.7 2010/01/08 13:41:23 mtms Exp $ */

#include <stdio.h>    /* NULL */
#include <stddef.h>   /* size_t */
#include <stdlib.h>   /* malloc() free() */
#include <math.h>     /* sin() cos() tan() fabs() sqrt() */
#include <float.h>    /* DBL_EPSILON */

#include <miscdefs.h> /* ODD() MAX() */
#include <asspmess.h> /* error handler */
#include <asspdsp.h>  /* FILTER FIR IIR IIR2 FILTER_.. KAISER... */

/*DOC

Sets the 'characteristics' part of the item 'type' in the structure 
pointed to by "fip" on the basis of the frequency values given in that 
structure. The item 'centreFreq' is hereby given the highest priority. 
In order to distinguish between resonance and anti-resonance the item 
'quality' should have a negative value for the latter even for FIR 
filters where it is not used.
The function returns the type code (> 0) upon success and -1 upon error.

DOC*/

int setFilterType(FILTER *fip)
{
  int fType;

  if(fip == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setFilterType");
    return(-1);
  }
  fType = FILTER_NONE;
  if(fip->centreFreq > 0.0) {
    if(fip->quality < 0.0)
      fType = FILTER_ARN;
    else
      fType = FILTER_RSN;
  }
  else if(fip->hpCutOff > 0.0) {
    if(fip->lpCutOff > 0.0) {
      if(fip->lpCutOff >= fip->hpCutOff)
	fType = FILTER_BP;
      else
	fType = FILTER_BS;
    }
    else
      fType = FILTER_HP;
  }
  else if(fip->lpCutOff > 0.0) {
    fType = FILTER_LP;
  }
  else {
    setAsspMsg(AEG_ERR_BUG, "Can't determine filter characteristics");
    return(-1);
  }
  fip->type &= ~FILT_MASK_C;           /* clear 'characteristic' bits */
  fip->type |= fType;                    /* set 'characteristic' bits */
  return(fType);
}

/*DOC

Verifies the validity of the frequency settings in the structure pointed 
to by "fip". Prior to a call of this function the items "fType" and 
"sampFreq" in that structure must have been set correctly.
The function returns -1 when there are conflicts otherwise 0.

DOC*/

int checkFilter(FILTER *fip)
{
  double Nyquist;

  if(fip == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "checkFilter");
    return(-1);
  }
  if(fip->sampFreq <= 0.0) {
    setAsspMsg(AEG_ERR_APPL, "checkFilter: sample rate undefined");
    return(-1);
  }
  Nyquist = fip->sampFreq / 2.0;    /* only check high-frequency part */
  switch(fip->type & FILT_MASK_C) {
  case FILTER_HP:
  case FILTER_LP:
  case FILTER_BP:
  case FILTER_BS:
    if(fip->hpCutOff >= Nyquist) {
      setAsspMsg(AEG_ERR_APPL, "checkFilter: high-pass cut-off frequency "\
		 "at or above Nyquist rate");
      return(-1);
    }
    switch(fip->type & FILT_MASK_S) {
    case FILTER_FIR:
      if(fip->tbWidth <= 0.0) {
	setAsspMsg(AEG_ERR_APPL, "checkFilter: transition band undefined");
	return(-1);
      }
      if(fip->lpCutOff >= (Nyquist - 0.1 * fip->tbWidth)) {
	setAsspMsg(AEG_ERR_APPL, "checkFilter: low-pass cut-off frequency "\
		   "above or too near to Nyquist rate");
	return(-1);
      }
      break;
    default:
      if(fip->lpCutOff >= Nyquist) {
	setAsspMsg(AEG_ERR_APPL, "checkFilter: low-pass cut-off frequency "\
		   "at or above Nyquist rate");
	return(-1);
      }
    }
    break;
  case FILTER_RSN:
  case FILTER_ARN:
    if((fip->type & FILT_MASK_S) == FILTER_IIR2 &&\
       fip->quality == 0.0) {
      setAsspMsg(AEG_ERR_APPL, "checkFilter: quality factor undefined");
      return(-1);
    }
    if(fip->centreFreq >= Nyquist) {
      setAsspMsg(AEG_ERR_APPL, "checkFilter: centre frequency "\
		 "at or above Nyquist rate");
      return(-1);
    }
    break;
  default:
    setAsspMsg(AEG_ERR_APPL, "checkFilter: unknown filter type");
    return(-1);
  }
  return(0);
}

/*DOC

Designs an FIR filter whose parameters are given in the structure 
pointed to by "fip", using the classical window-design method with a 
Kaiser-Bessel window. Filter length and coefficients are stored in 
the FIR structure included in the one pointed to by "fip".

From:
   Rabiner, L.R., Mc Gonegal, C.A. and Paul, D, (1979), "FIR Windowed 
   Filter Design Program - WINDOW" in: "Programs for Digital Signal 
   Processing." IEEE Press, Section 5.2

Note:
 - In the original code cut-off frequencies are specified to be in the 
   centre of the transition band (-6 dB point). Since this is not very 
   user-intuitive, I've replaced them by the edge-frequencies of the 
   pass-band. The OLD code is retained but commented out.

DOC*/

int designFIR(FILTER *fip)
{
  long   n, M, wfLen;
  double beta, *wf, Wl, Wh, x;
  FIR   *fir;

  if(fip == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "designFIR");
    return(-1);
  }
  if((fip->type & FILT_MASK_S) != FILTER_FIR) {
    setAsspMsg(AEB_BAD_CALL, "designFIR");
    return(-1);
  }
  fir = &(fip->data.fir);
  fir->numCoeffs = 0;
  fir->c = NULL;
  wf = NULL;
  switch(fip->winFunc) {
  case WF_KAISER_A:
  case WF_KAISER_B:
    beta = kaiserBeta(fip->stopDB);
    wfLen = kaiserLength(fip->sampFreq, fip->tbWidth, fip->stopDB);
    wf = makeWF_A(WF_KAISER_B, beta, wfLen, WF_FULL_SIZE);
    break;
  default:
    setAsspMsg(AEG_NOT_YET, "designFIR: window function other than Kaiser");
    return(-1);
  }
  if(wf == NULL) {
    setAsspMsg(AEG_ERR_MEM, NULL);
    return(-1);
  }
  M = wfLen / 2;                        /* NOTE: length is always odd */
  /* multiply with impulse response of ideal filter */
  switch(fip->type & FILT_MASK_C) {
  case FILTER_LP:
  /*  Wl = 0.0;       normalized lower and higher cut-off frequencies */
  /*  Wh = TWO_PI * fPtr->lpCutOff / fPtr->sampFreq; */
  /*   for(n = 1; n <= M; n++) { */
  /*     wf[M-n] *= (sin((double)n*Wh) / ((double)n*PI)); */
  /*     wf[M+n] = wf[M-n];                               symmetrical */
  /*   } */
  /*   wf[M] *= (Wh / PI);                 NOTE: length is always odd */
    Wl = 0.0;
    Wh = (2.0 * fip->lpCutOff + fip->tbWidth) / fip->sampFreq;
    for(n = 1; n <= M; n++) {
      x = (double)n * PI;
      wf[M-n] *= (sin(x * Wh) / x);
      wf[M+n] = wf[M-n];
    }
    wf[M] *= Wh;
    break;
  case FILTER_HP:
  /*   Wl = TWO_PI * fPtr->hpCutOff / fPtr->sampFreq; */
  /*   Wh = PI; */
  /*   for(n = 1; n <= M; n++) { */
  /*     wf[M-n] *= (-sin((double)n*Wl) / ((double)n*PI)); */
  /*     wf[M+n] = wf[M-n]; */
  /*   } */
  /*   wf[M] *= (1.0 - Wl / PI); */
    Wl = (2.0 * fip->hpCutOff - fip->tbWidth) / fip->sampFreq;
    Wh = 1.0;
    for(n = 1; n <= M; n++) {
      x = (double)n * PI;
      wf[M-n] *= (-sin(x * Wl) / x);
      wf[M+n] = wf[M-n];
    }
    wf[M] *= (1.0 - Wl);
    break;
  case FILTER_BP:
  /*   Wl = PI * fPtr->hpCutOff / fPtr->sampFreq; */
  /*   Wh = PI * fPtr->lpCutOff / fPtr->sampFreq; */
  /*   for(n = 1; n <= M; n++) { */
  /*     wf[M-n] *= (2.0 * cos((double)n*(Wl+Wh)) \ */
  /* 		    * sin((double)n*(Wh-Wl)) / ((double)n*PI)); */
  /*     wf[M+n] = wf[M-n]; */
  /*   } */
  /*   wf[M] *= (2.0 * (Wh-Wl) / PI); */
    Wl = (2.0 * fip->hpCutOff - fip->tbWidth) / fip->sampFreq;
    Wh = (2.0 * fip->lpCutOff + fip->tbWidth) / fip->sampFreq;
    for(n = 1; n <= M; n++) {
      x = (double)n * PI;
      wf[M-n] *= ((sin(x * Wh) - sin(x * Wl)) / x);
      wf[M+n] = wf[M-n];
    }
    wf[M] *= (Wh - Wl);
    break;
  case FILTER_BS:
  /*   Wl = PI * fPtr->lpCutOff / fPtr->sampFreq; */
  /*   Wh = PI * fPtr->hpCutOff / fPtr->sampFreq; */
  /*   for(n = 1; n <= M; n++) { */
  /*     wf[M-n] *= (2.0 * cos((double)n*(Wl+Wh)) \ */
  /*     	    * sin((double)n*(Wl-Wh)) / ((double)n*PI)); */
  /*     wf[M+n] = wf[M-n]; */
  /*   } */
  /*   wf[M] *= (1.0 - 2.0 * (Wh-Wl) / PI); */
    Wl = (2.0 * fip->lpCutOff + fip->tbWidth) / fip->sampFreq;
    Wh = (2.0 * fip->hpCutOff - fip->tbWidth) / fip->sampFreq;
    for(n = 1; n <= M; n++) {
      x = (double)n * PI;
      wf[M-n] *= ((sin(x * Wl) - sin(x * Wh)) / x);
      wf[M+n] = wf[M-n];
    }
    wf[M] *= (1.0 - (Wh - Wl));
    break;
  case FILTER_RSN:
    /* the following is in principle correct but can not be used with */
    /* the Kaiser-Bessel window as determined above (actual main lobe */
    /* is narrower than 2*tbWidth and side-lobes are higher than att) */
    /* double nrg, g, y; */
    /* g = wfCohGain(wf, wfLen); */
    /* nrg = 0.0; */
    /* x = TWO_PI * fip->centreFreq / fip->sampFreq; */
    /* for(n = 1; n <= M; n++) { */
    /*   y = cos((double)n * x); */
    /*   nrg += (y * y);; */
    /*   wf[M+n] = (wf[M-n] *= y); */
    /* } */
    /* nrg += nrg; */
    /* nrg += 1.0; */
    /* x = 1.0/ (nrg * g); */
    /* for(n = 0; n < wfLen; n++) */
    /*   wf[n] *= x; */
    /* emulate (top is flatter) by band-pass */
    Wl = (2.0 * fip->centreFreq - fip->tbWidth) / fip->sampFreq;
    Wh = (2.0 * fip->centreFreq + fip->tbWidth) / fip->sampFreq;
    for(n = 1; n <= M; n++) {
      x = (double)n * PI;
      wf[M-n] *= ((sin(x * Wh) - sin(x * Wl)) / x);
      wf[M+n] = wf[M-n];
    }
    wf[M] *= (Wh - Wl);
    break;
  case FILTER_ARN:
    /* impulse response would be something like: dirac(x) - cos(x) */
    /* emulate by band-stop */
    Wl = (2.0 * fip->centreFreq - fip->tbWidth) / fip->sampFreq;
    Wh = (2.0 * fip->centreFreq + fip->tbWidth) / fip->sampFreq;
    for(n = 1; n <= M; n++) {
      x = (double)n * PI;
      wf[M-n] *= ((sin(x * Wl) - sin(x * Wh)) / x);
      wf[M+n] = wf[M-n];
    }
    wf[M] *= (1.0 - (Wh - Wl));
    break;
  default:
    setAsspMsg(AEG_ERR_BUG, "designFIR: unknown filter type");
    freeWF(wf);
    return(-1);
  }
  fir->numCoeffs = (size_t)wfLen;
  fir->c = wf;
  return(0);
}

/*DOC

Designs a cascaded 2nd-order IIR filter whose parameters are given in 
the structure pointed to by "fip", using the classical analogue design 
method with Butterworth characteristics. Filter length and coefficients 
are stored in the IIR2 structure included in the one pointed to by "fip".
The design method is derived from:
Bauer, Ralf (1987) "Frequenzen wegrechnen," c`t 1987, Heft 12, pp. 92-100.

DOC*/

int designIIR2(FILTER *fip)
{
  int     fType;
  size_t  m, M;
  double  Fc, Q, W, W2, fac, B, C;
  double *a0, *a1, *a2;
  double *b1, *b2;
  double *z1, *z2;
  IIR2   *iir;

  if(fip == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "designIIR2");
    return(-1);
  }
  if((fip->type & FILT_MASK_S) != FILTER_IIR2) {
    setAsspMsg(AEB_BAD_CALL, "designIIR2");
    return(-1);
  }
  iir = &(fip->data.iir2);
  M = iir->numSections;
  if(M < 1){
    setAsspMsg(AEB_BAD_CALL, "designIIR2");
    return(-1);
  }
/*
 * clear pointers to coefficients and taps
 */
  iir->a0 = iir->a1 = iir->a2 = NULL;
  iir->b1 = iir->b2 = NULL;
  iir->z1 = iir->z2 = NULL;
/*
 * allocate memory for coefficients and taps and copy pointers
 */
  fType = fip->type & FILT_MASK_C;
  if(fType == FILTER_BP || fType == FILTER_BS)
    M *= 2; /* used serially or in parallel resp. */
  a0 = iir->a0 = (double *)calloc(M, sizeof(double));
  a1 = iir->a1 = (double *)calloc(M, sizeof(double));
  a2 = iir->a2 = (double *)calloc(M, sizeof(double));
  b1 = iir->b1 = (double *)calloc(M, sizeof(double));
  b2 = iir->b2 = (double *)calloc(M, sizeof(double));
  z1 = iir->z1 = (double *)calloc(M, sizeof(double));
  z2 = iir->z2 = (double *)calloc(M, sizeof(double));
  if(a0 == NULL || a1 == NULL || a2 == NULL ||
     b1 == NULL || b2 == NULL || z1 == NULL || z2 == NULL) {
    freeFilter(fip);
    setAsspMsg(AEG_ERR_MEM, "designIIR2");
    return(-1);
  }
  if(fType == FILTER_BP || fType == FILTER_BS)
    M /= 2; /* reset */

  switch(fType) {
  case FILTER_HP:
  case FILTER_BP:
  case FILTER_BS:
    Fc = fip->hpCutOff;
    W = tan(PI * Fc / fip->sampFreq);      /* warp cut-off frequency */
    W2 = W * W;
    fac = PI / (double)(4 * M);
    for(m = 0; m < M; m++) {
      B = 2.0 * cos(fac * (2*m + 1));     /* Butterworth coefficient */
      C = 1.0 / (1.0 + B*W + W2);                /* common factor in */
      a0[m] = C;                            /* bi-linear z-transform */
      a1[m] = -2.0 * C;
      a2[m] = C;
      b1[m] = C * 2.0 * (W2 - 1.0);
      b2[m] = C * (1.0 - B*W + W2);
    }
    if(fType == FILTER_HP)
      break;
    a0 = &a0[M]; /* shift pointers for low-pass part */
    a1 = &a1[M];
    a2 = &a2[M];
    b1 = &b1[M];
    b2 = &b2[M];
  case FILTER_LP: /* about equals HP mirrored around Fs/4 ! */
    Fc = fip->lpCutOff;
    W = 1.0 / tan(PI * Fc / fip->sampFreq);
    W2 = W * W;
    fac = PI / (double)(4 * M);
    for(m = 0; m < M; m++) {
      B = 2.0 * cos(fac * (2*m + 1));
      C = 1.0 / (1.0 + B*W + W2);
      a0[m] = C;
      a1[m] = 2.0 * C;
      a2[m] = C;
      b1[m] = C * 2.0 * (1.0 - W2);
      b2[m] = C * (1.0 - B*W + W2);
    }
    break;
  case FILTER_RSN: /* resonance filter */
    /* Fc = (fip->hpCutOff + fip->lpCutOff) / 2.0; */   /* centre frequency */
    /* Q = Fc / (fip->lpCutOff - fip->hpCutOff); */       /* quality factor */
    Fc = fip->centreFreq;
    Q = fip->quality;
    W = 1.0 / tan(PI * Fc / fip->sampFreq);
    W2 = W * W;
    fac = PI / (double)(4 * M);
    for(m = 0; m < M; m++) {
      B = 2.0 * cos(fac * (2*m + 1)) / Q;
      C = 1.0 / (1.0 + B*W + W2);
      a0[m] = C * W / Q;
      a1[m] = 0.0;
      a2[m] = C * -W / Q;;
      b1[m] = C * 2.0 * (1.0 - W2);
      b2[m] = C * (1.0 - B*W + W2);
    }
    a0[0] *= sqrt(2.0); /* undo -3 dB peak */
    a2[0] *= sqrt(2.0);
    break;
  case FILTER_ARN: /* anti-resonance filter */
    /* Fc = (fip->lpCutOff + fip->hpCutOff) / 2.0; */
    /* Q = Fc / (fip->hpCutOff - fip->lpCutOff); */
    Fc = fip->centreFreq;
    Q = -(fip->quality); /* negative value to indicate anti-resonance */
    W = 1.0 / tan(PI * Fc / fip->sampFreq);
    W2 = W * W;
    fac = PI / (double)(4 * M);
    for(m = 0; m < M; m++) {
      B = 2.0 * cos(fac * (2*m + 1)) / Q;
      C = 1.0 / (1.0 + B*W + W2);
      a0[m] = C * (1.0 + W2);
      a1[m] = C * 2.0 * (1.0 - W2);
      a2[m] = C * (1.0 + W2);
      b1[m] = C * 2.0 * (1.0 - W2);
      b2[m] = C * (1.0 - B*W + W2);
    }
    break;
  default:
    setAsspMsg(AEG_ERR_BUG, "designIIR2: unknown filter type");
    freeFilter(fip);
    return(-1);
  }
  return(0);
}

/*DOC

This function clears the taps (sets the 'z'-items to zero) in the FILTER 
structure pointed to by "fip".
It returns no value.

DOC*/

void clearTaps(FILTER *fip)
{
  register size_t  m, M;
  register double *z;
  register IIR    *iir;
  register IIR2   *iir2;
  int fType;

  fType = fip->type & FILT_MASK_C;
  switch(fip->type & FILT_MASK_S) {
    case FILTER_IIR:
      iir = &(fip->data.iir);
      M = MAX(iir->numZeros, iir->numPoles);
      if((z=iir->z) != NULL) {
        for(m = 0; m < M; m++)
          *(z++) = 0.0;
      }
      break;
    case FILTER_IIR2:
      iir2 = &(fip->data.iir2);
      M = iir2->numSections;
      if(fType == FILTER_BP || fType == FILTER_BS)
	M *= 2; /* used serially or in parallel resp. */
      if((z=iir2->z1) != NULL) {
        for(m = 0; m < M; m++)
          *(z++) = 0.0;
      }
      if((z=iir2->z2) != NULL) {
        for(m = 0; m < M; m++)
          *(z++) = 0.0;
      }
      break;
    default: /* only IIR filters have taps */
      break;
  }
  return;
}

/*DOC

This function returns the memory allocated for the filter coefficients 
and taps by the functions "designFIR()" and "designIIR2()" resp. It 
DOES NOT return the memory you may have allocated for the 'FILTER' 
structure itself.
This function returns no value.

DOC*/

void freeFilter(FILTER *fip)
{
  FIR  *fir;
  IIR  *iir;
  IIR2 *iir2;

  switch(fip->type & FILT_MASK_S) {
  case FILTER_FIR:
    fir = &(fip->data.fir);
    if(fir->c != NULL) {
      free((void *)(fir->c));
      fir->c = NULL;
    }
    break;
  case FILTER_IIR:
    iir = &(fip->data.iir);
    if(iir->a != NULL) {
      free((void *)(iir->a));
      iir->a = NULL;
    }
    if(iir->b != NULL) {
      free((void *)(iir->b));
      iir->b = NULL;
    }
    if(iir->z != NULL) {
      free((void *)(iir->z));
      iir->z = NULL;
    }
    break;
  case FILTER_IIR2:
    iir2 = &(fip->data.iir2);
    if(iir2->a0 != NULL) {
      free((void *)(iir2->a0));
      iir2->a0 = NULL;
    }
    if(iir2->a1 != NULL) {
      free((void *)(iir2->a1));
      iir2->a1 = NULL;
    }
    if(iir2->a2 != NULL) {
      free((void *)(iir2->a2));
      iir2->a2 = NULL;
    }
    if(iir2->b1 != NULL) {
      free((void *)(iir2->b1));
      iir2->b1 = NULL;
    }
    if(iir2->b2 != NULL) {
      free((void *)(iir2->b2));
      iir2->b2 = NULL;
    }
    if(iir2->z1 != NULL) {
      free((void *)(iir2->z1));
      iir2->z1 = NULL;
    }
    if(iir2->z2 != NULL) {
      free((void *)(iir2->z2));
      iir2->z2 = NULL;
    }
    break;
  default:
    break;
  }
  return;
}

/*DOC

Runs the FIR filter specified in the structure pointed to by "fip" over 
an array of samples starting from the one pointed to by "firstSample". 
The filter may have odd or even length but is assumed to be symmetrical.
The function returns the filtered centre sample.

DOC*/

double FIRfilter(FILTER *fip, double *firstSample)
{
  register size_t  i, length, delay;
  register double *sl, *sr, *c;
  double val;
  FIR   *fPtr;

  fPtr = &(fip->data.fir);
  length = fPtr->numCoeffs;
  delay = length / 2;
  sl = firstSample;
  sr = &sl[length - 1];
  c = fPtr->c;
  /* NOTE: smallest coefficients first */
  for(val = 0.0, i = 0; i < delay; i++)
    val += (*(c++) * (*(sl++) + *(sr--)));
  if(sl == sr)
     val += (*c * (*sl));
  return(val);
}

/*DOC

Runs sample "sample" through the IIR filter specified in the structure 
pointed to by "fip". Returns the filtered sample.

DOC*/

double IIRfilter(FILTER *fip, double sample)
{
  register size_t M, n, nz, np;
  register double *a, *b, *z;
  double y;
  IIR   *iPtr;

  iPtr = &(fip->data.iir);
  np = iPtr->numPoles;
  nz = iPtr->numZeros;
  a = iPtr->a;
  b = iPtr->b;
  z = iPtr->z;
  y = sample;
  for(z[0] = y, n = 1; n < np; n++)
    z[0] -= (b[n] * z[n]);
  for(y = 0.0, n = 0; n < nz; n++)
    y += (a[n] * z[n]);
  M = MAX(nz, np); /* shift taps */
  for(n = M-1; n > 0; n--)
    z[n] = z[n-1];
  return(y);
}

/*DOC

Runs sample "sample" through the cascaded IIR2 filter specified in the 
structure pointed to by "fip". Returns the filtered sample. 

DOC*/

double IIR2filter(FILTER *fip, double sample)
{
  register size_t m, M;
  register double *a0, *a1, *a2, *b1, *b2, *z1, *z2;
  int    fType;
  double z0, y, y2;
  IIR2  *iPtr;

  iPtr = &(fip->data.iir2);
  M = iPtr->numSections;
  fType = fip->type & FILT_MASK_C;
  if(fType == FILTER_BP)
    M *= 2; /* cascaded */
  a0 = iPtr->a0; a1 = iPtr->a1; a2 = iPtr->a2;
  b1 = iPtr->b1; b2 = iPtr->b2;
  z1 = iPtr->z1; z2 = iPtr->z2;
  y = sample;
  for(m = 0; m < M; m++) {
    z0 = y - b1[m]*z1[m] - b2[m]*z2[m];
    y = a0[m]*z0 + a1[m]*z1[m] + a2[m]*z2[m];
    z2[m] = z1[m];
    z1[m] = z0;
  }
  if(fType == FILTER_BS) { /* low-pass in parallel */
    a0 = &a0[M]; /* shift pointers */
    a1 = &a1[M];
    a2 = &a2[M];
    b1 = &b1[M];
    b2 = &b2[M];
    z1 = &z1[M];
    z2 = &z2[M];
    y2 = sample;
    for(m = 0; m < M; m++) {
      z0 = y2 - b1[m]*z1[m] - b2[m]*z2[m];
      y2 = a0[m]*z0 + a1[m]*z1[m] + a2[m]*z2[m];
      z2[m] = z1[m];
      z1[m] = z0;
    }
    y += y2;
  }
  return(y);
}

/***********************************************************************
* calculate coefficients of band-pass (pilot tone detection) filter    *
*   window design (4-Term Blackman-Harris) FIR, resonance at 19.2 kHz, *
*   -3 dB bandwitdh 430 Hz, > 92 dB stop-band attenuation              *
***********************************************************************/
/* void designBPF(void) */
/* { */
/*   size_t m; */
/*   double arg, nrg, piFac, fac; */
  
/*   piFac = TWO_PI / (double)(BPDELAY+BPDELAY+1); */
/*   arg = TWO_PI * BP_CF / (double)(SFR * dsRate);     */ /* 48 kHz data ! */
/*   bpCoef[0] = 1.0; */
/*   for(nrg = 0.0, m = 1; m <= BPDELAY; m++) { */
/*     bpCoef[m] = cos(arg*(double)m);             */ /* rectangular window */
/*     nrg += (bpCoef[m] * bpCoef[m]);                         */ /* energy */
/*   } */
/*   nrg += nrg;                                      */ /* *2 for symmetry */
/*   nrg += 1.0;                                          */ /* add 0-value */
/*   fac = 1.0 / (nrg * 0.35875);          */ /* correction factor for 0 dB */
/*   for(m = 0; m <= BPDELAY; m++) {    */ /* multiply with window function */
/*     bpCoef[m] *= (fac * (0.35875 + 0.48829*cos(piFac*m)\ */
/*              + 0.14128*cos(piFac*((double)(m+m)))\ */
/*              + 0.01168*cos(piFac*((double)(m+m+m)))) ); */
/*   } */
/*   return; */
/* } */

/* General formula of IIR filter: */

/*                    -1        -2              -n */
/*         a0 + a1 x z  + a2 x z  + ... + an x z   */
/* H(z) = ---------------------------------------- */
/*                    -1        -2              -m */
/*          1 - b1 x z  - b2 x z  + ... - bm x z   */

/* To invert: numerator and denominator should be exchanged and then  */
/*            be brought in the same form i.e., new b0 should become 1 */
/*   Thus:    divide all coefficients by -a0, e.g. */
/*            b1-new = -a1/a0 and a1-new =  -b1/a0 */

