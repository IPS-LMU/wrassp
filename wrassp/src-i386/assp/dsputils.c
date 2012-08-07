/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1989 - 2009  Michel Scheffers                          *
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
* File:     dsputils.c                                                 *
* Contents: Miscellaneous signal processing functions.                 *
*           Prototypes of these functions are in asspdsp.h.            *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: dsputils.c,v 1.12 2009/09/23 09:40:40 mtms Exp $ */

#include <stdio.h>     /* NULL */
#include <stdlib.h>    /* labs() */
#include <inttypes.h>  /* int16_t */
#include <math.h>      /* sqrt() fabs() */

#include <asspdsp.h>
#include <assptime.h>  /* PERIODtoFREQ() */

/*DOC

Function `removeDC'

Calculates and subtracts DC (mean) signal value from array.

Arguments:
 s[N]   array with signal values
 o[N]   array for output signal (may be identical to s)
 N      number of signal values

Returns:
 mean signal value
 
DOC*/

double removeDC(double *s, register double *o, register long N)
{
  register long    n;
  register double *in;
  double mean;
  
  mean = 0.0;
  if(s != NULL && o != NULL && N > 0) {
    in = s;
    for(n = 0; n < N; n++)
      mean += (*(in++));
    mean /= (double)N;
    in = s;
    for(n = 0; n < N; n++)
      *(o++) = *(in++) - mean;
  }
  return(mean);
}

/*DOC

Function `preEmphasis'
                                                           -1
Performs in-place pre-emphasis of signal with P(z) = 1 + uz
To emphasize high-frequency components, u should be negative.

Arguments:
 s[N]   array with signal values
 u      pre-emphasis factor (-1 < u < 1)
 tap    initial value of filter tap (un-emphasized s[-1])
 N      number of signal values

Returns:
  0 if OK
 -1 if invalid function argument
 
DOC*/

int preEmphasis(register double *s, double u, double tap, register long N)
{
  register long n;
  double save;
  
  if(s == NULL || N < 0)
    return(-1);
  if(u != 0.0) {
    for(n = 0; n < N; s++, n++) {
      save = *s;
      *s += (u * tap);
      tap = save;
    }
  }
  return(0);
}

/*DOC

Function `emphWinI16'

Convert a signal in 16-bit integer to double precision, emphasize it 
with  P(z) = 1 + uz^-1 (pre-emphasis for u < 0, de-emphasis for u > 0)
and multiply it with a window function. Emphasis may be supressed by 
setting the constant u to 0.0; windowing may be omitted by passing a 
NULL pointer for the coefficients.


Arguments:
 s[N]   array with signal values
 u      emphasis constant
 tap    initial value of filter tap (un-emphasized s[-1])
 wf[N]  array with coefficients of window function
 o[N]   array for output signal
 N      number of signal values

Returns:
  0 if OK
 -1 if invalid function argument
 
DOC*/

int emphWinI16(register int16_t *s, double u, double tap,\
	       register double *wf, register double *o, register long N)
{
  register long n;
  double save;
  
  if(s == NULL || o == NULL || N < 0)
    return(-1);
  if(N > 0) {
    if(u == 0.0) /* window or convert only */
      return(mulWinI16(s, wf, o, N));
    if(wf != NULL) {
      for(n = 0; n < N; n++) {
	save = (double)(*(s++));
	*(o++) = *(wf++) * (save + (u * tap));
	tap = save;
      }
    }
    else { /* emphasize only */
      for(n = 0; n < N; n++) {
	save = (double)(*(s++));
	*(o++) = save + (u * tap);
	tap = save;
      }
    }
  }
  return(0);
}

/*DOC

Function `mulWinI16'

Convert a signal in 16-bit integer to double precision and multiply it 
with a window function. If a NULL pointer is passed for the window 
coefficients this function will only convert the signal values.

Arguments:
 s[N]   array with signal values
 wf[N]  array with coefficients of window function
 o[N]   array for output signal
 N      number of signal values

Returns:
  0 if OK
 -1 if invalid function argument
 
DOC*/

int mulWinI16(register int16_t *s, register double *wf,\
	      register double *o, register long N)
{
  register long n;

  if(s == NULL || o == NULL || N < 0)
    return(-1);
  if(N > 0) {
    if(wf != NULL) {
      for(n = 0; n < N; n++) 
	*(o++) = *(wf++) * (double)(*(s++));
    }
    else { /* convert only */
      for(n = 0; n < N; n++) 
	*(o++) = (double)(*(s++));
    }
  }
  return(0);
}

/*DOC

Function 'getACF'

Calculates autocorrelation coefficients of a windowed signal
 
          n=N-m-1
   r[m] =  SUM (s[n]*s[n+m])    for m = 0 ... M
          n=0

Arguments:
 s[N]   array with signal values
 r[M+1] array for autocorrelation coefficients
 N      number of signal values to correlate
 M      order of the autocorrelation

Returns:
  0 when OK
 -1 when invalid function argument

DOC*/

