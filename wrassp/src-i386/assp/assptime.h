/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1989 - 2007  Michel Scheffers                          *
*                            IPdS, CAU Kiel                            *
*                            Leibnizstr. 10                            *
*                            24118 Kiel                                *
*                            Germany                                   *
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
* File:     assptime.h                                                 *
* Contents: Conversion standards for ASSP programs and functions.      *
*           - limits for sample/frame numbers and time                 *
*           - time (seconds) <-> sample number                         *
*           - time (seconds) <-> frame number                          *
*           - sample number  <-> frame number                          *
*           - centre time (seconds) of sample and frame                *
*           - frequency (Hz) <-> period duration in samples            *
*           - floor/ceil time (seconds) -> sample number               *
*           - floor/ceil time (seconds) -> frame number                *
*           - floor/ceil sample number  -> frame number                *
*           - sub-division of frame (head-shift-tail) in samples       *
* Arguments:                                                           *
*           double time    time in seconds                             *
*           double sfr     sampling frequency in Hz                    *
*           long   smpnr   sample number                               *
*           long   frmnr   frame number                                *
*           long   shift   frame shift in samples                      *
*           long   size    frame size in samples                       *
*           double freq    frequency in Hz                             *
*           long   period  period duration in samples                  *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: assptime.h,v 1.2 2007/08/17 13:12:27 mtms Exp $ */

#ifndef _ASSPTIME_H
#define _ASSPTIME_H

#include <math.h>   /* floor() ceil() */

#include <dlldef.h> /* ASSP_EXTERN */

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SMPNR (0x7FFFFFFFL)
#define MAX_FRMNR(size,shift) ((MAX_SMPNR-(size))/(shift))
#define MAX_TIME(sfr) ((double)MAX_SMPNR/(sfr))

#define TIMEtoSMPNR(time,sfr)        ((long)((double)(time)*(sfr) +0.5))
#define SMPNRtoTIME(smpnr,sfr)       ((double)(smpnr)/(sfr))
#define TIMEtoFRMNR(time,sfr,shift)  ((long)((time)*(sfr)/(double)(shift) +0.5))
#define FRMNRtoTIME(frmnr,sfr,shift) (((double)(frmnr)*(shift))/(sfr))
#define SMPNRtoFRMNR(smpnr,shift)    ((long)((double)(smpnr)/(double)(shift) +0.5))
#define FRMNRtoSMPNR(frmnr,shift)    ((long)(frmnr)*(shift))

#define SMPNRtoCTIME(smpnr,sfr)       (((double)(smpnr)+0.5)/(sfr))
#define FRMNRtoCTIME(frmnr,sfr,shift) (((double)(frmnr)+0.5)*(shift)/(sfr))

#define FREQtoPERIOD(freq,sfr)       ((long)((double)(sfr)/(freq) +0.5))
#define FREQftPERIOD(freq,sfr)       ((long)floor((double)(sfr)/(freq)))
#define FREQctPERIOD(freq,sfr)       ((long)ceil((double)(sfr)/(freq)))
#define PERIODtoFREQ(period,sfr)     ((sfr)/(double)(period))
/*
 * only for special cases like display of range!
 */
#define TIMEftSMPNR(time,sfr)       ((long)floor((double)(time)*(sfr)))
#define TIMEctSMPNR(time,sfr)       ((long)ceil((double)(time)*(sfr)))
#define TIMEftFRMNR(time,sfr,shift) ((long)floor((time)*(sfr)/(double)(shift)))
#define TIMEctFRMNR(time,sfr,shift) ((long)ceil((time)*(sfr)/(double)(shift)))
#define SMPNRftFRMNR(smpnr,shift)   ((long)floor((double)(smpnr)/(double)(shift)))
#define SMPNRctFRMNR(smpnr,shift)   ((long)ceil((double)(smpnr)/(double)(shift)))
/*
 * standard division of frame parts
 */
#define FRAMEHEAD(size,shift) (((long)(size)-(shift)+1L)/2L)
#define FRAMETAIL(size,shift) (((long)(size)-(shift))/2L)

/*
 * prototypes
 */
ASSP_EXTERN char *smp2dur(long smpNr, double smpRate);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _ASSPTIME_H */
