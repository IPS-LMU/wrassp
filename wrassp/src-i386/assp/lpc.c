/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1988 - 2010  Michel Scheffers                          *
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
* File:     lpc.c                                                      *
* Contents: Functions related to Linear Prediction.                    *
* Author:   Michel T.M. Scheffers                                      *
* See:                                                                 *
*  Makhoul (1975), "Linear prediction: A tutorial review,"             *
*     Proc. IEEE, 63, pp. 561-580.                                     *
*  Markel & Gray (1976), 'Linear prediction of speech'                 *
*     (Springer,Berlin/Heidelberg/New York), pp. 219, 231-233.         *
*  IEEE ASSP Society (1979), "Programs for digital signal processing"  *
*     (IEEE Press, New York), Section 4.3.                             *
*  Delsarte, P. and Genin, Y.V. (1986), "The Split Levinson Algorithm,"*
*     IEEE Trans. ASSP, Vol. 34, pp. 470-478                           *
*  Willems, L.F. (1987), "Robust formant analysis for speech synthesis *
*     applications," Proc. European Conference on Speech Technology,   *
*     Vol. 1  pp. 250-253                                              *
*                                                                      *
***********************************************************************/
/* $Id: lpc.c,v 1.14 2012/03/19 16:16:38 lasselasse Exp $ */

#ifndef _LPC_C
#define _LPC_C

#include <stdio.h>    /* NULL */
#include <float.h>    /* DBL_EPSILON */
#include <stdlib.h>   /* abs() */
#include <math.h>     /* fabs() log() exp() */

#include <miscdefs.h> /* PI TWO_PI */
#include <asspdsp.h>  /* TINYLIN MAXLPORDER MAXFORMANTS */
#include <asspmess.h> /* message handler */

/*DOC

Function 'durbin'

This function implements the Durbin recursion for deriving LP 
coefficients from the autocorrelation function. Both the direct 
filter parameters and the reflection coefficients are calculated. 
The latter may be suppressed by passing a NULL pointer.

Arguments
 acf[M+1] array with autocorrelation function  (acf[i] = R(i), i= 0..M)
 lpc[M+1] array for LPC A-parameters           (lpc[i] = A(i), i= 0..M)
 rfc[M]   array for reflection coefficients (may be a NULL-pointer)
                                               (rfc[i-1] = K(i), i= 1..M)
 sqerr    pointer to squared error signal
 M        prediction order

Returns
  0 if no problems
 -1 if squared error less than or equal to zero

DOC*/

int asspDurbin(register double *acf, register double *lpc, double *rfc,\
	   register double *sqerr, register int M)
{
  register int m, i, j;
  double sum, save;
  
  if(acf[0] <= 0.0) {               /* standard solution if no signal */
    lpc[0] = 1.0;
    for(m = 1; m <= M; m++)
      lpc[m] = 0.0;
    if(rfc != NULL) {
      for(m = 0; m < M; m++)
	rfc[m] = 0.0;
    }
    *sqerr = 0.0;
    return(0);
  }
  
  lpc[0] = 1.0;                                         /* initialize */
  lpc[1] = -acf[1] / acf[0];
  if(rfc != NULL)
    rfc[0] = lpc[1];
  *sqerr = acf[0] + lpc[1]*acf[1];
  for(m = 2; m <= M; m++) {                    /* loop over lpc-order */
    if((*sqerr) < 0.0) {                            /* rounding error */
      lpc[0] = 1.0;                         /* set to standard values */
      for(m = 1; m <= M; m++)
	lpc[m] = 0.0;
      if(rfc != NULL) {
	for(m = 0; m < M; m++)
	  rfc[m] = 0.0;
      }
      *sqerr = 0.0;
      setAsspMsg(AWG_ERR_ROUND, "in asspDurbin()");
      return(-1);
    }
    for(sum = acf[m], i = 1, j = m-1; i < m; i++, j--)
      sum += (lpc[i] * acf[j]);
    sum = -sum / (*sqerr);
    for(i = 1, j = m-1; i < j; i++, j--) {               /* recursion */
      save    = lpc[j];
      lpc[j] += (sum * lpc[i]);
      lpc[i] += (sum * save);
    }
    if(i == j)
      lpc[i] += (sum * lpc[i]);
    lpc[m] = sum;
    if(rfc != NULL)
      rfc[m-1] = sum;
    *sqerr *= (1.0 - sum*sum);
  }
  return(0);
}