int getACF(double *s, double *r, long N, register int M)
{
  register int     m;
  register long    n, N_m;
  register double *sPtr, *dPtr;
  
  if(s == NULL || r == NULL || M < 0 || N <= M)
    return(-1);
  for(m = 0, N_m = N; m <= M; m++, N_m--) {
    sPtr = s;
    dPtr = &(s[m]);
    for(*r = 0.0, n = 0; n < N_m; n++, sPtr++, dPtr++) {
      *r += ((*sPtr) * (*dPtr));
    }
    r++;
  }
  return(0);
}

/*DOC

Function 'getMeanACF'

Calculates average autocorrelation coefficients of a windowed signal.
 
           1   n=N-m-1
   r[m] = ---   SUM (s[n]*s[n+m])    for m = 0 ... M
          N-m  n=0

Arguments:
 s[N]   array with signal values
 r[M+1] array for autocorrelation coefficients
 N      number of signal values to correlate
 M      order of the autocorrelation

Returns:
  0 when OK
 -1 when invalid function argument

DOC*/

int getMeanACF(double *s, double *r, long N, register int M)
{
  register int     m;
  register long    n, N_m;
  register double *sPtr, *dPtr;
  
  if(s == NULL || r == NULL || M < 0 || N <= M)
    return(-1);
  for(m = 0, N_m = N; m <= M; m++, N_m--) {
    sPtr = s;
    dPtr = &(s[m]);
    for(*r = 0.0, n = 0; n < N_m; n++, sPtr++, dPtr++) {
      *r += ((*sPtr) * (*dPtr));
    }
    *r /= (double)N_m;
    r++;
  }
  return(0);
}

/*DOC

Function 'getNormACF'

Calculates normalized autocorrelation coefficients of a windowed signal.

   r[0] = 1 (per definition)

            1   n=N-m-1
   r[m] = ----   SUM (s[n]*s[n+m])    for m = 1 ... M
           R0   n=0

             n=N-1
   with R0 =  SUM (s[n]*s[n])
             n=0

Arguments:
 s[N]   array with signal values
 r[M+1] array for autocorrelation coefficients
 N      number of signal values to correlate
 M      order of the autocorrelation

Returns:
 R0 when OK
 -1 when invalid function argument

DOC*/

double getNormACF(register double *s, register double *r,\
		  register long N, register int M)
{
  register int m;
  double R0;

  if(getACF(s, r, N, M) < 0)
    return(-1.0);

  R0 = *r;
  *(r++) = 1.0;
  if(R0 <= 0.0) {                               /* exception handling */
    for(m = 1; m <= M; m++)
      *(r++) = 0.0;
  }
  else {
    for(m = 1; m <= M; m++)
      *(r++) /= R0;
  }
  return(R0);
}

/*DOC

Function 'getCCF'

Calculates cross correlation coefficients of two signals.

         n=N-1
  c[m] =  SUM a[n]*b[n+m]    for m = 0 ... M
         n=0

Arguments:
 a[N]     array with signal values
 b[N+M+1] array with other signal values
 c[M+1]   array for correlation coefficients
 N        number of signal values to correlate
 M        order of the correlation

Returns:
  0 when OK
 -1 when invalid function argument
 
DOC*/

int getCCF(double *a, double *b, register double *c,\
	   register long N, register int M)
{
  register int     m;
  register long    n;
  register double *aPtr, *bPtr;
  
  if(a == NULL || b == NULL || c == NULL || M < 0 || N <= M)
    return(-1);
  for(m = 0; m <= M; m++, b++, c++) {
    aPtr = a;
    bPtr = b;
    for(*c = 0.0, n = 0; n < N; n++, aPtr++, bPtr++) {
      *c += ((*aPtr) * (*bPtr));
    }
  }
  return(0);
}

/*DOC

Function 'getAMDF'

Computes unwindowed Average Magnitude Difference Function.

              n=N-1
c[m-minLag] =  SUM |s[n] - s[n+m]| / N 	for m = minLag .... maxLag
              n=0

input:  double s[N+maxLag]          array with input samples
        long   N                    base length of input array
        int    minLag               shortest lag to be evaluated
        int    maxLag               longest lag to be evaluated

output: double c[maxLag-minLag +1]  array for AMDF coefficients

returns  0 if all OK
        -1 when called with invalid arguments

DOC*/

int getAMDF(double *s, register double *c, register long N,\
	    register int minLag, register int maxLag)
{
  register int     m;
  register long    n;
  register double *sPtr, *mPtr;

   if(s == NULL || c == NULL || N < 1 || minLag < 0 || maxLag < minLag)
     return(-1);                                          /* bad call */
   if(minLag == 0) {                       /* no need to compute zero */
     *(c++) == 0.0;
     minLag++;
   }
   for(m = minLag; m <= maxLag; m++, c++) {
     sPtr = s;
     mPtr = &(sPtr[m]);
     for(*c = 0.0, n = 0; n < N; n++, sPtr++, mPtr++) {
       *c += fabs(*sPtr - *mPtr);
     }
     *c /= (double)N;
  }
  return(0);
}

