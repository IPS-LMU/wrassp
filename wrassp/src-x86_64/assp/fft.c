/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1989 - 2008  Michel Scheffers                          *
*                            IPdS, CAU Kiel                            *
*                            Leibnizstr. 10                            *
*                            24118 Kiel, Germany                       *
*                            ms@ipds.uni-kiel.de                       *
*                                                                      *
* This library is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 3 of the License, or    *
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
* File:     fft.c                                                      *
* Contents: Fast Fourier Transforms for complex and real signals;      *
*           functions for converting the output of the transform       *
*           into various spectra and conversions between bins and      *
*           frequencies.                                               *
*           Prototypes of these functions and definitions of constants *
*           are in "asspdsp.h".                                        *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: fft.c,v 1.2 2008/01/07 09:58:21 mtms Exp $ */

#include <math.h>     /* sin() cos() atan2() fabs() hypot() log10() */

#include <asspdsp.h>  /* MIN_NFFT PI TWO_PI TINY... */

/*DOC

Function 'fft'

The classical Cooley-Tukey FFT algorithm.
Adapted from Press et al. (2002), "Numerical recipes in C. 
The Art of Scientific Computing", 2nd Edition, Cambridge 
University Press, pp. 507-508.

This is not an optimized version for C, only the indices have been 
shifted by one to start at 0. 
The array 'x' should be of length 2 x 'N' with 'N' the number of FFT 
points and has the real part of the complex signal/spectrum at the 
odd, the imaginary part at the even cells. 
A positive value of DIRECT yields the forward transform, a negative 
one the inverse. The forward transform has the negative exponential. 
This function has no length normalization; the "de facto" standard 
would be to divide the inverse transform by 'N'.

DOC*/

int fft(register double *x, long N, int DIRECT)
{
  static   long oldN=0;
  register long NN, i, j, m, mmax, incr;
  double temp, cosf, sinf, ur, ui, vr, vi, arg;

  if(DIRECT == 0 || N < MIN_NFFT) {                          /* reset */
    oldN = 0;
    if(DIRECT == 0)
      return(0);
    return(-1);
  }
  if(oldN != N) {                         /* automatic initialization */
    for(m = 0, i = N; i > 1; i >>= 1)               /* get power of 2 */
      m++;
    i = 1 << m;                       /* recalculate number of points */
    if(N != i)                                    /* NOT a power of 2 */
      return(-1);
    oldN = N;
  }
  NN = N << 1;
  for(i = j = 1; i < NN; i += 2) {
    if(j > i) {
      temp = x[i-1];
      x[i-1] = x[j-1];
      x[j-1] = temp;
      temp = x[i];
      x[i] = x[j];
      x[j] = temp;
    }
    m = N;
    while(m >= 2 && j > m) {
      j -= m;
      m >>= 1;
    }
    j += m;
  }
  mmax = 2;
  while(NN > mmax) {
    incr = mmax << 1;
    arg = TWO_PI / (double)mmax;
    ur = cos(arg);
    ui = (DIRECT < 0) ? sin(arg) : -sin(arg);
    cosf = 1.0;
    sinf = 0.0;
    for(m = 1; m < mmax; m += 2) {
      for(i = m; i <= NN; i+= incr) {
	j = i + mmax;
	vr = (cosf * x[j-1]) - (sinf * x[j]);
	vi = (sinf * x[j-1]) + (cosf * x[j]);
	x[j-1] = x[i-1] - vr;
	x[j] = x[i] - vi;
	x[i-1] += vr;
	x[i] += vi;
      }
      temp = (cosf * ur) - (sinf * ui);
      sinf = (sinf * ur) + (cosf * ui);
      cosf = temp;
    }
    mmax = incr;
  }
  return(0);
}

/*DOC

Function 'rfft'

Calculates an in-place FFT for real signals.
See: Engeln-Muellges & Reuter (1987) 'Formelsammlung zur numerischen
     Mathematik mit C-Programmen' pp. 391-395.

Input arguments:
 x[N]    for DIRECT > 0: array with signal values
         for DIRECT < 0: array with discrete Fourier coefficients
                         in the format:
		        x[0] = a0
                        x[2*k-1] = ak for k = 1, 2, .. , N/2
                        x[2*k] = bk for k = 1, 2, .. , N/2 -1
 N       number of FFT points (at least 4)
 DIRECT  direction of transform:
                        > 0: forward
                        < 0: inverse
			= 0: reset internal constants; no calculations

Output arguments:
 x[N]    for DIRECT > 0: array with discrete Fourier coefficients
                         in the format:
                        a0 = x[0]
			ak = x[2*k-1] for k = 1, 2, .. , N/2
			bk = x[2*k] for k = 1, 2, .. , N/2 -1
         for DIRECT < 0: array with signal values
         
Returns:
  0 if OK
 -1 if N less than 4 or not a power of 2

Remarks:
 -- length normalization is performed in FORWARD mode and it seems 
    that x[0] then contains in fact a0 / 2 (the DC component)
 -- a(k) corresponds to Re{c(k)}, b(k) to Im{c(k)}

DOC*/