/*DOC

Function 'arf2rfc'

Converts area function to reflection coefficients.

Arguments
 arf[M+1]  array with area function           (arf[i] = ARF(i), i = 0 .. M)
 rfc[M]    array for reflection coefficients  (rfc[i-1] = K(i), i = 1 .. M)
 M         prediction order

Returns
  0 if no problems
 -1 if ARF[i]/ARF[i-1] == -1

DOC*/

int arf2rfc(register double *arf, register double *rfc, register int M)
{
  register int i, j;
  
  for(i = 0, j = 1; i < M; i++,j++) {
    if(arf[i] == -arf[j])
      return(-1);                           /* avoid division by zero */
    if(arf[i] == arf[j])
      rfc[i] = 0.0;
    else
      rfc[i] = (arf[i] - arf[j]) / (arf[i] + arf[j]);
  }
  return(0);
}

/*DOC

Function 'lar2rfc'

Converts log area ratios to reflection coefficients.

Arguments
 lar[M]  array with log area ratios         (lar[i-1] = LAR(i), i = 1 .. M)
 rfc[M]  array for reflection coefficients  (rfc[i-1] = K(i), i = 1 .. M)
 M       prediction order

Returns
  0 (no problems possible)

DOC*/

int lar2rfc(register double *lar, register double *rfc, register int M)
{
  register int i;
  double fac;
  
  for(i = 0; i < M; i++) {
    fac = exp(lar[i]);
    rfc[i] = (1.0 - fac) / (1.0 + fac);
  }
  return(0);
}

/*DOC

Function 'lpc2cep'

Converts Linear Prediction filter coefficients to cepstral coefficients.

Arguments
 lpc[M+1]  array with LP filter coefficients (lpc[i] = A(i), i = 0 .. M)
 sqerr     squared error signal
 cep[M+1]  array for cepstral coefficients   (cep[i] = CEP(i), i= 0 .. M)
 M         prediction order

Returns
  0 if no problems
 -1 if squared error less than or equal to zero

DOC*/

int lpc2cep(register double *lpc, double sqerr, register double *cep,\
	    register int M)
{
  register int i, j, k;
  double sum;

  if(sqerr <= 0.0)
    return(-1);                                     /* can't take log */
  cep[0] = log(sqerr);
  cep[1] = -lpc[1];
  for(i = 2; i <= M; i++) {
    sum = lpc[i] * (double)(i);
    for(j = 1, k = i-1; j < i; j++, k--)
      sum += (lpc[j] * cep[k] * (double)(k));
    cep[i] = -sum / (double)(i);
  }
  return(0);
}

/*DOC

Function 'lpc2rfc'

Converts Linear Prediction filter coefficients to reflection coefficients.

Arguments
 lpc[M+1]  array with LP filter coefficients  (lpc[i] = A(i), i = 0 .. M)
 rfc[M]    array for reflection coefficients  (rfc[i-1] = K(i), i = 1 .. M)
 M         prediction order

Returns
  0 if no problems
 -1 if | K(i) | >= 1 (unstable filter)

DOC*/

int lpc2rfc(register double *lpc, register double *rfc, register int M)
{
  register int i, j, k, l;
  double fac1, fac2, save;
   
  for(i = 0; i < M; i++)                                /* initialize */
    rfc[i] = lpc[i+1];
  for(i = M-1; i > 0; i--) {
    j = (i+1) >> 1;                                      /* int div 2 */
    fac1 = rfc[i];
    if(fabs(fac1) >= 1.0)                          /* unstable filter */
      return(-1);
    fac2 = 1.0 - fac1 * fac1;
    for(k = 0, l = i-1; k < j; k++, l--) {               /* recursion */
      save   = (rfc[k] - fac1*rfc[l]) / fac2;
      rfc[l] = (rfc[l] - fac1*rfc[k]) / fac2;
      rfc[k] = save;
    }
  }
  return(0);
}