/*DOC

Function 'getZCR'

Calculates the mean of the positive and negative zero-crossing rates 
of the signal values in an array.

Arguments:
 s[N]   array with signal values
 N      number of signal values
 sfr    sampling rate of signal

Returns:
 average zero-crossing rate in Hz
 -1 when invalid function argument

DOC*/

double getZCR(register double *s, register long N, double sfr)
{
  register int  POS;
  register long n, numZX;
  double first, last, prev;
  double avrPeriod, zxRate;

  if(s == NULL || N < 1 || sfr <= 0.0)
    return(-1.0);
  numZX = 0;
  first = last = -1.0;
  POS = (*s >= 0.0) ? TRUE : FALSE;
  prev = *s;
  s++;
  for(n = 1; n < N; n++, s++) {
    if(*s >= 0.0) {
      if(!POS) {
	POS = TRUE;
	numZX++;
	last = (double)n - *s / (*s - prev);
	if(first < 0.0)
	  first = last;
      }
    }
    else {
      if(POS) {
	POS = FALSE;
	numZX++;
	last = (double)n + *s / (prev - *s);
	if(first < 0.0)
	  first = last;
      }
    }
    prev = *s;
  }
/*   if(numZX > 1) { */
  if(numZX > 2) {
    avrPeriod = 2.0 * (last - first) / (double)(numZX - 1);
/*     if(numZX < ((double)(N-1) / avrPeriod)) { */
      /* looks like few noisy crossings: take average in frame */
/*       avrPeriod = 2.0 * (double)(N-1) / (double)(numZX -1); */
/*     } */
    zxRate = PERIODtoFREQ(avrPeriod, sfr);
  }
  else
    zxRate = 0.0;
  return(zxRate);
}

/*DOC

Function `getRMS'

Calculates the Root Mean Square (effective) amplitude of the signal 
values in an array.

Arguments:
 s[N]   array with signal values
 N      number of signal values

Returns:
 RMS amplitude (linear)
 -1 when invalid function argument

DOC*/

double getRMS(register double *s, register long N)
{
  register long n;
  double sum;

  if(s == NULL || N < 0)
    return(-1.0);
  if(N == 0)
    return(0.0);
  for(sum = 0.0, n = 0; n < N; s++, n++)
    sum += ((*s) * (*s));
  return(sqrt(sum/(double)N));
}

/*DOC

Function `getMaxMag'

Determines the maximum magnitude in an array of signal values.

Arguments:
 s[N]   array with signal values
 N      number of signal values

Returns:
 maximum magnitude (always positive)
 -1 when invalid function argument

DOC*/

double getMaxMag(register double *s, register long N)
{
  register long n;
  double min, max;

  if(s == NULL || N < 1)
    return(-1.0);
  min = max = *(s++);
  for(n = 1; n < N; s++, n++) {
    if(*s < min) min = *s;
    else if(*s > max) max = *s;
  }
  max = fabs(max); /* you never know! */
  min = fabs(min);
  return((max > min) ? max : min);
}

/*DOC

Function `getMaxMagI16'

Determines the maximum magnitude in an array of 16-bit integer 
signal values.

Arguments:
 s[N]   array with signal values
 N      number of signal values

Returns:
 maximum magnitude (always positive)
 -1 when invalid function argument

DOC*/

long getMaxMagI16(register int16_t *s, register long N)
{
  register long n, min, max, val;

  if(s == NULL || N < 1)
    return(-1);
  min = max = (long)(*(s++));
  for(n = 1; n < N; n++) {
    val = (long)(*(s++));
    if(val < min) min = val;
    else if(val > max) max = val;
  }
  max = labs(max); /* you never know! */
  min = labs(min);
  return((max > min) ? max : min);
}

/*DOC

Calculates pseudo-random number with a rectangular distribution in 
the interval [-1.0 , 1.0]. "seedPtr" points to the start value for 
the computation and will be updated to produce a new random number 
at the next call.

See: Presser et al. (2002) "Numerical Recipes in C", 2nd Edition, 
     Cambridge University Press, p. 284.

DOC*/

double randRPDF(uint32_t *seedPtr)
{
  double r;

  *seedPtr *= 1664525UL;
  *seedPtr += 1013904223UL;
  r = (double)(*seedPtr);                         /* 0 ... 4294967295 */
  return((r / 2147483647.5) - 1.0);                   /* -1.0 ... 1.0 */
}

/*DOC

Calculates pseudo-random number with a triangular distribution in 
the interval [-1.0 , 1.0]. "seedPtr" points to the start value for 
the computation and will be updated to produce a new random number 
at the next call. See randRPDF() above for the basic generator.

DOC*/

double randTPDF(uint32_t *seedPtr)
{
  double r;

  *seedPtr *= 1664525UL;
  *seedPtr += 1013904223UL;
  r = (double)(*seedPtr);                         /* 0 ... 4294967295 */
  *seedPtr *= 1664525UL;
  *seedPtr += 1013904223UL;
  r += (double)(*seedPtr);                        /* 0 ... 8589934590 */
  return((r / 4294967295.0) - 1.0);                   /* -1.0 ... 1.0 */
}
