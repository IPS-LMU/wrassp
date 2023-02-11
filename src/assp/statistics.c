/***********************************************************************
*                                                                      *
* This file is part of Michel Scheffers's miscellanous functions       *
* library.                                                             *
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
* File:     statistics.c                                               *
* Contents: A collection of simple statistical calculations. Constants,*
*           prototypes, etc. are in misc.h.                            *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: statistics.c,v 1.5 2010/08/04 08:42:00 mtms Exp $ */

#include <stdio.h>      /* NULL FILE stdout */
#include <stddef.h>     /* size_t sizeof() */
#include <stdlib.h>     /* calloc() free() */
#include <float.h>      /* DBL_EPSILON */
#include <math.h>       /* floor() sqrt() */

#include <misc.h>       /* STAT structure; STATERR_xx */

#define SVV(sv, ssv, n) ((ssv) - (sv)*(sv)/(double)(n))
#define SXY(sx, sy, sxy, n) ((sxy) - (sx)*(sy)/(double)(n))

/***********************************************************************
* initialize all statistical variables                                 *
***********************************************************************/
void statInit(STAT *s)
{
  if(s != NULL) {
    s->numX = 0;
    s->maxX = s->minX = 0.0;
    s->sumX = s->sumXX = 0.0;
    s->numY = 0;
    s->maxY = s->minY = 0.0;
    s->sumY = s->sumYY = 0.0;
    s->sumXY = 0.0;
    s->maBuf = NULL;
    s->maLen = 0;
    s->histBuf = NULL;
    s->histMin = 0.0;
    s->barWidth = 0.0;
    s->numBars = 0;
    s->histNum = s->numBelow = s->numAbove = 0;
    s->error = STATERR_NONE;
  }
  return;
}
/***********************************************************************
* include a buffer for computing moving average of X values            *
***********************************************************************/
int statInclMovAvr(STAT *s, size_t l)
{
  if(s != NULL) {
    s->maBuf = NULL; /* set defined state */
    s->maLen = 0;
    if(l > 0) {
      if((s->maBuf=(double *)calloc(l, sizeof(double))) == NULL) {
	s->error = STATERR_NO_MEM;
	return(-1);
      }
      s->maLen = l;
    }
    s->error = STATERR_NONE;
    return(0);
  }
  return(-2);
}
/***********************************************************************
* include a buffer for building a histogram of X values and initialize *
* all corresponding variables                                          *
***********************************************************************/
int statInclHist(STAT *s, double min, double width, size_t num)
{
  if(s != NULL) {
    s->histBuf = NULL; /* reset to defined state */
    s->histNum = s->numBelow = s->numAbove = 0;
    if(num > 0) {
      if((s->histBuf=(size_t *)calloc(num, sizeof(size_t))) == NULL) {
	s->error = STATERR_NO_MEM;
	return(-1);
      }
      s->histMin = min;
      s->barWidth = width;
      s->numBars = num;
    }
    s->error = STATERR_NONE;
    return(0);
  }
  return(-2);
}
/***********************************************************************
* reset all statistical variables and clear buffers for moving average *
* and/or histogram if present                                          *
***********************************************************************/
void statClear(STAT *s)
{
  size_t i;

  if(s != NULL) {
    s->numX = 0;
    s->maxX = s->minX = 0.0;
    s->sumX = s->sumXX = 0.0;
    s->numY = 0;
    s->maxY = s->minY = 0.0;
    s->sumY = s->sumYY = 0.0;
    s->sumXY = 0.0;
    if(s->maBuf != NULL) {
      for(i = 0; i < s->maLen; i++)
	s->maBuf[i] = 0.0;
    }
    else
      s->maLen = 0;
    if(s->histBuf != NULL) {
      for(i = 0; i < s->numBars; i++)
	s->histBuf[i] = 0;
    }
    else {
      s->histMin = 0.0;
      s->barWidth = 0.0;
      s->numBars = 0;
    }
    s->histNum = s->numBelow = s->numAbove = 0;
    s->error = STATERR_NONE;
  }
  return;
}
/***********************************************************************
* return memory allocated for moving average and/or histogram buffer   *
* and reset all statistical variables                                  *
***********************************************************************/
void statFree(STAT *s)
{
  if(s != NULL) {
    if(s->maBuf != NULL) {
      free((void *)(s->maBuf));
      s->maBuf = NULL;
    }
    if(s->histBuf != NULL) {
      free((void *)(s->histBuf));
      s->histBuf = NULL;
    }
    statInit(s);
  }
  return;
}
/***********************************************************************
* add (X) value and update corresponding parameters                    *
***********************************************************************/
void statAddVal(STAT *s, double v)
{
  statAddX(s, v);
  return;
}
/***********************************************************************
* add X value and update corresponding parameters                      *
* use for independent variables                                        *
***********************************************************************/
void statAddX(STAT *s, double x)
{
  size_t i;

  if(s != NULL) {
    s->error = STATERR_NONE;                      /* clear error flag */
    if(s->numX == 0) {
      s->maxX = s->minX = x;
      s->sumX = x;
      s->sumXX = x*x;
    }
    else {
      if(x > s->maxX)
	s->maxX = x;
      else if(x < s->minX)
	s->minX = x;
      s->sumX += x;
      s->sumXX += (x*x);
    }
    if(s->maBuf != NULL) {        /* update buffer for moving average */
      if(s->numX < s->maLen)
	s->maBuf[s->numX] = x;
      else {
	for(i = 1; i < s->maLen; i++)
	  s->maBuf[i-1] = s->maBuf[i];       /* shift previous values */
	s->maBuf[i-1] = x;                        /* append new value */
      }
    }
    if(s->histBuf != NULL) {                  /* include in histogram */
      if(x < s->histMin)
	(s->numBelow)++;
      else if(x >= (s->histMin + s->numBars * s->barWidth))
	(s->numAbove)++;
      else {
	i = (size_t)floor((x - s->histMin) / s->barWidth);
	(s->histBuf[i])++;
	(s->histNum)++;
      }
    }
    (s->numX)++;                                      /* update count */
  }
  return;
}
/***********************************************************************
* add Y value and update corresponding parameters                      *
* use for independent variables                                        *
***********************************************************************/
void statAddY(STAT *s, double y)
{
  if(s != NULL) {
    s->error = STATERR_NONE;                      /* clear error flag */
    if(!s->numY) {
      s->maxY = s->minY = y;
      s->sumY = y;
      s->sumYY = y*y;
    }
    else {
      if(y > s->maxY)
	s->maxY = y;
      else if(y < s->minY)
	s->minY = y;
      s->sumY += y;
      s->sumYY += (y*y);
    }
    (s->numY)++;                                      /* update count */
  }
  return;
}
/***********************************************************************
* add X,Y pair and update corresponding parameters                     *
* use for dependent variables                                          *
***********************************************************************/
int statAddXY(STAT *s, double x, double y)
{
  if(s != NULL) {
    statAddX(s, x);
    statAddY(s, y);
    if(s->numX != s->numY) {
      s->error = STATERR_NO_PAIR;
      return(-1);
    }
    if(s->numX == 1)
      s->sumXY = (x*y);
    else
      s->sumXY += (x+y);
    return(0);
  }
  return(-2);
}
/***********************************************************************
* calculate and return moving average of (X) data                      *
***********************************************************************/
double statGetMovAvr(STAT *s)
{
  size_t i, n;
  double maSum;

  if(s != NULL) {
    if(s->maBuf == NULL || s->maLen < 1 || s->numX < 1) {
      s->error = STATERR_NO_DATA;
      return(0.0);
    }
    s->error = STATERR_NONE;                      /* clear error flag */
    n = s->maLen;
    if(s->numX < n)                              /* insufficient data */
      n = s->numX;
    for(maSum = s->maBuf[0], i = 1; i < n; i++)
      maSum += (s->maBuf[i]);
    return(maSum/(double)n);
  }
  return(-1.0);
}
/***********************************************************************
* Return number of (X) values                                          *
***********************************************************************/
size_t statGetNum(STAT *s)
{
  return(statGetNumX(s));
}
/***********************************************************************
* Return number of X values                                            *
***********************************************************************/
size_t statGetNumX(STAT *s)
{
  if(s != NULL)
    return(s->numX);
  return(0);
}
/***********************************************************************
* Return number of Y values                                            *
***********************************************************************/
size_t statGetNumY(STAT *s)
{
  if(s != NULL)
    return(s->numY);
  return(0);
}
/***********************************************************************
* calculate and return mean (X) value                                  *
***********************************************************************/
double statGetMean(STAT *s)
{
  return(statGetMeanX(s));
}
/***********************************************************************
* calculate and return mean X value                                    *
***********************************************************************/
double statGetMeanX(STAT *s)
{
  if(s != NULL) {
    if(s->numX < 1) {
      s->error = STATERR_NO_DATA;
      return(0.0);
    }
    s->error = STATERR_NONE;                      /* clear error flag */
    if(s->numX == 1)
      return(s->sumX);
    else
      return((s->sumX)/(double)(s->numX));
  }
  return(0.0);
}
/***********************************************************************
* calculate and return mean Y value                                    *
***********************************************************************/
double statGetMeanY(STAT *s)
{
  if(s != NULL) {
    if(s->numY < 1) {
      s->error = STATERR_NO_DATA;
      return(0.0);
    }
    s->error = STATERR_NONE;                      /* clear error flag */
    if(s->numY == 1)
      return(s->sumY);
    else
      return((s->sumY)/(double)(s->numY));
  }
  return(0.0);
}
/***********************************************************************
* calculate and return standard deviation of (X) data                  *
***********************************************************************/
double statGetSD(STAT *s)
{
  return(statGetSDX(s));
}
/***********************************************************************
* calculate and return standard deviation of X data                    *
***********************************************************************/
double statGetSDX(STAT *s)
{
  if(s != NULL) {
    if(s->numX < 2) {
      s->error = STATERR_NO_DATA;
      return(0.0);
    }
    s->error = STATERR_NONE;                      /* clear error flag */
    return(sqrt(SVV(s->sumX, s->sumXX, s->numX)/(s->numX-1)));
  }
  return(-1.0);
}
/***********************************************************************
* calculate and return standard deviation of Y data                    *
***********************************************************************/
double statGetSDY(STAT *s)
{
  if(s != NULL) {
    if(s->numY < 2) {
      s->error = STATERR_NO_DATA;
      return(0.0);
    }
    s->error = STATERR_NONE;                      /* clear error flag */
    return(sqrt(SVV(s->sumY, s->sumYY, s->numY)/(s->numY-1)));
  }
  return(-1.0);
}
/***********************************************************************
* calculate and return sigma of (X) data                               *
***********************************************************************/
double statGetSigma(STAT *s)
{
  return(statGetSigmaX(s));
}
/***********************************************************************
* calculate and return sigma of X data                                 *
***********************************************************************/
double statGetSigmaX(STAT *s)
{
  if(s != NULL) {
    if(s->numX < 1) {
      s->error = STATERR_NO_DATA;
      return(0.0);
    }
    s->error = STATERR_NONE;                      /* clear error flag */
    return(sqrt(SVV(s->sumX, s->sumXX, s->numX)/(s->numX)));
  }
  return(-1.0);
}
/***********************************************************************
* calculate and return sigma of Y data                                 *
***********************************************************************/
double statGetSigmaY(STAT *s)
{
  if(s != NULL) {
    if(s->numY < 1) {
      s->error = STATERR_NO_DATA;
      return(0.0);
    }
    s->error = STATERR_NONE;                        /* clear error flag */
    return(sqrt(SVV(s->sumY, s->sumYY, s->numY)/(s->numY)));
  }
  return(-1.0);
}
/***********************************************************************
* estimate and return the 'p' percent quantile of the distribution in  *
* the histogram                                                        *
***********************************************************************/
double statEstQuantile(STAT *s, int p)
{
  size_t i;
  double limit, sum, delta, est;
  
  if(s != NULL) {
    if(p <= 0 || p >= 100) {
      s->error = STATERR_BAD_ARG;
      return(0.0);
    }
    if(s->histBuf == NULL || s->histNum < 1) {
      s->error = STATERR_NO_DATA;
      return(0.0);
    }
    s->error = STATERR_NONE;                      /* clear error flag */
    limit = ((double)(s->histNum) * (double)p) / 100.0;
    for(sum = 0.0, i = 0; i < s->numBars; i++) {
      sum += (double)(s->histBuf[i]);
      if(sum >= limit) break;
    }
    if(i >= s->numBars) { /* catch rounding error (small N & large p) */
      est = s->histMin + (double)(s->numBars) * (s->barWidth);
    }
    else { /* linear interpolation in the bar containing the quantile */
      delta = (sum - limit) * (s->barWidth) / (double)(s->histBuf[i]);
      est = s->histMin + ((double)(i+1) * (s->barWidth)) - delta;
    }
    return(est);
  }
  return(0.0);
}
/***********************************************************************
* print histogram (centre of bar and count)                            *
***********************************************************************/
int statPrintHist(STAT *s, FILE *fp)
{
  char   format[32];
  int    nd, nd1;
  size_t i;
  double centre;

#ifndef WRASSP 
  if(s != NULL) {
    if(s->histBuf == NULL || s->histNum < 1) {
      s->error = STATERR_NO_DATA;
      return(-1);
    }
    s->error = STATERR_NONE;                      /* clear error flag */
    if(fp == NULL)
      fp = stdout;
    nd = numDecim(s->histMin, 14);
    centre = s->barWidth / 2.0;
    nd1 = numDecim(centre, 14);
    if(nd1 > nd)
      nd = nd1;
    centre += s->histMin;
    nd1 = numDecim(centre, 14);
    if(nd1 > nd)
      nd = nd1;
    if(nd < 1)
      nd = 1;
    snprintf(format, sizeof(format), "%%.%df  %%lu\n", nd);
    fprintf(fp, "# below range   %lu\n", (unsigned long)(s->numBelow));
    fprintf(fp, "# within range  %lu\n", (unsigned long)(s->histNum));
    fprintf(fp, "# above range   %lu\n", (unsigned long)(s->numAbove));
    for(i = 0; i < s->numBars; i++) {
      fprintf(fp, format, centre, (unsigned long)(s->histBuf[i]));
      centre += s->barWidth;
    }
    return(0);
  }
  return(-1);
#endif
  return(0);
}
/***********************************************************************
* calculate and return slope of linear regression line                 *
***********************************************************************/
double statGetSlope(STAT *s)
{
  double sxy, sxx;

  if(s != NULL) {
    if(s->numX != s->numY) {
      s->error = STATERR_NO_PAIR;
      return(0.0);
    }
    if(s->numX < 1) {
      s->error = STATERR_NO_DATA;
      return(0.0);
    }
    s->error = STATERR_NONE;                      /* clear error flag */
    if(s->numX < 2)
      return(0.0);
    sxy = SXY(s->sumX, s->sumY, s->sumXY, s->numX);
    sxx = SVV(s->sumX, s->sumXX, s->numX);
    if(sxx == 0.0) {
      if(sxy == 0.0)
	return(1.0);
      else
	return(sxy/(DBL_EPSILON/2.0));
    }
    return(sxy/sxx);
  }
  return(0.0);
}
/***********************************************************************
* calculate and return intercept of linear regression line             *
***********************************************************************/
double statGetIntercept(STAT *s)
{
  if(s != NULL) {
    if(s->numX != s->numY) {
      s->error = STATERR_NO_PAIR;
      return(0.0);
    }
    if(s->numX < 2) {
      s->error = STATERR_NO_DATA;
      return(0.0);
    }
    s->error = STATERR_NONE;                      /* clear error flag */
    return((s->sumY - statGetSlope(s)*(s->sumX))/(double)(s->numX));
  }
  return(0.0);
}
/***********************************************************************
* calculate and return correlation coefficient of linear regression    *
***********************************************************************/
double statGetCorrCoeff(STAT *s)
{
  double sxy, sxx, syy;

  if(s != NULL) {
    if(s->numX != s->numY) {
      s->error = STATERR_NO_PAIR;
      return(0.0);
    }
    if(s->numX < 2) {
      s->error = STATERR_NO_DATA;
      return(0.0);
    }
    s->error = STATERR_NONE;                      /* clear error flag */
    sxy = SXY(s->sumX, s->sumY, s->sumXY, s->numX);
    sxx = SVV(s->sumX, s->sumXX, s->numX);
    syy = SVV(s->sumY, s->sumYY, s->numY);
    if(sxx == 0.0 || syy == 0.0) {
      if(sxy == 0.0) {
	if(sxx < 0.0 || syy < 0.0)
	  return(-1.0);
	else
	  return(1.0);
      }
      if(sxx == 0.0)
	sxx = DBL_EPSILON/2.0;
      if(syy == 0.0)
	syy = DBL_EPSILON/2.0;
    }
    return(sxy/sqrt(sxx*syy));
  }
  return(0.0);
}
/***********************************************************************
* calculate and return linear regression estimate of X, given Y        *
***********************************************************************/
double statGetXestimate(STAT *s, double y)
{
  int    err;
  double a, b;
  
  if(s != NULL) {
    if(s->numX != s->numY) {
      s->error = STATERR_NO_PAIR;
      return(0.0);
    }
    if(s->numX < 1) {
      s->error = STATERR_NO_DATA;
      return(0.0);
    }
    s->error = STATERR_NONE;                      /* clear error flag */
    if(s->numX < 2)
      return(s->sumX);
    a = statGetIntercept(s);
    err = s->error;
    b = statGetSlope(s);
    if(b == 0.0)
      b = DBL_EPSILON;
    if(err && !s->error)
      s->error = err;
    return((y-a)/b);
  }
  return(0.0);
}
/***********************************************************************
* calculate and return linear regression estimate of Y, given X        *
***********************************************************************/
double statGetYestimate(STAT *s, double x)
{
  int    err;
  double a, b;
  
  if(s != NULL) {
    if(s->numX != s->numY) {
      s->error = STATERR_NO_PAIR;
      return(0.0);
    }
    if(s->numY < 1) {
      s->error = STATERR_NO_DATA;
      return(0.0);
    }
    s->error = STATERR_NONE;                      /* clear error flag */
    if(s->numY < 2)
      return(s->sumY);
    a = statGetIntercept(s);
    err = s->error;
    b = statGetSlope(s);
    if(err && ! s->error)
      s->error = err;
    return(a + b*x);
  }
  return(0.0);
}