/*DOC

Function 'rfc2arf'

Converts reflection coefficients to area function.
(arf[0] corresponds to the area at the lips, 
 arf[M] to that at the glottis (normalized to 1)).

Arguments
 rfc[M]    array with reflection coefficients  (rfc[i-1] = K(i), i = 1 .. M)
 arf[M+1]  array for area function             (arf[i] = ARF(i), i = 0 .. M)
 M         prediction order

Returns
  0 if no problems
 -1 if | K(i) | >= 1 (unstable filter)

DOC*/

int rfc2arf(register double *rfc, register double *arf, register int M)
{
  register int i, j;
  
  arf[M] = 1;                                           /* normalized */
  for(i = M, j = M-1; i > 0; i--,j--) {
    if(fabs(rfc[j]) >= 1.0)                        /* unstable filter */
      return(-1);
    arf[j] = arf[i] * (1.0 + rfc[j]) / (1.0 - rfc[j]);
  }
  return(0);
}

/*DOC

Function 'rfc2lar'

Converts reflection coefficients to log area ratios.

Arguments
 rfc[M]  array with reflection coefficients  (rfc[i-1] = K(i), i = 1 .. M)
 lar[M]  array for log area ratios           (lar[i-1] = LAR(i), i = 1 .. M)
 M       prediction order

Returns
  0 if no problems
 -1 if | K(i) | >= 1 (unstable filter)

DOC*/

int rfc2lar(register double *rfc, register double *lar, register int M)
{
  register int i;
  
  for(i = M; i > 0; ) {
    --i;                                              /* predecrement */
    if(fabs(rfc[i]) >= 1.0)                        /* unstable filter */
      return(-1);
    lar[i] = log((1.0 - rfc[i]) / (1.0 + rfc[i]));
  }
  return(0);
}

/*DOC

Function 'rfc2lpc'

Converts reflection coefficients to LP filter coefficients.

Arguments
 rfc[M]    array with reflection coefficients  (rfc[i-1] = K(i), i = 1 .. M)
 lpc[M+1]  array for LP filter coefficients    (lpc[i] = A(i), i = 0 .. M)
 M         prediction order

Returns
  0 if no problems
 -1 if invalid parameters or | K(i) | >= 1 (unstable filter)

DOC*/

int rfc2lpc(register double *rfc, register double *lpc, register int M)
{
  register int i, j, k, m;
  double save, keep;

  if(rfc == NULL|| lpc == NULL || M < 1)        /* invalid parameters */
    return(-1);
  for(m = 0; m < M; m++) {
    if(fabs(rfc[m]) >= 1.0)
      return(-1);
  }
  lpc[0] = 1.0;                                     /* per definition */
  lpc[1] = rfc[0];
  for(m = 2; m <= M; m++) {
    i = m / 2;
    save = rfc[m-1];
    for(j = 1, k = m-1; j <= i; j++, k--) {
      keep = lpc[j] + save * lpc[k];
      lpc[k] += (save * lpc[j]);
      lpc[j] = keep;
    }
    lpc[m] = save;
  }
  return(0);
}

/*DOC

Function 'lpc2pqp'

Converts LP filter coefficients to PQ parameter pairs (coefficients of 
2nd-order resonance filters) using Bairstow's method for root-solving.

Arguments
  lpc[M+1]  array with LP filter coefficients (lpc[i] = A(i), i = 0 .. M)
  pqp[M]    array with PQ coefficient pairs
            (Pn = pqp[2*i] Qn = pqp[2*i + 1], i = 0 .. M/2-1)
            input starting, output final estimates of roots
  M         LP analysis order
  tPtr      pointer to 'BAIRSTOW' structure with termination criteria

Returns:
  Number of iterations for root-solving if no problems
  -1 if error in function arguments

Note:
 -   If the returned value exceeds the maximum number of iterations
     defined in "tPtr", the PQ parameters will be invalid.
 -   The input values of the PQ parameters should already be close to 
     the expected roots. See e.g. 'fmt.c' for ways to obtain these, 
     also for choosing the termination criteria.

DOC*/