int rfft(register double *x, long N, int DIRECT)
{
  static   long oldN = 0;             /* for automatic initialization */
  static   long M;
  static   double factor, argN, argHN;
  register long i, j, k, l, m;                           /* for speed */
  long   HN, pm, pm_1, pM_1_m, QN;
  double arg, cosf, sinf, save, keep;
  double ur, ui, vr, vi, wr, wi;
  
  if(DIRECT == 0 || N < MIN_NFFT) {                  /* reset */
    oldN = 0;
    if(DIRECT == 0)
      return(0);
    return(-1);
  }
  if(oldN != N) {                         /* automatic initialization */
    for(M = 0, i = N; i > 1; i >>= 1)               /* get power of 2 */
      M++;
    i = 1 << M;                       /* recalculate number of points */
    if(N != i)                                    /* NOT a power of 2 */
      return(-1);
    factor = 2.0 / (double)N;
    argN  = PI * factor;
    argHN = TWO_PI * factor;
    oldN = N;
  }
  HN = N >> 1;  /* N / 2 */
  QN = HN >> 1; /* N / 4 hence MIN_NFFT */
  if(DIRECT < 0) { /* INVERSE TRANSFORM */
    keep  = x[1];           /* use symmetry properties to get coeff's */
    x[1]  = x[0] - x[N-1];   /*     for complex IFFT with half length */
    x[0] += x[N-1];
    ur = cos(argN);                        /* real and imaginary part */
    ui = sin(argN);                        /*   of N-th root of unity */
    cosf = 1.0;
    sinf = 0.0;
    for(i = 1; i < QN; i++) {
      j = i << 1;
      k = N - j;
      save = cosf;                        /* trigonometric recurrence */
      cosf = save*ur - sinf*ui;                       /* cos(i*2PI/N) */
      sinf = sinf*ur + save*ui;                       /* sin(i*2PI/N) */
      vr = 0.5 * (sinf*(keep-x[k-1]) - cosf*(x[j]+x[k]));
      vi = 0.5 * (cosf*(keep-x[k-1]) + sinf*(x[j]+x[k]));
      wr = 0.5 * (keep + x[k-1]);
      wi = 0.5 * (x[j] - x[k]);
      keep = x[j+1];
      x[j]   = wr - vr;
      x[j+1] = vi - wi;
      x[k]   = vr + wr;
      x[k+1] = vi + wi;
    }
    x[HN+1] = x[HN];
    x[HN]   = keep;
  }
/*-- N/2-points FFT --------------------------------------------------*/
  for(i = 0; i < HN; i++) {
    for(j = i, k = 1, l = 0; k < M; k++) {
      l = (l << 1) + (j & 1);                          /* bit shuffle */
      j >>= 1;
    }
    if(i <= l) {
      j = i << 1;                             /* j & k are free again */
      k = l << 1;
      vr = x[j];
      vi = x[j+1];
      if(DIRECT > 0) { /* length normalization in FORWARD transform ! */
	x[j]   = x[k] * factor;
        x[j+1] = x[k+1] * factor;
        x[k]   = vr * factor;
        x[k+1] = vi * factor;
      }
      else {
 	x[j]   = x[k];
        x[j+1] = x[k+1];
        x[k]   = vr;
        x[k+1] = vi;
      }
    }
  }
  pM_1_m = HN;                                  /* init for 2^(M-1-m) */
  pm_1 = 1;                                       /* init for 2^(m-1) */
  for(m = 1; m < M; m++) {
    pM_1_m >>= 1;                                        /* 2^(M-1-m) */
    pm = pm_1 << 1;                                            /* 2^m */
    arg = argHN * pM_1_m;
    ur = cos(arg);
    ui = (DIRECT < 0) ? sin(arg) : -sin(arg);
    cosf = 1.0;
    sinf = 0.0;
    for(i = 0; i < HN; i += pm) {            /* j=0 case outside loop */
      k = i << 1;
      l = k + pm;
      vr = x[l];
      vi = x[l+1];
      x[l]    = x[k] - vr;
      x[l+1]  = x[k+1] - vi;
      x[k]   += vr;
      x[k+1] += vi;
    }
    for(j = 1; j < pm_1; j++) {
      save = cosf;                        /* trigonometric recurrence */
      cosf = save*ur - sinf*ui;                     /* cos(j*2PI/2^m) */
      sinf = sinf*ur + save*ui;                 /* -/+ sin(j*2PI/2^m) */
      for(i = 0; i < HN; i += pm) {
	k = (i+j) << 1;
        l = k + pm;
        vr = x[l]*cosf - x[l+1]*sinf;
        vi = x[l]*sinf + x[l+1]*cosf;
        x[l]    = x[k] - vr;
        x[l+1]  = x[k+1] - vi;
        x[k]   += vr;
        x[k+1] += vi;
      }
    }
    pm_1 = pm;
  }
/*-- End of FFT ------------------------------------------------------*/
  if(DIRECT > 0) { /* FORWARD TRANSFORM */
    keep = x[N-1];               /* use symmetry properties to double */
    x[N-1] = 0.5 * (x[0] - x[1]); /*    the number of Fourier coeff's */
    x[0]   = 0.5 * (x[0] + x[1]);
    ur = cos(argN);                        /* real and imaginary part */
    ui = -sin(argN);                       /*   of N-th root of unity */
    cosf = 1.0;
    sinf = 0.0;
    for(i = 1; i < QN; i++) {
      j = i << 1;
      k = N - j;
      save = cosf;
      cosf = save*ur - sinf*ui;                       /* cos(i*2PI/N) */
      sinf = sinf*ur + save*ui;                      /* -sin(i*2PI/N) */
      vr = 0.5 * (sinf*(x[j]-x[k]) + cosf*(x[j+1]+keep));
      vi = 0.5 * (cosf*(x[j]-x[k]) - sinf*(x[j+1]+keep));
      wr = 0.5 * (x[j] + x[k]);
      wi = 0.5 * (x[j+1] - keep);
      keep   = x[k-1];
      x[j-1] = vr + wr;
      x[j]   = vi - wi;
      x[k-1] = wr - vr;
      x[k]   = vi + wi;
    }
    x[HN-1] = x[HN];
    x[HN]   = keep;
  }
  
  return(0);
}

