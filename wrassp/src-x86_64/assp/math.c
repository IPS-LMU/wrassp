/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1989 - 2011  Michel Scheffers                          *
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
* File:     math.c                                                     *
* Contents: General numerical mathematics functions.                   *
*           Prototypes of these functions are in asspdsp.h.            *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: math.c,v 1.9 2011/02/17 10:09:30 mtms Exp $ */

#include <stdio.h>    /* NULL */
#include <math.h>     /* fabs() exp() sqrt() */
#include <inttypes.h> /* uint32_t */

#include <asspdsp.h>  /* BAIRSTOW */

/*DOC

Function 'GCD'

Computes the Greatest Common Divisor of the integral numbers "N" and "M".
Returns the GCD or 0 if "N" or "M" equals 0.

DOC*/

uint32_t GCD(uint32_t N, uint32_t M)
{
  uint32_t gcd, rest;

  if(N == 0 || M == 0)
    return(0);
  if(N > M) {                        /* should be the other way round */
    gcd = M;
    M = N;
  }
  else gcd = N;
  while((rest=M%gcd) != 0) {                    /* Euclid's algorithm */
    M = gcd;
    gcd = rest;
  }
  return(gcd);
}

/*DOC

Function 'LCM'

Computes the Least Common Multiple of the integral numbers "N" and "M".
Returns the LCM in double precision float or 0 if "N" or "M" equals 0.

DOC*/

double LCM(uint32_t N, uint32_t M)
{
  uint32_t gcd, rest;
  double   lcm=0.0;

  if(N == 0 || M == 0)
    return(0.0);
  gcd = GCD(N, M);
  if(gcd != 0) {
    rest = M / gcd;
    lcm = (double)N * (double)rest;
  }
  return(lcm);
}

/*DOC

Function 'linterpol'

Straight-forward linear inter/extrapolation of "y", given "x" through the 
data points (X1,Y1) and (X2,Y2). Just exchange all coordinates if you want 
to get an estimate of x, given y.

Returns:
  0 if OK
 -1 if called with invalid arguments

DOC*/

int linterpol(double X1, double Y1, double X2, double Y2,\
	      double x, double *yPtr)
{
  double dx;

  dx = fabs(X2 - X1);                        /* catch rounding errors */
  if(dx == 0.0 || yPtr == NULL)
    return(-1);
  *yPtr = Y1 + (x - X1) * (Y2 - Y1) / (X2 - X1);
  return(0);
}

/*DOC

Function 'parabola'

This function makes a parabolic fit through three equidistant data points.
                                                                2
It calculates the constants of the equation: y - Y0 = A*(x - X0)

Parameters:
  y1, y2, y3  input data points
  dx          distance between data points
  X0Ptr       pointer to the X-position of the parabola relative to that of y2
  Y0Ptr       pointer to the extremum value of the parabola
  APtr        pointer to the scaling factor

Returns:
  0 if OK
 -1 if no fit possible because the input points are on a straight line

DOC*/

int parabola(double y1, double y2, double y3, double dx,\
             double *X0Ptr, double *Y0Ptr, double *APtr)
{
  double dy, yy;

  dy = 2.0 * (y1 - y2 - y2 + y3);
  if(dy == 0.0) {
    if(X0Ptr != NULL)
      *X0Ptr = 0.0;
    if(Y0Ptr != NULL)
      *Y0Ptr = y2;
    if(APtr != NULL)
      *APtr = 0.0;
    return(-1);
  }
  yy = y1 - y3;
  if(X0Ptr != NULL)
    *X0Ptr = dx * yy / dy;
  if(Y0Ptr != NULL)
    *Y0Ptr = y2 - yy * yy / (4.0 * dy);
  if(APtr != NULL)
    *APtr = dy / (4.0 * dx * dx);
  return(0);
}

/*DOC

Function 'bairstow'

Bairstow's method for root solving of normalized polynomials with real 
coefficients. Determines p and q such that
  2            m=M-2      M-2-m    m=M        M-m
(X  + pX + q) * SUM (r * X     ) =  SUM (c * X   )   with r = c = 1
               m=0    m            m=0    m                0   0

Arguments:
  c[M+1]  array with coefficients of input polynomial
  p, q    pointers to variables for p and q
          input: starting estimates; output: final estimates
  r[M-1]  array for coefficients of rest polynomial
  M       order of input polynomial
  t[M-1]  array for storage of temporary data
  term    pointer to 'BAIRSTOW' structure with termination criteria

Returns:
  Number of iterations if no problems
  -1 if error in function arguments

Note: If the returned value exceeds the maximum number of iterations
      defined in "term", the values of p and/or q are not reliable.

See: Fröberg, C.E. (1969), "Introduction to numerical analysis."
       Chapter 2.3, Addison Wesley, Reading
     Presser. W.H., Teukolsky, S.A., Vettering, W.T. and Flannery, B.P.
       (1992), "Numerical Recipes in C". 2nd Edition pp. 376-379, 
       Cambridge University Press.
 
DOC*/