int lpc2pqp(double *lpc, double *pqp, int M, BAIRSTOW *tPtr)
{
  register int i, j, m, NF;
  int    iter, RETRY;
  double c[MAXLPORDER+1], r[MAXLPORDER-1], t[MAXLPORDER-1];
  double p[MAXFORMANTS], q[MAXFORMANTS];

  if(M > MAXLPORDER)
    return(-1);  
  NF = M / 2;
  for(i = 0; i <= M; i++)         /* copy coefficients to local array */
    c[i] = lpc[i];
  for(i = j = 0; i < NF; i++) {   /* copy start values to local array */
    p[i] = pqp[j++];
    q[i] = pqp[j++];
  }
/*
 * solve roots
 */
  m = M;
  i = iter = 0;
  RETRY = FALSE;
  while(m >= 2) {
    if((j=bairstow(c, &p[i], &q[i], r, m, t, tPtr)) > tPtr->maxIter) {
      if(RETRY || m < 4)    /* only one retry and exception for m = 3 */
	return(-1);       /* no convergence error can occur for m = 2 */
      RETRY = TRUE;
      p[i] = p[i+1];
      q[i] = q[i+1];
      iter += j;
    }
    else {
      RETRY = FALSE;
      iter += j;
      i++;
      m -= 2;                             /* order of rest polynomial */
      for(j = 0; j <= m; j++)                 /* copy rest polynomial */
        c[j] = r[j];
    }
  }
  for(j = i = 0; i < NF; i++) {              /* copy to output buffer */
    pqp[j++] = p[i];
    pqp[j++] = q[i];
  }
  return(iter);
}

/*DOC

Function 'ffb2pqp'

Converts formant frequency and bandwidth pairs to PQ parameter pairs 
(coefficients of 2nd-order resonance filter).

Arguments
 ffb[2*N]  array with formant frequencies and bandwidth pairs
           (Fn = ffb[2*i] Bn = ffb[2*i + 1], n = 1 .. N, i = 0 .. N-1)
 pqp[2*N]  array for PQ coefficient pairs
           (Pn = pqp[2*i] Qn = pqp[2*i + 1], n = 1 .. N, i = 0 .. N-1)
 N         number of pairs
 sampFreq  sampling frequency in Hz

Returns
  0 (no problems possible)

DOC*/

int ffb2pqp(register double *ffb, register double *pqp, register int N,\
	    double sampFreq)
{
  register int i, M;
  double PiT, twoPiT;

  M = 2 * N;
  PiT = PI / sampFreq;
  twoPiT = TWO_PI / sampFreq;
  for(i = 0; i < M; i += 2) {
    *(pqp++) = -2.0 * exp(-PiT * ffb[i+1]) * cos(twoPiT * ffb[i]);
    *(pqp++) = exp(-twoPiT * ffb[i+1]);
  }
  return(0);
}

/*DOC

Function 'pqp2rfb'

Converts PQ parameter pairs (coefficients of a second-order filters) to 
resonance frequency and bandwidth pairs and sorts these on increasing 
frequency. Only PQ pairs having complex conjugate roots are converted, 
any remaining FB pairs are set to zero.

Arguments
 pqp[2*N]  array with PQ coefficient pairs
           (Pn = pqp[2*i] Qn = pqp[2*i + 1], n = 1 .. N, i = 0 .. N-1)
 rfb[2*N]  array for resonance frequency and bandwidth pairs
           (Fn = rfb[2*i] Bn = rfb[2*i + 1], n = 1 .. N, i = 0 .. N-1)
 N         number of PQ pairs
 sampFreq  sampling frequency in Hz

Returns
  number of resonances.

DOC*/