/*DOC

Function 'rfftRe'

Extracts real part of the output of the rfft() function in FORWARD mode.

Arguments:
 c[N]      discrete Fourier coefficients
 r[N/2+1]  array for real part
 N         number of FFT points

Remarks:
-- "c" and "r" may point to the same array

DOC*/

void rfftRe(register double *c, register double *r, register long N)
{
  register long i;
  
  *(r++) = *c;                             /* DC component; Im(0) = 0 */
  for(i = 1; i < N; i +=2) {
    *(r++) = c[i];
  }

  return;
}

/*DOC

Function 'rfftIm'

Extracts imaginary part of the output of the rfft() function in FORWARD mode.

Arguments:
 c[N]      discrete Fourier coefficients
 r[N/2+1]  array for imaginary part
 N         number of FFT points

Remarks:
-- "c" and "r" may point to the same array

DOC*/

void rfftIm(register double *c, register double *r, register long N)
{
  register long i;
  
  *(r++) = 0.0;                             /* DC component; Im(0) = 0 */
  for(i = 2; i < N; i +=2) {
    *(r++) = c[i];
  }
  *r = 0.0;                             /* Fs/2 component; Im(N/2) = 0 */

  return;
}

/*DOC

Function 'rfftLinAmp'

Converts the output of the rfft() function in FORWARD mode 
to a linear amplitude spectrum.

Arguments:
 c[N]      discrete Fourier coefficients
 a[N/2+1]  array for amplitude spectrum
 N         number of FFT points

Remarks:
-- "c" and "a" may point to the same array

DOC*/

void rfftLinAmp(register double *c, register double *a, long N)
{
  register long i, HN;
  double Re, Im;
  
  HN = N >> 1;
  *(a++) = fabs(*(c++));                   /* DC component; Im(0) = 0 */
  for(i = 1; i < HN; i++) {
    Re = *(c++);
    Im = *(c++);
    *(a++) = hypot(Re, Im);
  }
  *a = fabs(*c);                       /* Fs/2 component; Im(N/2) = 0 */

  return;
}

/*DOC

Function 'rfftLinPow'

Converts the output of the rfft() function in FORWARD mode 
to a linear power spectrum.

Arguments:
 c[N]      discrete Fourier coefficients
 p[N/2+1]  array for power spectrum
 N         number of FFT points

Remarks:
-- "c" and "p" may point to the same array

DOC*/

void rfftLinPow(register double *c, register double *p, long N)
{
  register long i, HN;
  double Re, Im;
  
  HN = N >> 1;
  *(p++) = (*c) * (*c);                     /* DC component; Im(0) = 0 */
  c++;
  for(i = 1; i < HN; i++) {
    Re = *(c++);
    Im = *(c++);
    *(p++) = Re*Re + Im*Im;
  }
  *p = (*c) * (*c);                    /* Fs/2 component; Im(N/2) = 0 */

  return;
}

