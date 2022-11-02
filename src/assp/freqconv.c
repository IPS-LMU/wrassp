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
* File:     freqconv.c                                                 *
* Contents: functions for converting frequency scales (ST, Bark, mel)  *
*           prototypes of these functions are in asspdsp.h             *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: freqconv.c,v 1.2 2008/01/07 09:55:49 mtms Exp $ */

#include <math.h>    /* pow() log() exp() */

/* 
 * Converting frequencies to and from semitones
 *
 * 'f': frequency in Hz
 * 'R': reference frequency in Hz
 * 's': semitone value
 * 'r': relative frequency (f/R)
 *
 * Note:
 *  o   One standard reference is 440.0 Hz ('A' above middle 'C').
 *  o   When converting the difference between two frequencies to 
 *      semitones, however, there is no need to bother about finding 
 *      an appropriate (speaker-dependent) reference value; just use 
 *      the first/lower of the two frequencies.
 */

double hz2st(double f, double R)
{
  return(12.0 * log(f/R) / log(2.0));
}

double st2hz(double s, double R)
{
  return(R * pow(2.0, s/12.0));
}

double rel2st(double r)
{
  return(12.0 * log(r) / log(2.0));
}

double st2rel(double s)
{
  return(pow(2.0, s/12.0));
}

/*
 * From: H. Traunmüller (1997), "Auditory scales of frequency representation."
 *       http://www.ling.su.se/staff/hartmut/bark.htm
 */
/*
 * mel scale (1000 mel equals pitch of 1 kHz tone)
 * 'f' frequency in Hz
 * 'm' ratio pitch in mel
 */

double hz2mel(double f)
{
  return(1127.0 * log(1.0 + f/700.0));
}

double mel2hz(double m)
{
  return(700.0 * (exp(m/1127.0) - 1.0));
}

/*
 * Bark scale (Critical Bandwidth)
 * 'f' frequency in Hz
 * 'z' critical band rate in Bark
 */

double hz2bark(double f)
{
  double z;

  z = (26.81 / (1.0 + 1960.0/f)) - 0.53;
  /* valid from 200 Hz to 6.7 kHz, i.e. band 2 to band 20 */
  if(z < 2.0)
    z += (0.15 * (2.0 - z));
  else if(z > 20.1)
    z += (0.22 * (z - 20.1));
  return(z);
}

double bark2hz(double z)
{
  if(z < 2.0)
    z = (z - 0.3) / 0.85;
  else if(z > 20.1)
    z = (z + 4.422) / 1.22;
  return(1960.0 / (26.81/(z + 0.53) - 1.0));
}

double cb_hz_at_z(double z)
{
  return(52548.0 / (z * (z - 52.56) + 690.39));
}

/*
 * ERB scale (Equivalent Rectangular Bandwidth)
 * 'f' frequency in Hz
 * 'e' ERB rate in ERB units
 */

double hz2erb(double f)
{
  return(11.17 * log((f+312.0) / (f+14675.0)) + 43.0);
  /* valid from 100 Hz to 6.5 kHz */
}

double erb2hz(double e)
{
  double x;

  x = exp((e - 43.0) / 11.17);
  return((312.0 - 14675.0 * x) /(x - 1.0));
}

double erb_hz_at_f(double f)
{
  return(f * (6.23e-6*f + 9.339e-2) + 28.52);
}