int pqp2rfb(double *pqp, double *rfb, int N, double sampFreq)
{
  register int i, j, n, m, M;
  double fac, P, Q, F, B;

  M = 2 * N;
  fac = sampFreq / TWO_PI;
  for(n = m = 0; m < M; m += 2) {
    P = *(pqp++);
    Q = *(pqp++);
    if(hasCCR(P, Q)) {
      F = fac * acos(-P / (2.0 * sqrt(Q)));                /* convert */
      B = -fac * log(Q);
      for(i = 0; i < n; i +=2) {                 /* sort on frequency */
	if(F < rfb[i]) {
	  for(j = n-1; j >= i; --j)            /* shift to make place */
	    rfb[j+2] = rfb[j];
	  break;                                 /* leave for(i) loop */
	}
      }
      rfb[i++] = F;                                          /* store */
      rfb[i] = B;
      n += 2;
    }                         /* else real root; DON'T increase n !!! */
  }
  N = n / 2;                            /* number of valid resonances */
  while(n < M)
    rfb[n++] = 0.0;                                 /* zero remainder */
  return(N);
}

/*DOC

Function 'lpSLA'

Linear Prediction analysis using the Split-Levinson-Algorithm (optional 
analysis of Pisarenko frequencies).

Arguments:
 atc[M+1]  array with autocorrelation coefficients  (R(i), i = 0..M)
 lpc[M+1]  array for LP filter coefficients         (A(i), i = 0..M)
 normPtr   pointer to prediction error norm (squared)
 M         order of LP analysis
 pf[M/2]   array for Pisarenko frequencies (may be NULL)
 sampFreq  sampling frequency in Hz

Returns:
  -1 if rounding error in LP recursion or failure in seekSLAzx()
  else total number of iterations for finding Pisarenko frequencies

DOC*/

/* local global array */
LOCAL double slaTable[MAXFORMANTS+2][MAXFORMANTS+2];
/* prototypes of support functions */
LOCAL void setSLAtable(int nFreqs);
LOCAL int  findSLAzeros(double func[], int order, double zero[],\
			double eps);
LOCAL int  seekSLAzx(double func[], int degree, double xLo, double xHi,\
		     double *ZX, double eps);
LOCAL double slaFuncValue(double coeffs[], int degree, double arg);