int bairstow(register double *c, register double *p, register double *q,\
	     register double *r, int M, register double *t, BAIRSTOW *term)
{
  register int i, j, k, l, M_1;
  double rM, rM_1, t_r, det; 
  double dp, dq, pe, qe;
  
  if(c == NULL || p == NULL || q == NULL || r == NULL ||\
     M < 2 || t == NULL || term == NULL)
    return(-1);
  if(M == 2) {                                        /* trivial case */
    *p = c[1];
    *q = c[2];
    r[0] = 1.0;
    return(0);
  }
  M_1 = M - 1;
  dp = dq = 0.0;
  for(r[0] = t[0] = 1.0, i = 0; i < term->maxIter; i++) {
    r[1] = c[1] - (*p);                    /* j = 1 case outside loop */
    t[1] = r[1] - (*p);
    for(j = 2, k = 1, l = 0; j < M_1; j++, k++, l++) {
      r[j] = c[j] - (*p)*r[k] - (*q)*r[l];
      t[j] = r[j] - (*p)*t[k] - (*q)*t[l];
    }
    /* indices now have the following values: j=M-1, k=M-2, l=M-3 */
    /* we store the last values in local variables, NOT in the arrays */
    rM_1 = c[j] - (*p)*r[k] - (*q)*r[l];                    /* r[M-1] */
    t_r = - (*p)*t[k] - (*q)*t[l];                 /* t[M-1] - r[M-1] */
    rM = c[M] - (*p)*rM_1 - (*q)*r[k];                        /* r[M] */
    det = t[k]*t[k] - t_r*t[l];                        /* determinant */
    if(det == 0.0) {                     /* improbable but who knows? */
      dp = (dp < 0.0) ? -1.0 : 1.0;
      dq = (dq < 0.0) ? -1.0 : 1.0;
    }
    else {
      dp = (t[k]*rM_1 - t[l]*rM)/det;
      if(t[l] == 0.0)
	dq = (rM - t_r*dp)/t[k];
      else
	dq = (rM_1 - t[k]*dp)/t[l];
    }       /* Note: if t[l] AND t[k] zero then determinant also zero */
    *p += dp;
    *q += dq;
/* check on limits for stable LP filter */
/*     if(fabs(*p) > 2.0 || fabs(*q) > 1.0) { */
/*       if(*p < -2.0) *p = -2.0; */
/*       else if(*p > 2.0) *p = 2.0; */
/*       if(*q < -1.0) *q = -1.0; */
/*       else if(*q > 1.0) *q = 1.0; */
/*     } */
/*     else { } */
    pe = fabs(*p) * term->relPeps + term->absPeps;
    qe = fabs(*q) * term->relQeps + term->absQeps;
    if((fabs(dp) <= pe) && (fabs(dq) <= qe))
      break;
/* alternative: */
/*     pe = fabs(*p) * term->relPeps; */
/*     qe = fabs(*q) * term->relQeps; */
/*     if(fabs(dp) <= pe && fabs(dp) <= term->absPeps && */
/*        fabs(dq) <= qe && fabs(dq) <= term->absQeps) */
/*       break; */
  }
  r[0] = 1.0;                            /* construct rest polynomial */
  r[1] = c[1] - (*p);
  r[2] = c[2] - (*p)*r[1] - *q;
  for(j = 3, k = 2, l = 1; j < M_1; j++, k++, l++)
    r[j] = c[j] - (*p)*r[k] - (*q)*r[l];
  return(i+1);
}

/*DOC

Function 'hasCCR'
                                    2
Determines whether the polynomial  X + pX + q  has complex conjugate 
roots. Returns non-zero if true, otherwise 0 (false).

DOC*/

int hasCCR(double p, double q)
{
  return(q > 0.0 && q > (0.25 * p * p));
}

/*DOC

Function 'bessselI0'

Calculates the zeroth order modified Bessel function of the first kind
with user-definable accuracy.

Parameters:
double x    input value
double eps  approximate error in function value (MUST BE > 0)

Returns:
double      function value

DOC*/

double besselI0(double x, double eps)
{
  int    k;
  double f, x2, p, t, y;

  if(x == 0.0)
    return(1.0);
  if(eps <= 0.0)
    eps = 0.5e-6;       /* more sensible value */
  /* initialize (k = 1) */
  f = 1.0;              /* k! */
  p = x2 = 0.5 * x;     /* (x/2) ^ k */
  t = p * p;            /* 1st term */
  y = 1.0 + t;          /* function value */
  for(k = 2; t > eps; k++) {
    p *= x2;
    f *= k;
    t = p / f;
    t *= t;
    y += t;
  }
  return(y);
}

/*DOC

Function 'bessi0'

Polynomial approximation of the zeroth order modified Bessel function
of the first kind. ANSI-C replacement of the SVID/BSD function j0().
From: Presser et al. (2002) "Numerical Recipes in C", p.237
also: Abramowitz and Stegun (1964) "Handbook of mathematical functions", p.378

Parameters:
double x    input value

Returns:
double      function value (eps < 2e-7)

DOC*/

double bessi0(double x)
{
  double ax, t, y;

  if(x == 0.0)
    return(1.0);
  ax = fabs(x);
  if(ax < 3.75) {
    t = x / 3.75;
    t *= t;
    y = 1.0 + t*(3.5156229 + t*(3.0899424 + t*(1.2067492\
	    + t*(0.2659732 + t*(0.0360768 + t*0.0045813)))));
  }
  else {
    t = 3.75 / ax;
    y = (exp(ax)/sqrt(ax)) * (0.39894228 + t*(0.01328592\
	+ t*(0.00225319 + t*(-0.00157565 + t*(0.00916281\
	+ t*(-0.02057706 + t*(0.02635537 + t*(-0.01647633\
	+ t*0.00392377))))))));
  }
  return(y);
}