/*DOC

Function 'rfftLogPow'

Converts the output of the rfft() function in FORWARD mode 
to a log (base 10) power spectrum.

Arguments:
 c[N]      discrete Fourier coefficients
 p[N/2+1]  array for power spectrum
 N         number of FFT points

Remarks:
-- "c" and "p" may point to the same array
-- possible underflow and division by zero has been taken care of

DOC*/

void rfftLogPow(register double *c, register double *p, long N)
{
  register long i, HN;
  double power;
  
  HN = N >> 1;
  power = fabs(*(c++));                    /* DC component; Im(0) = 0 */
  if(power <= TINYLIN)
    *(p++) = TINYPLG;
  else
    *(p++) = 2.0 * log10(power);
  for(i = 1; i < HN; i++) {
    power = ((*c )* (*c));                                    /* Re^2 */
    c++;
    power += ((*c) * (*c));                                 /* + Im^2 */
    c++;
    if(power <= TINYSQR)
      *(p++) = TINYPLG;
    else
      *(p++) = log10(power);
  }
  power = fabs(*c);                    /* Fs/2 component; Im(N/2) = 0 */
  if(power <= TINYLIN)
    *p = TINYPLG;
  else
    *p = 2.0 * log10(power);

  return;
}

/*DOC

Function 'rfftPower'

Converts the output of the rfft() function in FORWARD mode 
to a power spectrum in dB.

Arguments:
 c[N]      discrete Fourier coefficients
 p[N/2+1]  array for power spectrum
 N         number of FFT points

Remarks:
-- "c" and "p" may point to the same array
-- possible underflow and division by zero has been taken care of

DOC*/

void rfftPower(register double *c, register double *p, long N)
{
  register long i, HN;
  double power;
  
  HN = N >> 1;
  power = fabs(*(c++));                    /* DC component; Im(0) = 0 */
  if(power <= TINYLIN)
    *(p++) = TINYPdB;
  else
    *(p++) = LINtodB(power);
  for(i = 1; i < HN; i++) {
    power = (*c * (*c));                                      /* Re^2 */
    c++;
    power += (*c * (*c));                                   /* + Im^2 */
    c++;
    if(power <= TINYSQR)
      *(p++) = TINYPdB;
    else
      *(p++) = SQRtodB(power);
  }
  power = fabs(*c);                    /* Fs/2 component; Im(N/2) = 0 */
  if(power <= TINYLIN)
    *p = TINYPdB;
  else
    *p = LINtodB(power);

  return;
}

/*DOC

Function 'rfftPhase'

Converts the output of the rfft() function in FORWARD mode 
to a phase spectrum in radians with -PI < Phi <= +PI. 

Arguments:
 c[N]      discrete Fourier coefficients
 p[N/2+1]  array for phase spectrum
 N         number of FFT points

Remarks:
-- "c" and "p" may point to the same array
-- possible underflow and division by zero has been taken care of

DOC*/

void rfftPhase(register double *c, register double *p, long N)
{
  register long i, HN;
  double Re, Im, phase;
  
  HN = N >> 1;
  if(*(c++) >= 0.0)
    *(p++) = 0.0;                          /* DC component; Im(0) = 0 */
  else
    *(p++) = PI;
  for(i = 1; i < HN; i++) {
    Re = *(c++);
    Im = *(c++);
    if(Re == 0.0) {                         /* avoid division by zero */
      if(Im > 0.0)
	phase = HLF_PI;
      else if(Im < 0.0)
	phase = -HLF_PI;
      else
	phase = 0.0;
    }
    else {
      if(Im == 0.0)                 /* catch exception because result */
	phase = 0.0;                  /* otherwise compiler-dependent */
      else
	phase = atan2(Im, Re);
    }
    *(p++) = phase;                           /* copy to output array */
  }
  if(*c >= 0.0)
    *p = 0.0;                          /* Fs/2 component; Im(N/2) = 0 */
  else
    *p = PI;

  return;
}

/*DOC

Function 'freq2bin'

Rounds frequency in Hz to nearest DFT bin number.

DOC*/

long freq2bin(double freq, double sampFreq, long nDFT)
{
  if(freq <= 0.0)
    return(0);
  if(freq >= sampFreq / 2.0)
    return(nDFT / 2);
  return((long)((double)nDFT * freq / sampFreq + 0.5));
}

/*DOC

Function 'bin2freq'

Converts DFT bin number to frequency in Hz.

DOC*/

double bin2freq(long bin, double sampFreq, long nDFT)
{
  if(bin <= 0)
    return(0.0);
  if(bin > nDFT / 2)
    bin = nDFT / 2;
  return(((double)bin * sampFreq) / (double)nDFT);
}
