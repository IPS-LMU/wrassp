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
* File:     smp2dur                                                    *
* Contents: Function to convert duration in sample numbers to a        *
*           a formatted time duration string.                          *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: smp2dur.c,v 1.3 2011/02/17 10:08:53 mtms Exp $ */

#include <stdio.h>       /* NULL */
#include <math.h>        /* floor() ceil() */

#include <assptime.h>    /* standard time conversion macros, prototype */

char *smp2dur(long smpNr, double smpRate)
{
  static char durStr[64];
  int    hrs, min;
  double sec;

  if(smpNr < 0 || smpRate <= 0.0)                          /* bug !!! */
    return(NULL);

  sec = SMPNRtoTIME(smpNr, smpRate);            /* convert to seconds */
  if(sec >= 3600.0)                              /* more than an hour */
    sec = ceil(sec);                               /* do full seconds */
  else {
    sec *= 1000.0;                                   /* upscale to ms */
    sec = ceil(sec);                                 /* round upwards */
    sec /= 1000.0;                                       /* downscale */
  }

  hrs = (int)floor(sec / 3600.0);
  if(hrs > 0)
    sec -= ((double)hrs * 3600.0);
  min = (int)floor(sec / 60.0);
  if(min > 0)
    sec -= ((double)min * 60.0);

  if(hrs > 0)
    snprintf(durStr, sizeof(durStr), "%i hrs %i min %.0f sec", hrs, min, sec);
  else if(min > 0)
    snprintf(durStr, sizeof(durStr), "%i min %.3f sec", min, sec);
  else
    snprintf(durStr, sizeof(durStr), "%.3f sec", sec);

  return(durStr);
}