int lpSLA(double *atc, double *lpc, double *normPtr, int order,\
	  double *pf, double sampFreq)
{
  static int oldM=0;
  static int nFreqs=0;
  static double oldSFR=0.0;
  static double twoPiT, eps;
  int    i, k, t, n, totIter;
  double tau, tauk, alfak, lambdak;
  double npk[MAXLPORDER+2], pk[MAXLPORDER+2], ppk[MAXLPORDER+2];
  double C_values[MAXFORMANTS+3];

  if(order <= 0 || sampFreq <= 0.0) {               /* to allow reset */
    oldSFR = 0.0;
    oldM = 0;
    return(0);
  }
  if(order != oldM || sampFreq != oldSFR) {          /* re-initialize */
    if(pf != NULL) {
      nFreqs = order / 2;
      setSLAtable(nFreqs);
      twoPiT = TWO_PI / sampFreq;
      /* from pf[i] = acos(zero[i]) / (2 * Pi / Fs) it follows: */
      /* zero[i] = cos(2 * Pi * pf[i] / Fs) */
      /* thus: dzero / dpf = 2 * Pi / Fs * sin(2 * Pi * pf / Fs) */
      eps = twoPiT * sin(twoPiT * 50.0);    /* < 1 Hz for f > 50 Hz */
      /* if(eps > 0.5E-12) */
      /* eps = 0.5E-12; */
    }
    oldM = order;
    oldSFR = sampFreq;
  }

  if(atc[0] <= 0.0) {               /* standard solution if no signal */
    lpc[0] = 1.0;
    for(i = 1; i <= order; i++)
      lpc[i] = 0.0;
    if(pf != NULL) {
      for(i = 0; i < nFreqs; i++)
	pf[i] = 0.0;
    }
    *normPtr = 0.0;
    return(0);
  }

  totIter = 0;
  ppk[0] = pk[0] = pk[1] = 1.0;/* NOTE: differs from Delsarte & Genin */
  tauk = atc[0] / 2.0;
  lambdak = 2.0;
  for(k = 1; k <= order; k++) {
    t = (k+1) / 2;
    tau  = tauk;
    tauk = atc[0] + atc[k];
    for(i = 1; i < t; i++)
      tauk += ((atc[i] + atc[k-i]) * pk[i]);
    if(EVEN(k))
      tauk += (atc[t] * pk[t]);
    if(tau == 0.0 || lambdak == 0.0) {
      lpc[0] = 1.0;
      for(i = 1; i <= order; i++)
	lpc[i] = 0.0;
      if(pf != NULL) {
	for(i = 0; i < nFreqs; i++)
	  pf[i] = 0.0;
      }
      *normPtr = 0.0;
      setAsspMsg(AWG_ERR_ROUND, "in lpSLA()");
      return(-1);
    }
    alfak = tauk / tau;
    lambdak = 2.0 - alfak / lambdak;
    npk[0] = 1.0;                      /* make next pk with equation: */
    for(i = t; i > 0; i--) {         /* (1+z) * pk - alfak * z * ppk  */
      npk[i] = pk[i] + pk[i-1] - alfak * ppk[i-1];
      ppk[i] = pk[i];                     /* shift here because npk[] */
      pk[i] = npk[i];                   /*  changed in findSLAzeros() */
    }
    if(EVEN(k))
      pk[t+1] = npk[t+1] = npk[t];               /* symmetry property */
    if(pf != NULL) {
/*    if(k == 2 || (k > 2 && k < order && EVEN(order) && ODD(k))) ) { */
/* searching only for odd orders works in general but not always !!!  */ 
      if(k > 1 && k < order) {
        /* ^        ^  because npk[] one order higher */
	n = findSLAzeros(npk, k, C_values, eps);
	if(n < 0)                                    /* search failed */
	  return(-1);
	totIter += n;
      }
    }
  }
  if(normPtr != NULL) {
    *normPtr = lambdak * tauk;
    if(*normPtr < 0.0) {
      lpc[0] = 1.0;
      for(i = 1; i <= order; i++)
	lpc[i] = 0.0;
      if(pf != NULL) {
	for(i = 0; i < nFreqs; i++)
	  pf[i] = 0.0;
      }
      *normPtr = 0.0;
      setAsspMsg(AWG_ERR_ROUND, "in lpSLA()");
      return(-1);
    }
  }
  if(lpc != NULL) {
    t = (order+1) / 2;
    for(i = 0; i < t; i++) {
      pk[order+1-i] = pk[i];                     /* symmetry property */
      ppk[order-i] = ppk[i];
    }
    lpc[0] = 1.0;
    for(i = 1; i <= order; i++)
      lpc[i] = lpc[i-1] + pk[i] - lambdak * ppk[i-1];
  }
  if(pf != NULL) {
    for(i = 0; i < nFreqs; i++)
      pf[i] = acos(C_values[i+1]) / twoPiT; /* convert to frequencies */
  }
  return(totIter);
}
/***********************************************************************
* initialize the global two-dimensional cosine array slaTable[i,j]     *
* with coeff's from cos(i*x) = SUM(coeff * cos(x)^k) and k = 0...j     *
***********************************************************************/
LOCAL void setSLAtable(int nFreqs)
{
  int i, j;

  for(i = 0; i <= nFreqs; i++) {                       /* clear table */
    for(j = 0; j <= nFreqs; j++)
      slaTable[i][j] = 0.0;
  }
  slaTable[0][0] = 2.0;                       /* preliminary value !! */
  slaTable[1][1] = 2.0;
  for(i = 2; i <= nFreqs; i++) {
    slaTable[i][0] = -slaTable[i-2][0];
    for(j = 1; j <= i; j++)
      slaTable[i][j] = 2.0 * slaTable[i-1][j-1] - slaTable[i-2][j];
  }
  slaTable[0][0] = 1.0;                          /* set correct value */
  return;
}
/***********************************************************************
* find zeros of function; returns summed number of iterations          *
***********************************************************************/
LOCAL int findSLAzeros(double func[], int order, double zero[],\
		       double eps)
{
  int    i, j, degree, sumIter;
  double newZero, px[MAXFORMANTS+3];

  sumIter = 0;
  if(order == 2) {               /* first time a true zero can be set */
    zero[0] = 1.0;
    zero[1] = -(func[1]-1.0) / 2.0;
    zero[2] = -1.0;
  }
  else {
    degree = (order+1) / 2;
    if(EVEN(order)) {        /* divide by (1-z) to remove zero at z=1 */
      for(i = 1; i <= degree; i++)
	func[i] -= func[i-1];
    }
    for(i = 0; i <= degree; i++)            /* substitution via table */
      px[i] = slaTable[degree][i];
    for(j = (degree-1); j >= 0; j--) {
      for(i = 0; i <= degree; i++)
        px[i] += (func[degree-j] * slaTable[j][i]);
    }
    for(i = degree; i > 0; i--) { /* search new zeros between old ones */
      j = seekSLAzx(px, degree, zero[i-1], zero[i], &newZero, eps);
      if(j < 0)
	return(-1);
      zero[i] = newZero;
      sumIter += j;
    }
    zero[degree+1] = -1.0;                         /* set upper bound */
  }
  return(sumIter);
}
/***********************************************************************
* search zero in function between xLo and xHi; return iterations       *
***********************************************************************/
LOCAL int seekSLAzx(double func[], int degree, double xLo, double xHi,\
		    double *ZX, double eps)
{
  int    fixBound, iterations;
  double fvLo, fvHi, fvZX, oldZX;

  fvLo = slaFuncValue(func, degree, xLo);
  fvHi = slaFuncValue(func, degree, xHi);
  if(SGN(fvLo) == SGN(fvHi)) {
    setAsspMsg(AEG_ERR_BUG, "\nseekSLAzx: even number of zero crossings");
    return(-1);
  }
  fixBound = 0;
  iterations = 0;
  *ZX = (xLo + xHi) / 2.0;            /*  start with interval halving */
  do {
    fvZX = slaFuncValue(func, degree, *ZX);
    if(SGN(fvLo) == SGN(fvZX)) {
      xLo = *ZX;
      fvLo = fvZX;
      fixBound++;
    }
    else if(SGN(fvHi) == SGN(fvZX)) {
      xHi = *ZX;
      fvHi = fvZX;
      fixBound--;
    }
    else {
      setAsspMsg(AEG_ERR_BUG, "\nseekSLAzx: even number of zero crossings");
      return(-1);
    }
    oldZX = *ZX;
    if(abs(fixBound) > 4) {                  /*  use interval halving */
      *ZX = (xLo + xHi) / 2.0;
      fixBound = 0;                                    /*  reset flag */
    }
    else {                                        /* use Regula Falsi */
      *ZX = xLo - fvLo * (xHi-xLo)/(fvHi-fvLo);
    }
    iterations++;
    if(fabs(fvZX) <= DBL_EPSILON)          /* secondary end condition */
      break;
  } while(fabs(*ZX - oldZX) > eps);          /* primary end condition */
  return(iterations);
}
/***********************************************************************
* return value of function given by 'coeffs' at 'arg'                  *
***********************************************************************/
LOCAL double slaFuncValue(double coeffs[], int degree, double arg)
{
  int    i;
  double fVal;

  for(fVal = 0.0, i = degree; i >= 0; i--)
    fVal = coeffs[i] + arg * fVal;
  return(fVal);
}

#endif /* _LPC_C */
